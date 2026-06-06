#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <string>
#include <vector>

namespace synth
{
enum class ParameterKind
{
    Float,
    Bool,
    Choice
};

struct ParameterSpec
{
    std::string id;
    std::string name;
    std::string group;
    ParameterKind kind = ParameterKind::Float;
    std::string unit;
    float minimum = 0.0f;
    float maximum = 1.0f;
    float defaultValue = 0.0f;
    float interval = 0.0f;
    float skew = 1.0f;
    bool automatable = true;
    bool presetSerialized = true;
    float smoothMs = 0.0f;
    int auVersionHint = 1;
    std::vector<std::string> choices;
    int defaultChoice = 0;
};

const std::vector<ParameterSpec>& getParameterSpecs();
const ParameterSpec* findParameterSpec(const std::string& id);
std::vector<std::string> validateParameterSpecs();

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

const char* toString(ParameterKind kind) noexcept;
} // namespace synth
