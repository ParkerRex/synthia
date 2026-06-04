#pragma once

#include "../dsp/Envelope.h"
#include "../dsp/Lfo.h"

namespace synth
{
enum class VoiceState
{
    Idle,
    Held,
    Releasing
};

struct VoiceSnapshot
{
    VoiceState state = VoiceState::Idle;
    int midiNote = -1;
    float velocity = 0.0f;
    float ampEnv = 0.0f;
    float modEnv = 0.0f;
    float lfo = 0.0f;
    float randomOnNote = 0.0f;
};

class Voice
{
public:
    void prepare(double sampleRate);
    void noteOn(int note, float normalizedVelocity, float randomValue) noexcept;
    void noteOff() noexcept;
    void reset() noexcept;
    void process(int numSamples) noexcept;

    bool isActive() const noexcept { return state != VoiceState::Idle; }
    bool isHeld() const noexcept { return state == VoiceState::Held; }
    int getMidiNote() const noexcept { return midiNote; }
    VoiceSnapshot snapshot() const noexcept;

private:
    VoiceState state = VoiceState::Idle;
    int midiNote = -1;
    float velocity = 0.0f;
    float randomOnNote = 0.0f;
    Envelope ampEnvelope;
    Envelope modEnvelope;
    Lfo lfo;
};
} // namespace synth

