#pragma once

#include "SynthParameters.h"

#include <array>

namespace synth
{
class OscillatorStack
{
public:
    void prepare(double newSampleRate) noexcept;
    void reset(float randomOnNote) noexcept;
    void resetToPhase(float normalizedPhase) noexcept;

    float renderSample(float midiNote, const SynthParameters& parameters,
                       float pitchModSemitones, float pulseWidthMod) noexcept;
    float renderSample(float midiNote, const OscillatorParameters& parameters,
                       float pitchModSemitones, float pulseWidthMod) noexcept;

    static float detuneOffsetCents(int index, int count, float detune) noexcept;

private:
    static constexpr int maxStackCount = 8;

    float renderSaw(float& phase, float phaseIncrement) noexcept;
    float renderPulse(float& phase, float phaseIncrement, float pulseWidth) noexcept;
    float renderSub(float phase, SubWave wave, float pulseWidth) const noexcept;
    float renderNoise() noexcept;
    float advancePhase(float& phase, float increment) noexcept;
    static float polyBlep(float phase, float phaseIncrement) noexcept;
    void resetState(float basePhase) noexcept;

    double sampleRate = 44100.0;
    std::array<float, maxStackCount> phases {};
    float subPhase = 0.0f;
    float syncMasterPhase = 0.0f;
    unsigned int noiseState = 0x76543210u;
};
} // namespace synth
