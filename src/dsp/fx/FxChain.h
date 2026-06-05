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
    FxStereoFrame processChorus(FxStereoFrame input, const SynthParameters& parameters) noexcept;
    FxStereoFrame processDelay(FxStereoFrame input, const SynthParameters& parameters) noexcept;
    FxStereoFrame processReverb(FxStereoFrame input, const SynthParameters& parameters) noexcept;
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
    float chorusPhase = 0.0f;
    bool reverbWasActive = false;
    int lastReverbCombCount = 0;
};
} // namespace synth
