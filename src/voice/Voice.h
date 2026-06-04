#pragma once

#include "../dsp/Envelope.h"
#include "../dsp/Filter.h"
#include "../dsp/Lfo.h"
#include "../dsp/OscillatorStack.h"
#include "../dsp/SynthParameters.h"

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

struct StereoFrame
{
    float left = 0.0f;
    float right = 0.0f;
};

class Voice
{
public:
    void prepare(double sampleRate);
    void setVoiceIndex(int index, int total) noexcept;
    void noteOn(int note, float normalizedVelocity, float randomValue,
                int newUnisonIndex, int newUnisonCount, const SynthParameters& parameters) noexcept;
    void noteOff() noexcept;
    void reset() noexcept;
    void process(int numSamples) noexcept;
    StereoFrame renderSample(const SynthParameters& parameters, const float* monoLfoValue = nullptr) noexcept;

    bool isActive() const noexcept { return state != VoiceState::Idle; }
    bool isHeld() const noexcept { return state == VoiceState::Held; }
    int getMidiNote() const noexcept { return midiNote; }
    VoiceSnapshot snapshot() const noexcept;

private:
    VoiceState state = VoiceState::Idle;
    int midiNote = -1;
    float velocity = 0.0f;
    float randomOnNote = 0.0f;
    int voiceIndex = 0;
    int voiceCount = 1;
    int unisonIndex = 0;
    int unisonCount = 1;
    Envelope ampEnvelope;
    Envelope modEnvelope;
    Lfo lfo;
    OscillatorStack oscillator;
    Filter filter;
};
} // namespace synth
