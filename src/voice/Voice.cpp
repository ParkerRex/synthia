#include "Voice.h"

#include <algorithm>
#include <cmath>

namespace synth
{
namespace
{
constexpr float halfPi = 1.57079632679489661923f;

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

EnvelopeSettings toEnvelopeSettings(EnvelopeParameters parameters) noexcept
{
    return { parameters.attackMs, parameters.decayMs, parameters.sustain, parameters.releaseMs };
}

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

    const auto beats = std::max(0.01f, syncDivisionBeats(parameters.lfo.syncDivision));
    return std::clamp(parameters.tempoBpm, 20.0f, 300.0f) / (60.0f * beats);
}

float bipolarIndex(int index, int count) noexcept
{
    if (count <= 1)
        return 0.0f;

    return (2.0f * static_cast<float>(index) / static_cast<float>(count - 1)) - 1.0f;
}

float sanitize(float value) noexcept
{
    return std::isfinite(value) ? value : 0.0f;
}
} // namespace

void Voice::prepare(double sampleRate)
{
    ampEnvelope.prepare(sampleRate);
    ampEnvelope.setSettings({ 2.0f, 300.0f, 0.0f, 180.0f });

    modEnvelope.prepare(sampleRate);
    modEnvelope.setSettings({ 1.0f, 400.0f, 0.0f, 160.0f });

    lfo.prepare(sampleRate);
    lfo.setShape(LfoShape::SawDown);
    lfo.setRateHz(2.0f);

    oscillator.prepare(sampleRate);
    filter.prepare(sampleRate);
}

void Voice::setVoiceIndex(int index, int total) noexcept
{
    voiceIndex = std::max(0, index);
    voiceCount = std::max(1, total);
}

void Voice::noteOn(int note, float normalizedVelocity, float randomValue,
                   int newUnisonIndex, int newUnisonCount, const SynthParameters& parameters) noexcept
{
    state = VoiceState::Held;
    midiNote = note;
    velocity = normalizedVelocity;
    randomOnNote = randomValue;
    unisonIndex = std::max(0, newUnisonIndex);
    unisonCount = std::max(1, newUnisonCount);
    ampEnvelope.noteOn();
    modEnvelope.noteOn();
    if (parameters.lfo.gateMode == LfoGateMode::Poly || parameters.lfo.gateMode == LfoGateMode::PolyOn)
        lfo.resetPhase();
    if (parameters.osc.phaseReset == 3)
        oscillator.reset(randomOnNote);
    else if (parameters.osc.phaseReset == 1 || parameters.osc.phaseReset == 2)
        oscillator.reset(0.0f);
    filter.reset();
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
    filter.reset();
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

StereoFrame Voice::renderSample(const SynthParameters& parameters, const float* monoLfoValue) noexcept
{
    if (state == VoiceState::Idle)
        return {};

    ampEnvelope.setSettings(toEnvelopeSettings(parameters.ampEnv));
    modEnvelope.setSettings(toEnvelopeSettings(parameters.modEnv));
    lfo.setShape(toLfoShape(parameters.lfo.shape));
    lfo.setRateHz(effectiveLfoRateHz(parameters));
    lfo.setPhaseDegrees(parameters.lfo.phaseDegrees);

    const auto amp = ampEnvelope.process();
    const auto mod = modEnvelope.process();
    const auto lfoValue = monoLfoValue != nullptr ? *monoLfoValue : lfo.process();

    if (state == VoiceState::Releasing && !ampEnvelope.isActive())
    {
        reset();
        return {};
    }

    const auto macroMotion = parameters.macro.motion - 0.5f;
    const auto oscPitchMod = lfoValue * parameters.direct.oscLfoSemitones
        + mod * parameters.direct.oscModEnvSemitones
        + macroMotion * 2.0f;
    const auto pulseMod = lfoValue * parameters.direct.pulseLfo
        + mod * parameters.direct.pulseModEnv;
    const auto cutoffMod = lfoValue * parameters.direct.filterLfoSemitones
        + mod * parameters.direct.filterModEnvSemitones
        + macroMotion * 12.0f;

    const auto unisonBi = bipolarIndex(unisonIndex, unisonCount);
    auto sample = oscillator.renderSample(midiNote, parameters,
                                          oscPitchMod + randomOnNote * parameters.amp.analog * 0.07f
                                              + unisonBi * parameters.amp.analog * 0.12f,
                                          pulseMod);
    sample = filter.process(sample, midiNote, parameters, cutoffMod);

    const auto ampDrive = clampUnit(parameters.amp.drive + parameters.macro.drive * 0.35f);
    const auto driveGain = 1.0f + ampDrive * 7.0f;
    sample = std::tanh(sample * driveGain) / std::tanh(driveGain);
    sample *= amp * (0.35f + 0.65f * velocity);
    sample *= decibelsToGain(parameters.amp.levelDb);

    const auto voiceSpread = randomOnNote * parameters.amp.panSpread * 0.85f
        + unisonBi * parameters.amp.unisonSpread * 0.7f
        + randomOnNote * parameters.macro.width * 0.25f;
    const auto pan = std::clamp(parameters.amp.pan + voiceSpread, -1.0f, 1.0f);
    const auto angle = (pan + 1.0f) * 0.5f * halfPi;

    return { sanitize(sample * std::cos(angle)), sanitize(sample * std::sin(angle)) };
}

VoiceSnapshot Voice::snapshot() const noexcept
{
    return { state, midiNote, velocity, ampEnvelope.getValue(), modEnvelope.getValue(), lfo.getValue(), randomOnNote };
}
} // namespace synth
