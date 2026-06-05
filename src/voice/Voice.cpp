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

float unipolarIndex(int index, int count) noexcept
{
    if (count <= 1)
        return 0.0f;

    return std::clamp(static_cast<float>(index) / static_cast<float>(count - 1), 0.0f, 1.0f);
}

float sanitize(float value) noexcept
{
    return std::isfinite(value) ? value : 0.0f;
}
} // namespace

void Voice::prepare(double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;

    ampEnvelope.prepare(sampleRate);
    ampEnvelope.setSettings({ 2.0f, 300.0f, 0.0f, 180.0f });

    modEnvelope.prepare(sampleRate);
    modEnvelope.setSettings({ 1.0f, 400.0f, 0.0f, 160.0f });

    lfo.prepare(sampleRate);
    lfo.setShape(LfoShape::SawDown);
    lfo.setRateHz(2.0f);

    oscillator.prepare(sampleRate);
    ramp.prepare(sampleRate);
    filter.prepare(sampleRate);
}

void Voice::setVoiceIndex(int index, int total) noexcept
{
    setAllocationIndices(index, total, unisonIndex, unisonCount);
}

void Voice::setAllocationIndices(int index, int total, int newUnisonIndex, int newUnisonCount) noexcept
{
    voiceIndex = std::max(0, index);
    voiceCount = std::max(1, total);
    unisonIndex = std::max(0, newUnisonIndex);
    unisonCount = std::max(1, newUnisonCount);
}

void Voice::noteOn(int note, float normalizedVelocity, float randomValue,
                   int newUnisonIndex, int newUnisonCount, const SynthParameters& parameters,
                   bool allowGlide, bool retriggerModulators) noexcept
{
    const auto wasActive = state != VoiceState::Idle && midiNote >= 0;
    const auto shouldGlide = wasActive && allowGlide;
    const auto shouldRetrigger = !wasActive || retriggerModulators;
    const auto previousNote = wasActive ? currentMidiNote : static_cast<float>(note);
    const auto previousVelocity = wasActive ? currentVelocity : normalizedVelocity;

    state = VoiceState::Held;
    midiNote = note;
    velocity = std::clamp(normalizedVelocity, 0.0f, 1.0f);
    startVelocity = previousVelocity;
    currentVelocity = previousVelocity;
    const auto velocityGlideMs = std::clamp(parameters.velocityGlideMs, 0.0f, 2000.0f);
    velocityGlideTotalSamples = shouldGlide && velocityGlideMs > 0.0f
        ? std::max(1, static_cast<int>(std::round(velocityGlideMs * static_cast<float>(sampleRate) * 0.001f)))
        : 0;
    velocityGlideSamples = 0;
    if (velocityGlideTotalSamples == 0)
        currentVelocity = velocity;

    startMidiNote = previousNote;
    targetMidiNote = static_cast<float>(note);
    currentMidiNote = previousNote;
    const auto glideMs = std::clamp(parameters.glideMs, 0.0f, 2000.0f);
    glideTotalSamples = shouldGlide && glideMs > 0.0f
        ? std::max(1, static_cast<int>(std::round(glideMs * static_cast<float>(sampleRate) * 0.001f)))
        : 0;
    glideSamples = 0;
    if (glideTotalSamples == 0)
        currentMidiNote = targetMidiNote;

    randomOnNote = randomValue;
    setAllocationIndices(voiceIndex, voiceCount, newUnisonIndex, newUnisonCount);
    if (shouldRetrigger)
    {
        ampEnvelope.noteOn();
        modEnvelope.noteOn();
        if (parameters.lfo.gateMode == LfoGateMode::Poly || parameters.lfo.gateMode == LfoGateMode::PolyOn)
            lfo.resetPhase();
        if (parameters.osc.phaseReset == 3)
            oscillator.reset(randomOnNote);
        else if (parameters.osc.phaseReset == 1 || parameters.osc.phaseReset == 2)
            oscillator.reset(0.0f);
        ramp.noteOn();
        filter.reset();
    }
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
    currentVelocity = 0.0f;
    startVelocity = 0.0f;
    velocityGlideSamples = 0;
    velocityGlideTotalSamples = 0;
    currentMidiNote = 0.0f;
    startMidiNote = 0.0f;
    targetMidiNote = 0.0f;
    glideSamples = 0;
    glideTotalSamples = 0;
    randomOnNote = 0.0f;
    ampEnvelope.reset();
    modEnvelope.reset();
    lfo.resetPhase();
    ramp.reset();
    filter.reset();
    lastDirectSums = {};
    lastTransModSums = {};
    lastRampValue = 0.0f;
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
    const auto rampValue = ramp.process(parameters.ramp, parameters.tempoBpm);
    const auto effectiveNote = processGlide(parameters);
    const auto glidedVelocity = processVelocityGlide(parameters);

    if (state == VoiceState::Releasing && !ampEnvelope.isActive())
    {
        reset();
        return {};
    }

    const auto macroMotion = parameters.macro.motion - 0.5f;
    const auto keytrackSource = (effectiveNote - 60.0f) / 12.0f;
    lastDirectSums.oscPitchSemitones = keytrackSource * parameters.direct.oscKeytrackSemitones
        + lfoValue * parameters.direct.oscLfoSemitones
        + mod * parameters.direct.oscModEnvSemitones
        + macroMotion * 2.0f;
    lastDirectSums.pulseWidth = keytrackSource * parameters.direct.pulseKeytrack
        + lfoValue * parameters.direct.pulseLfo
        + mod * parameters.direct.pulseModEnv;
    lastDirectSums.filterCutoffSemitones = (effectiveNote - 60.0f) * parameters.direct.filterKeytrack
        + lfoValue * parameters.direct.filterLfoSemitones
        + mod * parameters.direct.filterModEnvSemitones
        + macroMotion * 12.0f;
    lastDirectSums.ampLevelDb = 0.0f;
    lastDirectSums.pan = 0.0f;

    lastTransModSums = evaluateTransMod(parameters, lfoValue, rampValue, mod, amp, effectiveNote, glidedVelocity);
    lastRampValue = rampValue;

    const auto oscPitchMod = lastDirectSums.oscPitchSemitones
        + lastTransModSums.oscPitchSemitones;
    const auto pulseMod = lastDirectSums.pulseWidth
        + lastTransModSums.pulseWidth;
    const auto cutoffMod = lastDirectSums.filterCutoffSemitones
        + lastTransModSums.filterCutoffSemitones;

    const auto unisonBi = bipolarIndex(unisonIndex, unisonCount);
    auto sample = oscillator.renderSample(effectiveNote, parameters,
                                          oscPitchMod + randomOnNote * parameters.amp.analog * 0.07f
                                              + unisonBi * parameters.amp.analog * 0.12f,
                                          pulseMod);
    sample = filter.process(sample, effectiveNote, parameters, cutoffMod);

    const auto ampDrive = clampUnit(parameters.amp.drive + parameters.macro.drive * 0.35f);
    const auto driveGain = 1.0f + ampDrive * 7.0f;
    sample = std::tanh(sample * driveGain) / std::tanh(driveGain);
    sample *= amp * (0.35f + 0.65f * glidedVelocity);
    sample *= decibelsToGain(parameters.amp.levelDb + lastTransModSums.ampLevelDb);

    const auto voiceSpread = randomOnNote * parameters.amp.panSpread * 0.85f
        + unisonBi * parameters.amp.unisonSpread * 0.7f
        + randomOnNote * parameters.macro.width * 0.25f;
    const auto pan = std::clamp(parameters.amp.pan + voiceSpread + lastTransModSums.pan, -1.0f, 1.0f);
    const auto angle = (pan + 1.0f) * 0.5f * halfPi;

    return { sanitize(sample * std::cos(angle)), sanitize(sample * std::sin(angle)) };
}

float Voice::processGlide(const SynthParameters& parameters) noexcept
{
    if (glideTotalSamples <= 0)
    {
        currentMidiNote = targetMidiNote;
        return currentMidiNote;
    }

    const auto phase = glideTotalSamples <= 1
        ? 1.0f
        : clampUnit(static_cast<float>(glideSamples) / static_cast<float>(glideTotalSamples - 1));
    currentMidiNote = startMidiNote + (targetMidiNote - startMidiNote) * phase;
    if (glideSamples < glideTotalSamples - 1)
        ++glideSamples;
    else
        currentMidiNote = targetMidiNote;

    if (!std::isfinite(currentMidiNote))
        currentMidiNote = static_cast<float>(midiNote);

    (void) parameters;
    return currentMidiNote;
}

float Voice::processVelocityGlide(const SynthParameters& parameters) noexcept
{
    if (velocityGlideTotalSamples <= 0)
    {
        currentVelocity = velocity;
        return currentVelocity;
    }

    const auto phase = velocityGlideTotalSamples <= 1
        ? 1.0f
        : clampUnit(static_cast<float>(velocityGlideSamples) / static_cast<float>(velocityGlideTotalSamples - 1));
    currentVelocity = startVelocity + (velocity - startVelocity) * phase;
    if (velocityGlideSamples < velocityGlideTotalSamples - 1)
        ++velocityGlideSamples;
    else
        currentVelocity = velocity;

    if (!std::isfinite(currentVelocity))
        currentVelocity = velocity;

    currentVelocity = clampUnit(currentVelocity);
    (void) parameters;
    return currentVelocity;
}

float Voice::evalModSource(const SynthParameters& parameters, ModSource source, float lfoValue, float rampValue,
                           float modEnvValue, float ampEnvValue, float effectiveNote,
                           float velocityGlideValue) const noexcept
{
    switch (source)
    {
        case ModSource::None:
            return 0.0f;
        case ModSource::Lfo:
            return std::clamp(lfoValue, -1.0f, 1.0f);
        case ModSource::Ramp:
            return clampUnit(rampValue);
        case ModSource::ModEnv:
            return clampUnit(modEnvValue);
        case ModSource::AmpEnv:
            return clampUnit(ampEnvValue);
        case ModSource::Keytrack:
            return std::clamp((effectiveNote - 60.0f) / 36.0f, -1.0f, 1.0f);
        case ModSource::Velocity:
            return clampUnit(velocity);
        case ModSource::VelocityGlide:
            return clampUnit(velocityGlideValue);
        case ModSource::PitchBend:
            return std::clamp(parameters.performance.pitchBend, -1.0f, 1.0f);
        case ModSource::ModWheel:
            return clampUnit(parameters.performance.modWheel);
        case ModSource::Aftertouch:
            return clampUnit(parameters.performance.aftertouch);
        case ModSource::VoiceUni:
            return unipolarIndex(voiceIndex, voiceCount);
        case ModSource::VoiceBi:
            return bipolarIndex(voiceIndex, voiceCount);
        case ModSource::UnisonUni:
            return unipolarIndex(unisonIndex, unisonCount);
        case ModSource::UnisonBi:
            return bipolarIndex(unisonIndex, unisonCount);
        case ModSource::RandomOnNote:
            return std::clamp(randomOnNote, -1.0f, 1.0f);
        case ModSource::Macro1:
            return clampUnit(parameters.macro.motion);
        case ModSource::Macro2:
            return clampUnit(parameters.macro.width);
        case ModSource::Macro3:
            return clampUnit(parameters.macro.drive);
        case ModSource::Macro4:
            return clampUnit(parameters.macro.space);
    }

    return 0.0f;
}

Voice::ModulationSums Voice::evaluateTransMod(const SynthParameters& parameters, float lfoValue,
                                              float rampValue, float modEnvValue, float ampEnvValue,
                                              float effectiveNote, float velocityGlideValue) const noexcept
{
    ModulationSums sums;
    for (const auto& slot : parameters.transMod.slots)
    {
        if (!slot.enabled || slot.source == ModSource::None)
            continue;

        const auto source = evalModSource(parameters, slot.source, lfoValue, rampValue, modEnvValue, ampEnvValue,
                                          effectiveNote, velocityGlideValue);
        const auto scaler = slot.scaler == ModSource::None
            ? 1.0f
            : evalModSource(parameters, slot.scaler, lfoValue, rampValue, modEnvValue, ampEnvValue,
                            effectiveNote, velocityGlideValue);
        const auto amount = source * scaler;
        if (!std::isfinite(amount))
            continue;

        sums.oscPitchSemitones += amount * std::clamp(slot.oscPitchSemitones, -48.0f, 48.0f);
        sums.pulseWidth += amount * std::clamp(slot.pulseWidth, -1.0f, 1.0f);
        sums.filterCutoffSemitones += amount
            * (std::clamp(slot.filterCutoffSemitones, -72.0f, 72.0f) + std::clamp(slot.depth, -1.0f, 1.0f) * 72.0f);
        sums.ampLevelDb += amount * std::clamp(slot.ampLevelDb, -48.0f, 48.0f);
        sums.pan += amount * std::clamp(slot.pan, -1.0f, 1.0f);
    }

    sums.oscPitchSemitones = std::clamp(sanitize(sums.oscPitchSemitones), -96.0f, 96.0f);
    sums.pulseWidth = std::clamp(sanitize(sums.pulseWidth), -1.0f, 1.0f);
    sums.filterCutoffSemitones = std::clamp(sanitize(sums.filterCutoffSemitones), -136.0f, 136.0f);
    sums.ampLevelDb = std::clamp(sanitize(sums.ampLevelDb), -24.0f, 24.0f);
    sums.pan = std::clamp(sanitize(sums.pan), -2.0f, 2.0f);
    return sums;
}

VoiceSnapshot Voice::snapshot() const noexcept
{
    return {
        state,
        midiNote,
        velocity,
        ampEnvelope.getValue(),
        modEnvelope.getValue(),
        lfo.getValue(),
        lastRampValue,
        randomOnNote,
        currentMidiNote,
        currentVelocity,
        unipolarIndex(voiceIndex, voiceCount),
        bipolarIndex(voiceIndex, voiceCount),
        unipolarIndex(unisonIndex, unisonCount),
        bipolarIndex(unisonIndex, unisonCount),
        lastDirectSums.oscPitchSemitones,
        lastDirectSums.pulseWidth,
        lastDirectSums.filterCutoffSemitones,
        lastTransModSums.oscPitchSemitones,
        lastTransModSums.pulseWidth,
        lastTransModSums.filterCutoffSemitones,
        lastTransModSums.ampLevelDb,
        lastTransModSums.pan
    };
}
} // namespace synth
