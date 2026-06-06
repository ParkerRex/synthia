#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <filesystem>
#include <string>
#include <vector>

namespace synth
{
struct PresetSummary
{
    std::filesystem::path path;
    std::string id;
    std::string displayName;
    std::string author;
    std::string description;
    bool factory = false;
};

struct PresetLoadResult
{
    bool loaded = false;
    std::string message;
    std::string displayName;
    juce::ValueTree state;
};

std::vector<PresetSummary> scanPresetDirectory(const std::filesystem::path& directory, bool factory);
std::filesystem::path factoryPresetDirectory();
std::filesystem::path defaultUserPresetDirectory();
std::filesystem::path legacyUserPresetDirectory();
std::string presetIdFromDisplayName(const std::string& displayName);

PresetLoadResult preparePresetState(juce::AudioProcessorValueTreeState& parameters,
                                    const std::filesystem::path& path);

juce::ValueTree mergeParameterStateWithDefaults(juce::AudioProcessorValueTreeState& parameters,
                                                const juce::ValueTree& overrideState);

PresetLoadResult loadPresetIntoState(juce::AudioProcessorValueTreeState& parameters,
                                     const std::filesystem::path& path);

bool writeCurrentPreset(const juce::AudioProcessorValueTreeState& parameters,
                        const std::filesystem::path& path,
                        const std::string& displayName,
                        std::string& error);
} // namespace synth
