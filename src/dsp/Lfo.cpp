#include "Lfo.h"

#include <algorithm>
#include <cmath>

namespace synth
{
namespace
{
constexpr float twoPi = 6.28318530717958647692f;
}

void Lfo::prepare(double newSampleRate) noexcept
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    resetPhase();
}

void Lfo::setRateHz(float newRateHz) noexcept
{
    rateHz = std::clamp(newRateHz, 0.01f, 40.0f);
}

void Lfo::setShape(LfoShape newShape) noexcept
{
    shape = newShape;
}

void Lfo::setPhaseDegrees(float degrees) noexcept
{
    phaseOffset = std::fmod(std::max(0.0f, degrees) / 360.0f, 1.0f);
}

void Lfo::resetPhase() noexcept
{
    phase = phaseOffset;
    value = valueForPhase(phase);
}

float Lfo::process() noexcept
{
    const auto previousPhase = phase;
    phase += rateHz / static_cast<float>(sampleRate);
    if (phase >= 1.0f)
        phase -= std::floor(phase);

    if (shape == LfoShape::SampleHold && phase < previousPhase)
    {
        randomState = randomState * 1664525u + 1013904223u;
        heldRandom = (static_cast<float>((randomState >> 8) & 0x00ffffffu) / 8388607.5f) - 1.0f;
    }

    value = valueForPhase(phase);
    return value;
}

float Lfo::valueForPhase(float phaseValue) noexcept
{
    phaseValue -= std::floor(phaseValue);

    switch (shape)
    {
        case LfoShape::Sine:
            return std::sin(twoPi * phaseValue);
        case LfoShape::Triangle:
            return 1.0f - 4.0f * std::abs(phaseValue - 0.5f);
        case LfoShape::SawUp:
            return 2.0f * phaseValue - 1.0f;
        case LfoShape::SawDown:
            return 1.0f - 2.0f * phaseValue;
        case LfoShape::Square:
            return phaseValue < 0.5f ? 1.0f : -1.0f;
        case LfoShape::SampleHold:
            return heldRandom;
    }

    return 0.0f;
}
} // namespace synth

