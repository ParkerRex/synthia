#include "SynthEngine.h"

#include <algorithm>
#include <cmath>

namespace synth
{
void SynthEngine::prepare(double newSampleRate, int newMaxBlockSize)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    maxBlockSize = std::max(1, newMaxBlockSize);
    voices.prepare(sampleRate);
    reset();
}

void SynthEngine::reset() noexcept
{
    voices.panic();
}

void SynthEngine::noteOn(int midiNote, float velocity) noexcept
{
    voices.noteOn(midiNote, velocity);
}

void SynthEngine::noteOff(int midiNote) noexcept
{
    voices.noteOff(midiNote);
}

void SynthEngine::allNotesOff() noexcept
{
    voices.allNotesOff();
}

void SynthEngine::panic() noexcept
{
    voices.panic();
}

RenderStats SynthEngine::process(float* left, float* right, int numSamples) noexcept
{
    RenderStats stats;
    stats.samplesRendered = std::max(0, numSamples);
    voices.process(stats.samplesRendered);
    stats.activeVoices = voices.activeVoiceCount();

    for (int i = 0; i < stats.samplesRendered; ++i)
    {
        if (left != nullptr)
            left[i] = 0.0f;

        if (right != nullptr)
            right[i] = 0.0f;
    }

    for (int i = 0; i < stats.samplesRendered; ++i)
    {
        const auto l = left != nullptr ? left[i] : 0.0f;
        const auto r = right != nullptr ? right[i] : 0.0f;

        if (!std::isfinite(l) || !std::isfinite(r))
            ++stats.invalidSamples;

        stats.peak = std::max(stats.peak, std::max(std::abs(l), std::abs(r)));
    }

    return stats;
}
} // namespace synth
