#include "Filter.h"

#include <algorithm>
#include <cmath>

namespace synth
{
namespace
{
constexpr float twoPi = 6.28318530717958647692f;

#if defined(__clang__) || defined(__GNUC__)
#define SYNTHIA_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define SYNTHIA_ALWAYS_INLINE inline
#endif

SYNTHIA_ALWAYS_INLINE float softClip(float value) noexcept
{
    if (value < -64.0f)
        value = -64.0f;
    else if (value > 64.0f)
        value = 64.0f;

    return value >= 0.0f ? value / (1.0f + value)
                         : value / (1.0f - value);
}

// softClip(softClip(x)) collapses algebraically to clamp(x)/(1 + 2*|clamp(x)|):
// softClip is sign-preserving and maps into (-1, 1), so the outer clamp never
// engages and the two divisions fuse into one.
SYNTHIA_ALWAYS_INLINE float doubleSoftClip(float value) noexcept
{
    if (value < -64.0f)
        value = -64.0f;
    else if (value > 64.0f)
        value = 64.0f;

    return value >= 0.0f ? value / (1.0f + 2.0f * value)
                         : value / (1.0f - 2.0f * value);
}

SYNTHIA_ALWAYS_INLINE float clampKnownFinite(float value, float minimum, float maximum) noexcept
{
    if (value < minimum)
        return minimum;

    if (value > maximum)
        return maximum;

    return value;
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

SYNTHIA_ALWAYS_INLINE float flushTiny(float value) noexcept
{
    return value > -1.0e-20f && value < 1.0e-20f ? 0.0f : value;
}

SYNTHIA_ALWAYS_INLINE bool isTiny(float value) noexcept
{
    return value > -1.0e-15f && value < 1.0e-15f;
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
    clippedStage.fill(0.0f);
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

    const auto keytrack = clampKnownFinite(filter.keytrack, -1.0f, 2.0f);
    const auto keytrackSemitones = (midiNote - 60.0f) * keytrack;
    const auto cutoffSemitones = clampKnownFinite(filter.cutoffSemitones + keytrackSemitones + cutoffModSemitones,
                                                  0.0f, 136.0f);
    const auto oversampling = oversamplingFactor(filter.oversampling);
    const auto effectiveSampleRate = static_cast<float>(sampleRate) * static_cast<float>(oversampling);
    const auto driveAmount = clampKnownFinite(filter.drive, 0.0f, 1.0f);
    const auto resonance = clampKnownFinite(filter.resonance, 0.0f, 1.0f);
    const auto useL4FastPath = filter.mode == FilterMode::L4;

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
        const auto value = useL4FastPath
            ? processCoreL4(input, cachedCoefficient, cachedFeedback, cachedDriveGain)
            : processCore(input, cachedCoefficient, cachedFeedback, cachedDriveGain, filter.mode);
        return std::isfinite(value) ? value : 0.0f;
    }

    auto value = 0.0f;
    for (int i = 0; i < oversampling; ++i)
    {
        const auto t = static_cast<float>(i + 1) / static_cast<float>(oversampling);
        const auto subInput = previousInput + (input - previousInput) * t;
        value += useL4FastPath
            ? processCoreL4(subInput, cachedCoefficient, cachedFeedback, cachedDriveGain)
            : processCore(subInput, cachedCoefficient, cachedFeedback, cachedDriveGain, filter.mode);
    }
    previousInput = input;
    value /= static_cast<float>(oversampling);

    return std::isfinite(value) ? value : 0.0f;
}

// Per-block snapshot of every parameter-derived value that is constant within a
// host block. processPrepared() then only re-derives the cutoff-dependent
// coefficient, which can change per sample under modulation.
void Filter::prepareBlock(const SynthParameters& parameters) noexcept
{
    const auto& filter = parameters.filter;
    preparedEnabled = filter.enabled;
    if (!preparedEnabled)
        return;

    preparedKeytrack = clampKnownFinite(filter.keytrack, -1.0f, 2.0f);
    preparedOversampling = oversamplingFactor(filter.oversampling);
    preparedEffectiveSampleRate = static_cast<float>(sampleRate) * static_cast<float>(preparedOversampling);
    preparedMode = filter.mode;
    preparedUseL4 = filter.mode == FilterMode::L4;

    const auto driveAmount = clampKnownFinite(filter.drive, 0.0f, 1.0f);
    const auto resonance = clampKnownFinite(filter.resonance, 0.0f, 1.0f);
    if (std::abs(cachedDriveAmount - driveAmount) > 0.000001f
        || std::abs(cachedResonance - resonance) > 0.000001f)
    {
        cachedDriveGain = 1.0f + driveAmount * 9.0f;
        cachedFeedback = resonance * (3.85f / (1.0f + driveAmount * 1.35f));
        cachedDriveAmount = driveAmount;
        cachedResonance = resonance;
    }
}

float Filter::processPrepared(float input, float midiNote, const SynthParameters& parameters,
                              float cutoffModSemitones) noexcept
{
    if (!preparedEnabled)
        return input;

    const auto keytrackSemitones = (midiNote - 60.0f) * preparedKeytrack;
    const auto cutoffSemitones = clampKnownFinite(parameters.filter.cutoffSemitones + keytrackSemitones
                                                      + cutoffModSemitones,
                                                  0.0f, 136.0f);
    if (std::abs(cachedCutoffSemitones - cutoffSemitones) > 0.000001f
        || std::abs(cachedEffectiveSampleRate - preparedEffectiveSampleRate) > 0.001f)
    {
        const auto cutoffHz = clampFast(cutoffSemitonesToHz(cutoffSemitones), 8.0f,
                                        0.45f * preparedEffectiveSampleRate);
        const auto safeSampleRate = preparedEffectiveSampleRate > 1000.0f ? preparedEffectiveSampleRate : 1000.0f;
        const auto normalizedCutoff = twoPi * cutoffHz / safeSampleRate;
        cachedCoefficient = clampFast(normalizedCutoff / (1.0f + normalizedCutoff), 0.00001f, 0.99f);
        cachedCutoffSemitones = cutoffSemitones;
        cachedEffectiveSampleRate = preparedEffectiveSampleRate;
    }

    if (preparedOversampling <= 1)
    {
        previousInput = input;
        const auto value = preparedUseL4
            ? processCoreL4(input, cachedCoefficient, cachedFeedback, cachedDriveGain)
            : processCore(input, cachedCoefficient, cachedFeedback, cachedDriveGain, preparedMode);
        return std::isfinite(value) ? value : 0.0f;
    }

    auto value = 0.0f;
    for (int i = 0; i < preparedOversampling; ++i)
    {
        const auto t = static_cast<float>(i + 1) / static_cast<float>(preparedOversampling);
        const auto subInput = previousInput + (input - previousInput) * t;
        value += preparedUseL4
            ? processCoreL4(subInput, cachedCoefficient, cachedFeedback, cachedDriveGain)
            : processCore(subInput, cachedCoefficient, cachedFeedback, cachedDriveGain, preparedMode);
    }
    previousInput = input;
    value /= static_cast<float>(preparedOversampling);

    return std::isfinite(value) ? value : 0.0f;
}

// Block variant: identical per-sample calls so results stay bit-exact with the
// scalar prepared entry; batching keeps filter state hot across the loop.
void Filter::processPreparedBlock(float* samples, const float* midiNote, const SynthParameters& parameters,
                                  const float* cutoffModSemitones, int numSamples) noexcept
{
    if (!preparedEnabled)
        return;

    for (int i = 0; i < numSamples; ++i)
        samples[i] = processPrepared(samples[i], midiNote[i], parameters, cutoffModSemitones[i]);
}

float Filter::cutoffSemitonesToHz(float semitones) noexcept
{
    return semitonesToHz(clampFast(semitones, 0.0f, 136.0f));
}

SYNTHIA_ALWAYS_INLINE void Filter::processStages(float input, float coefficient, float feedback, float driveGain) noexcept
{
    const auto flushState = isTiny(input);

    auto previous = doubleSoftClip((input - feedback * clippedStage[3]) * driveGain);
    stage[0] += coefficient * (previous - clippedStage[0]);
    if (flushState)
        stage[0] = flushTiny(stage[0]);
    clippedStage[0] = softClip(stage[0]);

    previous = clippedStage[0];
    stage[1] += coefficient * (previous - clippedStage[1]);
    if (flushState)
        stage[1] = flushTiny(stage[1]);
    clippedStage[1] = softClip(stage[1]);

    previous = clippedStage[1];
    stage[2] += coefficient * (previous - clippedStage[2]);
    if (flushState)
        stage[2] = flushTiny(stage[2]);
    clippedStage[2] = softClip(stage[2]);

    previous = clippedStage[2];
    stage[3] += coefficient * (previous - clippedStage[3]);
    if (flushState)
        stage[3] = flushTiny(stage[3]);
    clippedStage[3] = softClip(stage[3]);
}

SYNTHIA_ALWAYS_INLINE float Filter::processCoreL4(float input, float coefficient, float feedback, float driveGain) noexcept
{
    processStages(input, coefficient, feedback, driveGain);
    return stage[3];
}

SYNTHIA_ALWAYS_INLINE float Filter::processCore(float input, float coefficient, float feedback, float driveGain,
                                                FilterMode mode) noexcept
{
    processStages(input, coefficient, feedback, driveGain);

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
