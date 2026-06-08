#include "Arpeggiator.h"

#include <algorithm>
#include <cmath>

namespace synth
{
namespace
{
float rateDivisionBeats(ArpRateDivision division) noexcept
{
    switch (division)
    {
        case ArpRateDivision::ThirtySecond: return 0.125f;
        case ArpRateDivision::Sixteenth: return 0.25f;
        case ArpRateDivision::Eighth: return 0.5f;
        case ArpRateDivision::Quarter: return 1.0f;
        case ArpRateDivision::Half: return 2.0f;
    }

    return 0.25f;
}
} // namespace

void Arpeggiator::reset() noexcept
{
    heldNotes.fill(false);
    heldVelocities.fill(0.0f);
    heldOrder.fill(0);
    noteOrderCounter = 0;
    heldNoteCount = 0;
    activeOutputNote = -1;
    samplesUntilStep = 0;
    samplesUntilGateOff = 0;
    sequenceStep = 0;
    currentStepTied = false;
    restartRequested = false;
}

void Arpeggiator::noteOn(int midiNote, float velocity) noexcept
{
    if (midiNote < 0 || midiNote >= static_cast<int>(heldNotes.size()))
        return;

    const auto wasEmpty = heldNoteCount == 0;
    const auto noteIndex = static_cast<std::size_t>(midiNote);
    if (!heldNotes[noteIndex])
        ++heldNoteCount;
    heldNotes[noteIndex] = true;
    heldVelocities[noteIndex] = std::clamp(velocity, 0.0f, 1.0f);
    heldOrder[noteIndex] = ++noteOrderCounter;

    if (wasEmpty)
    {
        sequenceStep = 0;
        samplesUntilStep = 0;
        samplesUntilGateOff = 0;
        currentStepTied = false;
        restartRequested = true;
    }
}

void Arpeggiator::noteOff(int midiNote, bool holdEnabled) noexcept
{
    if (midiNote < 0 || midiNote >= static_cast<int>(heldNotes.size()) || holdEnabled)
        return;

    const auto noteIndex = static_cast<std::size_t>(midiNote);
    if (!heldNotes[noteIndex])
        return;

    heldNotes[noteIndex] = false;
    heldVelocities[noteIndex] = 0.0f;
    heldOrder[noteIndex] = 0;
    heldNoteCount = std::max(0, heldNoteCount - 1);

    if (heldNoteCount == 0)
        restartRequested = false;
}

ArpGeneratedEvent Arpeggiator::processSample(const SynthParameters& parameters, double sampleRate) noexcept
{
    ArpGeneratedEvent event;

    if (heldNoteCount <= 0)
    {
        if (activeOutputNote >= 0)
        {
            event.noteOff = true;
            event.noteOffNumber = activeOutputNote;
        }

        activeOutputNote = -1;
        samplesUntilStep = 0;
        samplesUntilGateOff = 0;
        sequenceStep = 0;
        currentStepTied = false;
        return event;
    }

    if (restartRequested || samplesUntilStep <= 0)
    {
        std::array<Candidate, maxCandidates> candidates {};
        const auto candidateCount = collectCandidates(parameters, candidates);
        if (candidateCount <= 0)
        {
            if (activeOutputNote >= 0)
            {
                event.noteOff = true;
                event.noteOffNumber = activeOutputNote;
            }

            activeOutputNote = -1;
            samplesUntilStep = 0;
            samplesUntilGateOff = 0;
            sequenceStep = 0;
            currentStepTied = false;
            return event;
        }

        const auto stepCount = std::clamp(parameters.arp.stepCount, 1, arpStepCount);
        const auto stepIndex = sequenceStep % stepCount;
        const auto& step = parameters.arp.steps[static_cast<std::size_t>(stepIndex)];
        const auto octaves = std::clamp(parameters.arp.octaves, 1, 4);
        const auto totalPositions = std::max(1, candidateCount * octaves);
        const auto position = selectPosition(sequenceStep, totalPositions, parameters.arp.mode);
        const auto candidateIndex = position % candidateCount;
        const auto octaveIndex = position / candidateCount;
        const auto octaveOffset = parameters.arp.mode == ArpMode::Down
            ? (octaves - 1 - octaveIndex) * 12
            : octaveIndex * 12;
        const auto note = std::clamp(candidates[static_cast<std::size_t>(candidateIndex)].note
                                     + octaveOffset + step.pitchSemitones,
                                     0, 127);
        const auto velocity = std::clamp(candidates[static_cast<std::size_t>(candidateIndex)].velocity
                                         * step.velocity,
                                         0.0f, 1.0f);

        if (activeOutputNote >= 0)
        {
            event.noteOff = true;
            event.noteOffNumber = activeOutputNote;
        }

        activeOutputNote = -1;
        currentStepTied = false;

        const auto durationSamples = stepDurationSamples(parameters.arp, parameters.tempoBpm,
                                                         sampleRate, sequenceStep);
        if (step.enabled && velocity > 0.0f)
        {
            event.noteOn = true;
            event.noteOnNumber = note;
            event.velocity = velocity;
            activeOutputNote = note;
            currentStepTied = step.tie;

            const auto gate = std::clamp(parameters.arp.gate * step.gate, 0.02f, 1.0f);
            samplesUntilGateOff = std::clamp(static_cast<int>(std::round(static_cast<float>(durationSamples) * gate)),
                                             1, durationSamples);
        }
        else
        {
            samplesUntilGateOff = 0;
        }

        samplesUntilStep = durationSamples;
        ++sequenceStep;
        restartRequested = false;
    }
    else if (activeOutputNote >= 0 && !currentStepTied && samplesUntilGateOff <= 0)
    {
        event.noteOff = true;
        event.noteOffNumber = activeOutputNote;
        activeOutputNote = -1;
    }

    if (samplesUntilStep > 0)
        --samplesUntilStep;

    if (activeOutputNote >= 0 && !currentStepTied && samplesUntilGateOff > 0)
        --samplesUntilGateOff;

    return event;
}

int Arpeggiator::collectCandidates(const SynthParameters& parameters,
                                   std::array<Candidate, maxCandidates>& candidates) const noexcept
{
    auto count = 0;

    for (int midiNote = 0; midiNote < static_cast<int>(heldNotes.size()); ++midiNote)
    {
        if (!heldNotes[static_cast<std::size_t>(midiNote)])
            continue;

        auto appendCandidate = [&](int note, float velocity, int orderOffset) {
            if (count >= maxCandidates)
                return;

            auto& candidate = candidates[static_cast<std::size_t>(count++)];
            candidate.note = std::clamp(note, 0, 127);
            candidate.velocity = std::clamp(velocity, 0.0f, 1.0f);
            candidate.order = heldOrder[static_cast<std::size_t>(midiNote)] * 16u
                + static_cast<std::uint64_t>(orderOffset);
        };

        const auto baseVelocity = heldVelocities[static_cast<std::size_t>(midiNote)];
        if (!parameters.chord.enabled)
        {
            appendCandidate(midiNote, baseVelocity, 0);
            continue;
        }

        const auto voiceLimit = std::clamp(parameters.chord.voiceCount, 1, chordVoiceCount);
        auto addedChordVoice = false;
        for (int voiceIndex = 0; voiceIndex < voiceLimit; ++voiceIndex)
        {
            const auto& voice = parameters.chord.voices[static_cast<std::size_t>(voiceIndex)];
            if (!voice.enabled)
                continue;

            appendCandidate(midiNote + voice.pitchSemitones,
                            baseVelocity * voice.velocity,
                            voiceIndex);
            addedChordVoice = true;
        }

        if (!addedChordVoice)
            appendCandidate(midiNote, baseVelocity, 0);
    }

    sortCandidates(candidates, count, parameters.arp.mode);
    return count;
}

void Arpeggiator::sortCandidates(std::array<Candidate, maxCandidates>& candidates,
                                 int candidateCount,
                                 ArpMode mode) noexcept
{
    for (int i = 1; i < candidateCount; ++i)
    {
        const auto current = candidates[static_cast<std::size_t>(i)];
        auto j = i - 1;
        while (j >= 0 && candidateComesBefore(current, candidates[static_cast<std::size_t>(j)], mode))
        {
            const auto destinationIndex = static_cast<std::size_t>(j) + 1u;
            candidates[destinationIndex] = candidates[static_cast<std::size_t>(j)];
            --j;
        }
        const auto insertionIndex = j < 0 ? std::size_t { 0 } : static_cast<std::size_t>(j) + 1u;
        candidates[insertionIndex] = current;
    }
}

bool Arpeggiator::candidateComesBefore(const Candidate& left,
                                       const Candidate& right,
                                       ArpMode mode) noexcept
{
    if (mode == ArpMode::AsPlayed)
    {
        if (left.order != right.order)
            return left.order < right.order;
        return left.note < right.note;
    }

    if (mode == ArpMode::Down)
    {
        if (left.note != right.note)
            return left.note > right.note;
        return left.order < right.order;
    }

    if (left.note != right.note)
        return left.note < right.note;
    return left.order < right.order;
}

int Arpeggiator::stepDurationSamples(const ArpParameters& parameters,
                                     float tempoBpm,
                                     double sampleRate,
                                     int sequenceStep) noexcept
{
    const auto safeTempo = std::clamp(std::isfinite(tempoBpm) ? tempoBpm : 128.0f, 20.0f, 300.0f);
    const auto safeSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    const auto seconds = (60.0f / safeTempo) * rateDivisionBeats(parameters.rate);
    const auto swing = std::clamp(parameters.swing, 0.0f, 0.75f);
    const auto swingScale = (sequenceStep % 2) == 0 ? (1.0f - swing * 0.5f)
                                                    : (1.0f + swing * 0.5f);
    return std::max(1, static_cast<int>(std::round(seconds * swingScale
                                                   * static_cast<float>(safeSampleRate))));
}

int Arpeggiator::selectPosition(int sequenceStep, int totalPositions, ArpMode mode) noexcept
{
    if (totalPositions <= 1)
        return 0;

    if (mode != ArpMode::UpDown)
        return sequenceStep % totalPositions;

    const auto cycleLength = totalPositions * 2 - 2;
    const auto cyclePosition = sequenceStep % cycleLength;
    return cyclePosition < totalPositions ? cyclePosition : cycleLength - cyclePosition;
}
} // namespace synth
