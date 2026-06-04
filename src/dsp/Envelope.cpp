#include "Envelope.h"

#include <algorithm>

namespace synth
{
void Envelope::prepare(double newSampleRate) noexcept
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    reset();
}

void Envelope::setSettings(EnvelopeSettings newSettings) noexcept
{
    newSettings.attackMs = std::max(0.0f, newSettings.attackMs);
    newSettings.decayMs = std::max(0.0f, newSettings.decayMs);
    newSettings.sustain = std::clamp(newSettings.sustain, 0.0f, 1.0f);
    newSettings.releaseMs = std::max(0.0f, newSettings.releaseMs);
    settings = newSettings;
}

void Envelope::reset() noexcept
{
    stage = EnvelopeStage::Idle;
    value = 0.0f;
    releaseStart = 0.0f;
    releaseSamplesElapsed = 0.0f;
}

void Envelope::noteOn() noexcept
{
    releaseSamplesElapsed = 0.0f;
    stage = settings.attackMs <= 0.0f ? EnvelopeStage::Decay : EnvelopeStage::Attack;
    if (settings.attackMs <= 0.0f)
        value = 1.0f;
}

void Envelope::noteOff() noexcept
{
    if (stage == EnvelopeStage::Idle)
        return;

    releaseStart = value;
    releaseSamplesElapsed = 0.0f;
    stage = settings.releaseMs <= 0.0f ? EnvelopeStage::Idle : EnvelopeStage::Release;
    if (stage == EnvelopeStage::Idle)
        value = 0.0f;
}

float Envelope::process() noexcept
{
    if (stage == EnvelopeStage::Idle)
        return value;

    if (stage == EnvelopeStage::Attack)
    {
        value += 1.0f / samplesForMs(settings.attackMs);
        if (value >= 1.0f)
        {
            value = 1.0f;
            stage = EnvelopeStage::Decay;
        }
    }
    else if (stage == EnvelopeStage::Decay)
    {
        if (settings.decayMs <= 0.0f)
        {
            value = settings.sustain;
            stage = EnvelopeStage::Sustain;
        }
        else
        {
            value -= (1.0f - settings.sustain) / samplesForMs(settings.decayMs);
            if (value <= settings.sustain)
            {
                value = settings.sustain;
                stage = EnvelopeStage::Sustain;
            }
        }
    }
    else if (stage == EnvelopeStage::Release)
    {
        releaseSamplesElapsed += 1.0f;
        const auto progress = std::min(1.0f, releaseSamplesElapsed / samplesForMs(settings.releaseMs));
        value = releaseStart * (1.0f - progress);
        if (progress >= 1.0f)
        {
            value = 0.0f;
            stage = EnvelopeStage::Idle;
        }
    }

    return value;
}

float Envelope::samplesForMs(float ms) const noexcept
{
    return std::max(1.0f, static_cast<float>(sampleRate) * ms / 1000.0f);
}
} // namespace synth

