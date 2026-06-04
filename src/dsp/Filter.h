#pragma once

#include "SynthParameters.h"

#include <array>

namespace synth
{
class Filter
{
public:
    void prepare(double newSampleRate) noexcept;
    void reset() noexcept;
    float process(float input, int midiNote, const SynthParameters& parameters,
                  float cutoffModSemitones) noexcept;

    static float cutoffSemitonesToHz(float semitones) noexcept;

private:
    float processCore(float input, float cutoffHz, float resonance, float drive,
                      FilterMode mode, float effectiveSampleRate) noexcept;

    double sampleRate = 44100.0;
    std::array<float, 4> stage {};
    float previousInput = 0.0f;
};
} // namespace synth
