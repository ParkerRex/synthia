#include "../dsp/SynthEngine.h"
#include "../plugin/ParameterRegistry.h"
#include "../presets/PresetValidator.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
struct Options
{
    bool smoke = false;
    bool listParameters = false;
    bool validatePresets = false;
    bool voiceTest = false;
    std::filesystem::path output = "build/reports/smoke.json";
    std::filesystem::path presetPath = "presets/factory";
};

Options parseOptions(int argc, char* argv[])
{
    Options options;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        if (arg == "--smoke")
        {
            options.smoke = true;
        }
        else if (arg == "--list-parameters")
        {
            options.listParameters = true;
            options.output = "build/reports/parameters.json";
        }
        else if (arg == "--validate-presets" && i + 1 < argc)
        {
            options.validatePresets = true;
            options.presetPath = argv[++i];
            options.output = "build/reports/presets.json";
        }
        else if (arg == "--voice-test")
        {
            options.voiceTest = true;
            options.output = "build/reports/voice-core.json";
        }
        else if (arg == "--output" && i + 1 < argc)
        {
            options.output = argv[++i];
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout << "Usage:\n";
            std::cout << "  SynthRender --smoke --output <path>\n";
            std::cout << "  SynthRender --list-parameters --output <path>\n";
            std::cout << "  SynthRender --validate-presets <dir> --output <path>\n";
            std::cout << "  SynthRender --voice-test --output <path>\n";
            std::exit(0);
        }
    }

    return options;
}

void writeSmokeReport(const std::filesystem::path& path, const synth::RenderStats& stats)
{
    if (path.parent_path() != std::filesystem::path{})
        std::filesystem::create_directories(path.parent_path());

    std::ofstream out(path);
    out << "{\n";
    out << "  \"suite\": \"smoke\",\n";
    out << "  \"samples_rendered\": " << stats.samplesRendered << ",\n";
    out << "  \"invalid_samples\": " << stats.invalidSamples << ",\n";
    out << "  \"active_voices\": " << stats.activeVoices << ",\n";
    out << "  \"peak\": " << stats.peak << ",\n";
    out << "  \"passed\": " << (stats.invalidSamples == 0 ? "true" : "false") << "\n";
    out << "}\n";
}

void ensureParentDirectory(const std::filesystem::path& path)
{
    if (path.parent_path() != std::filesystem::path{})
        std::filesystem::create_directories(path.parent_path());
}

void writeParameterReport(const std::filesystem::path& path)
{
    ensureParentDirectory(path);

    const auto errors = synth::validateParameterSpecs();
    const auto& specs = synth::getParameterSpecs();

    std::ofstream out(path);
    out << "{\n";
    out << "  \"parameter_count\": " << specs.size() << ",\n";
    out << "  \"error_count\": " << errors.size() << ",\n";
    out << "  \"passed\": " << (errors.empty() ? "true" : "false") << ",\n";
    out << "  \"parameters\": [\n";

    for (std::size_t i = 0; i < specs.size(); ++i)
    {
        const auto& spec = specs[i];
        out << "    {";
        out << "\"id\": \"" << spec.id << "\", ";
        out << "\"name\": \"" << spec.name << "\", ";
        out << "\"group\": \"" << spec.group << "\", ";
        out << "\"kind\": \"" << synth::toString(spec.kind) << "\", ";
        out << "\"unit\": \"" << spec.unit << "\", ";
        out << "\"default\": " << spec.defaultValue;
        out << "}";
        out << (i + 1 == specs.size() ? "\n" : ",\n");
    }

    out << "  ],\n";
    out << "  \"errors\": [\n";
    for (std::size_t i = 0; i < errors.size(); ++i)
        out << "    \"" << errors[i] << "\"" << (i + 1 == errors.size() ? "\n" : ",\n");
    out << "  ]\n";
    out << "}\n";
}

int writePresetReport(const std::filesystem::path& path, const std::filesystem::path& presetPath)
{
    ensureParentDirectory(path);

    const auto results = synth::validatePresetDirectory(presetPath);
    int errorCount = 0;
    int warningCount = 0;

    for (const auto& result : results)
    {
        errorCount += static_cast<int>(result.errors.size());
        warningCount += static_cast<int>(result.warnings.size());
    }

    std::ofstream out(path);
    out << "{\n";
    out << "  \"preset_path\": \"" << presetPath.string() << "\",\n";
    out << "  \"preset_count\": " << results.size() << ",\n";
    out << "  \"error_count\": " << errorCount << ",\n";
    out << "  \"warning_count\": " << warningCount << ",\n";
    out << "  \"passed\": " << (errorCount == 0 ? "true" : "false") << ",\n";
    out << "  \"presets\": [\n";

    for (std::size_t i = 0; i < results.size(); ++i)
    {
        const auto& result = results[i];
        out << "    {";
        out << "\"path\": \"" << result.path.string() << "\", ";
        out << "\"passed\": " << (result.passed() ? "true" : "false") << ", ";
        out << "\"error_count\": " << result.errors.size() << ", ";
        out << "\"warning_count\": " << result.warnings.size();
        out << "}";
        out << (i + 1 == results.size() ? "\n" : ",\n");
    }

    out << "  ]\n";
    out << "}\n";

    return errorCount == 0 ? 0 : 1;
}

int writeVoiceReport(const std::filesystem::path& path)
{
    ensureParentDirectory(path);

    synth::SynthEngine engine;
    engine.prepare(48000.0, 128);
    engine.noteOn(60, 1.0f);

    std::vector<float> left(128);
    std::vector<float> right(128);
    auto stats = engine.process(left.data(), right.data(), static_cast<int>(left.size()));
    const auto activeAfterOn = stats.activeVoices;

    engine.noteOff(60);
    for (int i = 0; i < 200; ++i)
        stats = engine.process(left.data(), right.data(), static_cast<int>(left.size()));

    const auto activeAfterRelease = stats.activeVoices;
    const auto passed = activeAfterOn == 1 && activeAfterRelease == 0 && stats.invalidSamples == 0;

    std::ofstream out(path);
    out << "{\n";
    out << "  \"suite\": \"voice-core\",\n";
    out << "  \"active_after_note_on\": " << activeAfterOn << ",\n";
    out << "  \"active_after_release\": " << activeAfterRelease << ",\n";
    out << "  \"invalid_samples\": " << stats.invalidSamples << ",\n";
    out << "  \"passed\": " << (passed ? "true" : "false") << "\n";
    out << "}\n";

    return passed ? 0 : 1;
}
} // namespace

int main(int argc, char* argv[])
{
    const auto options = parseOptions(argc, argv);

    if (options.listParameters)
    {
        writeParameterReport(options.output);

        const auto errors = synth::validateParameterSpecs();
        if (!errors.empty())
        {
            std::cerr << "Parameter registry validation failed.\n";
            return 1;
        }

        std::cout << "Parameter report wrote " << synth::getParameterSpecs().size() << " parameters.\n";
        return 0;
    }

    if (options.validatePresets)
    {
        const auto exitCode = writePresetReport(options.output, options.presetPath);
        std::cout << "Preset validation " << (exitCode == 0 ? "passed." : "failed.") << "\n";
        return exitCode;
    }

    if (options.voiceTest)
    {
        const auto exitCode = writeVoiceReport(options.output);
        std::cout << "Voice core validation " << (exitCode == 0 ? "passed." : "failed.") << "\n";
        return exitCode;
    }

    if (!options.smoke)
    {
        std::cerr << "No render mode selected. Use --smoke, --list-parameters, --validate-presets, or --voice-test.\n";
        return 2;
    }

    synth::SynthEngine engine;
    engine.prepare(48000.0, 256);

    std::vector<float> left(4800);
    std::vector<float> right(4800);
    const auto stats = engine.process(left.data(), right.data(), static_cast<int>(left.size()));

    writeSmokeReport(options.output, stats);

    if (stats.invalidSamples != 0)
    {
        std::cerr << "Smoke render failed: invalid samples detected.\n";
        return 1;
    }

    std::cout << "Smoke render passed: " << stats.samplesRendered << " samples.\n";
    return 0;
}
