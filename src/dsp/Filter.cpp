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
    return softSaturate(value);
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
    const auto cutoffHz = clampFast(cutoffSemitonesToHz(cutoffSemitones), 8.0f, 0.45f * effectiveSampleRate);
    const auto safeSampleRate = effectiveSampleRate > 1000.0f ? effectiveSampleRate : 1000.0f;
    const auto normalizedCutoff = twoPi * cutoffHz / safeSampleRate;
    const auto coefficient = clampFast(normalizedCutoff / (1.0f + normalizedCutoff), 0.00001f, 0.99f);
    const auto driveAmount = clampUnitFast(filter.drive);
    const auto driveGain = 1.0f + driveAmount * 9.0f;
    const auto feedback = clampUnitFast(filter.resonance) * (3.85f / (1.0f + driveAmount * 1.35f));

    auto value = 0.0f;
    for (int i = 0; i < oversampling; ++i)
    {
        const auto t = static_cast<float>(i + 1) / static_cast<float>(oversampling);
        const auto subInput = previousInput + (input - previousInput) * t;
        value += processCore(subInput, coefficient, feedback, driveGain, filter.mode);
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

    auto previous = drivenInput;
    for (auto& onePole : stage)
    {
        onePole = flushTiny(onePole + coefficient * (softClip(previous) - softClip(onePole)));
        previous = onePole;
    }

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
