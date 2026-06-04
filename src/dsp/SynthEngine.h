#pragma once

#include "../voice/VoiceAllocator.h"

#include <cstddef>

namespace synth
{
struct RenderStats
{
    int samplesRendered = 0;
    int invalidSamples = 0;
    float peak = 0.0f;
    int activeVoices = 0;
};

class SynthEngine
{
public:
    void prepare(double newSampleRate, int newMaxBlockSize);
    void reset() noexcept;

    void noteOn(int midiNote, float velocity) noexcept;
    void noteOff(int midiNote) noexcept;
    void allNotesOff() noexcept;
    void panic() noexcept;

    RenderStats process(float* left, float* right, int numSamples) noexcept;

    double getSampleRate() const noexcept { return sampleRate; }
    int getMaxBlockSize() const noexcept { return maxBlockSize; }
    int getActiveVoiceCount() const noexcept { return voices.activeVoiceCount(); }
    const Voice* getVoice(int index) const noexcept { return voices.getVoice(index); }

private:
    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    VoiceAllocator voices { 32 };
};
} // namespace synth
