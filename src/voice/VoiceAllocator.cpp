#include "VoiceAllocator.h"

#include <algorithm>
#include <cmath>

namespace synth
{
namespace
{
float syncDivisionBeats(int division) noexcept
{
    switch (division)
    {
        case 0: return 0.25f;
        case 1: return 0.5f;
        case 2: return 0.75f;
        case 3: return 1.0f;
        case 4: return 2.0f;
        case 5: return 4.0f;
        default: return 1.0f;
    }
}

float effectiveLfoRateHz(const SynthParameters& parameters) noexcept
{
    if (parameters.lfo.rateMode == LfoRateMode::Hz)
        return parameters.lfo.rateHz;

    return std::clamp(parameters.tempoBpm, 20.0f, 300.0f)
        / (60.0f * std::max(0.01f, syncDivisionBeats(parameters.lfo.syncDivision)));
}

LfoShape toLfoShape(LfoShapeChoice choice) noexcept
{
    switch (choice)
    {
        case LfoShapeChoice::Sine: return LfoShape::Sine;
        case LfoShapeChoice::Triangle: return LfoShape::Triangle;
        case LfoShapeChoice::SawUp: return LfoShape::SawUp;
        case LfoShapeChoice::SawDown: return LfoShape::SawDown;
        case LfoShapeChoice::Square: return LfoShape::Square;
        case LfoShapeChoice::SampleHold:
        case LfoShapeChoice::Noise:
            return LfoShape::SampleHold;
    }

    return LfoShape::SawDown;
}
} // namespace

VoiceAllocator::VoiceAllocator(int maxVoices)
    : voices(static_cast<std::size_t>(std::max(1, maxVoices)))
{
}

void VoiceAllocator::prepare(double sampleRate)
{
    monoLfo.prepare(sampleRate);
    monoLfo.setShape(LfoShape::SawDown);
    monoLfo.setRateHz(2.0f);

    for (int i = 0; i < static_cast<int>(voices.size()); ++i)
    {
        auto& voice = voices[static_cast<std::size_t>(i)];
        voice.prepare(sampleRate);
        voice.setVoiceIndex(i, static_cast<int>(voices.size()));
    }
}

int VoiceAllocator::noteOn(int midiNote, float velocity, const SynthParameters& parameters) noexcept
{
    if (midiNote < 0 || midiNote >= static_cast<int>(heldNotes.size()))
        return -1;

    heldNotes[static_cast<std::size_t>(midiNote)] = true;
    sustainedNotes[static_cast<std::size_t>(midiNote)] = false;

    const auto unisonCount = std::clamp(parameters.unisonCount, 1, 8);
    const auto maxActiveVoices = std::min(static_cast<int>(voices.size()),
                                          std::max(1, parameters.polyphony) * unisonCount);
    auto firstVoiceIndex = -1;

    for (int unison = 0; unison < unisonCount; ++unison)
    {
        auto voiceIndex = activeVoiceCount() < maxActiveVoices ? findIdleVoice() : -1;
        if (voiceIndex < 0)
            voiceIndex = findVoiceToSteal(maxActiveVoices);

        if (voiceIndex < 0)
            continue;

        voices[static_cast<std::size_t>(voiceIndex)].noteOn(midiNote, std::clamp(velocity, 0.0f, 1.0f),
                                                            nextRandom(), unison, unisonCount, parameters);
        if (firstVoiceIndex < 0)
            firstVoiceIndex = voiceIndex;
    }

    return firstVoiceIndex;
}

void VoiceAllocator::noteOff(int midiNote) noexcept
{
    if (midiNote < 0 || midiNote >= static_cast<int>(heldNotes.size()))
        return;

    heldNotes[static_cast<std::size_t>(midiNote)] = false;
    if (sustainPedalDown)
    {
        sustainedNotes[static_cast<std::size_t>(midiNote)] = true;
        return;
    }

    for (auto& voice : voices)
    {
        if (voice.isActive() && voice.getMidiNote() == midiNote)
            voice.noteOff();
    }
}

void VoiceAllocator::setSustainPedal(bool down) noexcept
{
    if (sustainPedalDown == down)
        return;

    sustainPedalDown = down;
    if (!sustainPedalDown)
        releaseSustainedNotes();
}

void VoiceAllocator::allNotesOff() noexcept
{
    heldNotes.fill(false);
    if (sustainPedalDown)
    {
        for (auto& voice : voices)
        {
            if (voice.isActive() && voice.getMidiNote() >= 0 && voice.getMidiNote() < static_cast<int>(sustainedNotes.size()))
                sustainedNotes[static_cast<std::size_t>(voice.getMidiNote())] = true;
        }
        return;
    }

    for (auto& voice : voices)
    {
        if (voice.isActive())
            voice.noteOff();
    }
}

void VoiceAllocator::panic() noexcept
{
    sustainPedalDown = false;
    heldNotes.fill(false);
    sustainedNotes.fill(false);
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

StereoFrame VoiceAllocator::renderSample(const SynthParameters& parameters) noexcept
{
    const auto useMonoLfo = parameters.lfo.mono
        || parameters.lfo.gateMode == LfoGateMode::Mono
        || parameters.lfo.gateMode == LfoGateMode::Song;
    auto monoValue = 0.0f;
    if (useMonoLfo)
    {
        monoLfo.setShape(toLfoShape(parameters.lfo.shape));
        monoLfo.setRateHz(effectiveLfoRateHz(parameters));
        monoLfo.setPhaseDegrees(parameters.lfo.phaseDegrees);
        monoValue = monoLfo.process();
    }

    StereoFrame frame;
    auto renderedVoices = 0;
    for (auto& voice : voices)
    {
        if (!voice.isActive())
            continue;

        const auto voiceFrame = voice.renderSample(parameters, useMonoLfo ? &monoValue : nullptr);
        frame.left += voiceFrame.left;
        frame.right += voiceFrame.right;
        ++renderedVoices;
    }

    if (renderedVoices > 1)
    {
        const auto compensation = 1.0f / std::sqrt(static_cast<float>(renderedVoices));
        frame.left *= compensation;
        frame.right *= compensation;
    }

    return frame;
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

int VoiceAllocator::findVoiceToSteal(int maxActiveVoices) const noexcept
{
    for (int i = 0; i < static_cast<int>(voices.size()) && i < maxActiveVoices; ++i)
    {
        if (voices[static_cast<std::size_t>(i)].isActive() && !voices[static_cast<std::size_t>(i)].isHeld())
            return i;
    }

    return maxActiveVoices > 0 ? 0 : -1;
}

void VoiceAllocator::releaseSustainedNotes() noexcept
{
    for (auto& voice : voices)
    {
        const auto note = voice.getMidiNote();
        if (voice.isActive()
            && note >= 0
            && note < static_cast<int>(sustainedNotes.size())
            && sustainedNotes[static_cast<std::size_t>(note)]
            && !heldNotes[static_cast<std::size_t>(note)])
        {
            voice.noteOff();
        }
    }

    sustainedNotes.fill(false);
}
} // namespace synth
