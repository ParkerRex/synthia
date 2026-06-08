#include "MidiControllerMap.h"

#include <juce_core/juce_core.h>

#include <algorithm>
#include <memory>
#include <system_error>

namespace synth
{
namespace
{
juce::File juceFileForPath(const std::filesystem::path& path)
{
    if (path.is_absolute())
        return juce::File { juce::String(path.lexically_normal().string()) };

    std::error_code error;
    auto absolutePath = std::filesystem::current_path(error);
    if (!error)
        absolutePath /= path;
    else
        absolutePath = path;

    return juce::File { juce::String(absolutePath.lexically_normal().string()) };
}

bool validControllerNumber(int controllerNumber) noexcept
{
    return controllerNumber >= 0 && controllerNumber <= 127;
}
} // namespace

std::filesystem::path defaultMidiControllerMapFile()
{
    const auto music = juce::File::getSpecialLocation(juce::File::userMusicDirectory);
    return std::filesystem::path { music.getFullPathName().toStdString() }
           / "ParkerX" / "sylenth-ai" / "MidiControllerMap.json";
}

std::vector<MidiControllerAssignment> normalizeMidiControllerAssignments(
    std::vector<MidiControllerAssignment> assignments)
{
    assignments.erase(std::remove_if(assignments.begin(), assignments.end(), [](const auto& assignment) {
        return !validControllerNumber(assignment.controllerNumber) || assignment.parameterId.empty();
    }), assignments.end());

    std::vector<MidiControllerAssignment> normalized;
    normalized.reserve(assignments.size());
    for (const auto& assignment : assignments)
    {
        normalized.erase(std::remove_if(normalized.begin(), normalized.end(), [&assignment](const auto& existing) {
            return existing.controllerNumber == assignment.controllerNumber
                   || existing.parameterId == assignment.parameterId;
        }), normalized.end());
        normalized.push_back(assignment);
    }

    std::sort(normalized.begin(), normalized.end(), [](const auto& a, const auto& b) {
        return a.controllerNumber < b.controllerNumber;
    });
    return normalized;
}

std::vector<MidiControllerAssignment> readMidiControllerAssignments(const std::filesystem::path& path)
{
    const auto file = juceFileForPath(path);
    const auto parsed = juce::JSON::parse(file);
    if (!parsed.isObject())
        return {};

    const auto* root = parsed.getDynamicObject();
    if (root == nullptr)
        return {};

    const auto mappings = root->getProperty("mappings");
    if (!mappings.isArray())
        return {};

    std::vector<MidiControllerAssignment> assignments;
    if (const auto* mappingArray = mappings.getArray())
    {
        assignments.reserve(static_cast<std::size_t>(mappingArray->size()));
        for (const auto& mapping : *mappingArray)
        {
            if (!mapping.isObject())
                continue;

            const auto* object = mapping.getDynamicObject();
            if (object == nullptr)
                continue;

            const auto controller = object->getProperty("cc");
            const auto parameter = object->getProperty("parameter_id");
            if (!controller.isInt() || !parameter.isString())
                continue;

            assignments.push_back({
                static_cast<int>(controller),
                parameter.toString().toStdString()
            });
        }
    }

    return normalizeMidiControllerAssignments(std::move(assignments));
}

bool writeMidiControllerAssignments(const std::filesystem::path& path,
                                    const std::vector<MidiControllerAssignment>& assignments,
                                    std::string& error)
{
    std::error_code createError;
    std::filesystem::create_directories(path.parent_path(), createError);
    if (createError)
    {
        error = "could not create MIDI map directory: " + createError.message();
        return false;
    }

    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("schema_version", 1);

    juce::Array<juce::var> mappingArray;
    for (const auto& assignment : normalizeMidiControllerAssignments(assignments))
    {
        auto mapping = std::make_unique<juce::DynamicObject>();
        mapping->setProperty("cc", assignment.controllerNumber);
        mapping->setProperty("parameter_id", juce::String(assignment.parameterId));
        mappingArray.add(juce::var(mapping.release()));
    }
    root->setProperty("mappings", mappingArray);

    const auto file = juceFileForPath(path);
    if (!file.replaceWithText(juce::JSON::toString(juce::var(root.release()), true)))
    {
        error = "could not write MIDI map: " + path.string();
        return false;
    }

    error.clear();
    return true;
}
} // namespace synth
