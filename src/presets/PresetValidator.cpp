#include "PresetValidator.h"

#include "../plugin/ParameterRegistry.h"

#include <juce_core/juce_core.h>

#include <algorithm>

namespace synth
{
namespace
{
bool hasProperty(const juce::DynamicObject& object, const char* name)
{
    return object.hasProperty(juce::Identifier(name));
}

const juce::var getProperty(const juce::DynamicObject& object, const char* name)
{
    return object.getProperty(juce::Identifier(name));
}

bool isNumber(const juce::var& value)
{
    return value.isDouble() || value.isInt() || value.isInt64();
}

void validateParameterValue(PresetValidationResult& result, const ParameterSpec& spec, const juce::var& value)
{
    if (spec.kind == ParameterKind::Float)
    {
        if (!isNumber(value))
        {
            result.errors.push_back(spec.id + " must be numeric");
            return;
        }

        const auto numeric = static_cast<float>(static_cast<double>(value));
        if (numeric < spec.minimum || numeric > spec.maximum)
            result.errors.push_back(spec.id + " is outside allowed range");
    }
    else if (spec.kind == ParameterKind::Bool)
    {
        if (!value.isBool())
            result.errors.push_back(spec.id + " must be boolean");
    }
    else
    {
        if (!value.isString())
        {
            result.errors.push_back(spec.id + " must be a choice string");
            return;
        }

        const auto selected = value.toString().toStdString();
        if (std::find(spec.choices.begin(), spec.choices.end(), selected) == spec.choices.end())
            result.errors.push_back(spec.id + " has unknown choice '" + selected + "'");
    }
}
} // namespace

PresetValidationResult validatePresetFile(const std::filesystem::path& path)
{
    PresetValidationResult result;
    result.path = path;

    const juce::File file { juce::String(path.string()) };
    if (!file.existsAsFile())
    {
        result.errors.push_back("preset file missing");
        return result;
    }

    const auto parsed = juce::JSON::parse(file);
    if (!parsed.isObject())
    {
        result.errors.push_back("preset root must be a JSON object");
        return result;
    }

    const auto* object = parsed.getDynamicObject();
    if (object == nullptr)
    {
        result.errors.push_back("preset root object unavailable");
        return result;
    }

    const char* required[] = {
        "schema_version", "plugin_min_version", "id", "display_name",
        "author", "description", "tags", "parameters", "mod_slots", "macros"
    };

    for (const auto* field : required)
    {
        if (!hasProperty(*object, field))
            result.errors.push_back(std::string("missing required field: ") + field);
    }

    if (hasProperty(*object, "schema_version"))
    {
        const auto schema = getProperty(*object, "schema_version");
        if (!schema.isInt() || static_cast<int>(schema) < 1)
            result.errors.push_back("schema_version must be an integer >= 1");
    }

    if (hasProperty(*object, "tags") && !getProperty(*object, "tags").isArray())
        result.errors.push_back("tags must be an array");

    if (hasProperty(*object, "mod_slots") && !getProperty(*object, "mod_slots").isArray())
        result.errors.push_back("mod_slots must be an array");

    if (hasProperty(*object, "macros") && !getProperty(*object, "macros").isArray())
        result.errors.push_back("macros must be an array");

    if (hasProperty(*object, "parameters"))
    {
        const auto parameters = getProperty(*object, "parameters");
        if (!parameters.isObject())
        {
            result.errors.push_back("parameters must be an object");
        }
        else if (const auto* parameterObject = parameters.getDynamicObject())
        {
            for (const auto& property : parameterObject->getProperties())
            {
                const auto id = property.name.toString().toStdString();
                const auto* spec = findParameterSpec(id);

                if (spec == nullptr)
                {
                    result.errors.push_back("unknown parameter id: " + id);
                    continue;
                }

                validateParameterValue(result, *spec, property.value);
            }
        }
    }

    return result;
}

std::vector<PresetValidationResult> validatePresetDirectory(const std::filesystem::path& directory)
{
    std::vector<PresetValidationResult> results;

    if (!std::filesystem::exists(directory))
    {
        PresetValidationResult result;
        result.path = directory;
        result.errors.push_back("preset directory missing");
        results.push_back(result);
        return results;
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
            results.push_back(validatePresetFile(entry.path()));
    }

    if (results.empty())
    {
        PresetValidationResult result;
        result.path = directory;
        result.errors.push_back("preset directory contains no JSON presets");
        results.push_back(result);
    }

    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
        return a.path.string() < b.path.string();
    });

    return results;
}
} // namespace synth

