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

float flushTiny(float value) noexcept
{
    return std::abs(value) < 1.0e-20f ? 0.0f : value;
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

    const auto keytrack = std::clamp(filter.keytrack, -1.0f, 2.0f);
    const auto keytrackSemitones = (midiNote - 60.0f) * keytrack;
    const auto cutoffSemitones = std::clamp(filter.cutoffSemitones + keytrackSemitones + cutoffModSemitones,
                                           0.0f, 136.0f);
    const auto oversampling = oversamplingFactor(filter.oversampling);
    const auto effectiveSampleRate = static_cast<float>(sampleRate) * static_cast<float>(oversampling);
    const auto cutoffHz = std::clamp(cutoffSemitonesToHz(cutoffSemitones),
                                     8.0f, 0.45f * effectiveSampleRate);

    auto value = 0.0f;
    for (int i = 0; i < oversampling; ++i)
    {
        const auto t = static_cast<float>(i + 1) / static_cast<float>(oversampling);
        const auto subInput = previousInput + (input - previousInput) * t;
        value += processCore(subInput, cutoffHz, filter.resonance, filter.drive, filter.mode, effectiveSampleRate);
    }
    previousInput = input;
    value /= static_cast<float>(oversampling);

    return std::isfinite(value) ? value : 0.0f;
}

float Filter::cutoffSemitonesToHz(float semitones) noexcept
{
    return semitonesToHz(std::clamp(semitones, 0.0f, 136.0f));
}

float Filter::processCore(float input, float cutoffHz, float resonance, float drive,
                          FilterMode mode, float effectiveSampleRate) noexcept
{
    const auto safeSampleRate = std::max(1000.0f, effectiveSampleRate);
    const auto normalizedCutoff = twoPi * cutoffHz / safeSampleRate;
    const auto g = std::clamp(normalizedCutoff / (1.0f + normalizedCutoff), 0.00001f, 0.99f);
    const auto driveGain = 1.0f + clampUnit(drive) * 9.0f;
    const auto feedback = std::clamp(resonance, 0.0f, 1.0f) * (3.85f / (1.0f + clampUnit(drive) * 1.35f));
    const auto drivenInput = softClip((input - feedback * softClip(stage[3])) * driveGain);

    auto previous = drivenInput;
    for (auto& onePole : stage)
    {
        onePole = flushTiny(onePole + g * (softClip(previous) - softClip(onePole)));
        previous = onePole;
    }

    const auto lp2 = stage[1];
    const auto lp4 = stage[3];
    const auto hp2 = input - lp2;
    const auto hp4 = input - lp4;
    const auto bp2 = stage[0] - stage[1];
    const auto bp4 = stage[2] - stage[3];
    const auto peak2 = lp2 - hp2;
    const auto notch2 = lp2 + hp2;
    const auto notch4 = lp4 + hp4;

    switch (mode)
    {
        case FilterMode::L2:
            return lp2;
        case FilterMode::L4:
            return lp4;
        case FilterMode::B2:
            return bp2 * 2.0f;
        case FilterMode::B4:
            return bp4 * 3.0f;
        case FilterMode::H2:
            return hp2;
        case FilterMode::H4:
            return hp4;
        case FilterMode::Peak2:
            return peak2;
        case FilterMode::Notch2:
            return notch2;
        case FilterMode::Notch4:
            return notch4;
    }

    return lp4;
}
} // namespace synth
