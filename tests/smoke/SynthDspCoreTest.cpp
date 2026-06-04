#include "../../src/dsp/Filter.h"
#include "../../src/dsp/OscillatorStack.h"
#include "../../src/dsp/SynthEngine.h"

#include <cmath>
#include <iostream>
#include <vector>

namespace
{
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

bool testOscillatorTuning()
{
    synth::OscillatorStack oscillator;
    oscillator.prepare(48000.0);
    oscillator.reset(0.0f);

    synth::SynthParameters parameters;
    parameters.osc.sawLevel = 1.0f;
    parameters.osc.pulseLevel = 0.0f;
    parameters.osc.subLevel = 0.0f;
    parameters.osc.noiseLevel = 0.0f;
    parameters.amp.analog = 0.0f;

    std::vector<float> samples(48000);
    for (auto& sample : samples)
        sample = oscillator.renderSample(60, parameters, 0.0f, 0.0f);

    const auto frequency = estimateFrequency(samples, 48000.0);
    return std::abs(centsBetween(frequency, synth::midiNoteToHz(60.0f))) < 5.0f;
}

bool testPulseWidth()
{
    for (const auto width : { 0.10f, 0.25f, 0.50f, 0.75f, 0.90f })
    {
        synth::OscillatorStack oscillator;
        oscillator.prepare(48000.0);
        oscillator.reset(0.0f);

        synth::SynthParameters parameters;
        parameters.osc.sawLevel = 0.0f;
        parameters.osc.pulseLevel = 1.0f;
        parameters.osc.pulseWidth = width;
        parameters.osc.subLevel = 0.0f;

        int positive = 0;
        constexpr int sampleCount = 48000;
        for (int i = 0; i < sampleCount; ++i)
        {
            if (oscillator.renderSample(48, parameters, 0.0f, 0.0f) > 0.0f)
                ++positive;
        }

        const auto duty = static_cast<float>(positive) / static_cast<float>(sampleCount);
        if (std::abs(duty - width) >= 0.04f)
            return false;
    }

    return true;
}

bool testSubOctave()
{
    for (int octave = 1; octave <= 3; ++octave)
    {
        synth::OscillatorStack oscillator;
        oscillator.prepare(48000.0);
        oscillator.reset(0.0f);

        synth::SynthParameters parameters;
        parameters.osc.sawLevel = 0.0f;
        parameters.osc.pulseLevel = 0.0f;
        parameters.osc.subLevel = 1.0f;
        parameters.osc.subWave = synth::SubWave::Saw;
        parameters.osc.subOctave = octave;

        std::vector<float> samples(48000);
        for (auto& sample : samples)
            sample = oscillator.renderSample(60, parameters, 0.0f, 0.0f);

        const auto frequency = estimateFrequency(samples, 48000.0);
        if (std::abs(centsBetween(frequency, synth::midiNoteToHz(static_cast<float>(60 - octave * 12)))) >= 5.0f)
            return false;
    }

    return true;
}

bool testOscillatorDetuneNoiseAndSync()
{
    for (int count = 1; count <= 5; ++count)
    {
        for (int index = 0; index < count; ++index)
        {
            const auto opposite = count - 1 - index;
            const auto sum = synth::OscillatorStack::detuneOffsetCents(index, count, 0.7f)
                + synth::OscillatorStack::detuneOffsetCents(opposite, count, 0.7f);
            if (std::abs(sum) > 0.0001f)
                return false;
        }
    }

    synth::OscillatorStack noiseA;
    synth::OscillatorStack noiseB;
    noiseA.prepare(48000.0);
    noiseB.prepare(48000.0);
    noiseA.reset(0.25f);
    noiseB.reset(0.25f);
    synth::SynthParameters noiseParameters;
    noiseParameters.osc.sawLevel = 0.0f;
    noiseParameters.osc.pulseLevel = 0.0f;
    noiseParameters.osc.subLevel = 0.0f;
    noiseParameters.osc.noiseLevel = 1.0f;
    for (int i = 0; i < 512; ++i)
    {
        if (std::abs(noiseA.renderSample(60, noiseParameters, 0.0f, 0.0f)
            - noiseB.renderSample(60, noiseParameters, 0.0f, 0.0f)) >= 1.0e-7f)
        {
            return false;
        }
    }

    synth::OscillatorStack sync;
    sync.prepare(48000.0);
    sync.reset(0.0f);
    synth::SynthParameters syncParameters;
    syncParameters.osc.syncAmount = 1.0f;
    for (int i = 0; i < 4096; ++i)
    {
        const auto sample = sync.renderSample(96, syncParameters, 0.0f, 0.0f);
        if (!std::isfinite(sample) || std::abs(sample) > 1.21f)
            return false;
    }

    return true;
}

bool testFilterMappingAndStability()
{
    if (std::abs(synth::Filter::cutoffSemitonesToHz(69.0f) - 440.0f) > 0.1f)
        return false;

    synth::SynthParameters parameters;
    parameters.filter.cutoffSemitones = 72.0f;
    parameters.filter.resonance = 0.85f;
    parameters.filter.drive = 0.6f;
    parameters.filter.oversampling = 2;

    synth::Filter filter;
    filter.prepare(48000.0);

    auto previousOutput = 0.0f;
    for (int i = 0; i < 48000; ++i)
    {
        const auto input = i == 0 ? 1.0f : 0.0f;
        const auto output = filter.process(input, 60, parameters, 0.0f);
        if (!std::isfinite(output) || std::abs(output) > 4.0f)
            return false;
        previousOutput = output;
    }

    return std::isfinite(previousOutput);
}

bool testEngineProducesAudio()
{
    synth::SynthEngine engine;
    engine.prepare(48000.0, 128);
    engine.noteOn(60, 1.0f);

    std::vector<float> left(4096);
    std::vector<float> right(4096);
    const auto stats = engine.process(left.data(), right.data(), static_cast<int>(left.size()));
    return stats.activeVoices == 1 && stats.invalidSamples == 0 && stats.peak > 0.001f;
}
} // namespace

int main()
{
    if (!testOscillatorTuning())
    {
        std::cerr << "Oscillator tuning test failed.\n";
        return 1;
    }

    if (!testPulseWidth())
    {
        std::cerr << "Pulse width test failed.\n";
        return 1;
    }

    if (!testSubOctave())
    {
        std::cerr << "Sub octave test failed.\n";
        return 1;
    }

    if (!testOscillatorDetuneNoiseAndSync())
    {
        std::cerr << "Oscillator detune/noise/sync test failed.\n";
        return 1;
    }

    if (!testFilterMappingAndStability())
    {
        std::cerr << "Filter mapping/stability test failed.\n";
        return 1;
    }

    if (!testEngineProducesAudio())
    {
        std::cerr << "Engine audio test failed.\n";
        return 1;
    }

    return 0;
}
