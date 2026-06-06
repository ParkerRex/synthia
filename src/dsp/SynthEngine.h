#pragma once

#include "Arpeggiator.h"
#include "SynthParameters.h"
#include "fx/FxChain.h"
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
    void setSustainPedal(bool down) noexcept;
    void setPitchBend(float normalizedBipolar) noexcept;
    void setModWheel(float normalized) noexcept;
    void setAftertouch(float normalized) noexcept;
    void allNotesOff() noexcept;
    void panic() noexcept;
    void setParameters(const SynthParameters& newParameters) noexcept;

    RenderStats process(float* left, float* right, int numSamples) noexcept;

    double getSampleRate() const noexcept { return sampleRate; }
    int getMaxBlockSize() const noexcept { return maxBlockSize; }
    int getActiveVoiceCount() const noexcept { return voices.activeVoiceCount(); }
    const Voice* getVoice(int index) const noexcept { return voices.getVoice(index); }

private:
    void triggerDirectNoteOn(int midiNote, float velocity) noexcept;
    void triggerDirectNoteOff(int midiNote) noexcept;
    void processArpEvent(const ArpGeneratedEvent& event) noexcept;

    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    SynthParameters parameters;
    PerformanceState performance;
    Arpeggiator arpeggiator;
    VoiceAllocator voices { 32 };
    FxChain fx;
};
} // namespace synth
