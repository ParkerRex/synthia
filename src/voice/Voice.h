#pragma once

#include "../dsp/Envelope.h"
#include "../dsp/Filter.h"
#include "../dsp/Lfo.h"
#include "../dsp/OscillatorStack.h"
#include "../dsp/Ramp.h"
#include "../dsp/SynthParameters.h"

#include <array>

namespace synth
{
enum class VoiceState
{
    Idle,
    Held,
    Releasing
};

struct VoiceSnapshot
{
    VoiceState state = VoiceState::Idle;
    int midiNote = -1;
    float velocity = 0.0f;
    float ampEnv = 0.0f;
    float modEnv = 0.0f;
    float lfo = 0.0f;
    float ramp = 0.0f;
    float randomOnNote = 0.0f;
    float effectiveMidiNote = 0.0f;
    float velocityGlide = 0.0f;
    float voiceUni = 0.0f;
    float voiceBi = 0.0f;
    float unisonUni = 0.0f;
    float unisonBi = 0.0f;
    float directOscPitchSemitones = 0.0f;
    float directPulseWidth = 0.0f;
    float directFilterCutoffSemitones = 0.0f;
    float transModOscPitchSemitones = 0.0f;
    float transModPulseWidth = 0.0f;
    float transModFilterCutoffSemitones = 0.0f;
    float transModAmpLevelDb = 0.0f;
    float transModPan = 0.0f;
};

struct StereoFrame
{
    float left = 0.0f;
    float right = 0.0f;
};

class Voice
{
public:
    void prepare(double sampleRate);
    void setVoiceIndex(int index, int total) noexcept;
    void setAllocationIndices(int index, int total, int newUnisonIndex, int newUnisonCount) noexcept;
    void noteOn(int note, float normalizedVelocity, float randomValue,
                int newUnisonIndex, int newUnisonCount, const SynthParameters& parameters,
                bool allowGlide, bool retriggerModulators) noexcept;
    void noteOff() noexcept;
    void reset() noexcept;
    void process(int numSamples) noexcept;
    StereoFrame renderSample(const SynthParameters& parameters, const float* monoLfoValue = nullptr) noexcept;

    bool isActive() const noexcept { return state != VoiceState::Idle; }
    bool isHeld() const noexcept { return state == VoiceState::Held; }
    int getMidiNote() const noexcept { return midiNote; }
    VoiceSnapshot snapshot() const noexcept;

private:
    struct ModulationSums
    {
        float oscPitchSemitones = 0.0f;
        float pulseWidth = 0.0f;
        float filterCutoffSemitones = 0.0f;
        float ampLevelDb = 0.0f;
        float pan = 0.0f;
    };

    struct LayerOscillatorMix
    {
        float sample = 0.0f;
        float pan = 0.0f;
    };

    float processGlide(const SynthParameters& parameters) noexcept;
    float processVelocityGlide(const SynthParameters& parameters) noexcept;
    void resetLayerOscillators(const SynthParameters& parameters, float fallbackPhase) noexcept;
    LayerOscillatorMix renderLayerOscillators(const SynthParameters& parameters, float effectiveNote,
                                              float pitchModSemitones, float pulseWidthMod,
                                              float analogPitchMod, float unisonBi) noexcept;
    float evalModSource(const SynthParameters& parameters, ModSource source, float lfoValue, float rampValue,
                        float modEnvValue, float ampEnvValue, float effectiveNote,
                        float velocityGlideValue) const noexcept;
    ModulationSums evaluateTransMod(const SynthParameters& parameters, float lfoValue,
                                    float rampValue, float modEnvValue, float ampEnvValue,
                                    float effectiveNote, float velocityGlideValue) const noexcept;

    VoiceState state = VoiceState::Idle;
    int midiNote = -1;
    float velocity = 0.0f;
    float currentVelocity = 0.0f;
    float startVelocity = 0.0f;
    int velocityGlideSamples = 0;
    int velocityGlideTotalSamples = 0;
    float currentMidiNote = 0.0f;
    float startMidiNote = 0.0f;
    float targetMidiNote = 0.0f;
    int glideSamples = 0;
    int glideTotalSamples = 0;
    float randomOnNote = 0.0f;
    int voiceIndex = 0;
    int voiceCount = 1;
    int unisonIndex = 0;
    int unisonCount = 1;
    double sampleRate = 44100.0;
    Envelope ampEnvelope;
    Envelope modEnvelope;
    Lfo lfo;
    Ramp ramp;
    OscillatorStack oscillator;
    std::array<OscillatorStack, layerCount * oscillatorSlotsPerLayer> layerOscillators;
    Filter filter;
    ModulationSums lastDirectSums;
    ModulationSums lastTransModSums;
    float lastRampValue = 0.0f;
};
} // namespace synth
