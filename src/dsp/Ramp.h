#pragma once

#include "SynthParameters.h"

namespace synth
{
class Ramp
{
public:
    void prepare(double newSampleRate) noexcept;
    void noteOn() noexcept;
    void reset() noexcept;

    float process(const RampParameters& parameters, float tempoBpm) noexcept;
    float getValue() const noexcept { return value; }

private:
    float shape(float phase, RampCurve curve) const noexcept;
    int delaySamples(const RampParameters& parameters) const noexcept;
    int riseSamples(const RampParameters& parameters, float tempoBpm) const noexcept;

    double sampleRate = 44100.0;
    int sampleCounter = 0;
    float value = 0.0f;
};
} // namespace synth
