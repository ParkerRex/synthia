#include "../../src/plugin/ParameterRegistry.h"
#include "../../src/dsp/SynthParameters.h"
#include "../../src/presets/PresetManager.h"
#include "../../src/presets/PresetValidator.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <cmath>
#include <iostream>
#include <set>

namespace
{
class StateRoundTripProcessor final : public juce::AudioProcessor
{
public:
    StateRoundTripProcessor()
        : parameters(*this, nullptr, "SYLENTH_AI_STATE", synth::createParameterLayout())
    {
    }

    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override { return true; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    const juce::String getName() const override { return "StateRoundTripProcessor"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Init"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    juce::AudioProcessorValueTreeState parameters;
};

bool parameterValueMatches(const juce::AudioProcessorValueTreeState& parameters,
                           const char* id,
                           float expected,
                           float tolerance = 0.001f);

bool setPhysicalParameter(juce::AudioProcessorValueTreeState& parameters, const char* id, float value);

bool checkStateRoundTrip()
{
    StateRoundTripProcessor source;
    if (!setPhysicalParameter(source.parameters, "layer.2.enabled", 1.0f)
        || !setPhysicalParameter(source.parameters, "layer.2.osc.2.voices", 6.0f)
        || !setPhysicalParameter(source.parameters, "layer.2.osc.2.waveform", 3.0f)
        || !setPhysicalParameter(source.parameters, "layer.1.osc.2.invert", 1.0f))
    {
        return false;
    }

    auto state = source.parameters.copyState();
    state.setProperty("schema_version", 1, nullptr);
    state.setProperty("plugin_version", "0.1.0-test", nullptr);

    const auto xml = state.createXml();
    if (xml == nullptr)
        return false;

    const auto restoredState = juce::ValueTree::fromXml(*xml);
    if (!restoredState.isValid() || !restoredState.hasType("SYLENTH_AI_STATE"))
        return false;

    StateRoundTripProcessor destination;
    destination.parameters.replaceState(restoredState);

    return destination.parameters.state.hasProperty("schema_version")
        && destination.parameters.getRawParameterValue("filter.cutoff_semitones") != nullptr
        && parameterValueMatches(destination.parameters, "layer.2.enabled", 1.0f)
        && parameterValueMatches(destination.parameters, "layer.2.osc.2.voices", 6.0f)
        && parameterValueMatches(destination.parameters, "layer.2.osc.2.waveform", 3.0f)
        && parameterValueMatches(destination.parameters, "layer.1.osc.2.invert", 1.0f);
}

bool parameterValueMatches(const juce::AudioProcessorValueTreeState& parameters,
                           const char* id,
                           float expected,
                           float tolerance)
{
    const auto* value = parameters.getRawParameterValue(id);
    return value != nullptr && std::abs(value->load() - expected) <= tolerance;
}

bool setPhysicalParameter(juce::AudioProcessorValueTreeState& parameters, const char* id, float value)
{
    auto* parameter = parameters.getParameter(id);
    if (parameter == nullptr)
        return false;

    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    return true;
}

void removeParameterChildrenStartingWith(juce::ValueTree& state, const char* prefix)
{
    for (auto childIndex = state.getNumChildren(); --childIndex >= 0;)
    {
        const auto child = state.getChild(childIndex);
        if (!child.hasType("PARAM"))
            continue;

        if (child.getProperty("id").toString().startsWith(prefix))
            state.removeChild(childIndex, nullptr);
    }
}

bool checkHostStateDefaultMerge()
{
    StateRoundTripProcessor source;
    if (!setPhysicalParameter(source.parameters, "filter.cutoff_semitones", 58.0f))
        return false;

    auto oldState = source.parameters.copyState();
    oldState.setProperty("schema_version", 1, nullptr);
    oldState.setProperty("current_preset", "Old Host State", nullptr);
    removeParameterChildrenStartingWith(oldState, "layer.");

    StateRoundTripProcessor destination;
    if (!setPhysicalParameter(destination.parameters, "layer.1.enabled", 0.0f)
        || !setPhysicalParameter(destination.parameters, "layer.2.enabled", 1.0f)
        || !setPhysicalParameter(destination.parameters, "layer.2.osc.2.voices", 8.0f)
        || !setPhysicalParameter(destination.parameters, "layer.2.osc.2.invert", 1.0f))
    {
        return false;
    }

    const auto migratedState = synth::mergeParameterStateWithDefaults(destination.parameters, oldState);
    destination.parameters.replaceState(migratedState);

    return parameterValueMatches(destination.parameters, "filter.cutoff_semitones", 58.0f)
        && parameterValueMatches(destination.parameters, "layer.1.enabled", 1.0f)
        && parameterValueMatches(destination.parameters, "layer.2.enabled", 0.0f)
        && parameterValueMatches(destination.parameters, "layer.1.osc.1.enabled", 1.0f)
        && parameterValueMatches(destination.parameters, "layer.1.osc.1.voices", 1.0f)
        && parameterValueMatches(destination.parameters, "layer.2.osc.2.voices", 0.0f)
        && parameterValueMatches(destination.parameters, "layer.2.osc.2.invert", 0.0f);
}

bool checkLayerOscillatorVoiceCost()
{
    synth::SynthParameters parameters;
    if (synth::layerOscillatorVoiceCost(parameters) != 1)
    {
        std::cerr << "Default layer voice cost should be one enabled Layer A oscillator.\n";
        return false;
    }

    parameters.layers[0].oscillators[1].enabled = true;
    parameters.layers[0].oscillators[1].voices = 3;
    parameters.layers[1].enabled = true;
    parameters.layers[1].oscillators[0].enabled = true;
    parameters.layers[1].oscillators[0].voices = 8;
    if (synth::layerOscillatorVoiceCost(parameters) != 12)
    {
        std::cerr << "Layer voice cost did not include all active unsoloed slots.\n";
        return false;
    }

    parameters.layers[0].solo = true;
    if (synth::layerOscillatorVoiceCost(parameters) != 4)
    {
        std::cerr << "Layer solo should isolate the soloed layer voice cost.\n";
        return false;
    }

    parameters.layers[0].mute = true;
    if (synth::layerOscillatorVoiceCost(parameters) != 0)
    {
        std::cerr << "Muted solo layer should have zero voice cost.\n";
        return false;
    }

    return true;
}

bool checkPresetManagerLoadAndSave()
{
    StateRoundTripProcessor processor;
    if (!parameterValueMatches(processor.parameters, "filter.cutoff_semitones", 96.0f))
    {
        std::cerr << "Unexpected default filter cutoff before preset prepare.\n";
        return false;
    }

    if (!parameterValueMatches(processor.parameters, "layer.1.enabled", 1.0f)
        || !parameterValueMatches(processor.parameters, "layer.2.enabled", 0.0f)
        || !parameterValueMatches(processor.parameters, "layer.1.osc.1.enabled", 1.0f)
        || !parameterValueMatches(processor.parameters, "layer.1.osc.1.voices", 1.0f)
        || !parameterValueMatches(processor.parameters, "layer.1.osc.2.enabled", 0.0f)
        || !parameterValueMatches(processor.parameters, "layer.2.osc.2.voices", 0.0f))
    {
        std::cerr << "Layer/oscillator defaults do not match the Phase 1 backbone contract.\n";
        return false;
    }

    const auto prepared = synth::preparePresetState(processor.parameters, "presets/factory/pluck-core-01.json");
    if (!prepared.loaded || !prepared.state.isValid())
    {
        std::cerr << prepared.message << "\n";
        return false;
    }

    if (!parameterValueMatches(processor.parameters, "filter.cutoff_semitones", 96.0f))
    {
        std::cerr << "Preset prepare mutated live parameters before replaceState.\n";
        return false;
    }

    const auto load = synth::loadPresetIntoState(processor.parameters, "presets/factory/pluck-core-01.json");
    if (!load.loaded)
    {
        std::cerr << load.message << "\n";
        return false;
    }

    if (!parameterValueMatches(processor.parameters, "filter.cutoff_semitones", 58.0f))
    {
        std::cerr << "Preset manager did not apply filter cutoff.\n";
        return false;
    }

    if (!parameterValueMatches(processor.parameters, "layer.1.enabled", 1.0f)
        || !parameterValueMatches(processor.parameters, "layer.2.enabled", 0.0f)
        || !parameterValueMatches(processor.parameters, "layer.1.osc.1.enabled", 1.0f)
        || !parameterValueMatches(processor.parameters, "layer.2.osc.1.enabled", 0.0f))
    {
        std::cerr << "Legacy preset load did not preserve new layer defaults.\n";
        return false;
    }

    const auto scanInputFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("sylenth-ai-preset-scan-input");
    scanInputFile.replaceWithText("not a directory");
    const auto scannedFile = synth::scanPresetDirectory(scanInputFile.getFullPathName().toStdString(), false);
    scanInputFile.deleteFile();
    if (!scannedFile.empty())
    {
        std::cerr << "Preset directory scan should ignore non-directory paths.\n";
        return false;
    }

    const auto tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("sylenth-ai-preset-manager-test.json");
    tempFile.deleteFile();

    std::string error;
    if (!synth::writeCurrentPreset(processor.parameters, tempFile.getFullPathName().toStdString(),
                                   "Preset Manager Test", error))
    {
        std::cerr << error << "\n";
        return false;
    }

    const auto validation = synth::validatePresetFile(tempFile.getFullPathName().toStdString());
    if (!validation.passed())
    {
        std::cerr << "Preset manager saved invalid preset.\n";
        for (const auto& validationError : validation.errors)
            std::cerr << "  " << validationError << "\n";
        tempFile.deleteFile();
        return false;
    }

    const auto savedPreset = juce::JSON::parse(tempFile);
    const auto* savedObject = savedPreset.getDynamicObject();
    const auto savedParameters = savedObject != nullptr
        ? savedObject->getProperty(juce::Identifier("parameters"))
        : juce::var();
    const auto* savedParameterObject = savedParameters.getDynamicObject();
    if (savedParameterObject == nullptr
        || !savedParameterObject->hasProperty(juce::Identifier("layer.1.osc.1.voices"))
        || !savedParameterObject->hasProperty(juce::Identifier("layer.2.enabled"))
        || !savedParameterObject->hasProperty(juce::Identifier("layer.2.osc.2.invert")))
    {
        std::cerr << "Saved preset omitted layer/oscillator state.\n";
        tempFile.deleteFile();
        return false;
    }
    tempFile.deleteFile();

    const auto extensionlessFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("sylenth-ai-preset-manager-extension-test");
    const auto extensionlessJsonFile = extensionlessFile.withFileExtension(".json");
    extensionlessFile.deleteFile();
    extensionlessJsonFile.deleteFile();
    if (!synth::writeCurrentPreset(processor.parameters, extensionlessFile.getFullPathName().toStdString(),
                                   "Preset Extension Test", error))
    {
        std::cerr << error << "\n";
        return false;
    }

    const auto extensionValidation = synth::validatePresetFile(extensionlessJsonFile.getFullPathName().toStdString());
    extensionlessFile.deleteFile();
    extensionlessJsonFile.deleteFile();
    if (!extensionValidation.passed())
    {
        std::cerr << "Preset manager did not normalize extensionless save path.\n";
        return false;
    }

    return true;
}
} // namespace

int main()
{
    const auto errors = synth::validateParameterSpecs();
    if (!errors.empty())
    {
        for (const auto& error : errors)
            std::cerr << error << "\n";
        return 1;
    }

    const auto& specs = synth::getParameterSpecs();
    if (specs.size() < 200)
    {
        std::cerr << "Expected broad v1 parameter inventory, got " << specs.size() << "\n";
        return 1;
    }

    if (synth::findParameterSpec("filter.cutoff_semitones") == nullptr
        || synth::findParameterSpec("transmod.8.depth") == nullptr
        || synth::findParameterSpec("macro.space") == nullptr
        || synth::findParameterSpec("layer.1.enabled") == nullptr
        || synth::findParameterSpec("layer.2.enabled") == nullptr
        || synth::findParameterSpec("layer.1.osc.1.voices") == nullptr
        || synth::findParameterSpec("layer.2.osc.2.invert") == nullptr)
    {
        std::cerr << "Required parameter missing.\n";
        return 1;
    }

    const auto* transModAmp = synth::findParameterSpec("transmod.1.amp_level_db");
    if (transModAmp == nullptr
        || std::abs(transModAmp->minimum + 24.0f) > 0.0001f
        || std::abs(transModAmp->maximum - 24.0f) > 0.0001f)
    {
        std::cerr << "TransMod amp-level range mismatch.\n";
        return 1;
    }

    const auto* layerAEnabled = synth::findParameterSpec("layer.1.enabled");
    const auto* layerBEnabled = synth::findParameterSpec("layer.2.enabled");
    const auto* layerAOscillatorVoices = synth::findParameterSpec("layer.1.osc.1.voices");
    const auto* layerBOscillatorInvert = synth::findParameterSpec("layer.2.osc.2.invert");
    if (layerAEnabled == nullptr
        || layerBEnabled == nullptr
        || layerAOscillatorVoices == nullptr
        || layerBOscillatorInvert == nullptr
        || std::abs(layerAEnabled->defaultValue - 1.0f) > 0.0001f
        || std::abs(layerBEnabled->defaultValue) > 0.0001f
        || std::abs(layerAOscillatorVoices->minimum) > 0.0001f
        || std::abs(layerAOscillatorVoices->maximum - 8.0f) > 0.0001f
        || std::abs(layerAOscillatorVoices->defaultValue - 1.0f) > 0.0001f
        || std::abs(layerBOscillatorInvert->defaultValue) > 0.0001f)
    {
        std::cerr << "Layer/oscillator registry defaults or ranges mismatch.\n";
        return 1;
    }

    if (!checkStateRoundTrip())
    {
        std::cerr << "APVTS state round-trip failed.\n";
        return 1;
    }

    if (!checkHostStateDefaultMerge())
    {
        std::cerr << "Host state default merge failed.\n";
        return 1;
    }

    if (!checkLayerOscillatorVoiceCost())
        return 1;

    if (!checkPresetManagerLoadAndSave())
        return 1;

    const auto presetResults = synth::validatePresetDirectory("presets/factory");
    for (const auto& result : presetResults)
    {
        if (!result.passed())
        {
            std::cerr << "Preset failed: " << result.path << "\n";
            for (const auto& error : result.errors)
                std::cerr << "  " << error << "\n";
            return 1;
        }
    }

    const auto invalidModDepth = synth::validatePresetFile("fixtures/presets/invalid-mod-slot-depth.json");
    if (invalidModDepth.passed())
    {
        std::cerr << "Invalid mod slot depth fixture passed validation.\n";
        return 1;
    }

    return 0;
}
