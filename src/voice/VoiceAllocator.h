#pragma once

#include "Voice.h"

#include <vector>

namespace synth
{
class VoiceAllocator
{
public:
    explicit VoiceAllocator(int maxVoices = 32);

    void prepare(double sampleRate);
    int noteOn(int midiNote, float velocity) noexcept;
    void noteOff(int midiNote) noexcept;
    void allNotesOff() noexcept;
    void panic() noexcept;
    void process(int numSamples) noexcept;

    int activeVoiceCount() const noexcept;
    const Voice* getVoice(int index) const noexcept;

private:
    float nextRandom() noexcept;
    int findIdleVoice() const noexcept;

    std::vector<Voice> voices;
    unsigned int randomState = 0xdecafbadu;
};
} // namespace synth

