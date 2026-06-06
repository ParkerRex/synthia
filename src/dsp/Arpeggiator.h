#pragma once

#include "SynthParameters.h"

#include <array>
#include <cstdint>

namespace synth
{
struct ArpGeneratedEvent
{
    bool noteOff = false;
    int noteOffNumber = -1;
    bool noteOn = false;
    int noteOnNumber = -1;
    float velocity = 0.0f;
};

class Arpeggiator
{
public:
    void reset() noexcept;
    void noteOn(int midiNote, float velocity) noexcept;
    void noteOff(int midiNote, bool holdEnabled) noexcept;
    ArpGeneratedEvent processSample(const SynthParameters& parameters, double sampleRate) noexcept;

private:
    struct Candidate
    {
        int note = 0;
        float velocity = 0.0f;
        std::uint64_t order = 0;
    };

    static constexpr int maxCandidates = 64;

    int collectCandidates(const SynthParameters& parameters,
                          std::array<Candidate, maxCandidates>& candidates) const noexcept;
    static void sortCandidates(std::array<Candidate, maxCandidates>& candidates,
                               int candidateCount,
                               ArpMode mode) noexcept;
    static bool candidateComesBefore(const Candidate& left,
                                     const Candidate& right,
                                     ArpMode mode) noexcept;
    static int stepDurationSamples(const ArpParameters& parameters,
                                   float tempoBpm,
                                   double sampleRate,
                                   int sequenceStep) noexcept;
    static int selectPosition(int sequenceStep, int totalPositions, ArpMode mode) noexcept;

    std::array<bool, 128> heldNotes {};
    std::array<float, 128> heldVelocities {};
    std::array<std::uint64_t, 128> heldOrder {};
    std::uint64_t noteOrderCounter = 0;
    int heldNoteCount = 0;
    int activeOutputNote = -1;
    int samplesUntilStep = 0;
    int samplesUntilGateOff = 0;
    int sequenceStep = 0;
    bool currentStepTied = false;
    bool restartRequested = false;
};
} // namespace synth
