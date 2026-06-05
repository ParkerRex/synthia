#include "PresetManager.h"

#include "PresetValidator.h"
#include "../dsp/SynthParameters.h"
#include "../plugin/ParameterRegistry.h"

#include <juce_core/juce_core.h>

#include <algorithm>
#include <cctype>
#include <cmath>

#ifndef SYNTH_PROJECT_VERSION
#define SYNTH_PROJECT_VERSION "0.1.0"
#endif

namespace synth
{
namespace
{
bool isNumber(const juce::var& value)
{
    return value.isDouble() || value.isInt() || value.isInt64();
}

int choiceIndex(const ParameterSpec& spec, const juce::var& value)
{
    if (value.isString())
    {
        const auto selected = value.toString().toStdString();
        const auto found = std::find(spec.choices.begin(), spec.choices.end(), selected);
        if (found != spec.choices.end())
            return static_cast<int>(std::distance(spec.choices.begin(), found));
    }

    if (isNumber(value))
        return static_cast<int>(std::round(static_cast<double>(value)));

    return spec.defaultChoice;
}

float physicalValueFromPresetValue(const ParameterSpec& spec, const juce::var& value)
{
    if (spec.kind == ParameterKind::Bool)
        return value.isBool() && static_cast<bool>(value) ? 1.0f : 0.0f;

    if (spec.kind == ParameterKind::Choice)
        return static_cast<float>(std::clamp(choiceIndex(spec, value), 0,
                                             static_cast<int>(spec.choices.size()) - 1));

    if (isNumber(value))
    {
        const auto numeric = static_cast<float>(static_cast<double>(value));
        if (std::isfinite(numeric))
            return std::clamp(numeric, spec.minimum, spec.maximum);
    }

    return spec.defaultValue;
}

juce::var valueForPreset(const ParameterSpec& spec, const juce::RangedAudioParameter& parameter)
{
    const auto physical = parameter.convertFrom0to1(parameter.getValue());
    if (spec.kind == ParameterKind::Bool)
        return physical >= 0.5f;

    if (spec.kind == ParameterKind::Choice)
    {
        const auto index = std::clamp(static_cast<int>(std::round(physical)), 0,
                                      static_cast<int>(spec.choices.size()) - 1);
        return juce::String(spec.choices[static_cast<std::size_t>(index)]);
    }

    return physical;
}

bool setParameterValue(juce::AudioProcessorValueTreeState& parameters,
                       const ParameterSpec& spec,
                       float physicalValue)
{
    auto* parameter = parameters.getParameter(juce::String(spec.id));
    if (parameter == nullptr)
        return false;

    const auto normalized = std::clamp(parameter->convertTo0to1(physicalValue), 0.0f, 1.0f);
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(normalized);
    parameter->endChangeGesture();
    return true;
}

void resetStateToDefaults(juce::AudioProcessorValueTreeState& parameters)
{
    for (const auto& spec : getParameterSpecs())
        setParameterValue(parameters, spec, spec.kind == ParameterKind::Choice
            ? static_cast<float>(spec.defaultChoice)
            : spec.defaultValue);
}

void applyPresetParameter(juce::AudioProcessorValueTreeState& parameters,
                          const std::string& id,
                          const juce::var& value)
{
    const auto* spec = findParameterSpec(id);
    if (spec == nullptr)
        return;

    setParameterValue(parameters, *spec, physicalValueFromPresetValue(*spec, value));
}

void applyModSlotDepth(juce::AudioProcessorValueTreeState& parameters,
                       int slotNumber,
                       const std::string& targetId,
                       float value,
                       const juce::String& depthDomain)
{
    const auto normalized = depthDomain == "Normalized";
    auto parameterId = std::string {};
    auto physicalValue = value;

    if (targetId == "osc.pitch_semitones")
    {
        parameterId = "transmod." + std::to_string(slotNumber) + ".osc_pitch_semitones";
        physicalValue = normalized ? std::clamp(value, -1.0f, 1.0f) * 48.0f : value;
    }
    else if (targetId == "osc.pulse_width")
    {
        parameterId = "transmod." + std::to_string(slotNumber) + ".pulse_width";
        physicalValue = std::clamp(value, -1.0f, 1.0f);
    }
    else if (targetId == "filter.cutoff_semitones")
    {
        parameterId = "transmod." + std::to_string(slotNumber) + ".filter_cutoff_semitones";
        physicalValue = normalized ? std::clamp(value, -1.0f, 1.0f) * 72.0f : value;
    }
    else if (targetId == "amp.level_db")
    {
        parameterId = "transmod." + std::to_string(slotNumber) + ".amp_level_db";
        physicalValue = normalized ? std::clamp(value, -1.0f, 1.0f) * 24.0f : value;
    }
    else if (targetId == "amp.pan")
    {
        parameterId = "transmod." + std::to_string(slotNumber) + ".pan";
        physicalValue = std::clamp(value, -1.0f, 1.0f);
    }

    if (const auto* spec = findParameterSpec(parameterId))
        setParameterValue(parameters, *spec, physicalValue);
}

void applyModSlotObject(juce::AudioProcessorValueTreeState& parameters, const juce::var& slotVar)
{
    if (!slotVar.isObject())
        return;

    const auto* slotObject = slotVar.getDynamicObject();
    if (slotObject == nullptr)
        return;

    const auto slotId = slotObject->getProperty(juce::Identifier("slot_id"));
    if (!slotId.isInt())
        return;

    const auto slotNumber = static_cast<int>(slotId);
    if (slotNumber < 1 || slotNumber > transModSlotCount)
        return;

    const auto prefix = "transmod." + std::to_string(slotNumber) + ".";
    applyPresetParameter(parameters, prefix + "enabled", slotObject->getProperty(juce::Identifier("enabled")));
    applyPresetParameter(parameters, prefix + "source", slotObject->getProperty(juce::Identifier("source")));

    const auto scaler = slotObject->getProperty(juce::Identifier("scaler"));
    applyPresetParameter(parameters, prefix + "scaler", scaler.isVoid() ? juce::var("None") : scaler);

    const auto depthDomain = slotObject->getProperty(juce::Identifier("depth_domain")).toString();
    const auto depths = slotObject->getProperty(juce::Identifier("depths"));
    if (!depths.isObject())
        return;

    if (const auto* depthObject = depths.getDynamicObject())
    {
        for (const auto& property : depthObject->getProperties())
        {
            if (!isNumber(property.value))
                continue;

            applyModSlotDepth(parameters, slotNumber, property.name.toString().toStdString(),
                              static_cast<float>(static_cast<double>(property.value)),
                              depthDomain);
        }
    }
}

std::unique_ptr<juce::DynamicObject> readPresetObject(const std::filesystem::path& path)
{
    const auto file = juce::File(juce::String(path.string()));
    const auto parsed = juce::JSON::parse(file);
    if (!parsed.isObject())
        return {};

    const auto* source = parsed.getDynamicObject();
    if (source == nullptr)
        return {};

    return std::unique_ptr<juce::DynamicObject>(source->clone());
}

std::string propertyString(const juce::DynamicObject& object, const char* propertyName)
{
    const auto value = object.getProperty(juce::Identifier(propertyName));
    return value.isString() ? value.toString().toStdString() : std::string {};
}

juce::var currentParameterObject(const juce::AudioProcessorValueTreeState& parameters)
{
    auto object = std::make_unique<juce::DynamicObject>();
    for (const auto& spec : getParameterSpecs())
    {
        if (!spec.presetSerialized)
            continue;

        if (const auto* parameter = parameters.getParameter(juce::String(spec.id)))
            object->setProperty(juce::Identifier(spec.id), valueForPreset(spec, *parameter));
    }

    return juce::var(object.release());
}

juce::String choiceValue(const juce::AudioProcessorValueTreeState& parameters, const std::string& id)
{
    const auto* spec = findParameterSpec(id);
    const auto* parameter = parameters.getParameter(juce::String(id));
    if (spec == nullptr || parameter == nullptr || spec->choices.empty())
        return "None";

    const auto index = std::clamp(static_cast<int>(std::round(parameter->convertFrom0to1(parameter->getValue()))),
                                  0, static_cast<int>(spec->choices.size()) - 1);
    return juce::String(spec->choices[static_cast<std::size_t>(index)]);
}

float floatValue(const juce::AudioProcessorValueTreeState& parameters, const std::string& id)
{
    if (const auto* parameter = parameters.getParameter(juce::String(id)))
        return parameter->convertFrom0to1(parameter->getValue());

    return 0.0f;
}

bool boolValue(const juce::AudioProcessorValueTreeState& parameters, const std::string& id)
{
    return floatValue(parameters, id) >= 0.5f;
}

juce::var currentModSlotArray(const juce::AudioProcessorValueTreeState& parameters)
{
    auto slots = juce::Array<juce::var> {};

    for (int slot = 1; slot <= transModSlotCount; ++slot)
    {
        const auto prefix = "transmod." + std::to_string(slot) + ".";
        auto slotObject = std::make_unique<juce::DynamicObject>();
        slotObject->setProperty("slot_id", slot);
        slotObject->setProperty("enabled", boolValue(parameters, prefix + "enabled"));
        slotObject->setProperty("source", choiceValue(parameters, prefix + "source"));
        slotObject->setProperty("scaler", choiceValue(parameters, prefix + "scaler"));
        slotObject->setProperty("depth_domain", "Physical");

        auto depths = std::make_unique<juce::DynamicObject>();
        depths->setProperty("osc.pitch_semitones", floatValue(parameters, prefix + "osc_pitch_semitones"));
        depths->setProperty("osc.pulse_width", floatValue(parameters, prefix + "pulse_width"));
        depths->setProperty("filter.cutoff_semitones", floatValue(parameters, prefix + "filter_cutoff_semitones"));
        depths->setProperty("amp.level_db", floatValue(parameters, prefix + "amp_level_db"));
        depths->setProperty("amp.pan", floatValue(parameters, prefix + "pan"));
        slotObject->setProperty("depths", juce::var(depths.release()));

        slots.add(juce::var(slotObject.release()));
    }

    return slots;
}

juce::var currentMacroArray(const juce::AudioProcessorValueTreeState& parameters)
{
    struct Macro
    {
        const char* id;
        const char* displayName;
        const char* parameterId;
    };

    constexpr Macro macros[] = {
        { "motion", "Motion", "macro.motion" },
        { "width", "Width", "macro.width" },
        { "drive", "Drive", "macro.drive" },
        { "space", "Space", "macro.space" },
    };

    auto values = juce::Array<juce::var> {};
    for (const auto& macro : macros)
    {
        auto object = std::make_unique<juce::DynamicObject>();
        object->setProperty("id", macro.id);
        object->setProperty("display_name", macro.displayName);
        object->setProperty("value", floatValue(parameters, macro.parameterId));
        object->setProperty("assignments", juce::Array<juce::var> {});
        values.add(juce::var(object.release()));
    }

    return values;
}
} // namespace

std::vector<PresetSummary> scanPresetDirectory(const std::filesystem::path& directory, bool factory)
{
    std::vector<PresetSummary> presets;
    if (!std::filesystem::exists(directory))
        return presets;

    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
            continue;

        const auto validation = validatePresetFile(entry.path());
        if (!validation.passed())
            continue;

        auto object = readPresetObject(entry.path());
        if (object == nullptr)
            continue;

        PresetSummary summary;
        summary.path = entry.path();
        summary.id = propertyString(*object, "id");
        summary.displayName = propertyString(*object, "display_name");
        summary.author = propertyString(*object, "author");
        summary.description = propertyString(*object, "description");
        summary.factory = factory;
        presets.push_back(summary);
    }

    std::sort(presets.begin(), presets.end(), [](const auto& a, const auto& b) {
        if (a.factory != b.factory)
            return a.factory && !b.factory;
        return a.displayName < b.displayName;
    });

    return presets;
}

std::filesystem::path defaultUserPresetDirectory()
{
    const auto music = juce::File::getSpecialLocation(juce::File::userMusicDirectory);
    return std::filesystem::path { music.getFullPathName().toStdString() } / "ParkerX" / "Synth" / "Presets";
}

std::string presetIdFromDisplayName(const std::string& displayName)
{
    std::string id;
    auto pendingDash = false;

    for (const auto character : displayName)
    {
        const auto lower = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        if ((lower >= 'a' && lower <= 'z') || (lower >= '0' && lower <= '9'))
        {
            if (pendingDash && !id.empty())
                id.push_back('-');
            id.push_back(lower);
            pendingDash = false;
        }
        else
        {
            pendingDash = true;
        }
    }

    return id.empty() ? "user-preset" : id;
}

PresetLoadResult loadPresetIntoState(juce::AudioProcessorValueTreeState& parameters,
                                     const std::filesystem::path& path)
{
    PresetLoadResult result;
    const auto validation = validatePresetFile(path);
    if (!validation.passed())
    {
        result.message = "Preset load failed";
        if (!validation.errors.empty())
            result.message += ": " + validation.errors.front();
        return result;
    }

    auto object = readPresetObject(path);
    if (object == nullptr)
    {
        result.message = "Preset load failed: preset root object unavailable";
        return result;
    }

    resetStateToDefaults(parameters);

    const auto parameterVar = object->getProperty(juce::Identifier("parameters"));
    if (parameterVar.isObject())
    {
        if (const auto* parameterObject = parameterVar.getDynamicObject())
        {
            for (const auto& property : parameterObject->getProperties())
                applyPresetParameter(parameters, property.name.toString().toStdString(), property.value);
        }
    }

    const auto modSlots = object->getProperty(juce::Identifier("mod_slots"));
    if (modSlots.isArray())
    {
        if (const auto* slots = modSlots.getArray())
        {
            for (const auto& slot : *slots)
                applyModSlotObject(parameters, slot);
        }
    }

    result.loaded = true;
    result.displayName = propertyString(*object, "display_name");
    result.message = "Loaded preset: " + result.displayName;
    parameters.state.setProperty("schema_version", 1, nullptr);
    parameters.state.setProperty("plugin_version", SYNTH_PROJECT_VERSION, nullptr);
    parameters.state.setProperty("current_preset", juce::String(result.displayName), nullptr);
    return result;
}

bool writeCurrentPreset(const juce::AudioProcessorValueTreeState& parameters,
                        const std::filesystem::path& path,
                        const std::string& displayName,
                        std::string& error)
{
    const auto safeName = displayName.empty() ? std::string("User Preset") : displayName;
    std::error_code createError;
    std::filesystem::create_directories(path.parent_path(), createError);
    if (createError)
    {
        error = "could not create preset directory: " + createError.message();
        return false;
    }

    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("schema_version", 1);
    root->setProperty("plugin_min_version", SYNTH_PROJECT_VERSION);
    root->setProperty("id", juce::String(presetIdFromDisplayName(safeName)));
    root->setProperty("display_name", juce::String(safeName));
    root->setProperty("author", "User");
    root->setProperty("description", "User preset saved from the Synth editor.");
    root->setProperty("tags", juce::Array<juce::var> { juce::var("user") });
    root->setProperty("parameters", currentParameterObject(parameters));
    root->setProperty("mod_slots", currentModSlotArray(parameters));
    root->setProperty("macros", currentMacroArray(parameters));

    auto metadata = std::make_unique<juce::DynamicObject>();
    metadata->setProperty("clean_room", true);
    root->setProperty("metadata", juce::var(metadata.release()));

    const auto file = juce::File(juce::String(path.string()));
    if (!file.replaceWithText(juce::JSON::toString(juce::var(root.release()), true), false, false, "\n"))
    {
        error = "could not write preset file: " + path.string();
        return false;
    }

    const auto validation = validatePresetFile(path);
    if (!validation.passed())
    {
        error = "saved preset did not validate";
        if (!validation.errors.empty())
            error += ": " + validation.errors.front();
        return false;
    }

    return true;
}
} // namespace synth
