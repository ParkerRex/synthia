#include "../../src/plugin/ParameterRegistry.h"
#include "../../src/dsp/SynthParameters.h"
#include "../../src/presets/PresetManager.h"
#include "../../src/presets/PresetValidator.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <algorithm>
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
        || !setPhysicalParameter(source.parameters, "layer.1.osc.2.invert", 1.0f)
        || !setPhysicalParameter(source.parameters, "arp.enabled", 1.0f)
        || !setPhysicalParameter(source.parameters, "arp.step.2.pitch_semitones", 7.0f)
        || !setPhysicalParameter(source.parameters, "chord.enabled", 1.0f)
        || !setPhysicalParameter(source.parameters, "chord.voice.2.pitch_semitones", 4.0f))
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
        && parameterValueMatches(destination.parameters, "layer.1.osc.2.invert", 1.0f)
        && parameterValueMatches(destination.parameters, "arp.enabled", 1.0f)
        && parameterValueMatches(destination.parameters, "arp.step.2.pitch_semitones", 7.0f)
        && parameterValueMatches(destination.parameters, "chord.enabled", 1.0f)
        && parameterValueMatches(destination.parameters, "chord.voice.2.pitch_semitones", 4.0f);
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

bool containsString(const std::vector<std::string>& values, const std::string& expected)
{
    return std::find(values.begin(), values.end(), expected) != values.end();
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
    removeParameterChildrenStartingWith(oldState, "arp.");
    removeParameterChildrenStartingWith(oldState, "chord.");

    StateRoundTripProcessor destination;
    if (!setPhysicalParameter(destination.parameters, "layer.1.enabled", 0.0f)
        || !setPhysicalParameter(destination.parameters, "layer.2.enabled", 1.0f)
        || !setPhysicalParameter(destination.parameters, "layer.2.osc.2.voices", 8.0f)
        || !setPhysicalParameter(destination.parameters, "layer.2.osc.2.invert", 1.0f)
        || !setPhysicalParameter(destination.parameters, "arp.enabled", 1.0f)
        || !setPhysicalParameter(destination.parameters, "arp.step.1.tie", 1.0f)
        || !setPhysicalParameter(destination.parameters, "chord.enabled", 1.0f)
        || !setPhysicalParameter(destination.parameters, "chord.voice.3.enabled", 1.0f))
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
        && parameterValueMatches(destination.parameters, "layer.2.osc.2.invert", 0.0f)
        && parameterValueMatches(destination.parameters, "arp.enabled", 0.0f)
        && parameterValueMatches(destination.parameters, "arp.step.1.tie", 0.0f)
        && parameterValueMatches(destination.parameters, "chord.enabled", 0.0f)
        && parameterValueMatches(destination.parameters, "chord.voice.1.enabled", 1.0f)
        && parameterValueMatches(destination.parameters, "chord.voice.3.enabled", 0.0f);
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
        || !parameterValueMatches(processor.parameters, "layer.2.osc.2.voices", 0.0f)
        || !parameterValueMatches(processor.parameters, "arp.enabled", 0.0f)
        || !parameterValueMatches(processor.parameters, "arp.step_count", 16.0f)
        || !parameterValueMatches(processor.parameters, "arp.step.1.enabled", 1.0f)
        || !parameterValueMatches(processor.parameters, "chord.enabled", 0.0f)
        || !parameterValueMatches(processor.parameters, "chord.voice.1.enabled", 1.0f))
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
        || !parameterValueMatches(processor.parameters, "layer.2.osc.1.enabled", 0.0f)
        || !parameterValueMatches(processor.parameters, "arp.enabled", 0.0f)
        || !parameterValueMatches(processor.parameters, "chord.enabled", 0.0f))
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
        || !savedParameterObject->hasProperty(juce::Identifier("layer.2.osc.2.invert"))
        || !savedParameterObject->hasProperty(juce::Identifier("arp.enabled"))
        || !savedParameterObject->hasProperty(juce::Identifier("arp.step.16.tie"))
        || !savedParameterObject->hasProperty(juce::Identifier("chord.enabled"))
        || !savedParameterObject->hasProperty(juce::Identifier("chord.voice.8.pitch_semitones")))
    {
        std::cerr << "Saved preset omitted layer/oscillator state.\n";
        tempFile.deleteFile();
        return false;
    }

    const auto savedMetadata = savedObject != nullptr
        ? savedObject->getProperty(juce::Identifier("metadata"))
        : juce::var();
    const auto* savedMetadataObject = savedMetadata.getDynamicObject();
    const auto savedBrowserMetadata = savedMetadataObject != nullptr
        ? savedMetadataObject->getProperty(juce::Identifier("browser"))
        : juce::var();
    const auto* savedBrowserObject = savedBrowserMetadata.getDynamicObject();
    if (savedBrowserObject == nullptr
        || savedBrowserObject->getProperty(juce::Identifier("bank")).toString() != "User"
        || savedBrowserObject->getProperty(juce::Identifier("category")).toString() != "User"
        || savedBrowserObject->getProperty(juce::Identifier("source")).toString() != "user")
    {
        std::cerr << "Saved preset omitted browser metadata.\n";
        tempFile.deleteFile();
        return false;
    }
    tempFile.deleteFile();

    const auto factoryPresets = synth::scanPresetDirectory("presets/factory", synth::PresetSource::Factory, {});
    const auto pluckPreset = std::find_if(factoryPresets.begin(), factoryPresets.end(), [](const auto& preset) {
        return preset.id == "pluck-core-01";
    });
    if (pluckPreset == factoryPresets.end()
        || pluckPreset->bank != "Factory"
        || pluckPreset->category != "Plucks"
        || !containsString(pluckPreset->tags, "pluck")
        || pluckPreset->favorite
        || pluckPreset->favoriteKey != "factory:pluck-core-01")
    {
        std::cerr << "Factory preset browser metadata scan failed.\n";
        return false;
    }

    const auto browserDirectory = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("sylenth-ai-preset-browser-contract");
    browserDirectory.deleteRecursively();
    if (!browserDirectory.createDirectory())
    {
        std::cerr << "Could not create preset browser test directory.\n";
        return false;
    }

    const auto browserPresetFile = browserDirectory.getChildFile("browser-favorite-test.json");
    if (!synth::writeCurrentPreset(processor.parameters, browserPresetFile.getFullPathName().toStdString(),
                                   "Browser Favorite Test", error))
    {
        std::cerr << error << "\n";
        browserDirectory.deleteRecursively();
        return false;
    }

    const auto favoriteKey = synth::presetFavoriteKey(synth::PresetSource::User,
                                                      "browser-favorite-test",
                                                      browserPresetFile.getFullPathName().toStdString());
    const auto favoritesFile = browserDirectory.getChildFile("PresetFavorites.json");
    if (!synth::setPresetFavorite(favoritesFile.getFullPathName().toStdString(), favoriteKey, true, error))
    {
        std::cerr << error << "\n";
        browserDirectory.deleteRecursively();
        return false;
    }

    const auto favoriteKeys = synth::readFavoritePresetKeys(favoritesFile.getFullPathName().toStdString());
    const auto browserPresets = synth::scanPresetDirectory(browserDirectory.getFullPathName().toStdString(),
                                                           synth::PresetSource::User,
                                                           favoriteKeys);
    if (browserPresets.size() != 1
        || !browserPresets.front().favorite
        || browserPresets.front().bank != "User"
        || browserPresets.front().category != "User"
        || !containsString(browserPresets.front().tags, "user"))
    {
        std::cerr << "Preset browser scan did not apply sidecar favorite metadata.\n";
        browserDirectory.deleteRecursively();
        return false;
    }

    const auto duplicateBrowserPresetFile = browserDirectory.getChildFile("browser-favorite-test-duplicate.json");
    if (!synth::writeCurrentPreset(processor.parameters, duplicateBrowserPresetFile.getFullPathName().toStdString(),
                                   "Browser Favorite Test", error))
    {
        std::cerr << error << "\n";
        browserDirectory.deleteRecursively();
        return false;
    }

    const auto duplicateBrowserPresets = synth::scanPresetDirectory(browserDirectory.getFullPathName().toStdString(),
                                                                    synth::PresetSource::User,
                                                                    favoriteKeys);
    const auto duplicateFavoriteCount = std::count_if(duplicateBrowserPresets.begin(),
                                                      duplicateBrowserPresets.end(),
                                                      [](const auto& preset) {
                                                          return preset.favorite;
                                                      });
    if (duplicateBrowserPresets.size() != 2 || duplicateFavoriteCount != 1)
    {
        std::cerr << "Preset browser favorite keys must not collide for duplicate user preset IDs.\n";
        browserDirectory.deleteRecursively();
        return false;
    }

    synth::PresetBrowserFilter filter;
    filter.searchText = "favorite";
    filter.category = "User";
    filter.tags = { "user" };
    filter.favoritesOnly = true;
    const auto filteredFavorites = synth::filterPresetSummaries(browserPresets, filter);
    if (filteredFavorites.size() != 1 || filteredFavorites.front().id != "browser-favorite-test")
    {
        std::cerr << "Preset browser filter failed to match favorite user preset.\n";
        browserDirectory.deleteRecursively();
        return false;
    }

    filter.includeUser = false;
    if (!synth::filterPresetSummaries(browserPresets, filter).empty())
    {
        std::cerr << "Preset browser source filter did not exclude user preset.\n";
        browserDirectory.deleteRecursively();
        return false;
    }

    const auto catalog = synth::buildPresetBrowserCatalog(browserPresets);
    if (!containsString(catalog.banks, "User")
        || !containsString(catalog.categories, "User")
        || !containsString(catalog.tags, "user"))
    {
        std::cerr << "Preset browser catalog facets missing expected values.\n";
        browserDirectory.deleteRecursively();
        return false;
    }

    if (!synth::setPresetFavorite(favoritesFile.getFullPathName().toStdString(), favoriteKey, false, error)
        || !synth::readFavoritePresetKeys(favoritesFile.getFullPathName().toStdString()).empty())
    {
        std::cerr << "Preset favorite sidecar remove failed.\n";
        browserDirectory.deleteRecursively();
        return false;
    }
    browserDirectory.deleteRecursively();

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
        || synth::findParameterSpec("layer.2.osc.2.invert") == nullptr
        || synth::findParameterSpec("arp.enabled") == nullptr
        || synth::findParameterSpec("arp.step.16.tie") == nullptr
        || synth::findParameterSpec("chord.enabled") == nullptr
        || synth::findParameterSpec("chord.voice.8.velocity") == nullptr)
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
    const auto* arpEnabled = synth::findParameterSpec("arp.enabled");
    const auto* arpStepCount = synth::findParameterSpec("arp.step_count");
    const auto* arpStepPitch = synth::findParameterSpec("arp.step.16.pitch_semitones");
    const auto* chordVoiceCount = synth::findParameterSpec("chord.voice_count");
    const auto* chordVoiceOneEnabled = synth::findParameterSpec("chord.voice.1.enabled");
    if (layerAEnabled == nullptr
        || layerBEnabled == nullptr
        || layerAOscillatorVoices == nullptr
        || layerBOscillatorInvert == nullptr
        || arpEnabled == nullptr
        || arpStepCount == nullptr
        || arpStepPitch == nullptr
        || chordVoiceCount == nullptr
        || chordVoiceOneEnabled == nullptr
        || std::abs(layerAEnabled->defaultValue - 1.0f) > 0.0001f
        || std::abs(layerBEnabled->defaultValue) > 0.0001f
        || std::abs(layerAOscillatorVoices->minimum) > 0.0001f
        || std::abs(layerAOscillatorVoices->maximum - 8.0f) > 0.0001f
        || std::abs(layerAOscillatorVoices->defaultValue - 1.0f) > 0.0001f
        || std::abs(layerBOscillatorInvert->defaultValue) > 0.0001f
        || std::abs(arpEnabled->defaultValue) > 0.0001f
        || std::abs(arpStepCount->defaultValue - 16.0f) > 0.0001f
        || std::abs(arpStepPitch->minimum + 24.0f) > 0.0001f
        || std::abs(arpStepPitch->maximum - 24.0f) > 0.0001f
        || std::abs(chordVoiceCount->defaultValue - 1.0f) > 0.0001f
        || std::abs(chordVoiceOneEnabled->defaultValue - 1.0f) > 0.0001f
        || arpEnabled->auVersionHint < 2
        || arpStepPitch->auVersionHint < 2
        || chordVoiceCount->auVersionHint < 2
        || chordVoiceOneEnabled->auVersionHint < 2)
    {
        std::cerr << "Layer/oscillator/arp/chord registry defaults, ranges, or AU version hints mismatch.\n";
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
