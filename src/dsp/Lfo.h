#pragma once

namespace synth
{
enum class LfoShape
{
    Sine,
    Triangle,
    SawUp,
    SawDown,
    Square,
    SampleHold
};

class Lfo
{
public:
    void prepare(double newSampleRate) noexcept;
    void setRateHz(float newRateHz) noexcept;
    void setShape(LfoShape newShape) noexcept;
    void setPhaseDegrees(float degrees) noexcept;
    void resetPhase() noexcept;
    float process() noexcept;

    float getValue() const noexcept { return value; }
    float getPhase() const noexcept { return phase; }

private:
    float valueForPhase(float phaseValue) noexcept;
    void updatePhaseIncrement() noexcept;

    double sampleRate = 44100.0;
    float rateHz = 2.0f;
    float phaseIncrement = 2.0f / 44100.0f;
    float phase = 0.0f;
    float phaseOffset = 0.0f;
    float value = 0.0f;
    float heldRandom = 0.0f;
    unsigned int randomState = 0x12345678u;
    LfoShape shape = LfoShape::Sine;
};
} // namespace synth
