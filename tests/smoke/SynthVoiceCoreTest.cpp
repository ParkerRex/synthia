#include "../../src/dsp/Envelope.h"
#include "../../src/dsp/Lfo.h"
#include "../../src/dsp/SynthEngine.h"
#include "../../src/voice/VoiceAllocator.h"

#include <cmath>
#include <iostream>
#include <vector>

namespace
{
bool testEnvelope()
{
    synth::Envelope env;
    env.prepare(1000.0);
    env.setSettings({ 10.0f, 10.0f, 0.5f, 10.0f });
    env.noteOn();

    for (int i = 0; i < 10; ++i)
        env.process();

    if (env.getValue() < 0.99f)
        return false;

    env.noteOff();
    for (int i = 0; i < 12; ++i)
        env.process();

    return !env.isActive() && env.getValue() == 0.0f;
}

bool testLfoReset()
{
    synth::Lfo lfo;
    lfo.prepare(1000.0);
    lfo.setShape(synth::LfoShape::SawDown);
    lfo.setRateHz(1.0f);
    lfo.resetPhase();
    const auto first = lfo.getValue();

    for (int i = 0; i < 50; ++i)
        lfo.process();

    lfo.resetPhase();
    return std::abs(lfo.getValue() - first) < 0.0001f;
}

bool testAllocator()
{
    synth::VoiceAllocator allocator(4);
    allocator.prepare(48000.0);
    synth::SynthParameters parameters;
    const auto first = allocator.noteOn(60, 1.0f, parameters);
    const auto second = allocator.noteOn(64, 0.5f, parameters);

    if (first == second || allocator.activeVoiceCount() != 2)
        return false;

    allocator.noteOff(60);
    allocator.process(48000);

    if (allocator.activeVoiceCount() != 1)
        return false;

    allocator.panic();
    return allocator.activeVoiceCount() == 0;
}

bool testUnisonAndSustain()
{
    synth::VoiceAllocator allocator(8);
    allocator.prepare(48000.0);
    synth::SynthParameters parameters;
    parameters.unisonCount = 2;
    parameters.polyphony = 2;

    allocator.noteOn(60, 1.0f, parameters);
    if (allocator.activeVoiceCount() != 2)
        return false;

    allocator.setSustainPedal(true);
    allocator.noteOff(60);
    allocator.process(48000);
    if (allocator.activeVoiceCount() != 2)
        return false;

    allocator.setSustainPedal(false);
    allocator.process(48000);
    return allocator.activeVoiceCount() == 0;
}

bool testEngine()
{
    synth::SynthEngine engine;
    engine.prepare(48000.0, 128);
    engine.noteOn(60, 1.0f);

    std::vector<float> left(128);
    std::vector<float> right(128);
    auto stats = engine.process(left.data(), right.data(), static_cast<int>(left.size()));

    if (stats.activeVoices != 1 || stats.invalidSamples != 0)
        return false;

    engine.noteOff(60);
    for (int i = 0; i < 200; ++i)
        stats = engine.process(left.data(), right.data(), static_cast<int>(left.size()));

    return stats.activeVoices == 0;
}
} // namespace

int main()
{
    if (!testEnvelope())
    {
        std::cerr << "Envelope test failed.\n";
        return 1;
    }

    if (!testLfoReset())
    {
        std::cerr << "LFO reset test failed.\n";
        return 1;
    }

    if (!testAllocator())
    {
        std::cerr << "Voice allocator test failed.\n";
        return 1;
    }

    if (!testUnisonAndSustain())
    {
        std::cerr << "Unison/sustain test failed.\n";
        return 1;
    }

    if (!testEngine())
    {
        std::cerr << "Engine voice test failed.\n";
        return 1;
    }

    return 0;
}
