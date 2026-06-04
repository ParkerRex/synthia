#include "PluginProcessor.h"

#include "PluginEditor.h"

SynthAudioProcessor::SynthAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "SYNTH_STATE", synth::createParameterLayout())
{
    cacheParameterPointers();
}

void SynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock);
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
    engine.setParameters(readParameters(currentTempoBpm()));

    for (int channel = 2; channel < buffer.getNumChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    auto renderedUntil = 0;
    const auto totalSamples = buffer.getNumSamples();
    for (const auto metadata : midiMessages)
    {
        const auto eventSample = std::clamp(metadata.samplePosition, 0, totalSamples);
        renderSegment(buffer, renderedUntil, eventSample - renderedUntil);
        renderedUntil = eventSample;

        handleMidiMessage(metadata.getMessage());
    }

    renderSegment(buffer, renderedUntil, totalSamples - renderedUntil);
    midiMessages.clear();
}

void SynthAudioProcessor::handleMidiMessage(const juce::MidiMessage& message) noexcept
{
    if (message.isNoteOn(false))
        engine.noteOn(message.getNoteNumber(), message.getFloatVelocity());
    else if (message.isNoteOff(true))
        engine.noteOff(message.getNoteNumber());
    else if (message.isAllSoundOff())
        engine.panic();
    else if (message.isAllNotesOff())
        engine.allNotesOff();
    else if (message.isControllerOfType(64))
        engine.setSustainPedal(message.getControllerValue() >= 64);
}

void SynthAudioProcessor::renderSegment(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) noexcept
{
    if (numSamples <= 0)
        return;

    if (buffer.getNumChannels() <= 0)
    {
        engine.process(nullptr, nullptr, numSamples);
        return;
    }

    if (buffer.getNumChannels() > 1)
    {
        engine.process(buffer.getWritePointer(0, startSample),
                       buffer.getWritePointer(1, startSample),
                       numSamples);
        return;
    }

    auto* mono = buffer.getWritePointer(0, startSample);
    auto remaining = numSamples;
    auto offset = 0;
    while (remaining > 0)
    {
        const auto chunk = std::min(remaining, scratchCapacity);
        engine.process(scratchLeft.data(), scratchRight.data(), chunk);
        for (int i = 0; i < chunk; ++i)
            mono[offset + i] = 0.5f * (scratchLeft[static_cast<std::size_t>(i)]
                + scratchRight[static_cast<std::size_t>(i)]);

        offset += chunk;
        remaining -= chunk;
    }
}

void SynthAudioProcessor::cacheParameterPointers()
{
    auto get = [this](const char* id) {
        return parameters.getRawParameterValue(id);
    };

    raw.voicePolyphony = get("voice.polyphony");
    raw.voiceUnisonCount = get("voice.unison_count");
    raw.voiceRetrigger = get("voice.retrigger");
    raw.voiceGlideMs = get("voice.glide_ms");
    raw.voiceVelocityGlideMs = get("voice.velocity_glide_ms");
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
    raw.directFilterKeytrack = get("direct.filter_keytrack");
    raw.directFilterLfoSemitones = get("direct.filter_lfo_semitones");
    raw.directFilterModEnvSemitones = get("direct.filter_mod_env_semitones");
    raw.directOscLfoSemitones = get("direct.osc_lfo_semitones");
    raw.directOscModEnvSemitones = get("direct.osc_mod_env_semitones");
    raw.directPulseLfo = get("direct.pulse_lfo");
    raw.directPulseModEnv = get("direct.pulse_mod_env");
    raw.macroMotion = get("macro.motion");
    raw.macroWidth = get("macro.width");
    raw.macroDrive = get("macro.drive");
    raw.macroSpace = get("macro.space");
}

synth::SynthParameters SynthAudioProcessor::readParameters(float tempoBpm) const noexcept
{
    auto value = [](const std::atomic<float>* parameter, float fallback) noexcept {
        return parameter != nullptr ? parameter->load(std::memory_order_relaxed) : fallback;
    };

    synth::SynthParameters snapshot;
    snapshot.polyphony = static_cast<int>(std::round(value(raw.voicePolyphony, 8.0f)));
    snapshot.unisonCount = static_cast<int>(std::round(value(raw.voiceUnisonCount, 1.0f)));
    snapshot.retrigger = value(raw.voiceRetrigger, 1.0f) >= 0.5f;
    snapshot.glideMs = value(raw.voiceGlideMs, 0.0f);
    snapshot.velocityGlideMs = value(raw.voiceVelocityGlideMs, 0.0f);
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
    snapshot.direct.filterKeytrack = value(raw.directFilterKeytrack, 0.0f);
    snapshot.direct.filterLfoSemitones = value(raw.directFilterLfoSemitones, 0.0f);
    snapshot.direct.filterModEnvSemitones = value(raw.directFilterModEnvSemitones, 0.0f);
    snapshot.direct.oscLfoSemitones = value(raw.directOscLfoSemitones, 0.0f);
    snapshot.direct.oscModEnvSemitones = value(raw.directOscModEnvSemitones, 0.0f);
    snapshot.direct.pulseLfo = value(raw.directPulseLfo, 0.0f);
    snapshot.direct.pulseModEnv = value(raw.directPulseModEnv, 0.0f);
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

void SynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    state.setProperty("schema_version", 1, nullptr);
    state.setProperty("plugin_version", ProjectInfo::versionString, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void SynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName("SYNTH_STATE"))
        {
            auto state = juce::ValueTree::fromXml(*xml);
            if (state.isValid())
                parameters.replaceState(state);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthAudioProcessor();
}
