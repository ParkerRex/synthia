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
    float process(float input, float midiNote, const SynthParameters& parameters,
                  float cutoffModSemitones) noexcept;

    static float cutoffSemitonesToHz(float semitones) noexcept;

private:
    float processCore(float input, float coefficient, float feedback, float driveGain,
                      FilterMode mode) noexcept;

    double sampleRate = 44100.0;
    std::array<float, 4> stage {};
    float previousInput = 0.0f;
};
} // namespace synth
