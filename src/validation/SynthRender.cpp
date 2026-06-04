#include "../dsp/Filter.h"
#include "../dsp/OscillatorStack.h"
#include "../dsp/SynthEngine.h"
#include "../plugin/ParameterRegistry.h"
#include "../presets/PresetValidator.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
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
    bool oscTest = false;
    bool filterTest = false;
    bool presetRender = false;
    bool dry = false;
    std::filesystem::path output = "build/reports/smoke.json";
    std::filesystem::path report = "build/reports/render.json";
    std::filesystem::path presetPath = "presets/factory";
    std::filesystem::path fixturePath = "fixtures/midi/overlap-pluck.mid";
    std::string notes = "C1,C3,C5,C7";
};

struct RenderEvent
{
    int sample = 0;
    juce::MidiMessage message;
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
            std::cout << "  SynthRender --preset <json> --fixture <path> --dry --output <wav> --report <json>\n";
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

void writeSmokeReport(const std::filesystem::path& path, const synth::RenderStats& stats)
{
    ensureParentDirectory(path);

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
            sample = oscillator.renderSample(notes[noteIndex], parameters, 0.0f, 0.0f);

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
            sample = oscillator.renderSample(notes[noteIndex], parameters, 0.0f, 0.0f);

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

void applyPresetValue(synth::SynthParameters& parameters, const std::string& id, const juce::var& value)
{
    const auto* spec = synth::findParameterSpec(id);
    if (spec == nullptr)
        return;

    const auto numeric = value.isBool() ? (static_cast<bool>(value) ? 1.0f : 0.0f)
        : (value.isInt() || value.isInt64() || value.isDouble() ? static_cast<float>(static_cast<double>(value))
                                                                : spec->defaultValue);
    const auto choice = spec->kind == synth::ParameterKind::Choice ? choiceIndex(*spec, value) : 0;

    if (id == "voice.polyphony") parameters.polyphony = static_cast<int>(std::round(numeric));
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
    else if (id == "direct.osc_lfo_semitones") parameters.direct.oscLfoSemitones = numeric;
    else if (id == "direct.osc_mod_env_semitones") parameters.direct.oscModEnvSemitones = numeric;
    else if (id == "direct.pulse_lfo") parameters.direct.pulseLfo = numeric;
    else if (id == "direct.pulse_mod_env") parameters.direct.pulseModEnv = numeric;
    else if (id == "macro.motion") parameters.macro.motion = numeric;
    else if (id == "macro.width") parameters.macro.width = numeric;
    else if (id == "macro.drive") parameters.macro.drive = numeric;
    else if (id == "macro.space") parameters.macro.space = numeric;
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
    double correlation = 0.0;
    int invalid = 0;
    int nonzero = 0;
};

AudioMetrics analyzeAudio(const std::vector<float>& left, const std::vector<float>& right)
{
    AudioMetrics metrics;
    double sumSquares = 0.0;
    double sumLeftSquares = 0.0;
    double sumRightSquares = 0.0;
    double sumCross = 0.0;

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
    }

    metrics.rms = sampleCount > 0 ? std::sqrt(sumSquares / static_cast<double>(sampleCount)) : 0.0;
    metrics.correlation = sumLeftSquares > 0.0 && sumRightSquares > 0.0
        ? sumCross / std::sqrt(sumLeftSquares * sumRightSquares)
        : 0.0;
    return metrics;
}

int renderPreset(const Options& options)
{
    constexpr auto sampleRate = 48000;
    synth::SynthParameters parameters;
    std::string error;
    if (!loadPresetParameters(options.presetPath, parameters, error))
    {
        std::cerr << error << "\n";
        return 1;
    }

    std::vector<RenderEvent> events;
    if (!loadMidiFixture(options.fixturePath, sampleRate, events, error))
    {
        std::cerr << error << "\n";
        return 1;
    }

    const auto lastEventSample = events.empty() ? 0 : events.back().sample;
    const auto sampleCount = std::max(static_cast<int>(sampleRate * 1.35), lastEventSample + static_cast<int>(sampleRate * 0.4));

    synth::SynthEngine engine;
    engine.prepare(sampleRate, 1);
    engine.setParameters(parameters);

    std::vector<float> left(static_cast<std::size_t>(sampleCount));
    std::vector<float> right(static_cast<std::size_t>(sampleCount));

    auto nextEvent = std::size_t { 0 };
    auto maxNoteLocalLfoSpread = 0.0f;
    for (int i = 0; i < sampleCount; ++i)
    {
        while (nextEvent < events.size() && events[nextEvent].sample == i)
        {
            const auto& message = events[nextEvent].message;
            if (message.isNoteOn(false))
                engine.noteOn(message.getNoteNumber(), message.getFloatVelocity());
            else if (message.isNoteOff(true))
                engine.noteOff(message.getNoteNumber());
            else if (message.isAllSoundOff())
                engine.panic();
            else if (message.isAllNotesOff())
                engine.allNotesOff();
            else if (message.isControllerOfType(64))
                engine.setSustainPedal(message.getControllerValue() >= 64);

            ++nextEvent;
        }

        auto stats = engine.process(&left[static_cast<std::size_t>(i)], &right[static_cast<std::size_t>(i)], 1);
        if (stats.invalidSamples > 0)
        {
            left[static_cast<std::size_t>(i)] = 0.0f;
            right[static_cast<std::size_t>(i)] = 0.0f;
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
            maxNoteLocalLfoSpread = std::max(maxNoteLocalLfoSpread, maxLfo - minLfo);
    }

    const auto metrics = analyzeAudio(left, right);
    writeWav16(options.output, left, right, sampleRate);
    ensureParentDirectory(options.report);

    const auto noteLocalMotionPassed = maxNoteLocalLfoSpread > 0.01f;
    const auto passed = metrics.invalid == 0 && metrics.peak < 1.0f && metrics.rms > 0.001
        && metrics.nonzero > sampleRate / 8 && noteLocalMotionPassed;
    std::ofstream out(options.report);
    out << "{\n";
    out << "  \"suite\": \"preset-dry-render\",\n";
    out << "  \"preset\": \"" << options.presetPath.string() << "\",\n";
    out << "  \"fixture\": \"" << options.fixturePath.string() << "\",\n";
    out << "  \"fixture_events\": " << events.size() << ",\n";
    out << "  \"dry\": " << (options.dry ? "true" : "false") << ",\n";
    out << "  \"sample_rate\": " << sampleRate << ",\n";
    out << "  \"samples_rendered\": " << sampleCount << ",\n";
    out << "  \"peak\": " << metrics.peak << ",\n";
    out << "  \"rms\": " << metrics.rms << ",\n";
    out << "  \"stereo_correlation\": " << metrics.correlation << ",\n";
    out << "  \"note_local_lfo_spread\": " << maxNoteLocalLfoSpread << ",\n";
    out << "  \"note_local_motion_passed\": " << (noteLocalMotionPassed ? "true" : "false") << ",\n";
    out << "  \"nonzero_samples\": " << metrics.nonzero << ",\n";
    out << "  \"invalid_samples\": " << metrics.invalid << ",\n";
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
