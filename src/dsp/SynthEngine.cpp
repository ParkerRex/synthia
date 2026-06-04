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
    voices.noteOn(midiNote, velocity, parameters);
}

void SynthEngine::noteOff(int midiNote) noexcept
{
    voices.noteOff(midiNote);
}

void SynthEngine::setSustainPedal(bool down) noexcept
{
    voices.setSustainPedal(down);
}

void SynthEngine::allNotesOff() noexcept
{
    voices.allNotesOff();
}

void SynthEngine::panic() noexcept
{
    voices.panic();
}

void SynthEngine::setParameters(const SynthParameters& newParameters) noexcept
{
    parameters = newParameters;
    parameters.polyphony = std::clamp(parameters.polyphony, 1, 32);
    parameters.unisonCount = std::clamp(parameters.unisonCount, 1, 8);
    parameters.osc.stackCount = std::clamp(parameters.osc.stackCount, 1, 5);
    parameters.osc.stackDetune = std::clamp(parameters.osc.stackDetune, 0.0f, 1.0f);
    parameters.osc.pulseWidth = std::clamp(parameters.osc.pulseWidth, 0.05f, 0.95f);
    parameters.osc.subPulseWidth = std::clamp(parameters.osc.subPulseWidth, 0.05f, 0.95f);
    parameters.osc.subOctave = std::clamp(parameters.osc.subOctave, 1, 3);
    parameters.filter.cutoffSemitones = std::clamp(parameters.filter.cutoffSemitones, 0.0f, 136.0f);
    parameters.filter.resonance = std::clamp(parameters.filter.resonance, 0.0f, 1.0f);
    parameters.filter.drive = std::clamp(parameters.filter.drive, 0.0f, 1.0f);
    parameters.filter.oversampling = std::clamp(parameters.filter.oversampling, 0, 3);
    parameters.filter.mode = static_cast<FilterMode>(std::clamp(static_cast<int>(parameters.filter.mode), 0, 8));
    parameters.lfo.shape = static_cast<LfoShapeChoice>(std::clamp(static_cast<int>(parameters.lfo.shape), 0, 6));
    parameters.lfo.rateMode = static_cast<LfoRateMode>(std::clamp(static_cast<int>(parameters.lfo.rateMode), 0, 1));
    parameters.lfo.syncDivision = std::clamp(parameters.lfo.syncDivision, 0, 5);
    parameters.lfo.gateMode = static_cast<LfoGateMode>(std::clamp(static_cast<int>(parameters.lfo.gateMode), 0, 3));
    parameters.lfo.swing = std::clamp(parameters.lfo.swing, 0.0f, 1.0f);
    parameters.tempoBpm = std::clamp(parameters.tempoBpm, 20.0f, 300.0f);
    parameters.amp.levelDb = std::clamp(parameters.amp.levelDb, -48.0f, 12.0f);
}

RenderStats SynthEngine::process(float* left, float* right, int numSamples) noexcept
{
    RenderStats stats;
    stats.samplesRendered = std::max(0, numSamples);

    for (int i = 0; i < stats.samplesRendered; ++i)
    {
        const auto frame = voices.renderSample(parameters);
        auto l = frame.left;
        auto r = frame.right;

        if (!std::isfinite(l))
        {
            ++stats.invalidSamples;
            l = 0.0f;
        }

        if (!std::isfinite(r))
        {
            ++stats.invalidSamples;
            r = 0.0f;
        }

        if (left != nullptr)
            left[i] = l;

        if (right != nullptr)
            right[i] = r;

        stats.peak = std::max(stats.peak, std::max(std::abs(l), std::abs(r)));
    }

    stats.activeVoices = voices.activeVoiceCount();

    return stats;
}
} // namespace synth
