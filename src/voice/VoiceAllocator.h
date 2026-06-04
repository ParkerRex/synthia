#pragma once

#include "Voice.h"

#include <array>
#include <vector>

namespace synth
{
class VoiceAllocator
{
public:
    explicit VoiceAllocator(int maxVoices = 32);

    void prepare(double sampleRate);
    int noteOn(int midiNote, float velocity, const SynthParameters& parameters) noexcept;
    void noteOff(int midiNote) noexcept;
    void setSustainPedal(bool down) noexcept;
    void allNotesOff() noexcept;
    void panic() noexcept;
    void process(int numSamples) noexcept;
    StereoFrame renderSample(const SynthParameters& parameters) noexcept;

    int activeVoiceCount() const noexcept;
    const Voice* getVoice(int index) const noexcept;

private:
    float nextRandom() noexcept;
    int findIdleVoice() const noexcept;
    int findVoiceToSteal(int maxActiveVoices) const noexcept;
    void releaseSustainedNotes() noexcept;

    std::vector<Voice> voices;
    Lfo monoLfo;
    bool sustainPedalDown = false;
    std::array<bool, 128> heldNotes {};
    std::array<bool, 128> sustainedNotes {};
    unsigned int randomState = 0xdecafbadu;
};
} // namespace synth
