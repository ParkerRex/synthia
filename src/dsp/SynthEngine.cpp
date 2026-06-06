#include "SynthEngine.h"

#include <algorithm>
#include <cmath>

namespace synth
{
namespace
{
float finiteOr(float value, float fallback) noexcept
{
    return std::isfinite(value) ? value : fallback;
}
} // namespace

void SynthEngine::prepare(double newSampleRate, int newMaxBlockSize)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    maxBlockSize = std::max(1, newMaxBlockSize);
    voices.prepare(sampleRate);
    fx.prepare(sampleRate, maxBlockSize);
    reset();
}

void SynthEngine::reset() noexcept
{
    performance = {};
    parameters.performance = performance;
    arpeggiator.reset();
    voices.panic();
    fx.reset();
}

void SynthEngine::noteOn(int midiNote, float velocity) noexcept
{
    if (parameters.arp.enabled)
    {
        arpeggiator.noteOn(midiNote, velocity);
        return;
    }

    triggerDirectNoteOn(midiNote, velocity);
}

void SynthEngine::noteOff(int midiNote) noexcept
{
    if (parameters.arp.enabled)
    {
        arpeggiator.noteOff(midiNote, parameters.arp.hold);
        return;
    }

    triggerDirectNoteOff(midiNote);
}

void SynthEngine::setSustainPedal(bool down) noexcept
{
    voices.setSustainPedal(down);
}

void SynthEngine::setPitchBend(float normalizedBipolar) noexcept
{
    performance.pitchBend = std::clamp(finiteOr(normalizedBipolar, 0.0f), -1.0f, 1.0f);
    parameters.performance = performance;
}

void SynthEngine::setModWheel(float normalized) noexcept
{
    performance.modWheel = std::clamp(finiteOr(normalized, 0.0f), 0.0f, 1.0f);
    parameters.performance = performance;
}

void SynthEngine::setAftertouch(float normalized) noexcept
{
    performance.aftertouch = std::clamp(finiteOr(normalized, 0.0f), 0.0f, 1.0f);
    parameters.performance = performance;
}

void SynthEngine::allNotesOff() noexcept
{
    arpeggiator.reset();
    voices.allNotesOff();
}

void SynthEngine::panic() noexcept
{
    arpeggiator.reset();
    voices.panic();
    fx.reset();
}

void SynthEngine::setParameters(const SynthParameters& newParameters) noexcept
{
    const SynthParameters defaults;
    const auto arpWasEnabled = parameters.arp.enabled;
    parameters = newParameters;
    parameters.performance = performance;
    parameters.voiceMode = static_cast<VoiceMode>(std::clamp(static_cast<int>(parameters.voiceMode), 0, 3));
    parameters.polyphony = std::clamp(parameters.polyphony, 1, 32);
    parameters.unisonCount = std::clamp(parameters.unisonCount, 1, 8);
    parameters.glideMs = std::clamp(finiteOr(parameters.glideMs, defaults.glideMs), 0.0f, 2000.0f);
    parameters.velocityGlideMs = std::clamp(finiteOr(parameters.velocityGlideMs, defaults.velocityGlideMs), 0.0f, 2000.0f);
    for (std::size_t layerIndex = 0; layerIndex < parameters.layers.size(); ++layerIndex)
    {
        auto& layer = parameters.layers[layerIndex];
        const auto& defaultLayer = defaults.layers[layerIndex];
        layer.levelDb = std::clamp(finiteOr(layer.levelDb, defaultLayer.levelDb), -48.0f, 12.0f);
        layer.pan = std::clamp(finiteOr(layer.pan, defaultLayer.pan), -1.0f, 1.0f);

        for (std::size_t oscillatorIndex = 0; oscillatorIndex < layer.oscillators.size(); ++oscillatorIndex)
        {
            auto& oscillator = layer.oscillators[oscillatorIndex];
            const auto& defaultOscillator = defaultLayer.oscillators[oscillatorIndex];
            oscillator.voices = std::clamp(oscillator.voices, 0, 8);
            oscillator.waveform = static_cast<OscillatorSlotWaveform>(
                std::clamp(static_cast<int>(oscillator.waveform), 0, 3));
            oscillator.octave = std::clamp(oscillator.octave, -4, 4);
            oscillator.note = std::clamp(oscillator.note, -12, 12);
            oscillator.fineCents = std::clamp(finiteOr(oscillator.fineCents, defaultOscillator.fineCents),
                                              -100.0f, 100.0f);
            oscillator.level = std::clamp(finiteOr(oscillator.level, defaultOscillator.level), 0.0f, 1.0f);
            oscillator.phaseDegrees = std::clamp(finiteOr(oscillator.phaseDegrees,
                                                          defaultOscillator.phaseDegrees),
                                                 0.0f, 360.0f);
            oscillator.detune = std::clamp(finiteOr(oscillator.detune, defaultOscillator.detune), 0.0f, 1.0f);
            oscillator.stereo = std::clamp(finiteOr(oscillator.stereo, defaultOscillator.stereo), 0.0f, 1.0f);
            oscillator.pan = std::clamp(finiteOr(oscillator.pan, defaultOscillator.pan), -1.0f, 1.0f);
        }
    }
    parameters.osc.pitchSemitones = std::clamp(finiteOr(parameters.osc.pitchSemitones, defaults.osc.pitchSemitones), -48.0f, 48.0f);
    parameters.osc.fineCents = std::clamp(finiteOr(parameters.osc.fineCents, defaults.osc.fineCents), -100.0f, 100.0f);
    parameters.osc.stackCount = std::clamp(parameters.osc.stackCount, 1, 5);
    parameters.osc.stackDetune = std::clamp(finiteOr(parameters.osc.stackDetune, defaults.osc.stackDetune), 0.0f, 1.0f);
    parameters.osc.sawLevel = std::clamp(finiteOr(parameters.osc.sawLevel, defaults.osc.sawLevel), 0.0f, 1.0f);
    parameters.osc.pulseLevel = std::clamp(finiteOr(parameters.osc.pulseLevel, defaults.osc.pulseLevel), 0.0f, 1.0f);
    parameters.osc.noiseLevel = std::clamp(finiteOr(parameters.osc.noiseLevel, defaults.osc.noiseLevel), 0.0f, 1.0f);
    parameters.osc.pulseWidth = std::clamp(finiteOr(parameters.osc.pulseWidth, defaults.osc.pulseWidth), 0.05f, 0.95f);
    parameters.osc.subLevel = std::clamp(finiteOr(parameters.osc.subLevel, defaults.osc.subLevel), 0.0f, 1.0f);
    parameters.osc.subPulseWidth = std::clamp(finiteOr(parameters.osc.subPulseWidth, defaults.osc.subPulseWidth), 0.05f, 0.95f);
    parameters.osc.syncAmount = std::clamp(finiteOr(parameters.osc.syncAmount, defaults.osc.syncAmount), 0.0f, 1.0f);
    parameters.osc.subWave = static_cast<SubWave>(std::clamp(static_cast<int>(parameters.osc.subWave), 0, 3));
    parameters.osc.subOctave = std::clamp(parameters.osc.subOctave, 1, 3);
    parameters.osc.phaseReset = std::clamp(parameters.osc.phaseReset, 0, 3);
    parameters.filter.cutoffSemitones = std::clamp(finiteOr(parameters.filter.cutoffSemitones, defaults.filter.cutoffSemitones), 0.0f, 136.0f);
    parameters.filter.resonance = std::clamp(finiteOr(parameters.filter.resonance, defaults.filter.resonance), 0.0f, 1.0f);
    parameters.filter.drive = std::clamp(finiteOr(parameters.filter.drive, defaults.filter.drive), 0.0f, 1.0f);
    parameters.filter.keytrack = std::clamp(finiteOr(parameters.filter.keytrack, defaults.filter.keytrack), -1.0f, 2.0f);
    parameters.filter.oversampling = std::clamp(parameters.filter.oversampling, 0, 3);
    parameters.filter.mode = static_cast<FilterMode>(std::clamp(static_cast<int>(parameters.filter.mode), 0, 8));
    parameters.amp.drive = std::clamp(finiteOr(parameters.amp.drive, defaults.amp.drive), 0.0f, 1.0f);
    parameters.amp.levelDb = std::clamp(finiteOr(parameters.amp.levelDb, defaults.amp.levelDb), -48.0f, 12.0f);
    parameters.amp.pan = std::clamp(finiteOr(parameters.amp.pan, defaults.amp.pan), -1.0f, 1.0f);
    parameters.amp.panSpread = std::clamp(finiteOr(parameters.amp.panSpread, defaults.amp.panSpread), 0.0f, 1.0f);
    parameters.amp.unisonSpread = std::clamp(finiteOr(parameters.amp.unisonSpread, defaults.amp.unisonSpread), 0.0f, 1.0f);
    parameters.amp.analog = std::clamp(finiteOr(parameters.amp.analog, defaults.amp.analog), 0.0f, 1.0f);
    parameters.ampEnv.attackMs = std::clamp(finiteOr(parameters.ampEnv.attackMs, defaults.ampEnv.attackMs), 0.0f, 10000.0f);
    parameters.ampEnv.decayMs = std::clamp(finiteOr(parameters.ampEnv.decayMs, defaults.ampEnv.decayMs), 1.0f, 10000.0f);
    parameters.ampEnv.sustain = std::clamp(finiteOr(parameters.ampEnv.sustain, defaults.ampEnv.sustain), 0.0f, 1.0f);
    parameters.ampEnv.releaseMs = std::clamp(finiteOr(parameters.ampEnv.releaseMs, defaults.ampEnv.releaseMs), 1.0f, 10000.0f);
    parameters.modEnv.attackMs = std::clamp(finiteOr(parameters.modEnv.attackMs, defaults.modEnv.attackMs), 0.0f, 10000.0f);
    parameters.modEnv.decayMs = std::clamp(finiteOr(parameters.modEnv.decayMs, defaults.modEnv.decayMs), 1.0f, 10000.0f);
    parameters.modEnv.sustain = std::clamp(finiteOr(parameters.modEnv.sustain, defaults.modEnv.sustain), 0.0f, 1.0f);
    parameters.modEnv.releaseMs = std::clamp(finiteOr(parameters.modEnv.releaseMs, defaults.modEnv.releaseMs), 1.0f, 10000.0f);
    parameters.lfo.shape = static_cast<LfoShapeChoice>(std::clamp(static_cast<int>(parameters.lfo.shape), 0, 6));
    parameters.lfo.rateMode = static_cast<LfoRateMode>(std::clamp(static_cast<int>(parameters.lfo.rateMode), 0, 1));
    parameters.lfo.rateHz = std::clamp(finiteOr(parameters.lfo.rateHz, defaults.lfo.rateHz), 0.01f, 40.0f);
    parameters.lfo.syncDivision = std::clamp(parameters.lfo.syncDivision, 0, 5);
    parameters.lfo.phaseDegrees = std::clamp(finiteOr(parameters.lfo.phaseDegrees, defaults.lfo.phaseDegrees), 0.0f, 360.0f);
    parameters.lfo.gateMode = static_cast<LfoGateMode>(std::clamp(static_cast<int>(parameters.lfo.gateMode), 0, 3));
    parameters.lfo.swing = std::clamp(finiteOr(parameters.lfo.swing, defaults.lfo.swing), 0.0f, 1.0f);
    parameters.ramp.mode = static_cast<RampMode>(std::clamp(static_cast<int>(parameters.ramp.mode), 0, 2));
    parameters.ramp.delayMs = std::clamp(finiteOr(parameters.ramp.delayMs, defaults.ramp.delayMs), 0.0f, 10000.0f);
    parameters.ramp.riseMs = std::clamp(finiteOr(parameters.ramp.riseMs, defaults.ramp.riseMs), 1.0f, 10000.0f);
    parameters.ramp.curve = static_cast<RampCurve>(std::clamp(static_cast<int>(parameters.ramp.curve), 0, 2));
    parameters.arp.mode = static_cast<ArpMode>(std::clamp(static_cast<int>(parameters.arp.mode), 0, 3));
    parameters.arp.rate = static_cast<ArpRateDivision>(std::clamp(static_cast<int>(parameters.arp.rate), 0, 4));
    parameters.arp.gate = std::clamp(finiteOr(parameters.arp.gate, defaults.arp.gate), 0.02f, 1.0f);
    parameters.arp.octaves = std::clamp(parameters.arp.octaves, 1, 4);
    parameters.arp.swing = std::clamp(finiteOr(parameters.arp.swing, defaults.arp.swing), 0.0f, 0.75f);
    parameters.arp.stepCount = std::clamp(parameters.arp.stepCount, 1, arpStepCount);
    for (auto& step : parameters.arp.steps)
    {
        step.pitchSemitones = std::clamp(step.pitchSemitones, -24, 24);
        step.velocity = std::clamp(finiteOr(step.velocity, 1.0f), 0.0f, 1.0f);
        step.gate = std::clamp(finiteOr(step.gate, 1.0f), 0.02f, 1.0f);
    }
    parameters.chord.voiceCount = std::clamp(parameters.chord.voiceCount, 1, chordVoiceCount);
    for (auto& voice : parameters.chord.voices)
    {
        voice.pitchSemitones = std::clamp(voice.pitchSemitones, -24, 24);
        voice.velocity = std::clamp(finiteOr(voice.velocity, 1.0f), 0.0f, 1.0f);
    }
    parameters.direct.filterKeytrack = std::clamp(finiteOr(parameters.direct.filterKeytrack, defaults.direct.filterKeytrack), -1.0f, 1.0f);
    parameters.direct.filterLfoSemitones = std::clamp(finiteOr(parameters.direct.filterLfoSemitones, defaults.direct.filterLfoSemitones), -72.0f, 72.0f);
    parameters.direct.filterModEnvSemitones = std::clamp(finiteOr(parameters.direct.filterModEnvSemitones, defaults.direct.filterModEnvSemitones), -72.0f, 72.0f);
    parameters.direct.oscKeytrackSemitones = std::clamp(finiteOr(parameters.direct.oscKeytrackSemitones, defaults.direct.oscKeytrackSemitones), -48.0f, 48.0f);
    parameters.direct.oscLfoSemitones = std::clamp(finiteOr(parameters.direct.oscLfoSemitones, defaults.direct.oscLfoSemitones), -48.0f, 48.0f);
    parameters.direct.oscModEnvSemitones = std::clamp(finiteOr(parameters.direct.oscModEnvSemitones, defaults.direct.oscModEnvSemitones), -48.0f, 48.0f);
    parameters.direct.pulseKeytrack = std::clamp(finiteOr(parameters.direct.pulseKeytrack, defaults.direct.pulseKeytrack), -1.0f, 1.0f);
    parameters.direct.pulseLfo = std::clamp(finiteOr(parameters.direct.pulseLfo, defaults.direct.pulseLfo), -1.0f, 1.0f);
    parameters.direct.pulseModEnv = std::clamp(finiteOr(parameters.direct.pulseModEnv, defaults.direct.pulseModEnv), -1.0f, 1.0f);
    for (auto& slot : parameters.transMod.slots)
    {
        slot.source = static_cast<ModSource>(std::clamp(static_cast<int>(slot.source), 0, 19));
        slot.scaler = static_cast<ModSource>(std::clamp(static_cast<int>(slot.scaler), 0, 19));
        slot.depth = std::clamp(finiteOr(slot.depth, 0.0f), -1.0f, 1.0f);
        slot.oscPitchSemitones = std::clamp(finiteOr(slot.oscPitchSemitones, 0.0f), -48.0f, 48.0f);
        slot.pulseWidth = std::clamp(finiteOr(slot.pulseWidth, 0.0f), -1.0f, 1.0f);
        slot.filterCutoffSemitones = std::clamp(finiteOr(slot.filterCutoffSemitones, 0.0f), -72.0f, 72.0f);
        slot.ampLevelDb = std::clamp(finiteOr(slot.ampLevelDb, 0.0f), -24.0f, 24.0f);
        slot.pan = std::clamp(finiteOr(slot.pan, 0.0f), -1.0f, 1.0f);
    }
    parameters.macro.motion = std::clamp(finiteOr(parameters.macro.motion, defaults.macro.motion), 0.0f, 1.0f);
    parameters.macro.width = std::clamp(finiteOr(parameters.macro.width, defaults.macro.width), 0.0f, 1.0f);
    parameters.macro.drive = std::clamp(finiteOr(parameters.macro.drive, defaults.macro.drive), 0.0f, 1.0f);
    parameters.macro.space = std::clamp(finiteOr(parameters.macro.space, defaults.macro.space), 0.0f, 1.0f);
    parameters.fx.saturationMix = std::clamp(finiteOr(parameters.fx.saturationMix, defaults.fx.saturationMix), 0.0f, 1.0f);
    parameters.fx.saturationDrive = std::clamp(finiteOr(parameters.fx.saturationDrive, defaults.fx.saturationDrive), 0.0f, 1.0f);
    parameters.fx.delayMix = std::clamp(finiteOr(parameters.fx.delayMix, defaults.fx.delayMix), 0.0f, 1.0f);
    parameters.fx.delaySyncDivision = static_cast<DelaySyncDivision>(std::clamp(static_cast<int>(parameters.fx.delaySyncDivision), 0, 4));
    parameters.fx.delayFeedback = std::clamp(finiteOr(parameters.fx.delayFeedback, defaults.fx.delayFeedback), 0.0f, 0.86f);
    parameters.fx.reverbMix = std::clamp(finiteOr(parameters.fx.reverbMix, defaults.fx.reverbMix), 0.0f, 1.0f);
    parameters.fx.reverbDecay = std::clamp(finiteOr(parameters.fx.reverbDecay, defaults.fx.reverbDecay), 0.0f, 1.0f);
    parameters.fx.chorusMix = std::clamp(finiteOr(parameters.fx.chorusMix, defaults.fx.chorusMix), 0.0f, 1.0f);
    parameters.fx.chorusRateHz = std::clamp(finiteOr(parameters.fx.chorusRateHz, defaults.fx.chorusRateHz), 0.02f, 8.0f);
    parameters.fx.chorusDepthMs = std::clamp(finiteOr(parameters.fx.chorusDepthMs, defaults.fx.chorusDepthMs), 0.1f, 24.0f);
    parameters.quality.realtimeMode = static_cast<QualityMode>(std::clamp(static_cast<int>(parameters.quality.realtimeMode), 0, 2));
    parameters.quality.offlineMode = static_cast<QualityMode>(std::clamp(static_cast<int>(parameters.quality.offlineMode), 0, 2));
    parameters.quality.activeMode = static_cast<QualityMode>(std::clamp(static_cast<int>(parameters.quality.activeMode), 0, 2));
    parameters.tempoBpm = std::clamp(finiteOr(parameters.tempoBpm, defaults.tempoBpm), 20.0f, 300.0f);

    if (arpWasEnabled != parameters.arp.enabled)
    {
        arpeggiator.reset();
        voices.allNotesOff();
    }
}

RenderStats SynthEngine::process(float* left, float* right, int numSamples) noexcept
{
    RenderStats stats;
    stats.samplesRendered = std::max(0, numSamples);

    for (int i = 0; i < stats.samplesRendered; ++i)
    {
        if (parameters.arp.enabled)
            processArpEvent(arpeggiator.processSample(parameters, sampleRate));

        const auto frame = voices.renderSample(parameters);
        const auto fxFrame = fx.process({ frame.left, frame.right }, parameters);
        auto l = fxFrame.left;
        auto r = fxFrame.right;

        if (!std::isfinite(l))
        {
            ++stats.invalidSamples;
            l = 0.0f;
        }

        if (!std::isfinite(r))
        {
            ++stats.invalidSamples;
            r = 0.0f;
        }

        l = std::clamp(l, -1.0f, 1.0f);
        r = std::clamp(r, -1.0f, 1.0f);

        if (left != nullptr)
            left[i] = l;

        if (right != nullptr)
            right[i] = r;

        stats.peak = std::max(stats.peak, std::max(std::abs(l), std::abs(r)));
    }

    stats.activeVoices = voices.activeVoiceCount();

    return stats;
}

void SynthEngine::triggerDirectNoteOn(int midiNote, float velocity) noexcept
{
    if (!parameters.chord.enabled)
    {
        voices.noteOn(midiNote, velocity, parameters);
        return;
    }

    const auto voiceLimit = std::clamp(parameters.chord.voiceCount, 1, chordVoiceCount);
    auto triggered = false;
    for (int voiceIndex = 0; voiceIndex < voiceLimit; ++voiceIndex)
    {
        const auto& chordVoice = parameters.chord.voices[static_cast<std::size_t>(voiceIndex)];
        if (!chordVoice.enabled)
            continue;

        voices.noteOn(std::clamp(midiNote + chordVoice.pitchSemitones, 0, 127),
                      std::clamp(velocity * chordVoice.velocity, 0.0f, 1.0f),
                      parameters);
        triggered = true;
    }

    if (!triggered)
        voices.noteOn(midiNote, velocity, parameters);
}

void SynthEngine::triggerDirectNoteOff(int midiNote) noexcept
{
    if (!parameters.chord.enabled)
    {
        voices.noteOff(midiNote, parameters);
        return;
    }

    const auto voiceLimit = std::clamp(parameters.chord.voiceCount, 1, chordVoiceCount);
    auto released = false;
    for (int voiceIndex = 0; voiceIndex < voiceLimit; ++voiceIndex)
    {
        const auto& chordVoice = parameters.chord.voices[static_cast<std::size_t>(voiceIndex)];
        if (!chordVoice.enabled)
            continue;

        voices.noteOff(std::clamp(midiNote + chordVoice.pitchSemitones, 0, 127), parameters);
        released = true;
    }

    if (!released)
        voices.noteOff(midiNote, parameters);
}

void SynthEngine::processArpEvent(const ArpGeneratedEvent& event) noexcept
{
    if (event.noteOff)
        voices.noteOff(event.noteOffNumber, parameters);

    if (event.noteOn)
        voices.noteOn(event.noteOnNumber, event.velocity, parameters);
}
} // namespace synth
