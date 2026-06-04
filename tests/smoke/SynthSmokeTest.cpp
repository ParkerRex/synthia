#include "../../src/dsp/SynthEngine.h"

#include <cmath>
#include <iostream>
#include <vector>

int main()
{
    synth::SynthEngine engine;
    engine.prepare(44100.0, 128);

    if (engine.getSampleRate() != 44100.0)
    {
        std::cerr << "Unexpected sample rate.\n";
        return 1;
    }

    std::vector<float> left(512, 1.0f);
    std::vector<float> right(512, -1.0f);
    const auto stats = engine.process(left.data(), right.data(), static_cast<int>(left.size()));

    if (stats.samplesRendered != 512)
    {
        std::cerr << "Unexpected rendered sample count.\n";
        return 1;
    }

    if (stats.invalidSamples != 0)
    {
        std::cerr << "Invalid samples detected.\n";
        return 1;
    }

    if (stats.peak != 0.0f)
    {
        std::cerr << "Silent foundation engine produced non-zero output.\n";
        return 1;
    }

    for (float sample : left)
    {
        if (sample != 0.0f || !std::isfinite(sample))
        {
            std::cerr << "Left buffer was not cleared.\n";
            return 1;
        }
    }

    for (float sample : right)
    {
        if (sample != 0.0f || !std::isfinite(sample))
        {
            std::cerr << "Right buffer was not cleared.\n";
            return 1;
        }
    }

    return 0;
}

