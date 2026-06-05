#include "Ramp.h"

#include <algorithm>
#include <cmath>

namespace synth
{
void Ramp::prepare(double newSampleRate) noexcept
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    reset();
}

void Ramp::noteOn() noexcept
{
    sampleCounter = 0;
    value = 0.0f;
}

void Ramp::reset() noexcept
{
    sampleCounter = 0;
    value = 0.0f;
}

float Ramp::process(const RampParameters& parameters, float tempoBpm) noexcept
{
    if (!parameters.enabled)
    {
        value = 0.0f;
        ++sampleCounter;
        return value;
    }

    const auto delay = delaySamples(parameters);
    const auto rise = riseSamples(parameters, tempoBpm);
    if (sampleCounter < delay)
    {
        value = 0.0f;
        ++sampleCounter;
        return value;
    }

    const auto rampSample = sampleCounter - delay;
    auto phase = 1.0f;
    if (parameters.mode == RampMode::Loop || parameters.mode == RampMode::Sync)
        phase = static_cast<float>(rampSample % rise) / static_cast<float>(rise);
    else
        phase = rise <= 1
            ? 1.0f
            : std::min(1.0f, static_cast<float>(rampSample) / static_cast<float>(rise - 1));

    value = shape(phase, parameters.curve);
    if (!std::isfinite(value))
        value = 0.0f;

    ++sampleCounter;
    return value;
}

float Ramp::shape(float phase, RampCurve curve) const noexcept
{
    phase = clampUnit(phase);
    switch (curve)
    {
        case RampCurve::Linear:
            return phase;
        case RampCurve::Exponential:
            return phase * phase;
        case RampCurve::Snappy:
        {
            const auto inverted = 1.0f - phase;
            return 1.0f - inverted * inverted * inverted;
        }
    }

    return phase;
}

int Ramp::delaySamples(const RampParameters& parameters) const noexcept
{
    const auto delayMs = std::clamp(parameters.delayMs, 0.0f, 10000.0f);
    return std::max(0, static_cast<int>(std::round(delayMs * static_cast<float>(sampleRate) * 0.001f)));
}

int Ramp::riseSamples(const RampParameters& parameters, float tempoBpm) const noexcept
{
    auto riseMs = std::clamp(parameters.riseMs, 1.0f, 10000.0f);
    if (parameters.mode == RampMode::Sync)
    {
        const auto safeTempo = std::clamp(tempoBpm, 20.0f, 300.0f);
        riseMs = 60000.0f / safeTempo;
    }

    return std::max(1, static_cast<int>(std::round(riseMs * static_cast<float>(sampleRate) * 0.001f)));
}
} // namespace synth
