#include "../dsp/Filter.h"
#include "../dsp/OscillatorStack.h"
#include "../dsp/SynthEngine.h"
#include "../plugin/ParameterRegistry.h"
#include "../presets/PresetValidator.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
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
    bool oscTest = false;
    bool filterTest = false;
    bool modulationTest = false;
    bool presetRender = false;
    bool dry = false;
    bool wet = false;
    std::string suite;
    std::filesystem::path output = "build/reports/smoke.json";
    std::filesystem::path report = "build/reports/render.json";
    std::filesystem::path outputDir = "build/reports/core";
    std::filesystem::path presetPath = "presets/factory";
    std::filesystem::path fixturePath = "fixtures/midi/overlap-pluck.mid";
    std::string notes = "C1,C3,C5,C7";
};

struct RenderEvent
{
    int sample = 0;
    juce::MidiMessage message;
};

struct SuiteItem
{
    std::string name;
    std::filesystem::path reportPath;
    bool passed = false;
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
        else if (arg == "--osc-test")
        {
            options.oscTest = true;
            options.output = "build/reports/oscillator.json";
        }
        else if (arg == "--filter-test")
        {
            options.filterTest = true;
            options.output = "build/reports/filter.json";
        }
        else if (arg == "--modulation-test")
        {
            options.modulationTest = true;
            options.output = "build/reports/modulation.json";
        }
        else if (arg == "--suite" && i + 1 < argc)
        {
            options.suite = argv[++i];
        }
        else if (arg == "--output-dir" && i + 1 < argc)
        {
            options.outputDir = argv[++i];
        }
        else if (arg == "--preset" && i + 1 < argc)
        {
            options.presetRender = true;
            options.presetPath = argv[++i];
        }
        else if (arg == "--fixture" && i + 1 < argc)
        {
            options.fixturePath = argv[++i];
        }
        else if (arg == "--dry")
        {
            options.dry = true;
        }
        else if (arg == "--wet")
        {
            options.wet = true;
        }
        else if (arg == "--notes" && i + 1 < argc)
        {
            options.notes = argv[++i];
        }
        else if (arg == "--output" && i + 1 < argc)
        {
            options.output = argv[++i];
        }
        else if (arg == "--report" && i + 1 < argc)
        {
            options.report = argv[++i];
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout << "Usage:\n";
            std::cout << "  SynthRender --smoke --output <path>\n";
            std::cout << "  SynthRender --list-parameters --output <path>\n";
            std::cout << "  SynthRender --validate-presets <dir> --output <path>\n";
            std::cout << "  SynthRender --voice-test --output <path>\n";
            std::cout << "  SynthRender --osc-test --notes C1,C3,C5,C7 --output <path>\n";
            std::cout << "  SynthRender --filter-test --output <path>\n";
            std::cout << "  SynthRender --modulation-test --fixture <path> --output <path>\n";
            std::cout << "  SynthRender --suite core --output-dir <dir>\n";
            std::cout << "  SynthRender --preset <json> --fixture <path> --dry --output <wav> --report <json>\n";
            std::cout << "  SynthRender --preset <json> --fixture <path> --wet --output <wav> --report <json>\n";
            std::exit(0);
        }
    }

    return options;
}

void ensureParentDirectory(const std::filesystem::path& path)
{
    if (path.parent_path() != std::filesystem::path{})
        std::filesystem::create_directories(path.parent_path());
}

void ensureDirectory(const std::filesystem::path& path)
{
    if (path != std::filesystem::path{})
        std::filesystem::create_directories(path);
}

const char* boolString(bool value) noexcept
{
    return value ? "true" : "false";
}

std::string genericString(const std::filesystem::path& path)
{
    return path.generic_string();
}

void writeSmokeReport(const std::filesystem::path& path, const synth::RenderStats& stats)
{
    ensureParentDirectory(path);

    std::ofstream out(path);
    out << "{\n";
    out << "  \"schema_version\": 1,\n";
    out << "  \"suite\": \"smoke\",\n";
    out << "  \"samples_rendered\": " << stats.samplesRendered << ",\n";
    out << "  \"invalid_samples\": " << stats.invalidSamples << ",\n";
    out << "  \"active_voices\": " << stats.activeVoices << ",\n";
    out << "  \"peak\": " << stats.peak << ",\n";
    out << "  \"passed\": " << (stats.invalidSamples == 0 ? "true" : "false") << "\n";
    out << "}\n";
}

void writeParameterReport(const std::filesystem::path& path)
{
    ensureParentDirectory(path);

    const auto errors = synth::validateParameterSpecs();
    const auto& specs = synth::getParameterSpecs();

    std::ofstream out(path);
    out << "{\n";
    out << "  \"schema_version\": 1,\n";
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
    out << "  \"schema_version\": 1,\n";
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
    out << "  \"schema_version\": 1,\n";
    out << "  \"suite\": \"voice-core\",\n";
    out << "  \"active_after_note_on\": " << activeAfterOn << ",\n";
    out << "  \"active_after_release\": " << activeAfterRelease << ",\n";
    out << "  \"peak_after_note_on\": " << stats.peak << ",\n";
    out << "  \"invalid_samples\": " << stats.invalidSamples << ",\n";
    out << "  \"passed\": " << (passed ? "true" : "false") << "\n";
    out << "}\n";

    return passed ? 0 : 1;
}

float estimateFrequency(const std::vector<float>& samples, double sampleRate)
{
    int crossings = 0;
    auto first = -1;
    auto last = -1;

    for (int i = 1; i < static_cast<int>(samples.size()); ++i)
    {
        if (samples[static_cast<std::size_t>(i - 1)] <= 0.0f && samples[static_cast<std::size_t>(i)] > 0.0f)
        {
            if (first < 0)
                first = i;
            last = i;
            ++crossings;
        }
    }

    if (crossings < 2 || first < 0 || last <= first)
        return 0.0f;

    return static_cast<float>((static_cast<double>(crossings - 1) * sampleRate) / static_cast<double>(last - first));
}

float centsBetween(float actual, float expected)
{
    if (actual <= 0.0f || expected <= 0.0f)
        return 9999.0f;

    return 1200.0f * std::log2(actual / expected);
}

double highBandEnergyRatio(const std::vector<float>& samples)
{
    constexpr auto maxBins = 512;
    double total = 0.0;
    double high = 0.0;
    const auto sampleCount = static_cast<int>(samples.size());

    for (int bin = 1; bin <= maxBins; ++bin)
    {
        double real = 0.0;
        double imag = 0.0;
        const auto radiansPerSample = -2.0 * juce::MathConstants<double>::pi * static_cast<double>(bin)
            / static_cast<double>(sampleCount);
        for (int i = 0; i < sampleCount; ++i)
        {
            const auto angle = radiansPerSample * static_cast<double>(i);
            real += static_cast<double>(samples[static_cast<std::size_t>(i)]) * std::cos(angle);
            imag += static_cast<double>(samples[static_cast<std::size_t>(i)]) * std::sin(angle);
        }

        const auto energy = real * real + imag * imag;
        total += energy;
        if (bin > static_cast<int>(maxBins * 0.85))
            high += energy;
    }

    return total > 0.0 ? high / total : 0.0;
}

int noteNameToMidi(std::string note)
{
    if (note.empty())
        return 60;

    note.erase(std::remove_if(note.begin(), note.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    }), note.end());

    const auto letter = static_cast<char>(std::toupper(static_cast<unsigned char>(note[0])));
    auto semitone = 0;
    switch (letter)
    {
        case 'C': semitone = 0; break;
        case 'D': semitone = 2; break;
        case 'E': semitone = 4; break;
        case 'F': semitone = 5; break;
        case 'G': semitone = 7; break;
        case 'A': semitone = 9; break;
        case 'B': semitone = 11; break;
        default: return 60;
    }

    auto index = std::size_t { 1 };
    if (index < note.size() && (note[index] == '#' || note[index] == 'b'))
    {
        semitone += note[index] == '#' ? 1 : -1;
        ++index;
    }

    const auto octave = index < note.size() ? std::stoi(note.substr(index)) : 4;
    return (octave + 1) * 12 + semitone;
}

std::vector<int> parseNotes(const std::string& notes)
{
    std::vector<int> result;
    auto start = std::size_t { 0 };

    while (start < notes.size())
    {
        const auto comma = notes.find(',', start);
        const auto token = notes.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
        result.push_back(noteNameToMidi(token));
        if (comma == std::string::npos)
            break;
        start = comma + 1;
    }

    return result.empty() ? std::vector<int> { 60 } : result;
}

int writeOscillatorReport(const std::filesystem::path& path, const std::string& noteList)
{
    ensureParentDirectory(path);

    constexpr auto sampleRate = 48000.0;
    constexpr auto sampleCount = 48000;
    auto passed = true;
    const auto notes = parseNotes(noteList);

    std::ofstream out(path);
    out << "{\n";
    out << "  \"schema_version\": 1,\n";
    out << "  \"suite\": \"oscillator\",\n";
    out << "  \"sample_rate\": " << sampleRate << ",\n";
    out << "  \"alias_high_band_ratios\": [\n";
    for (std::size_t noteIndex = 0; noteIndex < notes.size(); ++noteIndex)
    {
        synth::OscillatorStack oscillator;
        oscillator.prepare(sampleRate);
        oscillator.reset(0.0f);

        synth::SynthParameters parameters;
        parameters.osc.sawLevel = 1.0f;
        parameters.osc.pulseLevel = 0.0f;
        parameters.osc.subLevel = 0.0f;
        parameters.osc.noiseLevel = 0.0f;
        parameters.amp.analog = 0.0f;

        std::vector<float> aliasSamples(4096);
        for (auto& sample : aliasSamples)
            sample = oscillator.renderSample(static_cast<float>(notes[noteIndex]), parameters, 0.0f, 0.0f);

        out << "    {\"midi\": " << notes[noteIndex]
            << ", \"high_band_ratio\": " << highBandEnergyRatio(aliasSamples) << "}";
        out << (noteIndex + 1 == notes.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"notes\": [\n";

    for (std::size_t noteIndex = 0; noteIndex < notes.size(); ++noteIndex)
    {
        synth::OscillatorStack oscillator;
        oscillator.prepare(sampleRate);
        oscillator.reset(0.0f);

        synth::SynthParameters parameters;
        parameters.osc.sawLevel = 1.0f;
        parameters.osc.pulseLevel = 0.0f;
        parameters.osc.subLevel = 0.0f;
        parameters.osc.noiseLevel = 0.0f;
        parameters.amp.analog = 0.0f;

        std::vector<float> samples(sampleCount);
        for (auto& sample : samples)
            sample = oscillator.renderSample(static_cast<float>(notes[noteIndex]), parameters, 0.0f, 0.0f);

        const auto frequency = estimateFrequency(samples, sampleRate);
        const auto expected = synth::midiNoteToHz(static_cast<float>(notes[noteIndex]));
        const auto cents = centsBetween(frequency, expected);
        const auto notePassed = std::abs(cents) < 5.0f;
        passed = passed && notePassed;

        out << "    {";
        out << "\"midi\": " << notes[noteIndex] << ", ";
        out << "\"expected_hz\": " << expected << ", ";
        out << "\"measured_hz\": " << frequency << ", ";
        out << "\"cents_error\": " << cents << ", ";
        out << "\"passed\": " << (notePassed ? "true" : "false");
        out << "}";
        out << (noteIndex + 1 == notes.size() ? "\n" : ",\n");
    }

    out << "  ],\n";
    out << "  \"pulse_widths\": [\n";
    const float pulseWidths[] = { 0.10f, 0.25f, 0.50f, 0.75f, 0.90f };
    auto pulsePassed = true;
    for (int widthIndex = 0; widthIndex < 5; ++widthIndex)
    {
        synth::OscillatorStack pulse;
        pulse.prepare(sampleRate);
        pulse.reset(0.0f);
        synth::SynthParameters pulseParameters;
        pulseParameters.osc.sawLevel = 0.0f;
        pulseParameters.osc.pulseLevel = 1.0f;
        pulseParameters.osc.pulseWidth = pulseWidths[widthIndex];
        int positive = 0;
        for (int i = 0; i < sampleCount; ++i)
        {
            if (pulse.renderSample(48, pulseParameters, 0.0f, 0.0f) > 0.0f)
                ++positive;
        }

        const auto duty = static_cast<float>(positive) / static_cast<float>(sampleCount);
        const auto widthPassed = std::abs(duty - pulseWidths[widthIndex]) < 0.04f;
        pulsePassed = pulsePassed && widthPassed;
        out << "    {\"target\": " << pulseWidths[widthIndex]
            << ", \"positive_ratio\": " << duty
            << ", \"passed\": " << (widthPassed ? "true" : "false") << "}";
        out << (widthIndex == 4 ? "\n" : ",\n");
    }
    passed = passed && pulsePassed;
    out << "  ],\n";
    out << "  \"sub_octaves\": [\n";
    auto subPassed = true;
    for (int octave = 1; octave <= 3; ++octave)
    {
        synth::OscillatorStack oscillator;
        oscillator.prepare(sampleRate);
        oscillator.reset(0.0f);
        synth::SynthParameters parameters;
        parameters.osc.sawLevel = 0.0f;
        parameters.osc.pulseLevel = 0.0f;
        parameters.osc.subLevel = 1.0f;
        parameters.osc.subWave = synth::SubWave::Saw;
        parameters.osc.subOctave = octave;
        std::vector<float> samples(sampleCount);
        for (auto& sample : samples)
            sample = oscillator.renderSample(60, parameters, 0.0f, 0.0f);

        const auto frequency = estimateFrequency(samples, sampleRate);
        const auto expected = synth::midiNoteToHz(static_cast<float>(60 - octave * 12));
        const auto cents = centsBetween(frequency, expected);
        const auto octavePassed = std::abs(cents) < 5.0f;
        subPassed = subPassed && octavePassed;
        out << "    {\"octave\": -" << octave
            << ", \"measured_hz\": " << frequency
            << ", \"cents_error\": " << cents
            << ", \"passed\": " << (octavePassed ? "true" : "false") << "}";
        out << (octave == 3 ? "\n" : ",\n");
    }
    passed = passed && subPassed;
    out << "  ],\n";

    auto detunePassed = true;
    for (int count = 1; count <= 5; ++count)
    {
        for (int index = 0; index < count; ++index)
        {
            const auto opposite = count - 1 - index;
            const auto sum = synth::OscillatorStack::detuneOffsetCents(index, count, 0.7f)
                + synth::OscillatorStack::detuneOffsetCents(opposite, count, 0.7f);
            detunePassed = detunePassed && std::abs(sum) < 0.0001f;
        }
    }

    synth::OscillatorStack noiseA;
    synth::OscillatorStack noiseB;
    noiseA.prepare(sampleRate);
    noiseB.prepare(sampleRate);
    noiseA.reset(0.25f);
    noiseB.reset(0.25f);
    synth::SynthParameters noiseParameters;
    noiseParameters.osc.sawLevel = 0.0f;
    noiseParameters.osc.pulseLevel = 0.0f;
    noiseParameters.osc.subLevel = 0.0f;
    noiseParameters.osc.noiseLevel = 1.0f;
    auto noisePassed = true;
    for (int i = 0; i < 512; ++i)
        noisePassed = noisePassed && std::abs(noiseA.renderSample(60, noiseParameters, 0.0f, 0.0f)
            - noiseB.renderSample(60, noiseParameters, 0.0f, 0.0f)) < 1.0e-7f;

    synth::OscillatorStack sync;
    sync.prepare(sampleRate);
    sync.reset(0.0f);
    synth::SynthParameters syncParameters;
    syncParameters.osc.syncAmount = 1.0f;
    auto syncPassed = true;
    for (int i = 0; i < 4096; ++i)
    {
        const auto sample = sync.renderSample(96, syncParameters, 0.0f, 0.0f);
        syncPassed = syncPassed && std::isfinite(sample) && std::abs(sample) <= 1.21f;
    }

    passed = passed && detunePassed && noisePassed && syncPassed;
    out << "  \"stack_detune_symmetry_passed\": " << (detunePassed ? "true" : "false") << ",\n";
    out << "  \"noise_determinism_passed\": " << (noisePassed ? "true" : "false") << ",\n";
    out << "  \"hard_sync_stability_passed\": " << (syncPassed ? "true" : "false") << ",\n";
    out << "  \"pulse_width_passed\": " << (pulsePassed ? "true" : "false") << ",\n";
    out << "  \"sub_octaves_passed\": " << (subPassed ? "true" : "false") << ",\n";
    out << "  \"passed\": " << (passed ? "true" : "false") << "\n";
    out << "}\n";

    return passed ? 0 : 1;
}

float renderFilterEnergy(synth::FilterMode mode, float drive, float resonance, int oversampling)
{
    synth::Filter filter;
    filter.prepare(48000.0);

    synth::SynthParameters parameters;
    parameters.filter.mode = mode;
    parameters.filter.cutoffSemitones = 72.0f;
    parameters.filter.resonance = resonance;
    parameters.filter.drive = drive;
    parameters.filter.oversampling = oversampling;

    auto energy = 0.0f;
    for (int i = 0; i < 24000; ++i)
    {
        const auto input = i == 0 ? 1.0f : 0.0f;
        const auto output = filter.process(input, 60, parameters, 0.0f);
        if (!std::isfinite(output))
            return -1.0f;
        energy += output * output;
    }

    return energy;
}

int writeFilterReport(const std::filesystem::path& path)
{
    ensureParentDirectory(path);

    const auto cutoffHz = synth::Filter::cutoffSemitonesToHz(69.0f);
    const auto mappingPassed = std::abs(cutoffHz - 440.0f) < 0.1f;
    auto stabilityPassed = true;

    std::ofstream out(path);
    out << "{\n";
    out << "  \"schema_version\": 1,\n";
    out << "  \"suite\": \"filter\",\n";
    out << "  \"cutoff_69_semitones_hz\": " << cutoffHz << ",\n";
    out << "  \"cutoff_mapping_passed\": " << (mappingPassed ? "true" : "false") << ",\n";
    out << "  \"mode_energies\": [\n";

    const synth::FilterMode modes[] = {
        synth::FilterMode::L2, synth::FilterMode::L4, synth::FilterMode::B2,
        synth::FilterMode::B4, synth::FilterMode::H2, synth::FilterMode::H4,
        synth::FilterMode::Peak2, synth::FilterMode::Notch2, synth::FilterMode::Notch4
    };

    for (std::size_t i = 0; i < 9; ++i)
    {
        const auto energy = renderFilterEnergy(modes[i], 0.35f, 0.45f, 1);
        stabilityPassed = stabilityPassed && energy >= 0.0f && energy < 20000.0f;
        out << "    {\"mode\": " << i << ", \"energy\": " << energy << "}";
        out << (i + 1 == 9 ? "\n" : ",\n");
    }

    out << "  ],\n";

    const auto lowDrive = renderFilterEnergy(synth::FilterMode::L4, 0.0f, 0.75f, 1);
    const auto highDrive = renderFilterEnergy(synth::FilterMode::L4, 0.85f, 0.75f, 2);
    const auto drivePassed = lowDrive >= 0.0f && highDrive >= 0.0f && std::abs(highDrive - lowDrive) > 0.00001f;
    const auto passed = mappingPassed && stabilityPassed && drivePassed;

    out << "  \"low_drive_energy\": " << lowDrive << ",\n";
    out << "  \"high_drive_energy\": " << highDrive << ",\n";
    out << "  \"drive_changes_response\": " << (drivePassed ? "true" : "false") << ",\n";
    out << "  \"stability_passed\": " << (stabilityPassed ? "true" : "false") << ",\n";
    out << "  \"passed\": " << (passed ? "true" : "false") << "\n";
    out << "}\n";

    return passed ? 0 : 1;
}

int choiceIndex(const synth::ParameterSpec& spec, const juce::var& value)
{
    if (value.isString())
    {
        const auto selected = value.toString().toStdString();
        const auto found = std::find(spec.choices.begin(), spec.choices.end(), selected);
        if (found != spec.choices.end())
            return static_cast<int>(std::distance(spec.choices.begin(), found));
    }

    if (value.isInt() || value.isInt64() || value.isDouble())
        return static_cast<int>(std::round(static_cast<double>(value)));

    return spec.defaultChoice;
}

float finiteOr(float value, float fallback) noexcept
{
    return std::isfinite(value) ? value : fallback;
}

synth::ModSource modSourceFromVar(const juce::var& value)
{
    if (const auto* spec = synth::findParameterSpec("transmod.1.source"))
        return static_cast<synth::ModSource>(choiceIndex(*spec, value));

    return synth::ModSource::None;
}

void applyModSlotDepth(synth::TransModSlotParameters& slot, const std::string& targetId,
                       float value, const juce::String& depthDomain)
{
    value = finiteOr(value, 0.0f);
    const auto normalized = depthDomain == "Normalized";

    if (targetId == "osc.pitch_semitones")
        slot.oscPitchSemitones = normalized ? std::clamp(value, -1.0f, 1.0f) * 48.0f : value;
    else if (targetId == "osc.pulse_width")
        slot.pulseWidth = std::clamp(value, -1.0f, 1.0f);
    else if (targetId == "filter.cutoff_semitones")
    {
        if (normalized)
            slot.depth = std::clamp(value, -1.0f, 1.0f);
        else
            slot.filterCutoffSemitones = value;
    }
    else if (targetId == "amp.level_db")
        slot.ampLevelDb = normalized ? std::clamp(value, -1.0f, 1.0f) * 24.0f : value;
    else if (targetId == "amp.pan")
        slot.pan = std::clamp(value, -1.0f, 1.0f);
}

void applyModSlotObject(synth::SynthParameters& parameters, const juce::var& slotVar)
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
    if (slotNumber < 1 || slotNumber > synth::transModSlotCount)
        return;

    auto& slot = parameters.transMod.slots[static_cast<std::size_t>(slotNumber - 1)];
    slot = {};
    const auto enabled = slotObject->getProperty(juce::Identifier("enabled"));
    slot.enabled = enabled.isBool() && static_cast<bool>(enabled);
    slot.source = modSourceFromVar(slotObject->getProperty(juce::Identifier("source")));
    const auto scaler = slotObject->getProperty(juce::Identifier("scaler"));
    slot.scaler = scaler.isVoid() ? synth::ModSource::None : modSourceFromVar(scaler);

    const auto depthDomain = slotObject->getProperty(juce::Identifier("depth_domain")).toString();
    const auto depths = slotObject->getProperty(juce::Identifier("depths"));
    if (!depths.isObject())
        return;

    if (const auto* depthObject = depths.getDynamicObject())
    {
        for (const auto& property : depthObject->getProperties())
        {
            if (property.value.isInt() || property.value.isInt64() || property.value.isDouble())
            {
                applyModSlotDepth(slot, property.name.toString().toStdString(),
                                  static_cast<float>(static_cast<double>(property.value)),
                                  depthDomain);
            }
        }
    }
}

void applyPresetValue(synth::SynthParameters& parameters, const std::string& id, const juce::var& value)
{
    const auto* spec = synth::findParameterSpec(id);
    if (spec == nullptr)
        return;

    const auto numeric = value.isBool() ? (static_cast<bool>(value) ? 1.0f : 0.0f)
        : (value.isInt() || value.isInt64() || value.isDouble()
            ? finiteOr(static_cast<float>(static_cast<double>(value)), spec->defaultValue)
            : spec->defaultValue);
    const auto choice = spec->kind == synth::ParameterKind::Choice ? choiceIndex(*spec, value) : 0;

    if (id == "voice.mode") parameters.voiceMode = static_cast<synth::VoiceMode>(choice);
    else if (id == "voice.polyphony") parameters.polyphony = static_cast<int>(std::round(numeric));
    else if (id == "voice.unison_count") parameters.unisonCount = static_cast<int>(std::round(numeric));
    else if (id == "voice.retrigger") parameters.retrigger = numeric >= 0.5f;
    else if (id == "voice.glide_ms") parameters.glideMs = numeric;
    else if (id == "voice.velocity_glide_ms") parameters.velocityGlideMs = numeric;
    else if (id == "osc.pitch_semitones") parameters.osc.pitchSemitones = numeric;
    else if (id == "osc.fine_cents") parameters.osc.fineCents = numeric;
    else if (id == "osc.stack_count") parameters.osc.stackCount = static_cast<int>(std::round(numeric));
    else if (id == "osc.stack_detune") parameters.osc.stackDetune = numeric;
    else if (id == "osc.saw_level") parameters.osc.sawLevel = numeric;
    else if (id == "osc.pulse_level") parameters.osc.pulseLevel = numeric;
    else if (id == "osc.noise_level") parameters.osc.noiseLevel = numeric;
    else if (id == "osc.pulse_width") parameters.osc.pulseWidth = numeric;
    else if (id == "osc.sub_wave") parameters.osc.subWave = static_cast<synth::SubWave>(choice);
    else if (id == "osc.sub_octave") parameters.osc.subOctave = choice + 1;
    else if (id == "osc.sub_level") parameters.osc.subLevel = numeric;
    else if (id == "osc.sub_pulse_width") parameters.osc.subPulseWidth = numeric;
    else if (id == "osc.sync_amount") parameters.osc.syncAmount = numeric;
    else if (id == "filter.enabled") parameters.filter.enabled = numeric >= 0.5f;
    else if (id == "filter.mode") parameters.filter.mode = static_cast<synth::FilterMode>(choice);
    else if (id == "filter.cutoff_semitones") parameters.filter.cutoffSemitones = numeric;
    else if (id == "filter.resonance") parameters.filter.resonance = numeric;
    else if (id == "filter.drive") parameters.filter.drive = numeric;
    else if (id == "filter.keytrack") parameters.filter.keytrack = numeric;
    else if (id == "filter.oversampling") parameters.filter.oversampling = choice;
    else if (id == "amp.drive") parameters.amp.drive = numeric;
    else if (id == "amp.level_db") parameters.amp.levelDb = numeric;
    else if (id == "amp.pan") parameters.amp.pan = numeric;
    else if (id == "amp.pan_spread") parameters.amp.panSpread = numeric;
    else if (id == "amp.unison_spread") parameters.amp.unisonSpread = numeric;
    else if (id == "amp.analog") parameters.amp.analog = numeric;
    else if (id == "amp_env.attack_ms") parameters.ampEnv.attackMs = numeric;
    else if (id == "amp_env.decay_ms") parameters.ampEnv.decayMs = numeric;
    else if (id == "amp_env.sustain") parameters.ampEnv.sustain = numeric;
    else if (id == "amp_env.release_ms") parameters.ampEnv.releaseMs = numeric;
    else if (id == "mod_env.attack_ms") parameters.modEnv.attackMs = numeric;
    else if (id == "mod_env.decay_ms") parameters.modEnv.decayMs = numeric;
    else if (id == "mod_env.sustain") parameters.modEnv.sustain = numeric;
    else if (id == "mod_env.release_ms") parameters.modEnv.releaseMs = numeric;
    else if (id == "lfo.shape") parameters.lfo.shape = static_cast<synth::LfoShapeChoice>(choice);
    else if (id == "lfo.rate_mode") parameters.lfo.rateMode = static_cast<synth::LfoRateMode>(choice);
    else if (id == "lfo.rate_hz") parameters.lfo.rateHz = numeric;
    else if (id == "lfo.sync_division") parameters.lfo.syncDivision = choice;
    else if (id == "lfo.phase_degrees") parameters.lfo.phaseDegrees = numeric;
    else if (id == "lfo.gate_mode") parameters.lfo.gateMode = static_cast<synth::LfoGateMode>(choice);
    else if (id == "lfo.mono") parameters.lfo.mono = numeric >= 0.5f;
    else if (id == "lfo.swing") parameters.lfo.swing = numeric;
    else if (id == "direct.filter_keytrack") parameters.direct.filterKeytrack = numeric;
    else if (id == "direct.filter_lfo_semitones") parameters.direct.filterLfoSemitones = numeric;
    else if (id == "direct.filter_mod_env_semitones") parameters.direct.filterModEnvSemitones = numeric;
    else if (id == "direct.osc_keytrack_semitones") parameters.direct.oscKeytrackSemitones = numeric;
    else if (id == "direct.osc_lfo_semitones") parameters.direct.oscLfoSemitones = numeric;
    else if (id == "direct.osc_mod_env_semitones") parameters.direct.oscModEnvSemitones = numeric;
    else if (id == "direct.pulse_keytrack") parameters.direct.pulseKeytrack = numeric;
    else if (id == "direct.pulse_lfo") parameters.direct.pulseLfo = numeric;
    else if (id == "direct.pulse_mod_env") parameters.direct.pulseModEnv = numeric;
    else if (id == "fx.enabled") parameters.fx.enabled = numeric >= 0.5f;
    else if (id == "fx.saturation_enabled") parameters.fx.saturationEnabled = numeric >= 0.5f;
    else if (id == "fx.saturation_mix") parameters.fx.saturationMix = numeric;
    else if (id == "fx.saturation_drive") parameters.fx.saturationDrive = numeric;
    else if (id == "fx.delay_enabled") parameters.fx.delayEnabled = numeric >= 0.5f;
    else if (id == "fx.delay_mix") parameters.fx.delayMix = numeric;
    else if (id == "fx.delay_sync_division") parameters.fx.delaySyncDivision = static_cast<synth::DelaySyncDivision>(choice);
    else if (id == "fx.delay_feedback") parameters.fx.delayFeedback = numeric;
    else if (id == "fx.reverb_enabled") parameters.fx.reverbEnabled = numeric >= 0.5f;
    else if (id == "fx.reverb_mix") parameters.fx.reverbMix = numeric;
    else if (id == "fx.reverb_decay") parameters.fx.reverbDecay = numeric;
    else if (id == "fx.chorus_enabled") parameters.fx.chorusEnabled = numeric >= 0.5f;
    else if (id == "fx.chorus_mix") parameters.fx.chorusMix = numeric;
    else if (id == "fx.chorus_rate_hz") parameters.fx.chorusRateHz = numeric;
    else if (id == "fx.chorus_depth_ms") parameters.fx.chorusDepthMs = numeric;
    else if (id == "quality.realtime_mode") parameters.quality.realtimeMode = static_cast<synth::QualityMode>(choice);
    else if (id == "quality.offline_mode") parameters.quality.offlineMode = static_cast<synth::QualityMode>(choice);
    else if (id == "ramp.enabled") parameters.ramp.enabled = numeric >= 0.5f;
    else if (id == "ramp.mode") parameters.ramp.mode = static_cast<synth::RampMode>(choice);
    else if (id == "ramp.delay_ms") parameters.ramp.delayMs = numeric;
    else if (id == "ramp.rise_ms") parameters.ramp.riseMs = numeric;
    else if (id == "ramp.curve") parameters.ramp.curve = static_cast<synth::RampCurve>(choice);
    else if (id == "macro.motion") parameters.macro.motion = numeric;
    else if (id == "macro.width") parameters.macro.width = numeric;
    else if (id == "macro.drive") parameters.macro.drive = numeric;
    else if (id == "macro.space") parameters.macro.space = numeric;
    else if (id.starts_with("transmod."))
    {
        const auto slotStart = std::string("transmod.").size();
        const auto slotEnd = id.find('.', slotStart);
        if (slotEnd == std::string::npos)
            return;

        const auto slotNumber = std::stoi(id.substr(slotStart, slotEnd - slotStart));
        if (slotNumber < 1 || slotNumber > synth::transModSlotCount)
            return;

        auto& slot = parameters.transMod.slots[static_cast<std::size_t>(slotNumber - 1)];
        const auto field = id.substr(slotEnd + 1);
        if (field == "enabled") slot.enabled = numeric >= 0.5f;
        else if (field == "source") slot.source = static_cast<synth::ModSource>(choice);
        else if (field == "scaler") slot.scaler = static_cast<synth::ModSource>(choice);
        else if (field == "depth") slot.depth = numeric;
        else if (field == "osc_pitch_semitones") slot.oscPitchSemitones = numeric;
        else if (field == "pulse_width") slot.pulseWidth = numeric;
        else if (field == "filter_cutoff_semitones") slot.filterCutoffSemitones = numeric;
        else if (field == "amp_level_db") slot.ampLevelDb = numeric;
        else if (field == "pan") slot.pan = numeric;
    }
}

bool loadPresetParameters(const std::filesystem::path& path, synth::SynthParameters& parameters, std::string& error)
{
    if (!std::filesystem::is_regular_file(path))
    {
        error = "preset file missing: " + path.string();
        return false;
    }

    const juce::File file { juce::String(path.string()) };
    const auto parsed = juce::JSON::parse(file);

    if (!parsed.isObject())
    {
        error = "preset root must be a JSON object: " + path.string();
        return false;
    }

    const auto* root = parsed.getDynamicObject();
    if (root == nullptr)
    {
        error = "preset root object unavailable: " + path.string();
        return false;
    }

    const auto validation = synth::validatePresetFile(path);
    if (!validation.passed())
    {
        error = "preset validation failed: " + path.string();
        if (!validation.errors.empty())
            error += ": " + validation.errors.front();
        return false;
    }

    const auto parameterVar = root->getProperty(juce::Identifier("parameters"));
    if (!parameterVar.isObject())
    {
        error = "preset parameters must be an object: " + path.string();
        return false;
    }

    const auto* parameterObject = parameterVar.getDynamicObject();
    if (parameterObject == nullptr)
    {
        error = "preset parameter object unavailable: " + path.string();
        return false;
    }

    for (const auto& property : parameterObject->getProperties())
        applyPresetValue(parameters, property.name.toString().toStdString(), property.value);

    const auto modSlots = root->getProperty(juce::Identifier("mod_slots"));
    if (modSlots.isArray())
    {
        if (const auto* slots = modSlots.getArray())
        {
            for (const auto& slot : *slots)
                applyModSlotObject(parameters, slot);
        }
    }

    return true;
}

bool loadMidiFixture(const std::filesystem::path& path, int sampleRate,
                     std::vector<RenderEvent>& events, std::string& error)
{
    if (!std::filesystem::is_regular_file(path))
    {
        error = "fixture file missing: " + path.string();
        return false;
    }

    juce::FileInputStream stream { juce::File { juce::String(path.string()) } };
    if (!stream.openedOk())
    {
        error = "fixture file could not be opened: " + path.string();
        return false;
    }

    juce::MidiFile midiFile;
    if (!midiFile.readFrom(stream))
    {
        error = "fixture is not a valid MIDI file: " + path.string();
        return false;
    }

    midiFile.convertTimestampTicksToSeconds();
    events.clear();

    for (int trackIndex = 0; trackIndex < midiFile.getNumTracks(); ++trackIndex)
    {
        const auto* track = midiFile.getTrack(trackIndex);
        if (track == nullptr)
            continue;

        for (int eventIndex = 0; eventIndex < track->getNumEvents(); ++eventIndex)
        {
            const auto* event = track->getEventPointer(eventIndex);
            if (event == nullptr)
                continue;

            auto message = event->message;
            if (!message.isNoteOnOrOff()
                && !message.isController()
                && !message.isPitchWheel()
                && !message.isAftertouch()
                && !message.isChannelPressure()
                && !message.isAllNotesOff()
                && !message.isAllSoundOff())
            {
                continue;
            }

            const auto sample = static_cast<int>(std::max(0.0, std::round(message.getTimeStamp() * sampleRate)));
            events.push_back({ sample, message });
        }
    }

    std::stable_sort(events.begin(), events.end(), [](const auto& a, const auto& b) {
        return a.sample < b.sample;
    });

    const auto hasNote = std::any_of(events.begin(), events.end(), [](const auto& event) {
        return event.message.isNoteOnOrOff();
    });
    if (!hasNote)
    {
        error = "fixture contains no note events: " + path.string();
        return false;
    }

    return true;
}

void handleRenderMidi(synth::SynthEngine& engine, const juce::MidiMessage& message)
{
    if (message.isNoteOn(false))
        engine.noteOn(message.getNoteNumber(), message.getFloatVelocity());
    else if (message.isNoteOff(true))
        engine.noteOff(message.getNoteNumber());
    else if (message.isAllSoundOff())
        engine.panic();
    else if (message.isAllNotesOff())
        engine.allNotesOff();
    else if (message.isPitchWheel())
        engine.setPitchBend((static_cast<float>(message.getPitchWheelValue()) - 8192.0f) / 8192.0f);
    else if (message.isControllerOfType(64))
        engine.setSustainPedal(message.getControllerValue() >= 64);
    else if (message.isControllerOfType(1))
        engine.setModWheel(static_cast<float>(message.getControllerValue()) / 127.0f);
    else if (message.isAftertouch())
        engine.setAftertouch(static_cast<float>(message.getAfterTouchValue()) / 127.0f);
    else if (message.isChannelPressure())
        engine.setAftertouch(static_cast<float>(message.getChannelPressureValue()) / 127.0f);
}

void writeLittleEndian16(std::ofstream& out, std::uint16_t value)
{
    out.put(static_cast<char>(value & 0xffu));
    out.put(static_cast<char>((value >> 8u) & 0xffu));
}

void writeLittleEndian32(std::ofstream& out, std::uint32_t value)
{
    out.put(static_cast<char>(value & 0xffu));
    out.put(static_cast<char>((value >> 8u) & 0xffu));
    out.put(static_cast<char>((value >> 16u) & 0xffu));
    out.put(static_cast<char>((value >> 24u) & 0xffu));
}

void writeWav16(const std::filesystem::path& path, const std::vector<float>& left,
                const std::vector<float>& right, int sampleRate)
{
    ensureParentDirectory(path);

    const auto sampleCount = std::min(left.size(), right.size());
    const auto dataBytes = static_cast<std::uint32_t>(sampleCount * 2u * sizeof(std::int16_t));

    std::ofstream out(path, std::ios::binary);
    out.write("RIFF", 4);
    writeLittleEndian32(out, 36u + dataBytes);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    writeLittleEndian32(out, 16u);
    writeLittleEndian16(out, 1u);
    writeLittleEndian16(out, 2u);
    const auto byteRate = static_cast<std::uint32_t>(sampleRate) * 2u * static_cast<std::uint32_t>(sizeof(std::int16_t));
    writeLittleEndian32(out, static_cast<std::uint32_t>(sampleRate));
    writeLittleEndian32(out, byteRate);
    writeLittleEndian16(out, static_cast<std::uint16_t>(2 * sizeof(std::int16_t)));
    writeLittleEndian16(out, 16u);
    out.write("data", 4);
    writeLittleEndian32(out, dataBytes);

    for (std::size_t i = 0; i < sampleCount; ++i)
    {
        const auto l = static_cast<std::int16_t>(std::clamp(left[i], -1.0f, 1.0f) * 32767.0f);
        const auto r = static_cast<std::int16_t>(std::clamp(right[i], -1.0f, 1.0f) * 32767.0f);
        writeLittleEndian16(out, static_cast<std::uint16_t>(l));
        writeLittleEndian16(out, static_cast<std::uint16_t>(r));
    }
}

struct AudioMetrics
{
    float peak = 0.0f;
    double rms = 0.0;
    double rmsDbfs = -240.0;
    double crestDb = 0.0;
    double correlation = 0.0;
    double dcOffset = 0.0;
    double spectralCentroidHz = 0.0;
    int invalid = 0;
    int nonzero = 0;
};

double spectralCentroidHz(const std::vector<float>& left, const std::vector<float>& right, int sampleRate)
{
    const auto sampleCount = static_cast<int>(std::min<std::size_t>({ left.size(), right.size(), 4096u }));
    if (sampleCount < 32)
        return 0.0;

    constexpr auto maxBins = 256;
    double weighted = 0.0;
    double magnitudeSum = 0.0;

    for (int bin = 1; bin <= maxBins; ++bin)
    {
        double real = 0.0;
        double imag = 0.0;
        const auto radiansPerSample = -2.0 * juce::MathConstants<double>::pi * static_cast<double>(bin)
            / static_cast<double>(sampleCount);

        for (int i = 0; i < sampleCount; ++i)
        {
            const auto mono = 0.5 * (static_cast<double>(left[static_cast<std::size_t>(i)])
                + static_cast<double>(right[static_cast<std::size_t>(i)]));
            const auto window = 0.5 - 0.5 * std::cos(2.0 * juce::MathConstants<double>::pi
                * static_cast<double>(i) / static_cast<double>(sampleCount - 1));
            const auto angle = radiansPerSample * static_cast<double>(i);
            real += mono * window * std::cos(angle);
            imag += mono * window * std::sin(angle);
        }

        const auto magnitude = std::sqrt(real * real + imag * imag);
        const auto frequency = static_cast<double>(bin) * static_cast<double>(sampleRate)
            / static_cast<double>(sampleCount);
        weighted += frequency * magnitude;
        magnitudeSum += magnitude;
    }

    return magnitudeSum > 0.0 ? weighted / magnitudeSum : 0.0;
}

AudioMetrics analyzeAudio(const std::vector<float>& left, const std::vector<float>& right, int sampleRate = 48000)
{
    AudioMetrics metrics;
    double sumSquares = 0.0;
    double sumLeftSquares = 0.0;
    double sumRightSquares = 0.0;
    double sumCross = 0.0;
    double sumMono = 0.0;

    const auto sampleCount = std::min(left.size(), right.size());
    for (std::size_t i = 0; i < sampleCount; ++i)
    {
        const auto l = left[i];
        const auto r = right[i];
        if (!std::isfinite(l) || !std::isfinite(r))
            ++metrics.invalid;

        metrics.peak = std::max(metrics.peak, std::max(std::abs(l), std::abs(r)));
        if (std::abs(l) > 1.0e-7f || std::abs(r) > 1.0e-7f)
            ++metrics.nonzero;

        sumSquares += 0.5 * (static_cast<double>(l) * l + static_cast<double>(r) * r);
        sumLeftSquares += static_cast<double>(l) * l;
        sumRightSquares += static_cast<double>(r) * r;
        sumCross += static_cast<double>(l) * r;
        sumMono += 0.5 * (static_cast<double>(l) + static_cast<double>(r));
    }

    metrics.rms = sampleCount > 0 ? std::sqrt(sumSquares / static_cast<double>(sampleCount)) : 0.0;
    metrics.rmsDbfs = 20.0 * std::log10(std::max(metrics.rms, 1.0e-12));
    metrics.crestDb = metrics.rms > 0.0
        ? 20.0 * std::log10(std::max(static_cast<double>(metrics.peak), 1.0e-12) / metrics.rms)
        : 0.0;
    metrics.correlation = sumLeftSquares > 0.0 && sumRightSquares > 0.0
        ? sumCross / std::sqrt(sumLeftSquares * sumRightSquares)
        : 0.0;
    metrics.dcOffset = sampleCount > 0 ? sumMono / static_cast<double>(sampleCount) : 0.0;
    metrics.spectralCentroidHz = spectralCentroidHz(left, right, sampleRate);
    return metrics;
}

synth::VoiceSnapshot firstActiveSnapshot(const synth::SynthEngine& engine)
{
    for (int i = 0; i < 32; ++i)
    {
        const auto* voice = engine.getVoice(i);
        if (voice == nullptr)
            continue;

        const auto snapshot = voice->snapshot();
        if (snapshot.state != synth::VoiceState::Idle)
            return snapshot;
    }

    return {};
}

synth::RenderStats processSingleSamples(synth::SynthEngine& engine, int sampleCount)
{
    synth::RenderStats lastStats;
    float left = 0.0f;
    float right = 0.0f;
    for (int i = 0; i < sampleCount; ++i)
        lastStats = engine.process(&left, &right, 1);
    return lastStats;
}

struct ModulationReportMetrics
{
    float rampHalfway = 0.0f;
    float rampEnd = 0.0f;
    bool rampTimingPassed = false;
    float glideStartNote = 0.0f;
    float glideEndNote = 0.0f;
    bool glidePassed = false;
    float velocityGlideStart = 0.0f;
    float velocityGlideEnd = 0.0f;
    bool velocityGlidePassed = false;
    float directOscPitch = 0.0f;
    float directPulse = 0.0f;
    float directFilter = 0.0f;
    bool directRoutesPassed = false;
    float transOscPitch = 0.0f;
    float transPulse = 0.0f;
    float transFilter = 0.0f;
    float transAmp = 0.0f;
    float transPan = 0.0f;
    bool transModScalerPassed = false;
    bool modSlotSchemaPassed = false;
    float performancePitch = 0.0f;
    float performanceWheel = 0.0f;
    float performanceAftertouch = 0.0f;
    bool performanceSourcesPassed = false;
    int fixtureEvents = 0;
    int fixtureMaxActiveVoices = 0;
    int fixtureInvalidSamples = 0;
    float fixtureRampMax = 0.0f;
    float fixtureVoiceUniRange = 0.0f;
    float fixtureVoiceBiRange = 0.0f;
    float fixtureUnisonUniRange = 0.0f;
    float fixtureUnisonBiRange = 0.0f;
    float fixtureRandomRange = 0.0f;
    bool fixtureSourcesPassed = false;
};

int writeModulationReport(const Options& options)
{
    constexpr auto sampleRate = 48000;
    ModulationReportMetrics metrics;

    {
        synth::SynthParameters parameters;
        parameters.voiceMode = synth::VoiceMode::MonoLegato;
        parameters.polyphony = 1;
        parameters.glideMs = 100.0f;
        parameters.velocityGlideMs = 100.0f;
        parameters.ramp.enabled = true;
        parameters.ramp.riseMs = 100.0f;
        parameters.ramp.curve = synth::RampCurve::Linear;
        parameters.filter.enabled = false;

        synth::SynthEngine engine;
        engine.prepare(sampleRate, 1);
        engine.setParameters(parameters);
        engine.noteOn(60, 0.25f);
        processSingleSamples(engine, sampleRate / 20);
        metrics.rampHalfway = firstActiveSnapshot(engine).ramp;

        engine.noteOn(72, 1.0f);
        processSingleSamples(engine, 16);
        auto snapshot = firstActiveSnapshot(engine);
        metrics.glideStartNote = snapshot.effectiveMidiNote;
        metrics.velocityGlideStart = snapshot.velocityGlide;

        processSingleSamples(engine, sampleRate / 10);
        snapshot = firstActiveSnapshot(engine);
        metrics.glideEndNote = snapshot.effectiveMidiNote;
        metrics.velocityGlideEnd = snapshot.velocityGlide;
        metrics.rampEnd = snapshot.ramp;
        metrics.rampTimingPassed = metrics.rampHalfway > 0.45f && metrics.rampHalfway < 0.55f && metrics.rampEnd > 0.98f;
        metrics.glidePassed = metrics.glideStartNote > 60.0f && metrics.glideStartNote < 61.0f
            && std::abs(metrics.glideEndNote - 72.0f) < 0.05f;
        metrics.velocityGlidePassed = metrics.velocityGlideStart > 0.25f && metrics.velocityGlideStart < 0.35f
            && std::abs(metrics.velocityGlideEnd - 1.0f) < 0.01f;
    }

    {
        synth::SynthParameters parameters;
        parameters.ramp.enabled = true;
        parameters.ramp.riseMs = 1.0f;
        parameters.direct.oscKeytrackSemitones = 12.0f;
        parameters.direct.pulseKeytrack = 0.25f;
        parameters.direct.filterKeytrack = 0.5f;
        parameters.macro.motion = 0.5f;
        parameters.filter.enabled = false;

        auto& slot = parameters.transMod.slots[0];
        slot.enabled = true;
        slot.source = synth::ModSource::Ramp;
        slot.scaler = synth::ModSource::Macro1;
        slot.oscPitchSemitones = 12.0f;
        slot.pulseWidth = 0.2f;
        slot.filterCutoffSemitones = 24.0f;
        slot.ampLevelDb = 6.0f;
        slot.pan = 0.4f;

        synth::SynthEngine engine;
        engine.prepare(sampleRate, 1);
        engine.setParameters(parameters);
        engine.noteOn(72, 1.0f);
        processSingleSamples(engine, 128);

        const auto snapshot = firstActiveSnapshot(engine);
        metrics.directOscPitch = snapshot.directOscPitchSemitones;
        metrics.directPulse = snapshot.directPulseWidth;
        metrics.directFilter = snapshot.directFilterCutoffSemitones;
        metrics.transOscPitch = snapshot.transModOscPitchSemitones;
        metrics.transPulse = snapshot.transModPulseWidth;
        metrics.transFilter = snapshot.transModFilterCutoffSemitones;
        metrics.transAmp = snapshot.transModAmpLevelDb;
        metrics.transPan = snapshot.transModPan;
        metrics.directRoutesPassed = std::abs(metrics.directOscPitch - 12.0f) < 0.05f
            && std::abs(metrics.directPulse - 0.25f) < 0.01f
            && std::abs(metrics.directFilter - 6.0f) < 0.05f;
        metrics.transModScalerPassed = std::abs(metrics.transOscPitch - 6.0f) < 0.05f
            && std::abs(metrics.transPulse - 0.1f) < 0.01f
            && std::abs(metrics.transFilter - 12.0f) < 0.05f
            && std::abs(metrics.transAmp - 3.0f) < 0.05f
            && std::abs(metrics.transPan - 0.2f) < 0.01f;
    }

    {
        synth::SynthParameters parameters;
        std::string error;
        if (loadPresetParameters("fixtures/presets/mod-slots-only.json", parameters, error))
        {
            const auto& slot = parameters.transMod.slots[0];
            metrics.modSlotSchemaPassed = slot.enabled
                && slot.source == synth::ModSource::Ramp
                && slot.scaler == synth::ModSource::Macro1
                && std::abs(slot.filterCutoffSemitones - 12.0f) < 0.01f
                && std::abs(slot.pan - 0.25f) < 0.01f;
        }
    }

    {
        synth::SynthParameters parameters;
        parameters.filter.enabled = false;
        parameters.transMod.slots[0].enabled = true;
        parameters.transMod.slots[0].source = synth::ModSource::PitchBend;
        parameters.transMod.slots[0].oscPitchSemitones = 12.0f;
        parameters.transMod.slots[1].enabled = true;
        parameters.transMod.slots[1].source = synth::ModSource::ModWheel;
        parameters.transMod.slots[1].filterCutoffSemitones = 24.0f;
        parameters.transMod.slots[2].enabled = true;
        parameters.transMod.slots[2].source = synth::ModSource::Aftertouch;
        parameters.transMod.slots[2].ampLevelDb = 6.0f;

        synth::SynthEngine engine;
        engine.prepare(sampleRate, 1);
        engine.setParameters(parameters);
        engine.setPitchBend(0.5f);
        engine.setModWheel(0.25f);
        engine.setAftertouch(0.5f);
        engine.noteOn(60, 1.0f);
        processSingleSamples(engine, 64);

        const auto snapshot = firstActiveSnapshot(engine);
        metrics.performancePitch = snapshot.transModOscPitchSemitones;
        metrics.performanceWheel = snapshot.transModFilterCutoffSemitones;
        metrics.performanceAftertouch = snapshot.transModAmpLevelDb;
        metrics.performanceSourcesPassed = std::abs(metrics.performancePitch - 6.0f) < 0.05f
            && std::abs(metrics.performanceWheel - 6.0f) < 0.05f
            && std::abs(metrics.performanceAftertouch - 3.0f) < 0.05f;
    }

    std::vector<RenderEvent> events;
    std::string error;
    if (!loadMidiFixture(options.fixturePath, sampleRate, events, error))
    {
        std::cerr << error << "\n";
        return 1;
    }
    metrics.fixtureEvents = static_cast<int>(events.size());

    {
        synth::SynthParameters parameters;
        parameters.polyphony = 4;
        parameters.unisonCount = 2;
        parameters.ramp.enabled = true;
        parameters.ramp.riseMs = 60.0f;
        parameters.velocityGlideMs = 35.0f;
        parameters.glideMs = 35.0f;
        parameters.filter.enabled = false;
        parameters.osc.sawLevel = 1.0f;
        parameters.amp.panSpread = 0.25f;
        parameters.amp.unisonSpread = 0.25f;

        synth::SynthEngine engine;
        engine.prepare(sampleRate, 1);
        engine.setParameters(parameters);

        const auto lastEventSample = events.empty() ? 0 : events.back().sample;
        const auto sampleCount = lastEventSample + sampleRate / 2;
        auto nextEvent = std::size_t { 0 };
        auto minVoiceUni = 1.0f;
        auto maxVoiceUni = 0.0f;
        auto minVoiceBi = 1.0f;
        auto maxVoiceBi = -1.0f;
        auto minUnisonUni = 1.0f;
        auto maxUnisonUni = 0.0f;
        auto minUnisonBi = 1.0f;
        auto maxUnisonBi = -1.0f;
        auto minRandom = 1.0f;
        auto maxRandom = -1.0f;

        for (int sample = 0; sample < sampleCount; ++sample)
        {
            while (nextEvent < events.size() && events[nextEvent].sample == sample)
            {
                handleRenderMidi(engine, events[nextEvent].message);
                ++nextEvent;
            }

            const auto stats = processSingleSamples(engine, 1);
            metrics.fixtureInvalidSamples += stats.invalidSamples;
            metrics.fixtureMaxActiveVoices = std::max(metrics.fixtureMaxActiveVoices, stats.activeVoices);

            for (int voiceIndex = 0; voiceIndex < 32; ++voiceIndex)
            {
                const auto* voice = engine.getVoice(voiceIndex);
                if (voice == nullptr)
                    continue;

                const auto snapshot = voice->snapshot();
                if (snapshot.state == synth::VoiceState::Idle)
                    continue;

                metrics.fixtureRampMax = std::max(metrics.fixtureRampMax, snapshot.ramp);
                minVoiceUni = std::min(minVoiceUni, snapshot.voiceUni);
                maxVoiceUni = std::max(maxVoiceUni, snapshot.voiceUni);
                minVoiceBi = std::min(minVoiceBi, snapshot.voiceBi);
                maxVoiceBi = std::max(maxVoiceBi, snapshot.voiceBi);
                minUnisonUni = std::min(minUnisonUni, snapshot.unisonUni);
                maxUnisonUni = std::max(maxUnisonUni, snapshot.unisonUni);
                minUnisonBi = std::min(minUnisonBi, snapshot.unisonBi);
                maxUnisonBi = std::max(maxUnisonBi, snapshot.unisonBi);
                minRandom = std::min(minRandom, snapshot.randomOnNote);
                maxRandom = std::max(maxRandom, snapshot.randomOnNote);
            }
        }

        metrics.fixtureVoiceUniRange = maxVoiceUni - minVoiceUni;
        metrics.fixtureVoiceBiRange = maxVoiceBi - minVoiceBi;
        metrics.fixtureUnisonUniRange = maxUnisonUni - minUnisonUni;
        metrics.fixtureUnisonBiRange = maxUnisonBi - minUnisonBi;
        metrics.fixtureRandomRange = maxRandom - minRandom;
        metrics.fixtureSourcesPassed = metrics.fixtureInvalidSamples == 0
            && metrics.fixtureMaxActiveVoices >= 2
            && metrics.fixtureRampMax > 0.75f
            && metrics.fixtureVoiceUniRange > 0.1f
            && metrics.fixtureVoiceBiRange > 0.2f
            && metrics.fixtureUnisonUniRange > 0.9f
            && metrics.fixtureUnisonBiRange > 1.8f
            && metrics.fixtureRandomRange > 0.001f;
    }

    const auto passed = metrics.rampTimingPassed
        && metrics.glidePassed
        && metrics.velocityGlidePassed
        && metrics.directRoutesPassed
        && metrics.transModScalerPassed
        && metrics.modSlotSchemaPassed
        && metrics.performanceSourcesPassed
        && metrics.fixtureSourcesPassed;

    ensureParentDirectory(options.output);
    std::ofstream out(options.output);
    out << "{\n";
    out << "  \"schema_version\": 1,\n";
    out << "  \"suite\": \"modulation\",\n";
    out << "  \"fixture\": \"" << options.fixturePath.string() << "\",\n";
    out << "  \"fixture_events\": " << metrics.fixtureEvents << ",\n";
    out << "  \"sample_rate\": " << sampleRate << ",\n";
    out << "  \"ramp_halfway\": " << metrics.rampHalfway << ",\n";
    out << "  \"ramp_end\": " << metrics.rampEnd << ",\n";
    out << "  \"ramp_timing_passed\": " << (metrics.rampTimingPassed ? "true" : "false") << ",\n";
    out << "  \"glide_start_note\": " << metrics.glideStartNote << ",\n";
    out << "  \"glide_end_note\": " << metrics.glideEndNote << ",\n";
    out << "  \"glide_passed\": " << (metrics.glidePassed ? "true" : "false") << ",\n";
    out << "  \"velocity_glide_start\": " << metrics.velocityGlideStart << ",\n";
    out << "  \"velocity_glide_end\": " << metrics.velocityGlideEnd << ",\n";
    out << "  \"velocity_glide_passed\": " << (metrics.velocityGlidePassed ? "true" : "false") << ",\n";
    out << "  \"direct_osc_pitch_semitones\": " << metrics.directOscPitch << ",\n";
    out << "  \"direct_pulse_width\": " << metrics.directPulse << ",\n";
    out << "  \"direct_filter_cutoff_semitones\": " << metrics.directFilter << ",\n";
    out << "  \"direct_routes_passed\": " << (metrics.directRoutesPassed ? "true" : "false") << ",\n";
    out << "  \"transmod_osc_pitch_semitones\": " << metrics.transOscPitch << ",\n";
    out << "  \"transmod_pulse_width\": " << metrics.transPulse << ",\n";
    out << "  \"transmod_filter_cutoff_semitones\": " << metrics.transFilter << ",\n";
    out << "  \"transmod_amp_level_db\": " << metrics.transAmp << ",\n";
    out << "  \"transmod_pan\": " << metrics.transPan << ",\n";
    out << "  \"transmod_scaler_passed\": " << (metrics.transModScalerPassed ? "true" : "false") << ",\n";
    out << "  \"mod_slot_schema_passed\": " << (metrics.modSlotSchemaPassed ? "true" : "false") << ",\n";
    out << "  \"performance_pitchbend_osc_pitch\": " << metrics.performancePitch << ",\n";
    out << "  \"performance_modwheel_filter_cutoff\": " << metrics.performanceWheel << ",\n";
    out << "  \"performance_aftertouch_amp_level\": " << metrics.performanceAftertouch << ",\n";
    out << "  \"performance_sources_passed\": " << (metrics.performanceSourcesPassed ? "true" : "false") << ",\n";
    out << "  \"fixture_max_active_voices\": " << metrics.fixtureMaxActiveVoices << ",\n";
    out << "  \"fixture_invalid_samples\": " << metrics.fixtureInvalidSamples << ",\n";
    out << "  \"fixture_ramp_max\": " << metrics.fixtureRampMax << ",\n";
    out << "  \"fixture_voice_uni_range\": " << metrics.fixtureVoiceUniRange << ",\n";
    out << "  \"fixture_voice_bi_range\": " << metrics.fixtureVoiceBiRange << ",\n";
    out << "  \"fixture_unison_uni_range\": " << metrics.fixtureUnisonUniRange << ",\n";
    out << "  \"fixture_unison_bi_range\": " << metrics.fixtureUnisonBiRange << ",\n";
    out << "  \"fixture_random_range\": " << metrics.fixtureRandomRange << ",\n";
    out << "  \"fixture_sources_passed\": " << (metrics.fixtureSourcesPassed ? "true" : "false") << ",\n";
    out << "  \"passed\": " << (passed ? "true" : "false") << "\n";
    out << "}\n";

    return passed ? 0 : 1;
}

enum class PresetRenderVariant
{
    Default,
    PerVoiceLfo,
    MonoLfo
};

enum class RenderFxMode
{
    Dry,
    Wet
};

const char* toString(PresetRenderVariant variant) noexcept
{
    switch (variant)
    {
        case PresetRenderVariant::Default: return "default";
        case PresetRenderVariant::PerVoiceLfo: return "per_voice_lfo";
        case PresetRenderVariant::MonoLfo: return "mono_lfo";
    }

    return "unknown";
}

const char* toString(RenderFxMode mode) noexcept
{
    switch (mode)
    {
        case RenderFxMode::Dry: return "dry";
        case RenderFxMode::Wet: return "wet";
    }

    return "unknown";
}

const char* toString(synth::QualityMode mode) noexcept
{
    switch (mode)
    {
        case synth::QualityMode::Eco: return "eco";
        case synth::QualityMode::Normal: return "normal";
        case synth::QualityMode::High: return "high";
    }

    return "unknown";
}

struct PresetRenderResult
{
    bool ok = false;
    std::string error;
    std::vector<float> left;
    std::vector<float> right;
    AudioMetrics metrics;
    int sampleRate = 48000;
    int sampleCount = 0;
    int fixtureEvents = 0;
    int lastEventSample = 0;
    int invalidDuringRender = 0;
    int tempoSyncedDelaySamples = 0;
    float delayDivisionBeats = 0.0f;
    float fxTailSeconds = 0.0f;
    float tempoBpm = 128.0f;
    float postLastEventSeconds = 0.0f;
    synth::QualityMode qualityMode = synth::QualityMode::Normal;
    float noteLocalLfoSpread = 0.0f;
    bool noteLocalMotionPassed = false;
    double wetDryMaxAbsDiff = 0.0;
    double wetDryRmsDiff = 0.0;
    bool wetMeaningfulPassed = false;
    bool passed = false;
};

struct AudioDiff
{
    double maxAbs = 0.0;
    double rms = 0.0;
    double peakDelta = 0.0;
};

AudioDiff compareAudio(const PresetRenderResult& a, const PresetRenderResult& b);
void evaluateWetDifference(PresetRenderResult& wet, const PresetRenderResult& dry);

PresetRenderResult renderPresetAudio(const std::filesystem::path& presetPath,
                                     const std::filesystem::path& fixturePath,
                                     PresetRenderVariant variant,
                                     RenderFxMode fxMode = RenderFxMode::Dry)
{
    PresetRenderResult result;
    synth::SynthParameters parameters;
    if (!loadPresetParameters(presetPath, parameters, result.error))
        return result;

    if (variant == PresetRenderVariant::MonoLfo)
    {
        parameters.lfo.mono = true;
        parameters.lfo.gateMode = synth::LfoGateMode::Mono;
    }
    else if (variant == PresetRenderVariant::PerVoiceLfo)
    {
        parameters.lfo.mono = false;
        parameters.lfo.gateMode = synth::LfoGateMode::PolyOn;
    }

    parameters.quality.activeMode = parameters.quality.offlineMode;
    if (fxMode == RenderFxMode::Dry)
        parameters.fx.enabled = false;
    else
        parameters.fx.enabled = true;

    result.qualityMode = parameters.quality.activeMode;
    result.tempoBpm = parameters.tempoBpm;
    result.tempoSyncedDelaySamples = synth::tempoSyncedDelaySamples(result.sampleRate, parameters.tempoBpm,
                                                                    parameters.fx.delaySyncDivision);
    result.delayDivisionBeats = synth::delayDivisionBeats(parameters.fx.delaySyncDivision);
    result.fxTailSeconds = synth::fxTailLengthSeconds(parameters);

    std::vector<RenderEvent> events;
    if (!loadMidiFixture(fixturePath, result.sampleRate, events, result.error))
        return result;

    result.fixtureEvents = static_cast<int>(events.size());
    result.lastEventSample = events.empty() ? 0 : events.back().sample;
    const auto voiceTailSeconds = std::max(parameters.ampEnv.releaseMs, parameters.modEnv.releaseMs) * 0.001f;
    result.postLastEventSeconds = std::max(0.4f, std::max(result.fxTailSeconds, voiceTailSeconds) + 0.1f);
    result.sampleCount = std::max(static_cast<int>(result.sampleRate * 1.35),
                                  result.lastEventSample
                                      + static_cast<int>(std::ceil(static_cast<double>(result.sampleRate)
                                                                  * result.postLastEventSeconds)));

    synth::SynthEngine engine;
    engine.prepare(result.sampleRate, 1);
    engine.setParameters(parameters);

    result.left.assign(static_cast<std::size_t>(result.sampleCount), 0.0f);
    result.right.assign(static_cast<std::size_t>(result.sampleCount), 0.0f);

    auto nextEvent = std::size_t { 0 };
    for (int i = 0; i < result.sampleCount; ++i)
    {
        while (nextEvent < events.size() && events[nextEvent].sample == i)
        {
            handleRenderMidi(engine, events[nextEvent].message);
            ++nextEvent;
        }

        const auto stats = engine.process(&result.left[static_cast<std::size_t>(i)],
                                          &result.right[static_cast<std::size_t>(i)], 1);
        if (stats.invalidSamples > 0)
        {
            result.invalidDuringRender += stats.invalidSamples;
            result.left[static_cast<std::size_t>(i)] = 0.0f;
            result.right[static_cast<std::size_t>(i)] = 0.0f;
        }

        auto activeVoiceCount = 0;
        auto firstNote = -1;
        auto hasMultipleNotes = false;
        auto minLfo = 1.0f;
        auto maxLfo = -1.0f;
        for (int voiceIndex = 0; voiceIndex < 32; ++voiceIndex)
        {
            const auto* voice = engine.getVoice(voiceIndex);
            if (voice == nullptr)
                continue;

            const auto snapshot = voice->snapshot();
            if (snapshot.state == synth::VoiceState::Idle)
                continue;

            if (firstNote < 0)
                firstNote = snapshot.midiNote;
            else if (snapshot.midiNote != firstNote)
                hasMultipleNotes = true;

            minLfo = std::min(minLfo, snapshot.lfo);
            maxLfo = std::max(maxLfo, snapshot.lfo);
            ++activeVoiceCount;
        }

        if (hasMultipleNotes && activeVoiceCount > 1)
            result.noteLocalLfoSpread = std::max(result.noteLocalLfoSpread, maxLfo - minLfo);
    }

    result.metrics = analyzeAudio(result.left, result.right, result.sampleRate);
    result.metrics.invalid += result.invalidDuringRender;
    result.noteLocalMotionPassed = result.noteLocalLfoSpread > 0.01f;

    const auto requiresNoteLocalMotion = variant != PresetRenderVariant::MonoLfo;
    result.passed = result.metrics.invalid == 0
        && result.metrics.peak < 1.0f
        && result.metrics.rms > 0.001
        && result.metrics.nonzero > result.sampleRate / 8
        && (!requiresNoteLocalMotion || result.noteLocalMotionPassed);
    result.ok = true;
    return result;
}

void writePresetRenderReport(const std::filesystem::path& reportPath,
                             const PresetRenderResult& result,
                             const std::filesystem::path& presetPath,
                             const std::filesystem::path& fixturePath,
                             const std::filesystem::path& wavPath,
                             RenderFxMode fxMode,
                             PresetRenderVariant variant)
{
    ensureParentDirectory(reportPath);

    std::ofstream out(reportPath);
    out << "{\n";
    out << "  \"schema_version\": 1,\n";
    out << "  \"suite\": \"preset-" << toString(fxMode) << "-render\",\n";
    out << "  \"variant\": \"" << toString(variant) << "\",\n";
    out << "  \"fx_mode\": \"" << toString(fxMode) << "\",\n";
    out << "  \"preset\": \"" << genericString(presetPath) << "\",\n";
    out << "  \"fixture\": \"" << genericString(fixturePath) << "\",\n";
    out << "  \"artifact_wav\": \"" << genericString(wavPath) << "\",\n";
    out << "  \"fixture_events\": " << result.fixtureEvents << ",\n";
    out << "  \"dry\": " << boolString(fxMode == RenderFxMode::Dry) << ",\n";
    out << "  \"wet\": " << boolString(fxMode == RenderFxMode::Wet) << ",\n";
    out << "  \"sample_rate\": " << result.sampleRate << ",\n";
    out << "  \"samples_rendered\": " << result.sampleCount << ",\n";
    out << "  \"last_event_sample\": " << result.lastEventSample << ",\n";
    out << "  \"post_last_event_seconds\": " << result.postLastEventSeconds << ",\n";
    out << "  \"tempo_bpm\": " << result.tempoBpm << ",\n";
    out << "  \"delay_division_beats\": " << result.delayDivisionBeats << ",\n";
    out << "  \"tempo_synced_delay_samples\": " << result.tempoSyncedDelaySamples << ",\n";
    out << "  \"fx_tail_seconds\": " << result.fxTailSeconds << ",\n";
    out << "  \"quality_mode\": \"" << toString(result.qualityMode) << "\",\n";
    out << "  \"peak\": " << result.metrics.peak << ",\n";
    out << "  \"rms\": " << result.metrics.rms << ",\n";
    out << "  \"rms_dbfs\": " << result.metrics.rmsDbfs << ",\n";
    out << "  \"crest_db\": " << result.metrics.crestDb << ",\n";
    out << "  \"stereo_correlation\": " << result.metrics.correlation << ",\n";
    out << "  \"dc_offset\": " << result.metrics.dcOffset << ",\n";
    out << "  \"spectral_centroid_hz\": " << result.metrics.spectralCentroidHz << ",\n";
    out << "  \"note_local_lfo_spread\": " << result.noteLocalLfoSpread << ",\n";
    out << "  \"note_local_motion_passed\": " << boolString(result.noteLocalMotionPassed) << ",\n";
    out << "  \"wet_dry_max_abs_diff\": " << result.wetDryMaxAbsDiff << ",\n";
    out << "  \"wet_dry_rms_diff\": " << result.wetDryRmsDiff << ",\n";
    out << "  \"wet_meaningful_passed\": " << boolString(result.wetMeaningfulPassed) << ",\n";
    out << "  \"nonzero_samples\": " << result.metrics.nonzero << ",\n";
    out << "  \"invalid_samples\": " << result.metrics.invalid << ",\n";
    out << "  \"passed\": " << boolString(result.passed) << "\n";
    out << "}\n";
}

int renderPreset(const Options& options)
{
    const auto fxMode = options.wet && !options.dry ? RenderFxMode::Wet : RenderFxMode::Dry;
    auto result = renderPresetAudio(options.presetPath, options.fixturePath,
                                    PresetRenderVariant::Default, fxMode);
    if (!result.ok)
    {
        std::cerr << result.error << "\n";
        return 1;
    }

    if (fxMode == RenderFxMode::Wet)
    {
        const auto dryReference = renderPresetAudio(options.presetPath, options.fixturePath,
                                                    PresetRenderVariant::Default, RenderFxMode::Dry);
        if (!dryReference.ok)
        {
            std::cerr << dryReference.error << "\n";
            return 1;
        }

        evaluateWetDifference(result, dryReference);
    }

    writeWav16(options.output, result.left, result.right, result.sampleRate);
    writePresetRenderReport(options.report, result, options.presetPath, options.fixturePath,
                            options.output, fxMode, PresetRenderVariant::Default);
    return result.passed ? 0 : 1;
}

void writeFailureReport(const std::filesystem::path& reportPath, const std::string& suite, const std::string& error)
{
    ensureParentDirectory(reportPath);

    std::ofstream out(reportPath);
    out << "{\n";
    out << "  \"schema_version\": 1,\n";
    out << "  \"suite\": \"" << suite << "\",\n";
    out << "  \"error\": \"" << error << "\",\n";
    out << "  \"passed\": false\n";
    out << "}\n";
}

AudioDiff compareAudio(const PresetRenderResult& a, const PresetRenderResult& b)
{
    AudioDiff diff;
    const auto sampleCount = std::min(a.left.size(), b.left.size());
    auto sumSquares = 0.0;

    for (std::size_t i = 0; i < sampleCount; ++i)
    {
        const auto leftDiff = static_cast<double>(a.left[i]) - static_cast<double>(b.left[i]);
        const auto rightDiff = static_cast<double>(a.right[i]) - static_cast<double>(b.right[i]);
        diff.maxAbs = std::max(diff.maxAbs, std::max(std::abs(leftDiff), std::abs(rightDiff)));
        sumSquares += 0.5 * (leftDiff * leftDiff + rightDiff * rightDiff);
    }

    diff.rms = sampleCount > 0 ? std::sqrt(sumSquares / static_cast<double>(sampleCount)) : 0.0;
    diff.peakDelta = std::abs(static_cast<double>(a.metrics.peak) - static_cast<double>(b.metrics.peak));
    return diff;
}

void evaluateWetDifference(PresetRenderResult& wet, const PresetRenderResult& dry)
{
    const auto diff = compareAudio(wet, dry);
    wet.wetDryMaxAbsDiff = diff.maxAbs;
    wet.wetDryRmsDiff = diff.rms;
    wet.wetMeaningfulPassed = wet.fxTailSeconds > 0.0f
        && diff.maxAbs > 1.0e-4
        && diff.rms > 1.0e-5;
    wet.passed = wet.passed && wet.wetMeaningfulPassed;
}

int writeDeterminismReport(const std::filesystem::path& reportPath,
                           const std::filesystem::path& failureDir,
                           const std::filesystem::path& presetPath,
                           const std::filesystem::path& fixturePath)
{
    const auto first = renderPresetAudio(presetPath, fixturePath, PresetRenderVariant::Default);
    const auto second = renderPresetAudio(presetPath, fixturePath, PresetRenderVariant::Default);
    if (!first.ok || !second.ok)
    {
        writeFailureReport(reportPath, "determinism", !first.ok ? first.error : second.error);
        return 1;
    }

    const auto diff = compareAudio(first, second);
    constexpr auto maxAbsTolerance = 1.0e-7;
    constexpr auto rmsTolerance = 1.0e-9;
    constexpr auto peakTolerance = 1.0e-7;
    const auto passed = first.passed
        && second.passed
        && diff.maxAbs <= maxAbsTolerance
        && diff.rms <= rmsTolerance
        && diff.peakDelta <= peakTolerance;

    auto firstFailureWav = std::filesystem::path {};
    auto secondFailureWav = std::filesystem::path {};
    if (!passed)
    {
        ensureDirectory(failureDir);
        firstFailureWav = failureDir / "determinism-first.wav";
        secondFailureWav = failureDir / "determinism-second.wav";
        writeWav16(firstFailureWav, first.left, first.right, first.sampleRate);
        writeWav16(secondFailureWav, second.left, second.right, second.sampleRate);
    }

    ensureParentDirectory(reportPath);
    std::ofstream out(reportPath);
    out << "{\n";
    out << "  \"schema_version\": 1,\n";
    out << "  \"suite\": \"determinism\",\n";
    out << "  \"preset\": \"" << genericString(presetPath) << "\",\n";
    out << "  \"fixture\": \"" << genericString(fixturePath) << "\",\n";
    out << "  \"max_abs_diff\": " << diff.maxAbs << ",\n";
    out << "  \"rms_diff\": " << diff.rms << ",\n";
    out << "  \"peak_delta\": " << diff.peakDelta << ",\n";
    out << "  \"max_abs_tolerance\": " << maxAbsTolerance << ",\n";
    out << "  \"rms_tolerance\": " << rmsTolerance << ",\n";
    out << "  \"peak_tolerance\": " << peakTolerance << ",\n";
    out << "  \"failure_first_wav\": \"" << genericString(firstFailureWav) << "\",\n";
    out << "  \"failure_second_wav\": \"" << genericString(secondFailureWav) << "\",\n";
    out << "  \"passed\": " << boolString(passed) << "\n";
    out << "}\n";

    return passed ? 0 : 1;
}

int writeLfoAblationReport(const std::filesystem::path& reportPath,
                           const std::filesystem::path& artifactDir,
                           const std::filesystem::path& presetPath,
                           const std::filesystem::path& fixturePath)
{
    const auto perVoice = renderPresetAudio(presetPath, fixturePath, PresetRenderVariant::PerVoiceLfo);
    const auto mono = renderPresetAudio(presetPath, fixturePath, PresetRenderVariant::MonoLfo);
    if (!perVoice.ok || !mono.ok)
    {
        writeFailureReport(reportPath, "lfo-ablation", !perVoice.ok ? perVoice.error : mono.error);
        return 1;
    }

    ensureDirectory(artifactDir);
    const auto perVoiceWav = artifactDir / "pluck-core-01-per-voice-lfo.wav";
    const auto monoWav = artifactDir / "pluck-core-01-mono-lfo.wav";
    writeWav16(perVoiceWav, perVoice.left, perVoice.right, perVoice.sampleRate);
    writeWav16(monoWav, mono.left, mono.right, mono.sampleRate);

    const auto diff = compareAudio(perVoice, mono);
    const auto lfoSpreadDelta = perVoice.noteLocalLfoSpread - mono.noteLocalLfoSpread;
    const auto rmsDbDelta = std::abs(perVoice.metrics.rmsDbfs - mono.metrics.rmsDbfs);
    const auto centroidDelta = std::abs(perVoice.metrics.spectralCentroidHz - mono.metrics.spectralCentroidHz);
    const auto passed = perVoice.passed
        && mono.passed
        && perVoice.noteLocalLfoSpread > 0.5f
        && mono.noteLocalLfoSpread < 0.01f
        && lfoSpreadDelta > 0.5f
        && diff.rms > 1.0e-5;

    ensureParentDirectory(reportPath);
    std::ofstream out(reportPath);
    out << "{\n";
    out << "  \"schema_version\": 1,\n";
    out << "  \"suite\": \"lfo-ablation\",\n";
    out << "  \"preset\": \"" << genericString(presetPath) << "\",\n";
    out << "  \"fixture\": \"" << genericString(fixturePath) << "\",\n";
    out << "  \"per_voice_wav\": \"" << genericString(perVoiceWav) << "\",\n";
    out << "  \"mono_wav\": \"" << genericString(monoWav) << "\",\n";
    out << "  \"per_voice_lfo_spread\": " << perVoice.noteLocalLfoSpread << ",\n";
    out << "  \"mono_lfo_spread\": " << mono.noteLocalLfoSpread << ",\n";
    out << "  \"lfo_spread_delta\": " << lfoSpreadDelta << ",\n";
    out << "  \"rms_diff\": " << diff.rms << ",\n";
    out << "  \"max_abs_diff\": " << diff.maxAbs << ",\n";
    out << "  \"rms_db_delta\": " << rmsDbDelta << ",\n";
    out << "  \"spectral_centroid_delta_hz\": " << centroidDelta << ",\n";
    out << "  \"passed\": " << boolString(passed) << "\n";
    out << "}\n";

    return passed ? 0 : 1;
}

void writeCoreSummary(const std::filesystem::path& path,
                      const std::filesystem::path& outputDir,
                      const std::vector<SuiteItem>& items)
{
    ensureParentDirectory(path);

    auto passedCount = 0;
    for (const auto& item : items)
    {
        if (item.passed)
            ++passedCount;
    }

    const auto failedCount = static_cast<int>(items.size()) - passedCount;
    std::ofstream out(path);
    out << "{\n";
    out << "  \"schema_version\": 1,\n";
    out << "  \"suite\": \"core\",\n";
    out << "  \"format\": \"standalone\",\n";
    out << "  \"output_dir\": \"" << genericString(outputDir) << "\",\n";
    out << "  \"fixture_id\": \"overlap-pluck\",\n";
    out << "  \"sample_rate\": 48000,\n";
    out << "  \"block_size\": 1,\n";
    out << "  \"tempo_bpm\": 128,\n";
    out << "  \"seed\": \"engine-default-deterministic\",\n";
    out << "  \"report_count\": " << items.size() << ",\n";
    out << "  \"passed_count\": " << passedCount << ",\n";
    out << "  \"failed_count\": " << failedCount << ",\n";
    out << "  \"reports\": [\n";
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        const auto& item = items[i];
        out << "    {\"name\": \"" << item.name << "\", ";
        out << "\"path\": \"" << genericString(item.reportPath) << "\", ";
        out << "\"passed\": " << boolString(item.passed) << "}";
        out << (i + 1 == items.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"passed\": " << boolString(failedCount == 0) << "\n";
    out << "}\n";
}

int writeCoreSuite(const Options& options)
{
    if (options.suite != "core")
    {
        std::cerr << "Unknown suite: " << options.suite << "\n";
        return 2;
    }

    const auto outputDir = options.outputDir;
    const auto artifactDir = outputDir / "artifacts";
    const auto failureDir = outputDir / "failures";
    const auto presetPath = std::filesystem::path { "presets/factory/pluck-core-01.json" };
    const auto presetDirectory = std::filesystem::path { "presets/factory" };
    const auto fixturePath = options.fixturePath;

    ensureDirectory(outputDir);
    ensureDirectory(artifactDir);

    std::vector<SuiteItem> items;
    auto addItem = [&items](std::string name, std::filesystem::path reportPath, int exitCode) {
        items.push_back({ std::move(name), std::move(reportPath), exitCode == 0 });
    };

    {
        synth::SynthEngine engine;
        engine.prepare(48000.0, 256);
        std::vector<float> left(4800);
        std::vector<float> right(4800);
        const auto stats = engine.process(left.data(), right.data(), static_cast<int>(left.size()));
        const auto reportPath = outputDir / "smoke.json";
        writeSmokeReport(reportPath, stats);
        addItem("smoke", reportPath, stats.invalidSamples == 0 ? 0 : 1);
    }

    {
        const auto reportPath = outputDir / "parameters.json";
        writeParameterReport(reportPath);
        addItem("parameters", reportPath, synth::validateParameterSpecs().empty() ? 0 : 1);
    }

    {
        const auto reportPath = outputDir / "presets.json";
        addItem("presets", reportPath, writePresetReport(reportPath, presetDirectory));
    }

    {
        const auto reportPath = outputDir / "voice-core.json";
        addItem("voice-core", reportPath, writeVoiceReport(reportPath));
    }

    {
        const auto reportPath = outputDir / "oscillator.json";
        addItem("oscillator", reportPath, writeOscillatorReport(reportPath, options.notes));
    }

    {
        const auto reportPath = outputDir / "filter.json";
        addItem("filter", reportPath, writeFilterReport(reportPath));
    }

    {
        auto modulationOptions = options;
        modulationOptions.output = outputDir / "modulation.json";
        addItem("modulation", modulationOptions.output, writeModulationReport(modulationOptions));
    }

    {
        const auto reportPath = outputDir / "pluck-core-01-dry.json";
        const auto wavPath = artifactDir / "pluck-core-01-dry.wav";
        const auto render = renderPresetAudio(presetPath, fixturePath, PresetRenderVariant::Default);
        if (!render.ok)
        {
            writeFailureReport(reportPath, "preset-dry-render", render.error);
            addItem("pluck-core-01-dry", reportPath, 1);
        }
        else
        {
            writeWav16(wavPath, render.left, render.right, render.sampleRate);
            writePresetRenderReport(reportPath, render, presetPath, fixturePath, wavPath,
                                    RenderFxMode::Dry, PresetRenderVariant::Default);
            addItem("pluck-core-01-dry", reportPath, render.passed ? 0 : 1);
        }
    }

    {
        const auto reportPath = outputDir / "pluck-core-01-wet.json";
        const auto wavPath = artifactDir / "pluck-core-01-wet.wav";
        const auto render = renderPresetAudio(presetPath, fixturePath,
                                              PresetRenderVariant::Default,
                                              RenderFxMode::Wet);
        if (!render.ok)
        {
            writeFailureReport(reportPath, "preset-wet-render", render.error);
            addItem("pluck-core-01-wet", reportPath, 1);
        }
        else
        {
            const auto dryReference = renderPresetAudio(presetPath, fixturePath,
                                                        PresetRenderVariant::Default,
                                                        RenderFxMode::Dry);
            auto checkedRender = render;
            if (dryReference.ok)
                evaluateWetDifference(checkedRender, dryReference);
            else
                checkedRender.passed = false;

            writeWav16(wavPath, render.left, render.right, render.sampleRate);
            writePresetRenderReport(reportPath, checkedRender, presetPath, fixturePath, wavPath,
                                    RenderFxMode::Wet, PresetRenderVariant::Default);
            const auto expectedDelaySamples = synth::tempoSyncedDelaySamples(checkedRender.sampleRate,
                                                                             checkedRender.tempoBpm,
                                                                             synth::DelaySyncDivision::Eighth);
            const auto wetPassed = dryReference.ok
                && checkedRender.passed
                && checkedRender.fxTailSeconds > 0.5f
                && checkedRender.wetMeaningfulPassed
                && checkedRender.postLastEventSeconds >= checkedRender.fxTailSeconds
                && std::abs(checkedRender.tempoSyncedDelaySamples - expectedDelaySamples) <= 1;
            addItem("pluck-core-01-wet", reportPath, wetPassed ? 0 : 1);
        }
    }

    {
        const auto reportPath = outputDir / "lfo-ablation.json";
        addItem("lfo-ablation", reportPath, writeLfoAblationReport(reportPath, artifactDir, presetPath, fixturePath));
    }

    {
        const auto reportPath = outputDir / "determinism.json";
        addItem("determinism", reportPath, writeDeterminismReport(reportPath, failureDir, presetPath, fixturePath));
    }

    const auto summaryPath = outputDir / "summary.json";
    writeCoreSummary(summaryPath, outputDir, items);

    const auto failed = std::count_if(items.begin(), items.end(), [](const auto& item) {
        return !item.passed;
    });

    std::cout << "Core suite wrote " << items.size() << " reports to " << outputDir << ".\n";
    return failed == 0 ? 0 : 1;
}
} // namespace

int main(int argc, char* argv[])
{
    const auto options = parseOptions(argc, argv);

    if (!options.suite.empty())
        return writeCoreSuite(options);

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

    if (options.oscTest)
    {
        const auto exitCode = writeOscillatorReport(options.output, options.notes);
        std::cout << "Oscillator validation " << (exitCode == 0 ? "passed." : "failed.") << "\n";
        return exitCode;
    }

    if (options.filterTest)
    {
        const auto exitCode = writeFilterReport(options.output);
        std::cout << "Filter validation " << (exitCode == 0 ? "passed." : "failed.") << "\n";
        return exitCode;
    }

    if (options.modulationTest)
    {
        const auto exitCode = writeModulationReport(options);
        std::cout << "Modulation validation " << (exitCode == 0 ? "passed." : "failed.") << "\n";
        return exitCode;
    }

    if (options.presetRender)
    {
        const auto exitCode = renderPreset(options);
        std::cout << "Preset render " << (exitCode == 0 ? "passed." : "failed.") << "\n";
        return exitCode;
    }

    if (!options.smoke)
    {
        std::cerr << "No render mode selected. Use --help for modes.\n";
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
