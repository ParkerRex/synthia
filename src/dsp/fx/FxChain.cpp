#include "FxChain.h"

#include <algorithm>
#include <cmath>

namespace synth
{
namespace
{
constexpr float twoPi = 6.28318530717958647692f;
constexpr float defaultTempoBpm = 128.0f;
constexpr float minTempoBpm = 20.0f;
constexpr float maxTempoBpm = 300.0f;
constexpr float maxTempoSyncedDelaySeconds = 6.0f;
constexpr float delayTailFloor = 0.001f;

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
    const auto normalizer = softSaturate(gain);
    if (normalizer <= 0.0f)
        return sample;

    return softSaturate(sample * gain) / normalizer;
}

float clipDistort(float sample, float drive) noexcept
{
    const auto gain = 1.0f + clampUnit(drive) * 10.0f;
    return std::clamp(sample * gain, -1.0f, 1.0f);
}

float foldDistort(float sample, float drive) noexcept
{
    auto folded = sample * (1.0f + clampUnit(drive) * 14.0f) + 1.0f;
    const auto wraps = static_cast<int>(folded * 0.25f);
    folded -= static_cast<float>(wraps) * 4.0f;
    if (folded < 0.0f)
        folded += 4.0f;
    else if (folded >= 4.0f)
        folded -= 4.0f;
    return 1.0f - std::abs(folded - 2.0f);
}

float dbToGain(float db) noexcept
{
    return decibelsToGain(std::clamp(db, -48.0f, 24.0f));
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
    return secondsToSamples(sampleRate, tempoSyncedDelaySeconds(tempoBpm, division));
}

float tempoSyncedDelaySeconds(float tempoBpm, DelaySyncDivision division) noexcept
{
    const auto safeTempo = std::clamp(std::isfinite(tempoBpm) ? tempoBpm : defaultTempoBpm,
                                      minTempoBpm, maxTempoBpm);
    return (60.0f / safeTempo) * delayDivisionBeats(division);
}

float fxTailLengthSeconds(const FxParameters& parameters) noexcept
{
    SynthParameters fullParameters;
    fullParameters.fx = parameters;
    return fxTailLengthSeconds(fullParameters);
}

float fxTailLengthSeconds(const SynthParameters& parameters) noexcept
{
    if (!parameters.fx.enabled)
        return 0.0f;

    auto delayTail = 0.0f;
    const auto delayMix = clampUnit(parameters.fx.delayMix + parameters.macro.space * 0.35f);
    if (parameters.fx.delayEnabled && delayMix > 0.0f)
    {
        const auto delaySeconds = tempoSyncedDelaySeconds(parameters.tempoBpm, parameters.fx.delaySyncDivision);
        const auto feedback = std::clamp(parameters.fx.delayFeedback, 0.0f, 0.86f);
        if (feedback <= 0.0f)
        {
            delayTail = delaySeconds;
        }
        else
        {
            const auto repeatCount = std::max(1.0f, std::ceil(std::log(delayTailFloor) / std::log(feedback)));
            delayTail = delaySeconds * repeatCount;
        }
    }

    const auto reverbMix = clampUnit(parameters.fx.reverbMix + parameters.macro.space * 0.45f);
    const auto reverbTail = parameters.fx.reverbEnabled && reverbMix > 0.0f
        ? 0.7f + clampUnit(parameters.fx.reverbDecay) * 1.8f
        : 0.0f;
    return std::max(delayTail, reverbTail);
}

void FxChain::DelayBuffer::prepare(int sampleCount)
{
    samples.assign(static_cast<std::size_t>(std::max(2, sampleCount)), 0.0f);
    writeIndex = 0;
    validSamples = 0;
}

void FxChain::DelayBuffer::reset() noexcept
{
    writeIndex = 0;
    validSamples = 0;
}

float FxChain::DelayBuffer::readAtWriteHead() const noexcept
{
    if (samples.empty())
        return 0.0f;

    if (validSamples < static_cast<int>(samples.size()))
        return 0.0f;

    return samples[static_cast<std::size_t>(writeIndex)];
}

float FxChain::DelayBuffer::read(int delaySamples) const noexcept
{
    const auto size = static_cast<int>(samples.size());
    if (size <= 1)
        return 0.0f;

    delaySamples = std::clamp(delaySamples, 1, size - 1);
    if (delaySamples > validSamples)
        return 0.0f;

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
    validSamples = std::min(validSamples + 1, static_cast<int>(samples.size()));
}

void FxChain::prepare(double newSampleRate, int newMaxBlockSize)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    maxBlockSize = std::max(1, newMaxBlockSize);
    compressorAttackCoefficient = 1.0f / (0.004f * static_cast<float>(sampleRate));
    compressorReleaseCoefficient = 1.0f / (0.080f * static_cast<float>(sampleRate));

    const auto delaySamples = secondsToSamples(sampleRate, maxTempoSyncedDelaySeconds) + 1;
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
    resetPhaser();
    eqLowStateLeft = 0.0f;
    eqLowStateRight = 0.0f;
    compressorEnvelope = 0.0f;
    chorusPhase = 0.0f;
    reverbWasActive = false;
    lastReverbCombCount = 0;
}

FxStereoFrame FxChain::process(FxStereoFrame input, const SynthParameters& parameters) noexcept
{
    input.left = finiteOrZero(input.left);
    input.right = finiteOrZero(input.right);

    if (!parameters.fx.enabled)
    {
        if (phaserWasActive)
            resetPhaser();
        return input;
    }

    auto output = processSaturation(input, parameters);
    output = processPhaser(output, parameters);
    output = processChorus(output, parameters);
    output = processEq(output, parameters);
    output = processDelay(output, parameters);
    output = processReverb(output, parameters);
    output = processCompressor(output, parameters);
    output.left = std::clamp(finiteOrZero(output.left), -1.0f, 1.0f);
    output.right = std::clamp(finiteOrZero(output.right), -1.0f, 1.0f);
    return output;
}

FxStereoFrame FxChain::processSaturation(FxStereoFrame input, const SynthParameters& parameters) const noexcept
{
    if (!parameters.fx.saturationEnabled || parameters.fx.saturationMix <= 0.0f)
        return input;

    const auto drive = clampUnit(parameters.fx.saturationDrive + parameters.macro.drive * 0.25f);
    auto distort = [mode = parameters.fx.distortionMode, drive](float sample) noexcept {
        switch (mode)
        {
            case DistortionMode::Soft: return saturate(sample, drive);
            case DistortionMode::Clip: return clipDistort(sample, drive);
            case DistortionMode::Fold: return foldDistort(sample, drive);
        }
        return saturate(sample, drive);
    };

    return {
        mix(input.left, distort(input.left), parameters.fx.saturationMix),
        mix(input.right, distort(input.right), parameters.fx.saturationMix)
    };
}

FxStereoFrame FxChain::processPhaser(FxStereoFrame input, const SynthParameters& parameters) noexcept
{
    if (!parameters.fx.phaserEnabled || parameters.fx.phaserMix <= 0.0f)
    {
        if (phaserWasActive)
            resetPhaser();
        return input;
    }

    phaserWasActive = true;
    const auto rateHz = std::clamp(parameters.fx.phaserRateHz, 0.02f, 8.0f);
    phaserPhase += rateHz / static_cast<float>(sampleRate);
    if (phaserPhase >= 1.0f)
        phaserPhase -= std::floor(phaserPhase);

    const auto sweep = 0.5f + 0.5f * std::sin(phaserPhase * twoPi);
    const auto depth = clampUnit(parameters.fx.phaserDepth);
    const auto coefficient = std::clamp(0.18f + sweep * (0.72f * depth), 0.05f, 0.95f);
    const auto feedbackAmount = std::clamp(parameters.fx.phaserFeedback, 0.0f, 0.95f) * 0.85f;
    const auto wetLeft = processPhaserChannel(input.left, phaserStagesLeft, phaserFeedbackLeft,
                                              coefficient, feedbackAmount);
    const auto wetRight = processPhaserChannel(input.right, phaserStagesRight, phaserFeedbackRight,
                                               coefficient * 0.97f, feedbackAmount);
    return {
        mix(input.left, wetLeft, parameters.fx.phaserMix),
        mix(input.right, wetRight, parameters.fx.phaserMix)
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

FxStereoFrame FxChain::processEq(FxStereoFrame input, const SynthParameters& parameters) noexcept
{
    if (!parameters.fx.eqEnabled)
    {
        eqLowStateLeft = input.left;
        eqLowStateRight = input.right;
        return input;
    }

    return {
        processEqChannel(input.left, eqLowStateLeft, parameters.fx.eqLowGainDb, parameters.fx.eqHighGainDb),
        processEqChannel(input.right, eqLowStateRight, parameters.fx.eqLowGainDb, parameters.fx.eqHighGainDb)
    };
}

FxStereoFrame FxChain::processDelay(FxStereoFrame input, const SynthParameters& parameters) noexcept
{
    const auto delayMix = clampUnit(parameters.fx.delayMix + parameters.macro.space * 0.35f);
    if (!parameters.fx.delayEnabled || delayMix <= 0.0f)
    {
        delayLeft.write(input.left);
        delayRight.write(input.right);
        return input;
    }

    const auto delaySamples = tempoSyncedDelaySamples(sampleRate, parameters.tempoBpm, parameters.fx.delaySyncDivision);
    const auto delayedLeft = delayLeft.read(delaySamples);
    const auto delayedRight = delayRight.read(delaySamples);
    const auto feedback = std::clamp(parameters.fx.delayFeedback, 0.0f, 0.86f);

    delayLeft.write(input.left + delayedRight * feedback);
    delayRight.write(input.right + delayedLeft * feedback);

    return {
        std::clamp(input.left + delayedLeft * delayMix, -1.0f, 1.0f),
        std::clamp(input.right + delayedRight * delayMix, -1.0f, 1.0f)
    };
}

FxStereoFrame FxChain::processReverb(FxStereoFrame input, const SynthParameters& parameters) noexcept
{
    const auto reverbMix = clampUnit(parameters.fx.reverbMix + parameters.macro.space * 0.45f);
    if (!parameters.fx.reverbEnabled || reverbMix <= 0.0f)
    {
        if (reverbWasActive)
            resetReverb();
        return input;
    }

    const auto decay = clampUnit(parameters.fx.reverbDecay);
    const auto feedback = 0.52f + decay * 0.34f;
    const auto damping = parameters.quality.activeMode == QualityMode::High ? 0.34f : 0.48f;
    const auto combCount = reverbCombCount(parameters.quality.activeMode);
    if (reverbWasActive && combCount != lastReverbCombCount)
        resetReverb();
    reverbWasActive = true;
    lastReverbCombCount = combCount;

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

    return {
        std::clamp(input.left + wetLeft * reverbMix, -1.0f, 1.0f),
        std::clamp(input.right + wetRight * reverbMix, -1.0f, 1.0f)
    };
}

FxStereoFrame FxChain::processCompressor(FxStereoFrame input, const SynthParameters& parameters) noexcept
{
    if (!parameters.fx.compressorEnabled || parameters.fx.compressorMix <= 0.0f)
        return input;

    const auto peak = std::max(std::abs(input.left), std::abs(input.right));
    const auto coefficient = peak > compressorEnvelope ? compressorAttackCoefficient : compressorReleaseCoefficient;
    compressorEnvelope += (peak - compressorEnvelope) * coefficient;

    const auto safeEnvelope = std::max(compressorEnvelope, 1.0e-6f);
    const auto envelopeDb = gainToDecibels(safeEnvelope);
    const auto thresholdDb = std::clamp(parameters.fx.compressorThresholdDb, -36.0f, 0.0f);
    const auto ratio = std::clamp(parameters.fx.compressorRatio, 1.0f, 8.0f);
    auto gainDb = 0.0f;
    if (envelopeDb > thresholdDb)
    {
        const auto compressedDb = thresholdDb + (envelopeDb - thresholdDb) / ratio;
        gainDb = compressedDb - envelopeDb;
    }

    const auto wetGain = dbToGain(gainDb + parameters.fx.compressorMakeupDb);
    const auto wetLeft = input.left * wetGain;
    const auto wetRight = input.right * wetGain;
    return {
        mix(input.left, wetLeft, parameters.fx.compressorMix),
        mix(input.right, wetRight, parameters.fx.compressorMix)
    };
}

float FxChain::processPhaserChannel(float input,
                                    std::array<float, 4>& stages,
                                    float& feedback,
                                    float coefficient,
                                    float feedbackAmount) noexcept
{
    auto stageInput = input + feedback * feedbackAmount;
    for (auto& state : stages)
    {
        const auto output = -coefficient * stageInput + state;
        state = stageInput + coefficient * output;
        stageInput = output;
    }
    feedback = std::clamp(stageInput, -1.0f, 1.0f);
    return stageInput;
}

float FxChain::processEqChannel(float input, float& lowState, float lowGainDb, float highGainDb) const noexcept
{
    const auto coefficient = std::clamp(160.0f / static_cast<float>(sampleRate), 0.001f, 0.2f);
    lowState += (input - lowState) * coefficient;
    const auto high = input - lowState;
    const auto lowGain = dbToGain(std::clamp(lowGainDb, -12.0f, 12.0f));
    const auto highGain = dbToGain(std::clamp(highGainDb, -12.0f, 12.0f));
    return std::clamp(input + (lowGain - 1.0f) * lowState + (highGain - 1.0f) * high, -1.0f, 1.0f);
}

void FxChain::resetPhaser() noexcept
{
    phaserStagesLeft.fill(0.0f);
    phaserStagesRight.fill(0.0f);
    phaserFeedbackLeft = 0.0f;
    phaserFeedbackRight = 0.0f;
    phaserPhase = 0.0f;
    phaserWasActive = false;
}

void FxChain::resetReverb() noexcept
{
    for (auto& comb : reverbCombs)
        comb.reset();
    reverbDampingStates.fill(0.0f);
    reverbWasActive = false;
    lastReverbCombCount = 0;
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
