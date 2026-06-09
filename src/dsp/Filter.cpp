#include "Filter.h"

#include <algorithm>
#include <cmath>

namespace synth
{
namespace
{
constexpr float twoPi = 6.28318530717958647692f;

float softClip(float value) noexcept
{
    if (value < -64.0f)
        value = -64.0f;
    else if (value > 64.0f)
        value = 64.0f;

    return value >= 0.0f ? value / (1.0f + value)
                         : value / (1.0f - value);
}

float clampFast(float value, float minimum, float maximum) noexcept
{
    if (!std::isfinite(value))
        return minimum;

    if (value < minimum)
        return minimum;

    if (value > maximum)
        return maximum;

    return value;
}

float clampUnitFast(float value) noexcept
{
    return clampFast(value, 0.0f, 1.0f);
}

float flushTiny(float value) noexcept
{
    return value > -1.0e-20f && value < 1.0e-20f ? 0.0f : value;
}
} // namespace

void Filter::prepare(double newSampleRate) noexcept
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    reset();
}

void Filter::reset() noexcept
{
    stage.fill(0.0f);
    previousInput = 0.0f;
    cachedCutoffSemitones = -1.0f;
    cachedEffectiveSampleRate = 0.0f;
    cachedCoefficient = 0.0f;
    cachedDriveAmount = -1.0f;
    cachedResonance = -1.0f;
    cachedDriveGain = 1.0f;
    cachedFeedback = 0.0f;
}

float Filter::process(float input, float midiNote, const SynthParameters& parameters,
                      float cutoffModSemitones) noexcept
{
    const auto& filter = parameters.filter;
    if (!filter.enabled)
        return input;

    const auto keytrack = clampFast(filter.keytrack, -1.0f, 2.0f);
    const auto keytrackSemitones = (midiNote - 60.0f) * keytrack;
    const auto cutoffSemitones = clampFast(filter.cutoffSemitones + keytrackSemitones + cutoffModSemitones,
                                           0.0f, 136.0f);
    const auto oversampling = oversamplingFactor(filter.oversampling);
    const auto effectiveSampleRate = static_cast<float>(sampleRate) * static_cast<float>(oversampling);
    const auto driveAmount = clampUnitFast(filter.drive);
    const auto resonance = clampUnitFast(filter.resonance);

    if (std::abs(cachedCutoffSemitones - cutoffSemitones) > 0.000001f
        || std::abs(cachedEffectiveSampleRate - effectiveSampleRate) > 0.001f)
    {
        const auto cutoffHz = clampFast(cutoffSemitonesToHz(cutoffSemitones), 8.0f, 0.45f * effectiveSampleRate);
        const auto safeSampleRate = effectiveSampleRate > 1000.0f ? effectiveSampleRate : 1000.0f;
        const auto normalizedCutoff = twoPi * cutoffHz / safeSampleRate;
        cachedCoefficient = clampFast(normalizedCutoff / (1.0f + normalizedCutoff), 0.00001f, 0.99f);
        cachedCutoffSemitones = cutoffSemitones;
        cachedEffectiveSampleRate = effectiveSampleRate;
    }

    if (std::abs(cachedDriveAmount - driveAmount) > 0.000001f
        || std::abs(cachedResonance - resonance) > 0.000001f)
    {
        cachedDriveGain = 1.0f + driveAmount * 9.0f;
        cachedFeedback = resonance * (3.85f / (1.0f + driveAmount * 1.35f));
        cachedDriveAmount = driveAmount;
        cachedResonance = resonance;
    }

    if (oversampling <= 1)
    {
        previousInput = input;
        const auto value = processCore(input, cachedCoefficient, cachedFeedback, cachedDriveGain, filter.mode);
        return std::isfinite(value) ? value : 0.0f;
    }

    auto value = 0.0f;
    for (int i = 0; i < oversampling; ++i)
    {
        const auto t = static_cast<float>(i + 1) / static_cast<float>(oversampling);
        const auto subInput = previousInput + (input - previousInput) * t;
        value += processCore(subInput, cachedCoefficient, cachedFeedback, cachedDriveGain, filter.mode);
    }
    previousInput = input;
    value /= static_cast<float>(oversampling);

    return std::isfinite(value) ? value : 0.0f;
}

float Filter::cutoffSemitonesToHz(float semitones) noexcept
{
    return semitonesToHz(clampFast(semitones, 0.0f, 136.0f));
}

float Filter::processCore(float input, float coefficient, float feedback, float driveGain,
                          FilterMode mode) noexcept
{
    const auto drivenInput = softClip((input - feedback * softClip(stage[3])) * driveGain);

    stage[0] = flushTiny(stage[0] + coefficient * (softClip(drivenInput) - softClip(stage[0])));
    stage[1] = flushTiny(stage[1] + coefficient * (softClip(stage[0]) - softClip(stage[1])));
    stage[2] = flushTiny(stage[2] + coefficient * (softClip(stage[1]) - softClip(stage[2])));
    stage[3] = flushTiny(stage[3] + coefficient * (softClip(stage[2]) - softClip(stage[3])));

    switch (mode)
    {
        case FilterMode::L2:
            return stage[1];
        case FilterMode::L4:
            return stage[3];
        case FilterMode::B2:
            return (stage[0] - stage[1]) * 2.0f;
        case FilterMode::B4:
            return (stage[2] - stage[3]) * 3.0f;
        case FilterMode::H2:
            return input - stage[1];
        case FilterMode::H4:
            return input - stage[3];
        case FilterMode::Peak2:
            return stage[1] - (input - stage[1]);
        case FilterMode::Notch2:
            return stage[1] + (input - stage[1]);
        case FilterMode::Notch4:
            return stage[3] + (input - stage[3]);
    }

    return stage[3];
}
} // namespace synth
