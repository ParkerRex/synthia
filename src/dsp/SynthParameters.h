#pragma once

#include <algorithm>
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
    float oscLfoSemitones = 0.0f;
    float oscModEnvSemitones = 0.0f;
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

struct SynthParameters
{
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
    MacroParameters macro;
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
