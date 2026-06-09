#include "OscillatorStack.h"

#include <algorithm>
#include <cmath>

namespace synth
{
namespace
{
constexpr float pi = 3.14159265358979323846f;

#if defined(__clang__) || defined(__GNUC__)
#define SYNTHIA_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define SYNTHIA_ALWAYS_INLINE inline
#endif

float wrapUnit(float value) noexcept
{
    value -= std::floor(value);
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

// Only for values proven finite by construction (e.g. derived from an
// already-clamped cached base frequency); skips the isfinite guard.
SYNTHIA_ALWAYS_INLINE float clampKnownFinite(float value, float minimum, float maximum) noexcept
{
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

int clampIntFast(int value, int minimum, int maximum) noexcept
{
    if (value < minimum)
        return minimum;

    if (value > maximum)
        return maximum;

    return value;
}
} // namespace

void OscillatorStack::prepare(double newSampleRate) noexcept
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    cachedBaseFrequencyValid = false;
    reset(0.0f);
}

void OscillatorStack::reset(float randomOnNote) noexcept
{
    const auto basePhase = wrapUnit((randomOnNote + 1.0f) * 0.5f);
    resetState(basePhase);
}

void OscillatorStack::resetToPhase(float normalizedPhase) noexcept
{
    resetState(wrapUnit(normalizedPhase));
}

void OscillatorStack::resetState(float basePhase) noexcept
{
    for (int i = 0; i < maxStackCount; ++i)
        phases[static_cast<std::size_t>(i)] = wrapUnit(basePhase + 0.173f * static_cast<float>(i));

    subPhase = wrapUnit(basePhase * 0.5f);
    syncMasterPhase = basePhase;
    noiseState = 0x76543210u ^ static_cast<unsigned int>((basePhase * 16777215.0f) + 1.0f);
    cachedBaseFrequencyValid = false;
    cachedDetunedIncrementStackCount = -1;
    cachedDetunedMasterIncrement = -1.0f;
}

float OscillatorStack::renderSample(float midiNote, const SynthParameters& parameters,
                                    float pitchModSemitones, float pulseWidthMod) noexcept
{
    return renderSample(midiNote, parameters.osc, pitchModSemitones, pulseWidthMod);
}

float OscillatorStack::renderSample(float midiNote, const OscillatorParameters& osc,
                                    float pitchModSemitones, float pulseWidthMod) noexcept
{
    const auto stackCount = clampIntFast(osc.stackCount, 1, maxStackCount);
    const auto sampleRateFloat = static_cast<float>(sampleRate);
    const auto maxOscFrequency = sampleRateFloat * 0.45f;
    const auto minOscIncrement = 1.0f / sampleRateFloat;
    const auto maxOscIncrement = maxOscFrequency / sampleRateFloat;
    const auto basePitch = midiNote
        + osc.pitchSemitones
        + osc.fineCents / 100.0f
        + pitchModSemitones;
    auto baseFrequency = cachedBaseFrequency;
    if (!cachedBaseFrequencyValid
        || !std::isfinite(basePitch)
        || std::abs(cachedBasePitch - basePitch) > 0.000001f
        || std::abs(cachedMaxFrequency - maxOscFrequency) > 0.001f)
    {
        baseFrequency = clampFast(midiNoteToHz(basePitch), 1.0f, maxOscFrequency);
        cachedBaseFrequency = baseFrequency;
        cachedBasePitch = basePitch;
        cachedMaxFrequency = maxOscFrequency;
        cachedBaseFrequencyValid = std::isfinite(basePitch);
    }
    const auto syncAmount = clampUnitFast(osc.syncAmount);
    const auto masterIncrement = clampKnownFinite(baseFrequency / sampleRateFloat, minOscIncrement, maxOscIncrement);
    const auto previousMaster = syncMasterPhase;
    advancePhase(syncMasterPhase, masterIncrement);
    const auto syncWrapped = syncAmount > 0.001f && syncMasterPhase < previousMaster;

    const auto stackDetune = clampUnitFast(osc.stackDetune);
    const auto sawLevel = clampUnitFast(osc.sawLevel);
    const auto pulseLevel = clampUnitFast(osc.pulseLevel);
    const auto subLevel = clampUnitFast(osc.subLevel);
    const auto noiseLevel = clampUnitFast(osc.noiseLevel);
    if (cachedDetuneStackCount != stackCount || std::abs(cachedDetuneAmount - stackDetune) > 0.000001f)
        updateDetuneRatios(stackCount, stackDetune);

    if (sawLevel > 0.0f && pulseLevel <= 0.0f && subLevel <= 0.0f
        && noiseLevel <= 0.0f && syncAmount <= 0.001f)
    {
        auto stackedSaw = 0.0f;
        for (int i = 0; i < stackCount; ++i)
        {
            const auto increment = clampFast(masterIncrement * detuneRatios[static_cast<std::size_t>(i)],
                                             minOscIncrement, maxOscIncrement);
            auto& phase = phases[static_cast<std::size_t>(i)];
            stackedSaw += renderSaw(phase, increment);
            advancePhase(phase, increment);
        }

        const auto compensation = 0.7f / std::max(1.0f, sawLevel);
        return clampFast(stackedSaw * inverseSqrtForCount(stackCount) * sawLevel * compensation,
                         -1.2f, 1.2f);
    }

    auto stacked = 0.0f;
    const auto syncSlaveRatio = 1.0f + syncAmount * 4.0f;
    const auto pulseWidth = clampFast(osc.pulseWidth + pulseWidthMod * 0.45f, 0.05f, 0.95f);

    for (int i = 0; i < stackCount; ++i)
    {
        if (syncWrapped)
            phases[static_cast<std::size_t>(i)] = syncMasterPhase;

        const auto increment = clampFast(masterIncrement * detuneRatios[static_cast<std::size_t>(i)] * syncSlaveRatio,
                                         minOscIncrement, maxOscIncrement);

        auto& phase = phases[static_cast<std::size_t>(i)];
        auto voice = 0.0f;
        if (sawLevel > 0.0f)
            voice += sawLevel * renderSaw(phase, increment);

        if (pulseLevel > 0.0f)
        {
            auto pulsePhase = phase;
            voice += pulseLevel * renderPulse(pulsePhase, increment, pulseWidth);
        }

        advancePhase(phase, increment);
        stacked += voice;
    }

    stacked *= inverseSqrtForCount(stackCount);

    auto sub = 0.0f;
    if (subLevel > 0.0f)
    {
        const auto octave = clampIntFast(osc.subOctave, 1, 3);
        const auto subIncrement = clampFast((baseFrequency / static_cast<float>(1 << octave)) / sampleRateFloat,
                                            0.0f, 0.49f);
        sub = subLevel * renderSub(subPhase, osc.subWave, osc.subPulseWidth);
        advancePhase(subPhase, subIncrement);
    }

    const auto noise = noiseLevel > 0.0f ? noiseLevel * renderNoise() : 0.0f;
    const auto levelSum = sawLevel + pulseLevel + subLevel + noiseLevel;
    const auto compensation = 0.7f / std::max(1.0f, levelSum);
    return clampFast((stacked + sub + noise) * compensation, -1.2f, 1.2f);
}

float OscillatorStack::renderSawStack(float midiNote, const OscillatorParameters& osc,
                                      float pitchModSemitones, float outputGain) noexcept
{
    const auto stackCount = clampIntFast(osc.stackCount, 1, maxStackCount);
    const auto sampleRateFloat = static_cast<float>(sampleRate);
    const auto maxOscFrequency = sampleRateFloat * 0.45f;
    const auto minOscIncrement = 1.0f / sampleRateFloat;
    const auto maxOscIncrement = maxOscFrequency / sampleRateFloat;
    const auto basePitch = midiNote
        + osc.pitchSemitones
        + osc.fineCents / 100.0f
        + pitchModSemitones;
    auto baseFrequency = cachedBaseFrequency;
    if (!cachedBaseFrequencyValid
        || !std::isfinite(basePitch)
        || std::abs(cachedBasePitch - basePitch) > 0.000001f
        || std::abs(cachedMaxFrequency - maxOscFrequency) > 0.001f)
    {
        baseFrequency = clampFast(midiNoteToHz(basePitch), 1.0f, maxOscFrequency);
        cachedBaseFrequency = baseFrequency;
        cachedBasePitch = basePitch;
        cachedMaxFrequency = maxOscFrequency;
        cachedBaseFrequencyValid = std::isfinite(basePitch);
    }

    const auto masterIncrement = clampKnownFinite(baseFrequency / sampleRateFloat, minOscIncrement, maxOscIncrement);
    advancePhase(syncMasterPhase, masterIncrement);

    const auto stackDetune = clampUnitFast(osc.stackDetune);
    if (cachedDetuneStackCount != stackCount || std::abs(cachedDetuneAmount - stackDetune) > 0.000001f)
        updateDetuneRatios(stackCount, stackDetune);
    if (cachedDetunedIncrementStackCount != stackCount
        || std::abs(cachedDetunedMasterIncrement - masterIncrement) > 0.000000001f)
    {
        updateDetunedIncrements(stackCount, masterIncrement, minOscIncrement, maxOscIncrement);
    }

    const auto renderDetunedSaw = [this](int index) noexcept {
        const auto arrayIndex = static_cast<std::size_t>(index);
        const auto increment = detunedIncrements[arrayIndex];
        auto& phase = phases[arrayIndex];
        const auto sample = renderSaw(phase, increment);
        advancePhase(phase, increment);
        return sample;
    };

    auto stackedSaw = 0.0f;
    switch (stackCount)
    {
        case 1:
            stackedSaw = renderDetunedSaw(0);
            break;
        case 2:
            stackedSaw = renderDetunedSaw(0) + renderDetunedSaw(1);
            break;
        case 3:
            stackedSaw = renderDetunedSaw(0) + renderDetunedSaw(1) + renderDetunedSaw(2);
            break;
        case 4:
            stackedSaw = renderDetunedSaw(0) + renderDetunedSaw(1) + renderDetunedSaw(2) + renderDetunedSaw(3);
            break;
        default:
            for (int i = 0; i < stackCount; ++i)
                stackedSaw += renderDetunedSaw(i);
            break;
    }

    return clampFast(stackedSaw * outputGain, -1.2f, 1.2f);
}

// Block variant of renderSawStack: identical per-sample math and cache logic so
// results stay bit-exact with the scalar entry; only the block-constant detune
// ratio check is hoisted, and phases stay register-resident across the loop.
void OscillatorStack::renderSawStackBlock(const float* midiNote, const float* pitchModSemitones,
                                          float analogPitchMod, const OscillatorParameters& osc,
                                          float outputGain, float* out, int numSamples) noexcept
{
    const auto stackCount = clampIntFast(osc.stackCount, 1, maxStackCount);
    const auto sampleRateFloat = static_cast<float>(sampleRate);
    const auto maxOscFrequency = sampleRateFloat * 0.45f;
    const auto minOscIncrement = 1.0f / sampleRateFloat;
    const auto maxOscIncrement = maxOscFrequency / sampleRateFloat;

    const auto stackDetune = clampUnitFast(osc.stackDetune);
    if (cachedDetuneStackCount != stackCount || std::abs(cachedDetuneAmount - stackDetune) > 0.000001f)
        updateDetuneRatios(stackCount, stackDetune);

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const auto effectivePitchMod = pitchModSemitones[sampleIndex] + analogPitchMod;
        const auto basePitch = midiNote[sampleIndex]
            + osc.pitchSemitones
            + osc.fineCents / 100.0f
            + effectivePitchMod;
        auto baseFrequency = cachedBaseFrequency;
        if (!cachedBaseFrequencyValid
            || !std::isfinite(basePitch)
            || std::abs(cachedBasePitch - basePitch) > 0.000001f
            || std::abs(cachedMaxFrequency - maxOscFrequency) > 0.001f)
        {
            baseFrequency = clampFast(midiNoteToHz(basePitch), 1.0f, maxOscFrequency);
            cachedBaseFrequency = baseFrequency;
            cachedBasePitch = basePitch;
            cachedMaxFrequency = maxOscFrequency;
            cachedBaseFrequencyValid = std::isfinite(basePitch);
        }

        const auto masterIncrement = clampKnownFinite(baseFrequency / sampleRateFloat,
                                                      minOscIncrement, maxOscIncrement);
        advancePhase(syncMasterPhase, masterIncrement);

        if (cachedDetunedIncrementStackCount != stackCount
            || std::abs(cachedDetunedMasterIncrement - masterIncrement) > 0.000000001f)
        {
            updateDetunedIncrements(stackCount, masterIncrement, minOscIncrement, maxOscIncrement);
        }

        const auto renderDetunedSaw = [this](int index) noexcept {
            const auto arrayIndex = static_cast<std::size_t>(index);
            const auto increment = detunedIncrements[arrayIndex];
            auto& phase = phases[arrayIndex];
            const auto sample = renderSaw(phase, increment);
            advancePhase(phase, increment);
            return sample;
        };

        auto stackedSaw = 0.0f;
        switch (stackCount)
        {
            case 1:
                stackedSaw = renderDetunedSaw(0);
                break;
            case 2:
                stackedSaw = renderDetunedSaw(0) + renderDetunedSaw(1);
                break;
            case 3:
                stackedSaw = renderDetunedSaw(0) + renderDetunedSaw(1) + renderDetunedSaw(2);
                break;
            case 4:
                stackedSaw = renderDetunedSaw(0) + renderDetunedSaw(1) + renderDetunedSaw(2) + renderDetunedSaw(3);
                break;
            default:
                for (int i = 0; i < stackCount; ++i)
                    stackedSaw += renderDetunedSaw(i);
                break;
        }

        out[sampleIndex] = clampFast(stackedSaw * outputGain, -1.2f, 1.2f);
    }
}

SYNTHIA_ALWAYS_INLINE float OscillatorStack::renderSaw(float& phase, float phaseIncrement) noexcept
{
    return 2.0f * phase - 1.0f - polyBlep(phase, phaseIncrement);
}

SYNTHIA_ALWAYS_INLINE float OscillatorStack::renderPulse(float& phase, float phaseIncrement, float pulseWidth) noexcept
{
    auto value = phase < pulseWidth ? 1.0f : -1.0f;
    value += polyBlep(phase, phaseIncrement);
    auto fallingEdgePhase = phase - pulseWidth;
    if (fallingEdgePhase < 0.0f)
        fallingEdgePhase += 1.0f;
    value -= polyBlep(fallingEdgePhase, phaseIncrement);
    return value;
}

SYNTHIA_ALWAYS_INLINE float OscillatorStack::renderSub(float phase, SubWave wave, float pulseWidth) const noexcept
{
    phase = wrapUnit(phase);
    switch (wave)
    {
        case SubWave::Sine:
            return std::sin(2.0f * pi * phase);
        case SubWave::Triangle:
            return 1.0f - 4.0f * std::abs(phase - 0.5f);
        case SubWave::Saw:
            return 2.0f * phase - 1.0f;
        case SubWave::Pulse:
            return phase < clampFast(pulseWidth, 0.05f, 0.95f) ? 1.0f : -1.0f;
    }

    return 0.0f;
}

float OscillatorStack::renderNoise() noexcept
{
    noiseState = noiseState * 1664525u + 1013904223u;
    return (static_cast<float>((noiseState >> 8) & 0x00ffffffu) / 8388607.5f) - 1.0f;
}

SYNTHIA_ALWAYS_INLINE float OscillatorStack::advancePhase(float& phase, float increment) noexcept
{
    phase += increment;
    if (phase >= 1.0f)
        phase -= 1.0f;

    return phase;
}

SYNTHIA_ALWAYS_INLINE float OscillatorStack::polyBlep(float phase, float phaseIncrement) noexcept
{
    if (phaseIncrement <= 0.0f)
        return 0.0f;

    if (phase < phaseIncrement)
    {
        phase /= phaseIncrement;
        return phase + phase - phase * phase - 1.0f;
    }

    if (phase > 1.0f - phaseIncrement)
    {
        phase = (phase - 1.0f) / phaseIncrement;
        return phase * phase + phase + phase + 1.0f;
    }

    return 0.0f;
}

float OscillatorStack::detuneOffsetCents(int index, int count, float detune) noexcept
{
    if (count <= 1)
        return 0.0f;

    const auto normalizedIndex = (2.0f * static_cast<float>(index) / static_cast<float>(count - 1)) - 1.0f;
    const auto shapedDetune = clampUnitFast(detune);
    const auto cents = 45.0f * shapedDetune * shapedDetune;
    return normalizedIndex * cents;
}

void OscillatorStack::updateDetuneRatios(int stackCount, float stackDetune) noexcept
{
    if (cachedDetuneStackCount == stackCount && std::abs(cachedDetuneAmount - stackDetune) <= 0.000001f)
        return;

    cachedDetuneStackCount = stackCount;
    cachedDetuneAmount = stackDetune;
    cachedDetunedIncrementStackCount = -1;
    cachedDetunedMasterIncrement = -1.0f;
    for (int i = 0; i < stackCount; ++i)
    {
        const auto cents = detuneOffsetCents(i, stackCount, stackDetune);
        detuneRatios[static_cast<std::size_t>(i)] = std::pow(2.0f, cents / 1200.0f);
    }
}

void OscillatorStack::updateDetunedIncrements(int stackCount, float masterIncrement,
                                              float minIncrement, float maxIncrement) noexcept
{
    cachedDetunedIncrementStackCount = stackCount;
    cachedDetunedMasterIncrement = masterIncrement;
    for (int i = 0; i < stackCount; ++i)
    {
        const auto arrayIndex = static_cast<std::size_t>(i);
        detunedIncrements[arrayIndex] = clampFast(masterIncrement * detuneRatios[arrayIndex],
                                                  minIncrement, maxIncrement);
    }
}
} // namespace synth
