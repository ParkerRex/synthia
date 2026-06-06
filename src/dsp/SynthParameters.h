#pragma once

#include <algorithm>
#include <array>
#include <cmath>

namespace synth
{
enum class SubWave
{
    Sine = 0,
    Triangle = 1,
    Saw = 2,
    Pulse = 3
};

enum class FilterMode
{
    L2 = 0,
    L4 = 1,
    B2 = 2,
    B4 = 3,
    H2 = 4,
    H4 = 5,
    Peak2 = 6,
    Notch2 = 7,
    Notch4 = 8
};

enum class LfoShapeChoice
{
    Sine = 0,
    Triangle = 1,
    SawUp = 2,
    SawDown = 3,
    Square = 4,
    SampleHold = 5,
    Noise = 6
};

enum class LfoRateMode
{
    Hz = 0,
    Sync = 1
};

enum class LfoGateMode
{
    Poly = 0,
    PolyOn = 1,
    Mono = 2,
    Song = 3
};

enum class VoiceMode
{
    Mono = 0,
    MonoLegato = 1,
    Poly = 2,
    Unison = 3
};

enum class RampMode
{
    OneShot = 0,
    Loop = 1,
    Sync = 2
};

enum class RampCurve
{
    Linear = 0,
    Exponential = 1,
    Snappy = 2
};

enum class QualityMode
{
    Eco = 0,
    Normal = 1,
    High = 2
};

enum class DelaySyncDivision
{
    Sixteenth = 0,
    Eighth = 1,
    DottedEighth = 2,
    Quarter = 3,
    Half = 4
};

enum class DistortionMode
{
    Soft = 0,
    Clip = 1,
    Fold = 2
};

enum class ArpMode
{
    Up = 0,
    Down = 1,
    UpDown = 2,
    AsPlayed = 3
};

enum class ArpRateDivision
{
    ThirtySecond = 0,
    Sixteenth = 1,
    Eighth = 2,
    Quarter = 3,
    Half = 4
};

enum class OscillatorSlotWaveform
{
    Saw = 0,
    Pulse = 1,
    Noise = 2,
    Sub = 3
};

enum class ModSource
{
    None = 0,
    Lfo = 1,
    Ramp = 2,
    ModEnv = 3,
    AmpEnv = 4,
    Keytrack = 5,
    Velocity = 6,
    VelocityGlide = 7,
    PitchBend = 8,
    ModWheel = 9,
    Aftertouch = 10,
    VoiceUni = 11,
    VoiceBi = 12,
    UnisonUni = 13,
    UnisonBi = 14,
    RandomOnNote = 15,
    Macro1 = 16,
    Macro2 = 17,
    Macro3 = 18,
    Macro4 = 19
};

inline constexpr int transModSlotCount = 8;
inline constexpr int layerCount = 2;
inline constexpr int oscillatorSlotsPerLayer = 2;
inline constexpr int arpStepCount = 16;
inline constexpr int chordVoiceCount = 8;

struct EnvelopeParameters
{
    float attackMs = 2.0f;
    float decayMs = 300.0f;
    float sustain = 0.8f;
    float releaseMs = 200.0f;
};

struct OscillatorParameters
{
    float pitchSemitones = 0.0f;
    float fineCents = 0.0f;
    int stackCount = 1;
    float stackDetune = 0.0f;
    float sawLevel = 1.0f;
    float pulseLevel = 0.0f;
    float noiseLevel = 0.0f;
    float pulseWidth = 0.5f;
    SubWave subWave = SubWave::Saw;
    int subOctave = 1;
    float subLevel = 0.0f;
    float subPulseWidth = 0.5f;
    float syncAmount = 0.0f;
    int phaseReset = 0;
};

struct LayerOscillatorParameters
{
    bool enabled = false;
    int voices = 0;
    OscillatorSlotWaveform waveform = OscillatorSlotWaveform::Saw;
    int octave = 0;
    int note = 0;
    float fineCents = 0.0f;
    float level = 0.0f;
    float phaseDegrees = 0.0f;
    float detune = 0.0f;
    float stereo = 0.0f;
    float pan = 0.0f;
    bool retrigger = true;
    bool invert = false;
};

struct LayerParameters
{
    bool enabled = false;
    float levelDb = 0.0f;
    float pan = 0.0f;
    bool solo = false;
    bool mute = false;
    std::array<LayerOscillatorParameters, oscillatorSlotsPerLayer> oscillators {};
};

struct FilterParameters
{
    bool enabled = true;
    FilterMode mode = FilterMode::L4;
    float cutoffSemitones = 96.0f;
    float resonance = 0.0f;
    float drive = 0.0f;
    float keytrack = 0.5f;
    int oversampling = 1;
};

struct AmpParameters
{
    float drive = 0.0f;
    float levelDb = -6.0f;
    float pan = 0.0f;
    float panSpread = 0.0f;
    float unisonSpread = 0.0f;
    float analog = 0.0f;
};

struct DirectModParameters
{
    float filterKeytrack = 0.0f;
    float filterLfoSemitones = 0.0f;
    float filterModEnvSemitones = 0.0f;
    float oscKeytrackSemitones = 0.0f;
    float oscLfoSemitones = 0.0f;
    float oscModEnvSemitones = 0.0f;
    float pulseKeytrack = 0.0f;
    float pulseLfo = 0.0f;
    float pulseModEnv = 0.0f;
};

struct LfoParameters
{
    LfoShapeChoice shape = LfoShapeChoice::SawDown;
    LfoRateMode rateMode = LfoRateMode::Sync;
    float rateHz = 2.0f;
    int syncDivision = 3;
    float phaseDegrees = 0.0f;
    LfoGateMode gateMode = LfoGateMode::PolyOn;
    bool mono = false;
    float swing = 0.0f;
};

struct MacroParameters
{
    float motion = 0.5f;
    float width = 0.0f;
    float drive = 0.0f;
    float space = 0.0f;
};

struct FxParameters
{
    bool enabled = false;
    bool saturationEnabled = true;
    DistortionMode distortionMode = DistortionMode::Soft;
    float saturationMix = 0.0f;
    float saturationDrive = 0.35f;
    bool phaserEnabled = false;
    float phaserMix = 0.0f;
    float phaserRateHz = 0.25f;
    float phaserDepth = 0.45f;
    float phaserFeedback = 0.15f;
    bool delayEnabled = true;
    float delayMix = 0.0f;
    DelaySyncDivision delaySyncDivision = DelaySyncDivision::Eighth;
    float delayFeedback = 0.22f;
    bool reverbEnabled = true;
    float reverbMix = 0.0f;
    float reverbDecay = 0.35f;
    bool chorusEnabled = false;
    float chorusMix = 0.0f;
    float chorusRateHz = 0.35f;
    float chorusDepthMs = 5.0f;
    bool eqEnabled = false;
    float eqLowGainDb = 0.0f;
    float eqHighGainDb = 0.0f;
    bool compressorEnabled = false;
    float compressorThresholdDb = -18.0f;
    float compressorRatio = 2.0f;
    float compressorMakeupDb = 0.0f;
    float compressorMix = 0.0f;
};

struct QualityParameters
{
    QualityMode realtimeMode = QualityMode::Normal;
    QualityMode offlineMode = QualityMode::High;
    QualityMode activeMode = QualityMode::Normal;
};

struct RampParameters
{
    bool enabled = false;
    RampMode mode = RampMode::OneShot;
    float delayMs = 0.0f;
    float riseMs = 1000.0f;
    RampCurve curve = RampCurve::Linear;
};

struct ArpStepParameters
{
    bool enabled = true;
    int pitchSemitones = 0;
    float velocity = 1.0f;
    float gate = 1.0f;
    bool tie = false;
};

struct ArpParameters
{
    bool enabled = false;
    ArpMode mode = ArpMode::Up;
    ArpRateDivision rate = ArpRateDivision::Sixteenth;
    float gate = 0.75f;
    int octaves = 1;
    bool hold = false;
    float swing = 0.0f;
    int stepCount = arpStepCount;
    std::array<ArpStepParameters, arpStepCount> steps {};
};

struct ChordVoiceParameters
{
    bool enabled = false;
    int pitchSemitones = 0;
    float velocity = 1.0f;
};

struct ChordParameters
{
    bool enabled = false;
    int voiceCount = 1;
    std::array<ChordVoiceParameters, chordVoiceCount> voices {
        ChordVoiceParameters { true, 0, 1.0f },
        ChordVoiceParameters {},
        ChordVoiceParameters {},
        ChordVoiceParameters {},
        ChordVoiceParameters {},
        ChordVoiceParameters {},
        ChordVoiceParameters {},
        ChordVoiceParameters {}
    };
};

struct TransModSlotParameters
{
    bool enabled = false;
    ModSource source = ModSource::None;
    ModSource scaler = ModSource::None;
    float depth = 0.0f;
    float oscPitchSemitones = 0.0f;
    float pulseWidth = 0.0f;
    float filterCutoffSemitones = 0.0f;
    float ampLevelDb = 0.0f;
    float pan = 0.0f;
};

struct TransModParameters
{
    std::array<TransModSlotParameters, transModSlotCount> slots {};
};

struct PerformanceState
{
    float pitchBend = 0.0f;
    float modWheel = 0.0f;
    float aftertouch = 0.0f;
};

struct SynthParameters
{
    VoiceMode voiceMode = VoiceMode::Poly;
    int polyphony = 8;
    int unisonCount = 1;
    bool retrigger = true;
    float glideMs = 0.0f;
    float velocityGlideMs = 0.0f;
    std::array<LayerParameters, layerCount> layers {
        LayerParameters {
            true,
            0.0f,
            0.0f,
            false,
            false,
            std::array<LayerOscillatorParameters, oscillatorSlotsPerLayer> {
                LayerOscillatorParameters {
                    true,
                    1,
                    OscillatorSlotWaveform::Saw,
                    0,
                    0,
                    0.0f,
                    1.0f,
                    0.0f,
                    0.0f,
                    0.0f,
                    0.0f,
                    true,
                    false
                },
                LayerOscillatorParameters {}
            }
        },
        LayerParameters {}
    };
    OscillatorParameters osc;
    FilterParameters filter;
    AmpParameters amp;
    EnvelopeParameters ampEnv;
    EnvelopeParameters modEnv { 1.0f, 400.0f, 0.0f, 160.0f };
    DirectModParameters direct;
    LfoParameters lfo;
    RampParameters ramp;
    ArpParameters arp;
    ChordParameters chord;
    TransModParameters transMod;
    MacroParameters macro;
    FxParameters fx;
    QualityParameters quality;
    PerformanceState performance;
    float tempoBpm = 128.0f;
};

struct PatchCostEstimate
{
    int noteLimit = 0;
    int unisonVoices = 0;
    int maxActiveVoices = 0;
    int oscillatorSlotVoices = 0;
    int voiceUnits = 0;
    int filterOversampling = 1;
    int activeFxModules = 0;
    float filterMultiplier = 1.0f;
    float fxMultiplier = 1.0f;
    float totalUnits = 0.0f;
    int loadPercent = 0;
    bool elevated = false;
    bool high = false;
    bool overBudget = false;
};

inline float clampUnit(float value) noexcept
{
    return std::clamp(value, 0.0f, 1.0f);
}

inline float decibelsToGain(float db) noexcept
{
    return std::pow(10.0f, db / 20.0f);
}

inline float midiNoteToHz(float midiNote) noexcept
{
    return 440.0f * std::pow(2.0f, (midiNote - 69.0f) / 12.0f);
}

inline bool isLegacyCompatibilityOscillatorSlot(int layerIndex, int oscillatorIndex) noexcept
{
    return layerIndex == 0 && oscillatorIndex == 0;
}

inline int oscillatorSlotVoiceCost(const LayerOscillatorParameters& oscillator, int renderedVoiceCount) noexcept
{
    return oscillator.enabled && oscillator.voices > 0 && oscillator.level > 0.0f
        ? std::clamp(renderedVoiceCount, 0, 8)
        : 0;
}

inline int oscillatorSlotVoiceCost(const LayerOscillatorParameters& oscillator) noexcept
{
    return oscillatorSlotVoiceCost(oscillator, oscillator.voices);
}

inline int layerVoiceCost(const LayerParameters& layer) noexcept
{
    if (!layer.enabled || layer.mute)
        return 0;

    auto voices = 0;
    for (const auto& oscillator : layer.oscillators)
        voices += oscillatorSlotVoiceCost(oscillator);

    return voices;
}

inline int layerVoiceCost(const SynthParameters& parameters, int layerIndex) noexcept
{
    if (layerIndex < 0 || layerIndex >= layerCount)
        return 0;

    const auto& layer = parameters.layers[static_cast<std::size_t>(layerIndex)];
    if (!layer.enabled || layer.mute)
        return 0;

    auto voices = 0;
    for (int oscillatorIndex = 0; oscillatorIndex < oscillatorSlotsPerLayer; ++oscillatorIndex)
    {
        const auto& oscillator = layer.oscillators[static_cast<std::size_t>(oscillatorIndex)];
        const auto renderedVoices = isLegacyCompatibilityOscillatorSlot(layerIndex, oscillatorIndex)
            ? parameters.osc.stackCount
            : oscillator.voices;
        voices += oscillatorSlotVoiceCost(oscillator, renderedVoices);
    }

    return voices;
}

inline int layerOscillatorVoiceCost(const SynthParameters& parameters) noexcept
{
    auto hasSolo = false;
    for (const auto& layer : parameters.layers)
        hasSolo = hasSolo || (layer.enabled && layer.solo);

    auto voices = 0;
    for (int layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        const auto& layer = parameters.layers[static_cast<std::size_t>(layerIndex)];
        if (!hasSolo || layer.solo)
            voices += layerVoiceCost(parameters, layerIndex);
    }

    return voices;
}

inline float semitonesToHz(float semitonesFromMidiZero) noexcept
{
    return midiNoteToHz(semitonesFromMidiZero);
}

inline int oversamplingFactor(int choice) noexcept
{
    switch (choice)
    {
        case 1: return 2;
        case 2: return 4;
        case 3: return 8;
        default: return 1;
    }
}

inline int patchCostNoteLimit(const SynthParameters& parameters) noexcept
{
    if (parameters.voiceMode == VoiceMode::Poly)
        return std::clamp(parameters.polyphony, 1, 32);

    return 1;
}

inline int patchCostUnisonVoices(const SynthParameters& parameters) noexcept
{
    if (parameters.voiceMode == VoiceMode::Mono || parameters.voiceMode == VoiceMode::MonoLegato)
        return 1;

    return std::clamp(parameters.unisonCount, 1, 8);
}

inline int patchCostActiveFxModules(const FxParameters& fx) noexcept
{
    if (!fx.enabled)
        return 0;

    return (fx.saturationEnabled ? 1 : 0)
         + (fx.phaserEnabled ? 1 : 0)
         + (fx.chorusEnabled ? 1 : 0)
         + (fx.delayEnabled ? 1 : 0)
         + (fx.reverbEnabled ? 1 : 0)
         + (fx.eqEnabled ? 1 : 0)
         + (fx.compressorEnabled ? 1 : 0);
}

inline PatchCostEstimate estimatePatchCost(const SynthParameters& parameters) noexcept
{
    PatchCostEstimate estimate;
    estimate.noteLimit = patchCostNoteLimit(parameters);
    estimate.unisonVoices = patchCostUnisonVoices(parameters);
    estimate.maxActiveVoices = std::clamp(estimate.noteLimit * estimate.unisonVoices, 1, 32);
    estimate.oscillatorSlotVoices = layerOscillatorVoiceCost(parameters);
    estimate.voiceUnits = estimate.maxActiveVoices * estimate.oscillatorSlotVoices;
    estimate.filterOversampling = parameters.filter.enabled ? oversamplingFactor(parameters.filter.oversampling) : 1;
    estimate.activeFxModules = patchCostActiveFxModules(parameters.fx);
    estimate.filterMultiplier = parameters.filter.enabled
        ? 0.5f + 0.5f * static_cast<float>(estimate.filterOversampling)
        : 1.0f;
    estimate.fxMultiplier = parameters.fx.enabled
        ? 1.0f + 0.06f * static_cast<float>(estimate.activeFxModules)
        : 1.0f;
    estimate.totalUnits = static_cast<float>(estimate.voiceUnits)
        * estimate.filterMultiplier
        * estimate.fxMultiplier;
    estimate.loadPercent = std::max(0, static_cast<int>(std::round(estimate.totalUnits / 240.0f * 100.0f)));
    estimate.elevated = estimate.loadPercent > 60;
    estimate.high = estimate.loadPercent > 90;
    estimate.overBudget = estimate.loadPercent > 100;
    return estimate;
}
} // namespace synth
