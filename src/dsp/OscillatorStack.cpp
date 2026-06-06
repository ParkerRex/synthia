#include "OscillatorStack.h"

#include <algorithm>
#include <cmath>

namespace synth
{
namespace
{
constexpr float pi = 3.14159265358979323846f;

float wrapUnit(float value) noexcept
{
    value -= std::floor(value);
    return value;
}
} // namespace

void OscillatorStack::prepare(double newSampleRate) noexcept
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
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
}

float OscillatorStack::renderSample(float midiNote, const SynthParameters& parameters,
                                    float pitchModSemitones, float pulseWidthMod) noexcept
{
    return renderSample(midiNote, parameters.osc, pitchModSemitones, pulseWidthMod);
}

float OscillatorStack::renderSample(float midiNote, const OscillatorParameters& osc,
                                    float pitchModSemitones, float pulseWidthMod) noexcept
{
    const auto stackCount = std::clamp(osc.stackCount, 1, maxStackCount);
    const auto basePitch = midiNote
        + osc.pitchSemitones
        + osc.fineCents / 100.0f
        + pitchModSemitones;
    const auto baseFrequency = std::clamp(midiNoteToHz(basePitch), 1.0f, static_cast<float>(sampleRate * 0.45));
    const auto syncAmount = clampUnit(osc.syncAmount);
    const auto syncSlaveRatio = 1.0f + syncAmount * 4.0f;
    const auto masterIncrement = std::clamp(baseFrequency / static_cast<float>(sampleRate), 0.0f, 0.49f);
    const auto previousMaster = syncMasterPhase;
    advancePhase(syncMasterPhase, masterIncrement);
    const auto syncWrapped = syncAmount > 0.001f && syncMasterPhase < previousMaster;

    auto stacked = 0.0f;
    const auto pulseWidth = std::clamp(osc.pulseWidth + pulseWidthMod * 0.45f, 0.05f, 0.95f);

    for (int i = 0; i < stackCount; ++i)
    {
        if (syncWrapped)
            phases[static_cast<std::size_t>(i)] = syncMasterPhase;

        const auto cents = detuneOffsetCents(i, stackCount, osc.stackDetune);
        const auto frequency = std::clamp(baseFrequency * std::pow(2.0f, cents / 1200.0f) * syncSlaveRatio,
                                          1.0f, static_cast<float>(sampleRate * 0.45));
        const auto increment = std::clamp(frequency / static_cast<float>(sampleRate), 0.0f, 0.49f);

        auto& phase = phases[static_cast<std::size_t>(i)];
        auto voice = 0.0f;
        if (osc.sawLevel > 0.0f)
            voice += clampUnit(osc.sawLevel) * renderSaw(phase, increment);

        if (osc.pulseLevel > 0.0f)
        {
            auto pulsePhase = phase;
            voice += clampUnit(osc.pulseLevel) * renderPulse(pulsePhase, increment, pulseWidth);
        }

        advancePhase(phase, increment);
        stacked += voice;
    }

    stacked /= std::sqrt(static_cast<float>(stackCount));

    auto sub = 0.0f;
    if (osc.subLevel > 0.0f)
    {
        const auto octave = std::clamp(osc.subOctave, 1, 3);
        const auto subIncrement = std::clamp((baseFrequency / static_cast<float>(1 << octave)) / static_cast<float>(sampleRate),
                                             0.0f, 0.49f);
        sub = clampUnit(osc.subLevel) * renderSub(subPhase, osc.subWave, osc.subPulseWidth);
        advancePhase(subPhase, subIncrement);
    }

    const auto noise = osc.noiseLevel > 0.0f ? clampUnit(osc.noiseLevel) * renderNoise() : 0.0f;
    const auto levelSum = clampUnit(osc.sawLevel) + clampUnit(osc.pulseLevel) + clampUnit(osc.subLevel) + clampUnit(osc.noiseLevel);
    const auto compensation = 0.7f / std::max(1.0f, levelSum);
    return std::clamp((stacked + sub + noise) * compensation, -1.2f, 1.2f);
}

float OscillatorStack::renderSaw(float& phase, float phaseIncrement) noexcept
{
    return 2.0f * phase - 1.0f - polyBlep(phase, phaseIncrement);
}

float OscillatorStack::renderPulse(float& phase, float phaseIncrement, float pulseWidth) noexcept
{
    auto value = phase < pulseWidth ? 1.0f : -1.0f;
    value += polyBlep(phase, phaseIncrement);
    value -= polyBlep(wrapUnit(phase - pulseWidth), phaseIncrement);
    return value;
}

float OscillatorStack::renderSub(float phase, SubWave wave, float pulseWidth) const noexcept
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
            return phase < std::clamp(pulseWidth, 0.05f, 0.95f) ? 1.0f : -1.0f;
    }

    return 0.0f;
}

float OscillatorStack::renderNoise() noexcept
{
    noiseState = noiseState * 1664525u + 1013904223u;
    return (static_cast<float>((noiseState >> 8) & 0x00ffffffu) / 8388607.5f) - 1.0f;
}

float OscillatorStack::advancePhase(float& phase, float increment) noexcept
{
    phase += increment;
    if (phase >= 1.0f)
        phase -= std::floor(phase);

    return phase;
}

float OscillatorStack::polyBlep(float phase, float phaseIncrement) noexcept
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
    const auto shapedDetune = clampUnit(detune);
    const auto cents = 45.0f * shapedDetune * shapedDetune;
    return normalizedIndex * cents;
}
} // namespace synth
