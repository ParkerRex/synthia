#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace synth
{
struct MidiControllerAssignment
{
    int controllerNumber = -1;
    std::string parameterId;
};

std::filesystem::path defaultMidiControllerMapFile();

std::vector<MidiControllerAssignment> normalizeMidiControllerAssignments(
    std::vector<MidiControllerAssignment> assignments);

std::vector<MidiControllerAssignment> readMidiControllerAssignments(const std::filesystem::path& path);

bool writeMidiControllerAssignments(const std::filesystem::path& path,
                                    const std::vector<MidiControllerAssignment>& assignments,
                                    std::string& error);
} // namespace synth
