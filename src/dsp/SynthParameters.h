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
    float saturationMix = 0.0f;
    float saturationDrive = 0.35f;
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
    OscillatorParameters osc;
    FilterParameters filter;
    AmpParameters amp;
    EnvelopeParameters ampEnv;
    EnvelopeParameters modEnv { 1.0f, 400.0f, 0.0f, 160.0f };
    DirectModParameters direct;
    LfoParameters lfo;
    RampParameters ramp;
    TransModParameters transMod;
    MacroParameters macro;
    FxParameters fx;
    QualityParameters quality;
    PerformanceState performance;
    float tempoBpm = 128.0f;
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
} // namespace synth
