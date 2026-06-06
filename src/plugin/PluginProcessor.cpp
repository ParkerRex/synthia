#include "PluginProcessor.h"

#include "PluginEditor.h"
#include "../presets/PresetManager.h"

#include <algorithm>

namespace
{
class ScopedParameterStateUpdate final
{
public:
    explicit ScopedParameterStateUpdate(std::atomic<std::uint64_t>& sequenceToUpdate) noexcept
        : sequence(sequenceToUpdate)
    {
        sequence.fetch_add(1, std::memory_order_acq_rel);
    }

    ~ScopedParameterStateUpdate()
    {
        sequence.fetch_add(1, std::memory_order_release);
    }

private:
    std::atomic<std::uint64_t>& sequence;
};

juce::String binaryArchitecture()
{
#if defined(__arm64__) || defined(__aarch64__)
    return "arm64";
#elif defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#else
    return "unknown";
#endif
}
} // namespace

SynthAudioProcessor::SynthAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "SYLENTH_AI_STATE", synth::createParameterLayout())
{
    cacheParameterPointers();
}

void SynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock);
    diagnosticSampleRate.store(sampleRate, std::memory_order_relaxed);
    diagnosticBlockSize.store(samplesPerBlock, std::memory_order_relaxed);
}

void SynthAudioProcessor::releaseResources()
{
    engine.reset();
}

bool SynthAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono()
        || output == juce::AudioChannelSet::stereo();
}

void SynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const auto totalSamples = buffer.getNumSamples();

    auto clearForStateUpdate = [this, &buffer, &midiMessages, totalSamples] {
        buffer.clear();
        midiMessages.clear();
        diagnosticPeak.store(0.0f, std::memory_order_relaxed);
        diagnosticActiveVoices.store(0, std::memory_order_relaxed);
        diagnosticBlockSize.store(totalSamples, std::memory_order_relaxed);
    };

    const auto sequenceBeforeRead = parameterStateSequence.load(std::memory_order_acquire);
    if ((sequenceBeforeRead & 1u) != 0u)
    {
        clearForStateUpdate();
        return;
    }

    const auto tempoBpm = currentTempoBpm();
    diagnosticTempoBpm.store(tempoBpm, std::memory_order_relaxed);
    auto parameterSnapshot = readParameters(tempoBpm, isNonRealtime());
    const auto sequenceAfterRead = parameterStateSequence.load(std::memory_order_acquire);
    if (sequenceBeforeRead != sequenceAfterRead || (sequenceAfterRead & 1u) != 0u)
    {
        clearForStateUpdate();
        return;
    }

    if (panicRequested.exchange(false, std::memory_order_acq_rel))
        engine.panic();

    engine.setParameters(parameterSnapshot);

    for (int channel = 2; channel < buffer.getNumChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    auto renderedUntil = 0;
    auto blockPeak = 0.0f;
    auto blockInvalidSamples = 0;
    auto blockActiveVoices = 0;

    for (const auto metadata : midiMessages)
    {
        const auto eventSample = std::clamp(metadata.samplePosition, 0, totalSamples);
        const auto segmentStats = renderSegment(buffer, renderedUntil, eventSample - renderedUntil);
        blockPeak = std::max(blockPeak, segmentStats.peak);
        blockInvalidSamples += segmentStats.invalidSamples;
        blockActiveVoices = segmentStats.activeVoices;
        renderedUntil = eventSample;

        handleMidiMessage(metadata.getMessage());
    }

    const auto finalStats = renderSegment(buffer, renderedUntil, totalSamples - renderedUntil);
    blockPeak = std::max(blockPeak, finalStats.peak);
    blockInvalidSamples += finalStats.invalidSamples;
    blockActiveVoices = finalStats.activeVoices;

    diagnosticPeak.store(blockPeak, std::memory_order_relaxed);
    diagnosticInvalidSamples.fetch_add(blockInvalidSamples, std::memory_order_relaxed);
    diagnosticActiveVoices.store(blockActiveVoices, std::memory_order_relaxed);
    diagnosticBlockSize.store(totalSamples, std::memory_order_relaxed);
    midiMessages.clear();
}

double SynthAudioProcessor::getTailLengthSeconds() const
{
    auto snapshot = readParameters(diagnosticTempoBpm.load(std::memory_order_relaxed), false);
    const auto voiceTail = std::max(snapshot.ampEnv.releaseMs, snapshot.modEnv.releaseMs) * 0.001f;
    return std::max(static_cast<double>(voiceTail),
                    static_cast<double>(synth::fxTailLengthSeconds(snapshot)));
}

void SynthAudioProcessor::handleMidiMessage(const juce::MidiMessage& message) noexcept
{
    diagnosticMidiEvents.fetch_add(1, std::memory_order_relaxed);

    if (message.isNoteOn(false))
        engine.noteOn(message.getNoteNumber(), message.getFloatVelocity());
    else if (message.isNoteOff(true))
        engine.noteOff(message.getNoteNumber());
    else if (message.isAllSoundOff())
        engine.panic();
    else if (message.isAllNotesOff())
        engine.allNotesOff();
    else if (message.isPitchWheel())
        engine.setPitchBend((static_cast<float>(message.getPitchWheelValue()) - 8192.0f) / 8192.0f);
    else if (message.isControllerOfType(64))
        engine.setSustainPedal(message.getControllerValue() >= 64);
    else if (message.isControllerOfType(1))
        engine.setModWheel(static_cast<float>(message.getControllerValue()) / 127.0f);
    else if (message.isAftertouch())
        engine.setAftertouch(static_cast<float>(message.getAfterTouchValue()) / 127.0f);
    else if (message.isChannelPressure())
        engine.setAftertouch(static_cast<float>(message.getChannelPressureValue()) / 127.0f);
}

synth::RenderStats SynthAudioProcessor::renderSegment(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) noexcept
{
    if (numSamples <= 0)
        return {};

    if (buffer.getNumChannels() <= 0)
    {
        return engine.process(nullptr, nullptr, numSamples);
    }

    if (buffer.getNumChannels() > 1)
    {
        return engine.process(buffer.getWritePointer(0, startSample),
                              buffer.getWritePointer(1, startSample),
                              numSamples);
    }

    auto* mono = buffer.getWritePointer(0, startSample);
    auto remaining = numSamples;
    auto offset = 0;
    synth::RenderStats stats;
    stats.samplesRendered = numSamples;

    while (remaining > 0)
    {
        const auto chunk = std::min(remaining, scratchCapacity);
        const auto chunkStats = engine.process(scratchLeft.data(), scratchRight.data(), chunk);
        stats.invalidSamples += chunkStats.invalidSamples;
        stats.peak = std::max(stats.peak, chunkStats.peak);
        stats.activeVoices = chunkStats.activeVoices;

        for (int i = 0; i < chunk; ++i)
            mono[offset + i] = 0.5f * (scratchLeft[static_cast<std::size_t>(i)]
                + scratchRight[static_cast<std::size_t>(i)]);

        offset += chunk;
        remaining -= chunk;
    }

    return stats;
}

void SynthAudioProcessor::cacheParameterPointers()
{
    auto get = [this](const char* id) {
        return parameters.getRawParameterValue(id);
    };

    raw.voiceMode = get("voice.mode");
    raw.voicePolyphony = get("voice.polyphony");
    raw.voiceUnisonCount = get("voice.unison_count");
    raw.voiceRetrigger = get("voice.retrigger");
    raw.voiceGlideMs = get("voice.glide_ms");
    raw.voiceVelocityGlideMs = get("voice.velocity_glide_ms");
    for (int layer = 0; layer < synth::layerCount; ++layer)
    {
        const auto layerPrefix = "layer." + std::to_string(layer + 1) + ".";
        auto& rawLayer = raw.layers[static_cast<std::size_t>(layer)];
        rawLayer.enabled = get((layerPrefix + "enabled").c_str());
        rawLayer.levelDb = get((layerPrefix + "level_db").c_str());
        rawLayer.pan = get((layerPrefix + "pan").c_str());
        rawLayer.solo = get((layerPrefix + "solo").c_str());
        rawLayer.mute = get((layerPrefix + "mute").c_str());

        for (int oscillator = 0; oscillator < synth::oscillatorSlotsPerLayer; ++oscillator)
        {
            const auto oscillatorPrefix = layerPrefix + "osc." + std::to_string(oscillator + 1) + ".";
            auto& rawOscillator = rawLayer.oscillators[static_cast<std::size_t>(oscillator)];
            rawOscillator.enabled = get((oscillatorPrefix + "enabled").c_str());
            rawOscillator.voices = get((oscillatorPrefix + "voices").c_str());
            rawOscillator.waveform = get((oscillatorPrefix + "waveform").c_str());
            rawOscillator.octave = get((oscillatorPrefix + "octave").c_str());
            rawOscillator.note = get((oscillatorPrefix + "note").c_str());
            rawOscillator.fineCents = get((oscillatorPrefix + "fine_cents").c_str());
            rawOscillator.level = get((oscillatorPrefix + "level").c_str());
            rawOscillator.phaseDegrees = get((oscillatorPrefix + "phase_degrees").c_str());
            rawOscillator.detune = get((oscillatorPrefix + "detune").c_str());
            rawOscillator.stereo = get((oscillatorPrefix + "stereo").c_str());
            rawOscillator.pan = get((oscillatorPrefix + "pan").c_str());
            rawOscillator.retrigger = get((oscillatorPrefix + "retrigger").c_str());
            rawOscillator.invert = get((oscillatorPrefix + "invert").c_str());
        }
    }
    raw.oscPitchSemitones = get("osc.pitch_semitones");
    raw.oscFineCents = get("osc.fine_cents");
    raw.oscStackCount = get("osc.stack_count");
    raw.oscStackDetune = get("osc.stack_detune");
    raw.oscSawLevel = get("osc.saw_level");
    raw.oscPulseLevel = get("osc.pulse_level");
    raw.oscNoiseLevel = get("osc.noise_level");
    raw.oscPulseWidth = get("osc.pulse_width");
    raw.oscSubWave = get("osc.sub_wave");
    raw.oscSubOctave = get("osc.sub_octave");
    raw.oscSubLevel = get("osc.sub_level");
    raw.oscSubPulseWidth = get("osc.sub_pulse_width");
    raw.oscSyncAmount = get("osc.sync_amount");
    raw.oscPhaseReset = get("osc.phase_reset");
    raw.filterEnabled = get("filter.enabled");
    raw.filterMode = get("filter.mode");
    raw.filterCutoffSemitones = get("filter.cutoff_semitones");
    raw.filterResonance = get("filter.resonance");
    raw.filterDrive = get("filter.drive");
    raw.filterKeytrack = get("filter.keytrack");
    raw.filterOversampling = get("filter.oversampling");
    raw.ampDrive = get("amp.drive");
    raw.ampLevelDb = get("amp.level_db");
    raw.ampPan = get("amp.pan");
    raw.ampPanSpread = get("amp.pan_spread");
    raw.ampUnisonSpread = get("amp.unison_spread");
    raw.ampAnalog = get("amp.analog");
    raw.ampAttack = get("amp_env.attack_ms");
    raw.ampDecay = get("amp_env.decay_ms");
    raw.ampSustain = get("amp_env.sustain");
    raw.ampRelease = get("amp_env.release_ms");
    raw.modAttack = get("mod_env.attack_ms");
    raw.modDecay = get("mod_env.decay_ms");
    raw.modSustain = get("mod_env.sustain");
    raw.modRelease = get("mod_env.release_ms");
    raw.lfoShape = get("lfo.shape");
    raw.lfoRateMode = get("lfo.rate_mode");
    raw.lfoRateHz = get("lfo.rate_hz");
    raw.lfoSyncDivision = get("lfo.sync_division");
    raw.lfoPhaseDegrees = get("lfo.phase_degrees");
    raw.lfoGateMode = get("lfo.gate_mode");
    raw.lfoMono = get("lfo.mono");
    raw.lfoSwing = get("lfo.swing");
    raw.rampEnabled = get("ramp.enabled");
    raw.rampMode = get("ramp.mode");
    raw.rampDelayMs = get("ramp.delay_ms");
    raw.rampRiseMs = get("ramp.rise_ms");
    raw.rampCurve = get("ramp.curve");
    raw.directFilterKeytrack = get("direct.filter_keytrack");
    raw.directFilterLfoSemitones = get("direct.filter_lfo_semitones");
    raw.directFilterModEnvSemitones = get("direct.filter_mod_env_semitones");
    raw.directOscKeytrackSemitones = get("direct.osc_keytrack_semitones");
    raw.directOscLfoSemitones = get("direct.osc_lfo_semitones");
    raw.directOscModEnvSemitones = get("direct.osc_mod_env_semitones");
    raw.directPulseKeytrack = get("direct.pulse_keytrack");
    raw.directPulseLfo = get("direct.pulse_lfo");
    raw.directPulseModEnv = get("direct.pulse_mod_env");
    raw.fxEnabled = get("fx.enabled");
    raw.fxSaturationEnabled = get("fx.saturation_enabled");
    raw.fxSaturationMix = get("fx.saturation_mix");
    raw.fxSaturationDrive = get("fx.saturation_drive");
    raw.fxDelayEnabled = get("fx.delay_enabled");
    raw.fxDelayMix = get("fx.delay_mix");
    raw.fxDelaySyncDivision = get("fx.delay_sync_division");
    raw.fxDelayFeedback = get("fx.delay_feedback");
    raw.fxReverbEnabled = get("fx.reverb_enabled");
    raw.fxReverbMix = get("fx.reverb_mix");
    raw.fxReverbDecay = get("fx.reverb_decay");
    raw.fxChorusEnabled = get("fx.chorus_enabled");
    raw.fxChorusMix = get("fx.chorus_mix");
    raw.fxChorusRateHz = get("fx.chorus_rate_hz");
    raw.fxChorusDepthMs = get("fx.chorus_depth_ms");
    raw.qualityRealtimeMode = get("quality.realtime_mode");
    raw.qualityOfflineMode = get("quality.offline_mode");
    for (int slot = 0; slot < synth::transModSlotCount; ++slot)
    {
        const auto prefix = "transmod." + std::to_string(slot + 1) + ".";
        auto& rawSlot = raw.transMod[static_cast<std::size_t>(slot)];
        rawSlot.enabled = get((prefix + "enabled").c_str());
        rawSlot.source = get((prefix + "source").c_str());
        rawSlot.scaler = get((prefix + "scaler").c_str());
        rawSlot.depth = get((prefix + "depth").c_str());
        rawSlot.oscPitchSemitones = get((prefix + "osc_pitch_semitones").c_str());
        rawSlot.pulseWidth = get((prefix + "pulse_width").c_str());
        rawSlot.filterCutoffSemitones = get((prefix + "filter_cutoff_semitones").c_str());
        rawSlot.ampLevelDb = get((prefix + "amp_level_db").c_str());
        rawSlot.pan = get((prefix + "pan").c_str());
    }
    raw.macroMotion = get("macro.motion");
    raw.macroWidth = get("macro.width");
    raw.macroDrive = get("macro.drive");
    raw.macroSpace = get("macro.space");
}

synth::SynthParameters SynthAudioProcessor::readParameters(float tempoBpm, bool offlineRender) const noexcept
{
    auto value = [](const std::atomic<float>* parameter, float fallback) noexcept {
        const auto loaded = parameter != nullptr ? parameter->load(std::memory_order_relaxed) : fallback;
        return std::isfinite(loaded) ? loaded : fallback;
    };

    const synth::SynthParameters defaults;
    synth::SynthParameters snapshot;
    snapshot.voiceMode = static_cast<synth::VoiceMode>(static_cast<int>(std::round(value(raw.voiceMode, 2.0f))));
    snapshot.polyphony = static_cast<int>(std::round(value(raw.voicePolyphony, 8.0f)));
    snapshot.unisonCount = static_cast<int>(std::round(value(raw.voiceUnisonCount, 1.0f)));
    snapshot.retrigger = value(raw.voiceRetrigger, 1.0f) >= 0.5f;
    snapshot.glideMs = value(raw.voiceGlideMs, 0.0f);
    snapshot.velocityGlideMs = value(raw.voiceVelocityGlideMs, 0.0f);
    for (int layer = 0; layer < synth::layerCount; ++layer)
    {
        const auto& rawLayer = raw.layers[static_cast<std::size_t>(layer)];
        const auto& defaultLayer = defaults.layers[static_cast<std::size_t>(layer)];
        auto& layerSnapshot = snapshot.layers[static_cast<std::size_t>(layer)];
        layerSnapshot.enabled = value(rawLayer.enabled, defaultLayer.enabled ? 1.0f : 0.0f) >= 0.5f;
        layerSnapshot.levelDb = value(rawLayer.levelDb, defaultLayer.levelDb);
        layerSnapshot.pan = value(rawLayer.pan, defaultLayer.pan);
        layerSnapshot.solo = value(rawLayer.solo, defaultLayer.solo ? 1.0f : 0.0f) >= 0.5f;
        layerSnapshot.mute = value(rawLayer.mute, defaultLayer.mute ? 1.0f : 0.0f) >= 0.5f;

        for (int oscillator = 0; oscillator < synth::oscillatorSlotsPerLayer; ++oscillator)
        {
            const auto& rawOscillator = rawLayer.oscillators[static_cast<std::size_t>(oscillator)];
            const auto& defaultOscillator = defaultLayer.oscillators[static_cast<std::size_t>(oscillator)];
            auto& oscillatorSnapshot = layerSnapshot.oscillators[static_cast<std::size_t>(oscillator)];
            oscillatorSnapshot.enabled = value(rawOscillator.enabled,
                                               defaultOscillator.enabled ? 1.0f : 0.0f) >= 0.5f;
            oscillatorSnapshot.voices = static_cast<int>(std::round(value(rawOscillator.voices,
                                                                          static_cast<float>(defaultOscillator.voices))));
            oscillatorSnapshot.waveform = static_cast<synth::OscillatorSlotWaveform>(
                static_cast<int>(std::round(value(rawOscillator.waveform,
                                                  static_cast<float>(static_cast<int>(
                                                      defaultOscillator.waveform))))));
            oscillatorSnapshot.octave = static_cast<int>(std::round(value(rawOscillator.octave,
                                                                          static_cast<float>(defaultOscillator.octave))));
            oscillatorSnapshot.note = static_cast<int>(std::round(value(rawOscillator.note,
                                                                        static_cast<float>(defaultOscillator.note))));
            oscillatorSnapshot.fineCents = value(rawOscillator.fineCents, defaultOscillator.fineCents);
            oscillatorSnapshot.level = value(rawOscillator.level, defaultOscillator.level);
            oscillatorSnapshot.phaseDegrees = value(rawOscillator.phaseDegrees, defaultOscillator.phaseDegrees);
            oscillatorSnapshot.detune = value(rawOscillator.detune, defaultOscillator.detune);
            oscillatorSnapshot.stereo = value(rawOscillator.stereo, defaultOscillator.stereo);
            oscillatorSnapshot.pan = value(rawOscillator.pan, defaultOscillator.pan);
            oscillatorSnapshot.retrigger = value(rawOscillator.retrigger,
                                                 defaultOscillator.retrigger ? 1.0f : 0.0f) >= 0.5f;
            oscillatorSnapshot.invert = value(rawOscillator.invert,
                                              defaultOscillator.invert ? 1.0f : 0.0f) >= 0.5f;
        }
    }
    snapshot.osc.pitchSemitones = value(raw.oscPitchSemitones, 0.0f);
    snapshot.osc.fineCents = value(raw.oscFineCents, 0.0f);
    snapshot.osc.stackCount = static_cast<int>(std::round(value(raw.oscStackCount, 1.0f)));
    snapshot.osc.stackDetune = value(raw.oscStackDetune, 0.0f);
    snapshot.osc.sawLevel = value(raw.oscSawLevel, 1.0f);
    snapshot.osc.pulseLevel = value(raw.oscPulseLevel, 0.0f);
    snapshot.osc.noiseLevel = value(raw.oscNoiseLevel, 0.0f);
    snapshot.osc.pulseWidth = value(raw.oscPulseWidth, 0.5f);
    snapshot.osc.subWave = static_cast<synth::SubWave>(static_cast<int>(std::round(value(raw.oscSubWave, 2.0f))));
    snapshot.osc.subOctave = static_cast<int>(std::round(value(raw.oscSubOctave, 0.0f))) + 1;
    snapshot.osc.subLevel = value(raw.oscSubLevel, 0.0f);
    snapshot.osc.subPulseWidth = value(raw.oscSubPulseWidth, 0.5f);
    snapshot.osc.syncAmount = value(raw.oscSyncAmount, 0.0f);
    snapshot.osc.phaseReset = static_cast<int>(std::round(value(raw.oscPhaseReset, 0.0f)));
    snapshot.filter.enabled = value(raw.filterEnabled, 1.0f) >= 0.5f;
    snapshot.filter.mode = static_cast<synth::FilterMode>(static_cast<int>(std::round(value(raw.filterMode, 1.0f))));
    snapshot.filter.cutoffSemitones = value(raw.filterCutoffSemitones, 96.0f);
    snapshot.filter.resonance = value(raw.filterResonance, 0.0f);
    snapshot.filter.drive = value(raw.filterDrive, 0.0f);
    snapshot.filter.keytrack = value(raw.filterKeytrack, 0.5f);
    snapshot.filter.oversampling = static_cast<int>(std::round(value(raw.filterOversampling, 1.0f)));
    snapshot.amp.drive = value(raw.ampDrive, 0.0f);
    snapshot.amp.levelDb = value(raw.ampLevelDb, -6.0f);
    snapshot.amp.pan = value(raw.ampPan, 0.0f);
    snapshot.amp.panSpread = value(raw.ampPanSpread, 0.0f);
    snapshot.amp.unisonSpread = value(raw.ampUnisonSpread, 0.0f);
    snapshot.amp.analog = value(raw.ampAnalog, 0.0f);
    snapshot.ampEnv = { value(raw.ampAttack, 2.0f), value(raw.ampDecay, 300.0f),
                        value(raw.ampSustain, 0.8f), value(raw.ampRelease, 200.0f) };
    snapshot.modEnv = { value(raw.modAttack, 1.0f), value(raw.modDecay, 400.0f),
                        value(raw.modSustain, 0.0f), value(raw.modRelease, 160.0f) };
    snapshot.lfo.shape = static_cast<synth::LfoShapeChoice>(static_cast<int>(std::round(value(raw.lfoShape, 3.0f))));
    snapshot.lfo.rateMode = static_cast<synth::LfoRateMode>(static_cast<int>(std::round(value(raw.lfoRateMode, 1.0f))));
    snapshot.lfo.rateHz = value(raw.lfoRateHz, 2.0f);
    snapshot.lfo.syncDivision = static_cast<int>(std::round(value(raw.lfoSyncDivision, 3.0f)));
    snapshot.lfo.phaseDegrees = value(raw.lfoPhaseDegrees, 0.0f);
    snapshot.lfo.gateMode = static_cast<synth::LfoGateMode>(static_cast<int>(std::round(value(raw.lfoGateMode, 1.0f))));
    snapshot.lfo.mono = value(raw.lfoMono, 0.0f) >= 0.5f;
    snapshot.lfo.swing = value(raw.lfoSwing, 0.0f);
    snapshot.ramp.enabled = value(raw.rampEnabled, 0.0f) >= 0.5f;
    snapshot.ramp.mode = static_cast<synth::RampMode>(static_cast<int>(std::round(value(raw.rampMode, 0.0f))));
    snapshot.ramp.delayMs = value(raw.rampDelayMs, 0.0f);
    snapshot.ramp.riseMs = value(raw.rampRiseMs, 1000.0f);
    snapshot.ramp.curve = static_cast<synth::RampCurve>(static_cast<int>(std::round(value(raw.rampCurve, 0.0f))));
    snapshot.direct.filterKeytrack = value(raw.directFilterKeytrack, 0.0f);
    snapshot.direct.filterLfoSemitones = value(raw.directFilterLfoSemitones, 0.0f);
    snapshot.direct.filterModEnvSemitones = value(raw.directFilterModEnvSemitones, 0.0f);
    snapshot.direct.oscKeytrackSemitones = value(raw.directOscKeytrackSemitones, 0.0f);
    snapshot.direct.oscLfoSemitones = value(raw.directOscLfoSemitones, 0.0f);
    snapshot.direct.oscModEnvSemitones = value(raw.directOscModEnvSemitones, 0.0f);
    snapshot.direct.pulseKeytrack = value(raw.directPulseKeytrack, 0.0f);
    snapshot.direct.pulseLfo = value(raw.directPulseLfo, 0.0f);
    snapshot.direct.pulseModEnv = value(raw.directPulseModEnv, 0.0f);
    snapshot.fx.enabled = value(raw.fxEnabled, 0.0f) >= 0.5f;
    snapshot.fx.saturationEnabled = value(raw.fxSaturationEnabled, 1.0f) >= 0.5f;
    snapshot.fx.saturationMix = value(raw.fxSaturationMix, 0.0f);
    snapshot.fx.saturationDrive = value(raw.fxSaturationDrive, 0.35f);
    snapshot.fx.delayEnabled = value(raw.fxDelayEnabled, 1.0f) >= 0.5f;
    snapshot.fx.delayMix = value(raw.fxDelayMix, 0.0f);
    snapshot.fx.delaySyncDivision = static_cast<synth::DelaySyncDivision>(static_cast<int>(std::round(value(raw.fxDelaySyncDivision, 1.0f))));
    snapshot.fx.delayFeedback = value(raw.fxDelayFeedback, 0.22f);
    snapshot.fx.reverbEnabled = value(raw.fxReverbEnabled, 1.0f) >= 0.5f;
    snapshot.fx.reverbMix = value(raw.fxReverbMix, 0.0f);
    snapshot.fx.reverbDecay = value(raw.fxReverbDecay, 0.35f);
    snapshot.fx.chorusEnabled = value(raw.fxChorusEnabled, 0.0f) >= 0.5f;
    snapshot.fx.chorusMix = value(raw.fxChorusMix, 0.0f);
    snapshot.fx.chorusRateHz = value(raw.fxChorusRateHz, 0.35f);
    snapshot.fx.chorusDepthMs = value(raw.fxChorusDepthMs, 5.0f);
    snapshot.quality.realtimeMode = static_cast<synth::QualityMode>(static_cast<int>(std::round(value(raw.qualityRealtimeMode, 1.0f))));
    snapshot.quality.offlineMode = static_cast<synth::QualityMode>(static_cast<int>(std::round(value(raw.qualityOfflineMode, 2.0f))));
    snapshot.quality.activeMode = offlineRender ? snapshot.quality.offlineMode : snapshot.quality.realtimeMode;
    for (int slot = 0; slot < synth::transModSlotCount; ++slot)
    {
        const auto& rawSlot = raw.transMod[static_cast<std::size_t>(slot)];
        auto& slotSnapshot = snapshot.transMod.slots[static_cast<std::size_t>(slot)];
        slotSnapshot.enabled = value(rawSlot.enabled, 0.0f) >= 0.5f;
        slotSnapshot.source = static_cast<synth::ModSource>(static_cast<int>(std::round(value(rawSlot.source, 0.0f))));
        slotSnapshot.scaler = static_cast<synth::ModSource>(static_cast<int>(std::round(value(rawSlot.scaler, 0.0f))));
        slotSnapshot.depth = value(rawSlot.depth, 0.0f);
        slotSnapshot.oscPitchSemitones = value(rawSlot.oscPitchSemitones, 0.0f);
        slotSnapshot.pulseWidth = value(rawSlot.pulseWidth, 0.0f);
        slotSnapshot.filterCutoffSemitones = value(rawSlot.filterCutoffSemitones, 0.0f);
        slotSnapshot.ampLevelDb = value(rawSlot.ampLevelDb, 0.0f);
        slotSnapshot.pan = value(rawSlot.pan, 0.0f);
    }
    snapshot.macro.motion = value(raw.macroMotion, 0.5f);
    snapshot.macro.width = value(raw.macroWidth, 0.0f);
    snapshot.macro.drive = value(raw.macroDrive, 0.0f);
    snapshot.macro.space = value(raw.macroSpace, 0.0f);
    snapshot.tempoBpm = tempoBpm;
    return snapshot;
}

float SynthAudioProcessor::currentTempoBpm() const noexcept
{
    if (auto* playHead = getPlayHead())
    {
        if (const auto position = playHead->getPosition())
        {
            if (const auto bpm = position->getBpm())
                return static_cast<float>(*bpm);
        }
    }

    return 128.0f;
}

juce::AudioProcessorEditor* SynthAudioProcessor::createEditor()
{
    return new SynthAudioProcessorEditor(*this);
}

std::vector<SynthAudioProcessor::PresetListItem> SynthAudioProcessor::getPresetList() const
{
    std::vector<PresetListItem> items;

    auto append = [&items](const std::vector<synth::PresetSummary>& presets) {
        for (const auto& preset : presets)
        {
            PresetListItem item;
            item.displayName = preset.displayName.empty() ? juce::String(preset.path.stem().string())
                                                          : juce::String(preset.displayName);
            item.file = juce::File(juce::String(preset.path.string()));
            item.factory = preset.factory;
            items.push_back(item);
        }
    };

    append(synth::scanPresetDirectory(synth::factoryPresetDirectory(), true));
    append(synth::scanPresetDirectory(synth::defaultUserPresetDirectory(), false));
    append(synth::scanPresetDirectory(synth::legacyUserPresetDirectory(), false));
    return items;
}

juce::File SynthAudioProcessor::getUserPresetDirectory() const
{
    return juce::File(juce::String(synth::defaultUserPresetDirectory().string()));
}

bool SynthAudioProcessor::loadPresetFile(const juce::File& file, juce::String& message)
{
    const auto result = synth::preparePresetState(parameters, file.getFullPathName().toStdString());
    message = result.message;
    if (result.loaded)
    {
        {
            ScopedParameterStateUpdate update(parameterStateSequence);
            parameters.replaceState(result.state);
        }

        const auto presetName = result.displayName.empty() ? file.getFileNameWithoutExtension()
                                                           : juce::String(result.displayName);
        setPresetMetadata(presetName, message);
        panicRequested.store(true, std::memory_order_release);
    }
    else
    {
        setPresetMetadata(getCurrentPresetName(), message);
    }

    return result.loaded;
}

bool SynthAudioProcessor::savePresetFile(const juce::File& file,
                                         const juce::String& displayName,
                                         juce::String& message)
{
    std::string error;
    const auto saved = synth::writeCurrentPreset(parameters, file.getFullPathName().toStdString(),
                                                 displayName.toStdString(), error);
    if (saved)
    {
        const auto presetName = displayName.isNotEmpty() ? displayName : file.getFileNameWithoutExtension();
        message = "Saved preset: " + presetName;
        setPresetMetadata(presetName, message);
        return true;
    }

    message = "Preset save failed: " + juce::String(error);
    setPresetMetadata(getCurrentPresetName(), message);
    return false;
}

juce::String SynthAudioProcessor::getCurrentPresetName() const
{
    const juce::CriticalSection::ScopedLockType lock(presetMetadataLock);
    return currentPresetName;
}

SynthAudioProcessor::DiagnosticsSnapshot SynthAudioProcessor::getDiagnosticsSnapshot() const
{
    DiagnosticsSnapshot snapshot;
    snapshot.sampleRate = diagnosticSampleRate.load(std::memory_order_relaxed);
    snapshot.blockSize = diagnosticBlockSize.load(std::memory_order_relaxed);
    snapshot.activeVoices = diagnosticActiveVoices.load(std::memory_order_relaxed);
    snapshot.midiEvents = diagnosticMidiEvents.load(std::memory_order_relaxed);
    snapshot.invalidSamples = diagnosticInvalidSamples.load(std::memory_order_relaxed);
    snapshot.peak = diagnosticPeak.load(std::memory_order_relaxed);
    snapshot.architecture = binaryArchitecture();
    const juce::CriticalSection::ScopedLockType lock(presetMetadataLock);
    snapshot.currentPreset = currentPresetName;
    snapshot.lastPresetStatus = lastPresetStatus;
    return snapshot;
}

void SynthAudioProcessor::requestPanic() noexcept
{
    panicRequested.store(true, std::memory_order_release);
}

void SynthAudioProcessor::setPresetMetadata(const juce::String& presetName, const juce::String& status)
{
    const juce::CriticalSection::ScopedLockType lock(presetMetadataLock);
    currentPresetName = presetName;
    lastPresetStatus = status;
}

void SynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    state.setProperty("schema_version", 1, nullptr);
    state.setProperty("plugin_version", ProjectInfo::versionString, nullptr);
    state.setProperty("current_preset", getCurrentPresetName(), nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void SynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName("SYLENTH_AI_STATE") || xml->hasTagName("SYNTH_STATE"))
        {
            auto state = juce::ValueTree::fromXml(*xml);
            if (state.isValid())
            {
                const auto presetName = state.getProperty("current_preset", "Restored State").toString();
                const auto migratedState = synth::mergeParameterStateWithDefaults(parameters, state);
                {
                    ScopedParameterStateUpdate update(parameterStateSequence);
                    parameters.replaceState(migratedState);
                }
                setPresetMetadata(presetName, "Host state restored");
                panicRequested.store(true, std::memory_order_release);
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthAudioProcessor();
}
