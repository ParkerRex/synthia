#include "../../src/plugin/ParameterRegistry.h"
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
        : parameters(*this, nullptr, "SYNTH_STATE", synth::createParameterLayout())
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

bool checkStateRoundTrip()
{
    StateRoundTripProcessor source;
    auto state = source.parameters.copyState();
    state.setProperty("schema_version", 1, nullptr);
    state.setProperty("plugin_version", "0.1.0-test", nullptr);

    const auto xml = state.createXml();
    if (xml == nullptr)
        return false;

    const auto restoredState = juce::ValueTree::fromXml(*xml);
    if (!restoredState.isValid() || !restoredState.hasType("SYNTH_STATE"))
        return false;

    StateRoundTripProcessor destination;
    destination.parameters.replaceState(restoredState);

    return destination.parameters.state.hasProperty("schema_version")
        && destination.parameters.getRawParameterValue("filter.cutoff_semitones") != nullptr;
}

bool checkPresetManagerLoadAndSave()
{
    StateRoundTripProcessor processor;
    const auto load = synth::loadPresetIntoState(processor.parameters, "presets/factory/pluck-core-01.json");
    if (!load.loaded)
    {
        std::cerr << load.message << "\n";
        return false;
    }

    const auto* cutoff = processor.parameters.getRawParameterValue("filter.cutoff_semitones");
    if (cutoff == nullptr || std::abs(cutoff->load() - 58.0f) > 0.001f)
    {
        std::cerr << "Preset manager did not apply filter cutoff.\n";
        return false;
    }

    const auto tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("synth-preset-manager-test.json");
    tempFile.deleteFile();

    std::string error;
    if (!synth::writeCurrentPreset(processor.parameters, tempFile.getFullPathName().toStdString(),
                                   "Preset Manager Test", error))
    {
        std::cerr << error << "\n";
        return false;
    }

    const auto validation = synth::validatePresetFile(tempFile.getFullPathName().toStdString());
    tempFile.deleteFile();
    if (!validation.passed())
    {
        std::cerr << "Preset manager saved invalid preset.\n";
        for (const auto& validationError : validation.errors)
            std::cerr << "  " << validationError << "\n";
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
    if (specs.size() < 80)
    {
        std::cerr << "Expected broad v1 parameter inventory, got " << specs.size() << "\n";
        return 1;
    }

    if (synth::findParameterSpec("filter.cutoff_semitones") == nullptr
        || synth::findParameterSpec("transmod.8.depth") == nullptr
        || synth::findParameterSpec("macro.space") == nullptr)
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

    if (!checkStateRoundTrip())
    {
        std::cerr << "APVTS state round-trip failed.\n";
        return 1;
    }

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
