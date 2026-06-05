#pragma once

#include <JuceHeader.h>

#include "../dsp/SynthEngine.h"
#include "ParameterRegistry.h"

#include <array>
#include <atomic>
#include <vector>

class SynthAudioProcessor final : public juce::AudioProcessor
{
public:
    SynthAudioProcessor();
    ~SynthAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Init"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() noexcept { return parameters; }

    struct PresetListItem
    {
        juce::String displayName;
        juce::File file;
        bool factory = false;
    };

    struct DiagnosticsSnapshot
    {
        double sampleRate = 0.0;
        int blockSize = 0;
        int activeVoices = 0;
        int midiEvents = 0;
        int invalidSamples = 0;
        float peak = 0.0f;
        juce::String architecture;
        juce::String currentPreset;
        juce::String lastPresetStatus;
    };

    std::vector<PresetListItem> getPresetList() const;
    juce::File getUserPresetDirectory() const;
    bool loadPresetFile(const juce::File& file, juce::String& message);
    bool savePresetFile(const juce::File& file, const juce::String& displayName, juce::String& message);
    juce::String getCurrentPresetName() const;
    DiagnosticsSnapshot getDiagnosticsSnapshot() const;
    void requestPanic() noexcept;

private:
    struct RawParameterPointers
    {
        struct RawTransModSlot
        {
            std::atomic<float>* enabled = nullptr;
            std::atomic<float>* source = nullptr;
            std::atomic<float>* scaler = nullptr;
            std::atomic<float>* depth = nullptr;
            std::atomic<float>* oscPitchSemitones = nullptr;
            std::atomic<float>* pulseWidth = nullptr;
            std::atomic<float>* filterCutoffSemitones = nullptr;
            std::atomic<float>* ampLevelDb = nullptr;
            std::atomic<float>* pan = nullptr;
        };

        std::atomic<float>* voiceMode = nullptr;
        std::atomic<float>* voicePolyphony = nullptr;
        std::atomic<float>* voiceUnisonCount = nullptr;
        std::atomic<float>* voiceRetrigger = nullptr;
        std::atomic<float>* voiceGlideMs = nullptr;
        std::atomic<float>* voiceVelocityGlideMs = nullptr;
        std::atomic<float>* oscPitchSemitones = nullptr;
        std::atomic<float>* oscFineCents = nullptr;
        std::atomic<float>* oscStackCount = nullptr;
        std::atomic<float>* oscStackDetune = nullptr;
        std::atomic<float>* oscSawLevel = nullptr;
        std::atomic<float>* oscPulseLevel = nullptr;
        std::atomic<float>* oscNoiseLevel = nullptr;
        std::atomic<float>* oscPulseWidth = nullptr;
        std::atomic<float>* oscSubWave = nullptr;
        std::atomic<float>* oscSubOctave = nullptr;
        std::atomic<float>* oscSubLevel = nullptr;
        std::atomic<float>* oscSubPulseWidth = nullptr;
        std::atomic<float>* oscSyncAmount = nullptr;
        std::atomic<float>* oscPhaseReset = nullptr;
        std::atomic<float>* filterEnabled = nullptr;
        std::atomic<float>* filterMode = nullptr;
        std::atomic<float>* filterCutoffSemitones = nullptr;
        std::atomic<float>* filterResonance = nullptr;
        std::atomic<float>* filterDrive = nullptr;
        std::atomic<float>* filterKeytrack = nullptr;
        std::atomic<float>* filterOversampling = nullptr;
        std::atomic<float>* ampDrive = nullptr;
        std::atomic<float>* ampLevelDb = nullptr;
        std::atomic<float>* ampPan = nullptr;
        std::atomic<float>* ampPanSpread = nullptr;
        std::atomic<float>* ampUnisonSpread = nullptr;
        std::atomic<float>* ampAnalog = nullptr;
        std::atomic<float>* ampAttack = nullptr;
        std::atomic<float>* ampDecay = nullptr;
        std::atomic<float>* ampSustain = nullptr;
        std::atomic<float>* ampRelease = nullptr;
        std::atomic<float>* modAttack = nullptr;
        std::atomic<float>* modDecay = nullptr;
        std::atomic<float>* modSustain = nullptr;
        std::atomic<float>* modRelease = nullptr;
        std::atomic<float>* lfoShape = nullptr;
        std::atomic<float>* lfoRateMode = nullptr;
        std::atomic<float>* lfoRateHz = nullptr;
        std::atomic<float>* lfoSyncDivision = nullptr;
        std::atomic<float>* lfoPhaseDegrees = nullptr;
        std::atomic<float>* lfoGateMode = nullptr;
        std::atomic<float>* lfoMono = nullptr;
        std::atomic<float>* lfoSwing = nullptr;
        std::atomic<float>* rampEnabled = nullptr;
        std::atomic<float>* rampMode = nullptr;
        std::atomic<float>* rampDelayMs = nullptr;
        std::atomic<float>* rampRiseMs = nullptr;
        std::atomic<float>* rampCurve = nullptr;
        std::atomic<float>* directFilterKeytrack = nullptr;
        std::atomic<float>* directFilterLfoSemitones = nullptr;
        std::atomic<float>* directFilterModEnvSemitones = nullptr;
        std::atomic<float>* directOscKeytrackSemitones = nullptr;
        std::atomic<float>* directOscLfoSemitones = nullptr;
        std::atomic<float>* directOscModEnvSemitones = nullptr;
        std::atomic<float>* directPulseKeytrack = nullptr;
        std::atomic<float>* directPulseLfo = nullptr;
        std::atomic<float>* directPulseModEnv = nullptr;
        std::array<RawTransModSlot, synth::transModSlotCount> transMod {};
        std::atomic<float>* macroMotion = nullptr;
        std::atomic<float>* macroWidth = nullptr;
        std::atomic<float>* macroDrive = nullptr;
        std::atomic<float>* macroSpace = nullptr;
    };

    void cacheParameterPointers();
    synth::SynthParameters readParameters(float tempoBpm) const noexcept;
    float currentTempoBpm() const noexcept;
    void handleMidiMessage(const juce::MidiMessage& message) noexcept;
    synth::RenderStats renderSegment(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) noexcept;

    juce::AudioProcessorValueTreeState parameters;
    synth::SynthEngine engine;
    RawParameterPointers raw;
    static constexpr int scratchCapacity = 32768;
    std::array<float, scratchCapacity> scratchLeft {};
    std::array<float, scratchCapacity> scratchRight {};
    std::atomic<double> diagnosticSampleRate { 0.0 };
    std::atomic<int> diagnosticBlockSize { 0 };
    std::atomic<int> diagnosticActiveVoices { 0 };
    std::atomic<int> diagnosticMidiEvents { 0 };
    std::atomic<int> diagnosticInvalidSamples { 0 };
    std::atomic<float> diagnosticPeak { 0.0f };
    std::atomic<bool> panicRequested { false };
    juce::String currentPresetName { "Init" };
    juce::String lastPresetStatus { "Init state" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthAudioProcessor)
};
