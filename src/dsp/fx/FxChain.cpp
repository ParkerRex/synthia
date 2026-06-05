#include "FxChain.h"

#include <algorithm>
#include <cmath>

namespace synth
{
namespace
{
constexpr float twoPi = 6.28318530717958647692f;

float finiteOrZero(float value) noexcept
{
    return std::isfinite(value) ? value : 0.0f;
}

float mix(float dry, float wet, float amount) noexcept
{
    amount = clampUnit(amount);
    return dry + (wet - dry) * amount;
}

float saturate(float sample, float drive) noexcept
{
    const auto gain = 1.0f + clampUnit(drive) * 12.0f;
    const auto normalizer = std::tanh(gain);
    if (normalizer <= 0.0f)
        return sample;

    return std::tanh(sample * gain) / normalizer;
}

int secondsToSamples(double sampleRate, float seconds) noexcept
{
    return std::max(1, static_cast<int>(std::round(std::max(0.0f, seconds) * static_cast<float>(sampleRate))));
}
} // namespace

float delayDivisionBeats(DelaySyncDivision division) noexcept
{
    switch (division)
    {
        case DelaySyncDivision::Sixteenth: return 0.25f;
        case DelaySyncDivision::Eighth: return 0.5f;
        case DelaySyncDivision::DottedEighth: return 0.75f;
        case DelaySyncDivision::Quarter: return 1.0f;
        case DelaySyncDivision::Half: return 2.0f;
    }

    return 0.5f;
}

int tempoSyncedDelaySamples(double sampleRate, float tempoBpm, DelaySyncDivision division) noexcept
{
    const auto safeTempo = std::clamp(std::isfinite(tempoBpm) ? tempoBpm : 128.0f, 20.0f, 300.0f);
    const auto seconds = (60.0f / safeTempo) * delayDivisionBeats(division);
    return secondsToSamples(sampleRate, seconds);
}

float fxTailLengthSeconds(const FxParameters& parameters) noexcept
{
    if (!parameters.enabled)
        return 0.0f;

    const auto delayTail = parameters.delayEnabled && parameters.delayMix > 0.0f ? 1.5f : 0.0f;
    const auto reverbTail = parameters.reverbEnabled && parameters.reverbMix > 0.0f
        ? 0.7f + clampUnit(parameters.reverbDecay) * 1.8f
        : 0.0f;
    return std::max(delayTail, reverbTail);
}

void FxChain::DelayBuffer::prepare(int sampleCount)
{
    samples.assign(static_cast<std::size_t>(std::max(2, sampleCount)), 0.0f);
    writeIndex = 0;
}

void FxChain::DelayBuffer::reset() noexcept
{
    std::fill(samples.begin(), samples.end(), 0.0f);
    writeIndex = 0;
}

float FxChain::DelayBuffer::readAtWriteHead() const noexcept
{
    if (samples.empty())
        return 0.0f;

    return samples[static_cast<std::size_t>(writeIndex)];
}

float FxChain::DelayBuffer::read(int delaySamples) const noexcept
{
    const auto size = static_cast<int>(samples.size());
    if (size <= 1)
        return 0.0f;

    delaySamples = std::clamp(delaySamples, 1, size - 1);
    auto index = writeIndex - delaySamples;
    while (index < 0)
        index += size;

    return samples[static_cast<std::size_t>(index)];
}

float FxChain::DelayBuffer::readInterpolated(float delaySamples) const noexcept
{
    const auto size = static_cast<int>(samples.size());
    if (size <= 1)
        return 0.0f;

    delaySamples = std::clamp(delaySamples, 1.0f, static_cast<float>(size - 2));
    const auto floorDelay = static_cast<int>(std::floor(delaySamples));
    const auto fraction = delaySamples - static_cast<float>(floorDelay);
    const auto first = read(floorDelay);
    const auto second = read(floorDelay + 1);
    return first + (second - first) * fraction;
}

void FxChain::DelayBuffer::write(float value) noexcept
{
    if (samples.empty())
        return;

    samples[static_cast<std::size_t>(writeIndex)] = finiteOrZero(value);
    writeIndex = (writeIndex + 1) % static_cast<int>(samples.size());
}

void FxChain::prepare(double newSampleRate, int newMaxBlockSize)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    maxBlockSize = std::max(1, newMaxBlockSize);

    const auto delaySamples = secondsToSamples(sampleRate, 2.1f);
    delayLeft.prepare(delaySamples);
    delayRight.prepare(delaySamples);

    const auto chorusSamples = secondsToSamples(sampleRate, 0.08f);
    chorusLeft.prepare(chorusSamples);
    chorusRight.prepare(chorusSamples);

    const std::array<float, 8> combSeconds {
        0.0297f, 0.0371f, 0.0411f, 0.0437f,
        0.0311f, 0.0399f, 0.0449f, 0.0477f
    };

    for (std::size_t i = 0; i < reverbCombs.size(); ++i)
        reverbCombs[i].prepare(secondsToSamples(sampleRate, combSeconds[i]));

    reset();
}

void FxChain::reset() noexcept
{
    delayLeft.reset();
    delayRight.reset();
    chorusLeft.reset();
    chorusRight.reset();
    for (auto& comb : reverbCombs)
        comb.reset();
    reverbDampingStates.fill(0.0f);
    chorusPhase = 0.0f;
}

FxStereoFrame FxChain::process(FxStereoFrame input, const SynthParameters& parameters) noexcept
{
    input.left = finiteOrZero(input.left);
    input.right = finiteOrZero(input.right);

    if (!parameters.fx.enabled)
        return input;

    auto output = processSaturation(input, parameters);
    output = processChorus(output, parameters);
    output = processDelay(output, parameters);
    output = processReverb(output, parameters);
    output.left = std::clamp(finiteOrZero(output.left), -1.0f, 1.0f);
    output.right = std::clamp(finiteOrZero(output.right), -1.0f, 1.0f);
    return output;
}

FxStereoFrame FxChain::processSaturation(FxStereoFrame input, const SynthParameters& parameters) const noexcept
{
    if (!parameters.fx.saturationEnabled || parameters.fx.saturationMix <= 0.0f)
        return input;

    const auto drive = clampUnit(parameters.fx.saturationDrive + parameters.macro.drive * 0.25f);
    return {
        mix(input.left, saturate(input.left, drive), parameters.fx.saturationMix),
        mix(input.right, saturate(input.right, drive), parameters.fx.saturationMix)
    };
}

FxStereoFrame FxChain::processChorus(FxStereoFrame input, const SynthParameters& parameters) noexcept
{
    if (!parameters.fx.chorusEnabled || parameters.fx.chorusMix <= 0.0f)
    {
        chorusLeft.write(input.left);
        chorusRight.write(input.right);
        return input;
    }

    const auto rateHz = std::clamp(parameters.fx.chorusRateHz, 0.02f, 8.0f);
    const auto phaseStep = rateHz / static_cast<float>(sampleRate);
    chorusPhase += phaseStep;
    if (chorusPhase >= 1.0f)
        chorusPhase -= std::floor(chorusPhase);

    const auto depthMs = std::clamp(parameters.fx.chorusDepthMs, 0.1f, 24.0f);
    const auto baseMs = parameters.quality.activeMode == QualityMode::Eco ? 8.0f : 11.0f;
    const auto phaseRadians = chorusPhase * twoPi;
    const auto leftDelay = (baseMs + std::sin(phaseRadians) * depthMs) * 0.001f * static_cast<float>(sampleRate);
    const auto rightDelay = (baseMs + std::sin(phaseRadians + 2.09439510239f) * depthMs) * 0.001f * static_cast<float>(sampleRate);
    const auto wetLeft = chorusLeft.readInterpolated(leftDelay);
    const auto wetRight = chorusRight.readInterpolated(rightDelay);

    chorusLeft.write(input.left);
    chorusRight.write(input.right);

    return {
        mix(input.left, wetLeft, parameters.fx.chorusMix),
        mix(input.right, wetRight, parameters.fx.chorusMix)
    };
}

FxStereoFrame FxChain::processDelay(FxStereoFrame input, const SynthParameters& parameters) noexcept
{
    if (!parameters.fx.delayEnabled || parameters.fx.delayMix <= 0.0f)
    {
        delayLeft.write(input.left);
        delayRight.write(input.right);
        return input;
    }

    const auto delaySamples = tempoSyncedDelaySamples(sampleRate, parameters.tempoBpm, parameters.fx.delaySyncDivision);
    const auto delayedLeft = delayLeft.read(delaySamples);
    const auto delayedRight = delayRight.read(delaySamples);
    const auto feedback = std::clamp(parameters.fx.delayFeedback, 0.0f, 0.86f);
    const auto delayMix = clampUnit(parameters.fx.delayMix + parameters.macro.space * 0.35f);

    delayLeft.write(input.left + delayedRight * feedback);
    delayRight.write(input.right + delayedLeft * feedback);

    return {
        std::clamp(input.left + delayedLeft * delayMix, -1.0f, 1.0f),
        std::clamp(input.right + delayedRight * delayMix, -1.0f, 1.0f)
    };
}

FxStereoFrame FxChain::processReverb(FxStereoFrame input, const SynthParameters& parameters) noexcept
{
    if (!parameters.fx.reverbEnabled || parameters.fx.reverbMix <= 0.0f)
    {
        for (int i = 0; i < reverbCombCount(parameters.quality.activeMode) * 2; ++i)
            processComb(i, 0.0f, 0.0f, 0.0f);
        return input;
    }

    const auto decay = clampUnit(parameters.fx.reverbDecay);
    const auto feedback = 0.52f + decay * 0.34f;
    const auto damping = parameters.quality.activeMode == QualityMode::High ? 0.34f : 0.48f;
    const auto combCount = reverbCombCount(parameters.quality.activeMode);

    auto wetLeft = 0.0f;
    auto wetRight = 0.0f;
    const auto monoInput = 0.5f * (input.left + input.right);
    for (int i = 0; i < combCount; ++i)
    {
        wetLeft += processComb(i, monoInput + input.left * 0.35f, feedback, damping);
        wetRight += processComb(i + 4, monoInput + input.right * 0.35f, feedback, damping);
    }

    const auto reverbGain = 0.33f / static_cast<float>(std::max(1, combCount));
    wetLeft *= reverbGain;
    wetRight *= reverbGain;

    const auto reverbMix = clampUnit(parameters.fx.reverbMix + parameters.macro.space * 0.45f);
    return {
        std::clamp(input.left + wetLeft * reverbMix, -1.0f, 1.0f),
        std::clamp(input.right + wetRight * reverbMix, -1.0f, 1.0f)
    };
}

float FxChain::processComb(int index, float input, float feedback, float damping) noexcept
{
    if (index < 0 || index >= static_cast<int>(reverbCombs.size()))
        return 0.0f;

    auto& comb = reverbCombs[static_cast<std::size_t>(index)];
    auto& dampingState = reverbDampingStates[static_cast<std::size_t>(index)];
    const auto delayed = comb.readAtWriteHead();
    dampingState = delayed * (1.0f - damping) + dampingState * damping;
    comb.write(input + dampingState * feedback);
    return delayed;
}

int FxChain::reverbCombCount(QualityMode mode) const noexcept
{
    switch (mode)
    {
        case QualityMode::Eco: return 2;
        case QualityMode::Normal: return 3;
        case QualityMode::High: return 4;
    }

    return 3;
}
} // namespace synth
