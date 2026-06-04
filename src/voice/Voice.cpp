#include "Voice.h"

namespace synth
{
void Voice::prepare(double sampleRate)
{
    ampEnvelope.prepare(sampleRate);
    ampEnvelope.setSettings({ 2.0f, 300.0f, 0.0f, 180.0f });

    modEnvelope.prepare(sampleRate);
    modEnvelope.setSettings({ 1.0f, 400.0f, 0.0f, 160.0f });

    lfo.prepare(sampleRate);
    lfo.setShape(LfoShape::SawDown);
    lfo.setRateHz(2.0f);
}

void Voice::noteOn(int note, float normalizedVelocity, float randomValue) noexcept
{
    state = VoiceState::Held;
    midiNote = note;
    velocity = normalizedVelocity;
    randomOnNote = randomValue;
    ampEnvelope.noteOn();
    modEnvelope.noteOn();
    lfo.resetPhase();
}

void Voice::noteOff() noexcept
{
    if (state == VoiceState::Idle)
        return;

    state = VoiceState::Releasing;
    ampEnvelope.noteOff();
    modEnvelope.noteOff();
}

void Voice::reset() noexcept
{
    state = VoiceState::Idle;
    midiNote = -1;
    velocity = 0.0f;
    randomOnNote = 0.0f;
    ampEnvelope.reset();
    modEnvelope.reset();
    lfo.resetPhase();
}

void Voice::process(int numSamples) noexcept
{
    for (int i = 0; i < numSamples; ++i)
    {
        ampEnvelope.process();
        modEnvelope.process();
        lfo.process();
    }

    if (state == VoiceState::Releasing && !ampEnvelope.isActive())
        reset();
}

VoiceSnapshot Voice::snapshot() const noexcept
{
    return { state, midiNote, velocity, ampEnvelope.getValue(), modEnvelope.getValue(), lfo.getValue(), randomOnNote };
}
} // namespace synth

