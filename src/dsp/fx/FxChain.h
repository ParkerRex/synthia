#pragma once

#include "../SynthParameters.h"

#include <array>
#include <vector>

namespace synth
{
struct FxStereoFrame
{
    float left = 0.0f;
    float right = 0.0f;
};

float delayDivisionBeats(DelaySyncDivision division) noexcept;
int tempoSyncedDelaySamples(double sampleRate, float tempoBpm, DelaySyncDivision division) noexcept;
float tempoSyncedDelaySeconds(float tempoBpm, DelaySyncDivision division) noexcept;
float fxTailLengthSeconds(const FxParameters& parameters) noexcept;
float fxTailLengthSeconds(const SynthParameters& parameters) noexcept;

class FxChain
{
public:
    void prepare(double newSampleRate, int newMaxBlockSize);
    void reset() noexcept;
    FxStereoFrame process(FxStereoFrame input, const SynthParameters& parameters) noexcept;

private:
    struct DelayBuffer
    {
        std::vector<float> samples;
        int writeIndex = 0;
        int validSamples = 0;

        void prepare(int sampleCount);
        void reset() noexcept;
        float readAtWriteHead() const noexcept;
        float read(int delaySamples) const noexcept;
        float readInterpolated(float delaySamples) const noexcept;
        void write(float value) noexcept;
    };

    FxStereoFrame processSaturation(FxStereoFrame input, const SynthParameters& parameters) const noexcept;
    FxStereoFrame processPhaser(FxStereoFrame input, const SynthParameters& parameters) noexcept;
    FxStereoFrame processChorus(FxStereoFrame input, const SynthParameters& parameters) noexcept;
    FxStereoFrame processEq(FxStereoFrame input, const SynthParameters& parameters) noexcept;
    FxStereoFrame processDelay(FxStereoFrame input, const SynthParameters& parameters) noexcept;
    FxStereoFrame processReverb(FxStereoFrame input, const SynthParameters& parameters) noexcept;
    FxStereoFrame processCompressor(FxStereoFrame input, const SynthParameters& parameters) noexcept;
    float processPhaserChannel(float input,
                               std::array<float, 4>& stages,
                               float& feedback,
                               float coefficient,
                               float feedbackAmount) noexcept;
    float processEqChannel(float input, float& lowState, float lowGainDb, float highGainDb) const noexcept;
    void resetPhaser() noexcept;
    void resetReverb() noexcept;
    float processComb(int index, float input, float feedback, float damping) noexcept;
    int reverbCombCount(QualityMode mode) const noexcept;

    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    DelayBuffer delayLeft;
    DelayBuffer delayRight;
    DelayBuffer chorusLeft;
    DelayBuffer chorusRight;
    std::array<DelayBuffer, 8> reverbCombs;
    std::array<float, 8> reverbDampingStates {};
    std::array<float, 4> phaserStagesLeft {};
    std::array<float, 4> phaserStagesRight {};
    float phaserFeedbackLeft = 0.0f;
    float phaserFeedbackRight = 0.0f;
    float phaserPhase = 0.0f;
    bool phaserWasActive = false;
    float eqLowStateLeft = 0.0f;
    float eqLowStateRight = 0.0f;
    float compressorEnvelope = 0.0f;
    float compressorAttackCoefficient = 0.0f;
    float compressorReleaseCoefficient = 0.0f;
    float chorusPhase = 0.0f;
    bool reverbWasActive = false;
    int lastReverbCombCount = 0;
};
} // namespace synth
