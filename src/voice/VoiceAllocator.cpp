#include "VoiceAllocator.h"

#include <algorithm>

namespace synth
{
VoiceAllocator::VoiceAllocator(int maxVoices)
    : voices(static_cast<std::size_t>(std::max(1, maxVoices)))
{
}

void VoiceAllocator::prepare(double sampleRate)
{
    for (auto& voice : voices)
        voice.prepare(sampleRate);
}

int VoiceAllocator::noteOn(int midiNote, float velocity) noexcept
{
    auto voiceIndex = findIdleVoice();
    if (voiceIndex < 0)
        voiceIndex = 0;

    voices[static_cast<std::size_t>(voiceIndex)].noteOn(midiNote, std::clamp(velocity, 0.0f, 1.0f), nextRandom());
    return voiceIndex;
}

void VoiceAllocator::noteOff(int midiNote) noexcept
{
    for (auto& voice : voices)
    {
        if (voice.isActive() && voice.getMidiNote() == midiNote)
            voice.noteOff();
    }
}

void VoiceAllocator::allNotesOff() noexcept
{
    for (auto& voice : voices)
    {
        if (voice.isActive())
            voice.noteOff();
    }
}

void VoiceAllocator::panic() noexcept
{
    for (auto& voice : voices)
        voice.reset();
}

void VoiceAllocator::process(int numSamples) noexcept
{
    for (auto& voice : voices)
    {
        if (voice.isActive())
            voice.process(numSamples);
    }
}

int VoiceAllocator::activeVoiceCount() const noexcept
{
    return static_cast<int>(std::count_if(voices.begin(), voices.end(), [](const auto& voice) {
        return voice.isActive();
    }));
}

const Voice* VoiceAllocator::getVoice(int index) const noexcept
{
    if (index < 0 || index >= static_cast<int>(voices.size()))
        return nullptr;

    return &voices[static_cast<std::size_t>(index)];
}

float VoiceAllocator::nextRandom() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    return (static_cast<float>((randomState >> 8) & 0x00ffffffu) / 8388607.5f) - 1.0f;
}

int VoiceAllocator::findIdleVoice() const noexcept
{
    for (int i = 0; i < static_cast<int>(voices.size()); ++i)
    {
        if (!voices[static_cast<std::size_t>(i)].isActive())
            return i;
    }

    return -1;
}
} // namespace synth

