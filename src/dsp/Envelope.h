#pragma once

namespace synth
{
enum class EnvelopeStage
{
    Idle,
    Attack,
    Decay,
    Sustain,
    Release
};

struct EnvelopeSettings
{
    float attackMs = 2.0f;
    float decayMs = 300.0f;
    float sustain = 0.8f;
    float releaseMs = 200.0f;
};

class Envelope
{
public:
    void prepare(double newSampleRate) noexcept;
    void setSettings(EnvelopeSettings newSettings) noexcept;
    void reset() noexcept;
    void noteOn() noexcept;
    void noteOff() noexcept;
    float process() noexcept;

    float getValue() const noexcept { return value; }
    EnvelopeStage getStage() const noexcept { return stage; }
    bool isActive() const noexcept { return stage != EnvelopeStage::Idle; }

private:
    float samplesForMs(float ms) const noexcept;

    double sampleRate = 44100.0;
    EnvelopeSettings settings;
    EnvelopeStage stage = EnvelopeStage::Idle;
    float value = 0.0f;
    float releaseStart = 0.0f;
    float releaseSamplesElapsed = 0.0f;
};
} // namespace synth

