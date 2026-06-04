#include "ParameterRegistry.h"

#include <algorithm>
#include <set>

namespace synth
{
namespace
{
ParameterSpec floatParam(std::string id, std::string name, std::string group, std::string unit,
                         float minimum, float maximum, float defaultValue,
                         float interval = 0.0f, float skew = 1.0f, float smoothMs = 0.0f)
{
    ParameterSpec spec;
    spec.id = std::move(id);
    spec.name = std::move(name);
    spec.group = std::move(group);
    spec.unit = std::move(unit);
    spec.minimum = minimum;
    spec.maximum = maximum;
    spec.defaultValue = defaultValue;
    spec.interval = interval;
    spec.skew = skew;
    spec.smoothMs = smoothMs;
    return spec;
}

ParameterSpec boolParam(std::string id, std::string name, std::string group, bool defaultValue)
{
    ParameterSpec spec;
    spec.id = std::move(id);
    spec.name = std::move(name);
    spec.group = std::move(group);
    spec.kind = ParameterKind::Bool;
    spec.unit = "boolean";
    spec.defaultValue = defaultValue ? 1.0f : 0.0f;
    return spec;
}

ParameterSpec choiceParam(std::string id, std::string name, std::string group,
                          std::vector<std::string> choices, int defaultChoice)
{
    ParameterSpec spec;
    spec.id = std::move(id);
    spec.name = std::move(name);
    spec.group = std::move(group);
    spec.kind = ParameterKind::Choice;
    spec.unit = "enum";
    spec.choices = std::move(choices);
    spec.defaultChoice = defaultChoice;
    spec.defaultValue = static_cast<float>(defaultChoice);
    return spec;
}

std::vector<std::string> sourceChoices()
{
    return {
        "None", "LFO", "Ramp", "ModEnv", "AmpEnv", "Keytrack", "Velocity",
        "VelocityGlide", "PitchBend", "ModWheel", "Aftertouch", "VoiceUni",
        "VoiceBi", "UnisonUni", "UnisonBi", "RandomOnNote", "Macro1",
        "Macro2", "Macro3", "Macro4"
    };
}

std::vector<ParameterSpec> buildSpecs()
{
    std::vector<ParameterSpec> specs = {
        choiceParam("voice.mode", "Voice Mode", "voice", {"Mono", "MonoLegato", "Poly", "Unison"}, 2),
        floatParam("voice.polyphony", "Polyphony", "voice", "voices", 1.0f, 32.0f, 8.0f, 1.0f),
        floatParam("voice.unison_count", "Unison Count", "voice", "voices", 1.0f, 8.0f, 1.0f, 1.0f),
        boolParam("voice.retrigger", "Retrigger", "voice", true),
        floatParam("voice.glide_ms", "Glide", "voice", "milliseconds", 0.0f, 2000.0f, 0.0f, 0.01f, 0.5f),
        floatParam("voice.velocity_glide_ms", "Velocity Glide", "voice", "milliseconds", 0.0f, 2000.0f, 0.0f, 0.01f, 0.5f),

        floatParam("osc.pitch_semitones", "Osc Pitch", "osc", "semitones", -48.0f, 48.0f, 0.0f, 0.01f, 1.0f, 5.0f),
        floatParam("osc.fine_cents", "Osc Fine", "osc", "cents", -100.0f, 100.0f, 0.0f, 0.01f, 1.0f, 5.0f),
        floatParam("osc.stack_count", "Stack Count", "osc", "voices", 1.0f, 5.0f, 1.0f, 1.0f),
        floatParam("osc.stack_detune", "Stack Detune", "osc", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f, 0.5f, 10.0f),
        floatParam("osc.saw_level", "Saw Level", "osc", "normalized", 0.0f, 1.0f, 1.0f, 0.0001f, 1.0f, 5.0f),
        floatParam("osc.pulse_level", "Pulse Level", "osc", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f, 1.0f, 5.0f),
        floatParam("osc.noise_level", "Noise Level", "osc", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f, 1.0f, 5.0f),
        floatParam("osc.pulse_width", "Pulse Width", "osc", "percent", 0.05f, 0.95f, 0.5f, 0.0001f, 1.0f, 5.0f),
        choiceParam("osc.sub_wave", "Sub Wave", "osc", {"Sine", "Triangle", "Saw", "Pulse"}, 2),
        choiceParam("osc.sub_octave", "Sub Octave", "osc", {"-1", "-2", "-3"}, 0),
        floatParam("osc.sub_level", "Sub Level", "osc", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f, 1.0f, 5.0f),
        floatParam("osc.sub_pulse_width", "Sub Pulse Width", "osc", "percent", 0.05f, 0.95f, 0.5f, 0.0001f, 1.0f, 5.0f),
        floatParam("osc.sync_amount", "Sync Amount", "osc", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f, 1.0f, 5.0f),
        choiceParam("osc.phase_reset", "Phase Reset", "osc", {"Off", "Note", "Voice", "Random"}, 0),

        boolParam("filter.enabled", "Filter Enabled", "filter", true),
        choiceParam("filter.mode", "Filter Mode", "filter",
                    {"L2", "L4", "B2", "B4", "H2", "H4", "Peak2", "Notch2", "Notch4"}, 1),
        floatParam("filter.cutoff_semitones", "Filter Cutoff", "filter", "semitones", 0.0f, 136.0f, 96.0f, 0.01f, 1.0f, 10.0f),
        floatParam("filter.resonance", "Resonance", "filter", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f, 1.0f, 10.0f),
        floatParam("filter.drive", "Filter Drive", "filter", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f, 1.0f, 10.0f),
        floatParam("filter.keytrack", "Filter Keytrack", "filter", "normalized", 0.0f, 1.0f, 0.5f, 0.0001f, 1.0f, 5.0f),
        choiceParam("filter.oversampling", "Filter Oversampling", "filter", {"Off", "2x", "4x", "8x"}, 1),

        floatParam("amp.drive", "Amp Drive", "amp", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f, 1.0f, 5.0f),
        floatParam("amp.level_db", "Level", "amp", "dB", -48.0f, 12.0f, -6.0f, 0.01f, 1.0f, 5.0f),
        floatParam("amp.pan", "Pan", "amp", "normalized", -1.0f, 1.0f, 0.0f, 0.0001f, 1.0f, 5.0f),
        floatParam("amp.pan_spread", "Pan Spread", "amp", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f, 1.0f, 5.0f),
        floatParam("amp.unison_spread", "Unison Spread", "amp", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f, 1.0f, 5.0f),
        floatParam("amp.analog", "Analog", "amp", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f, 1.0f, 5.0f),

        floatParam("amp_env.attack_ms", "Amp Attack", "amp_env", "milliseconds", 0.0f, 10000.0f, 2.0f, 0.01f, 0.35f),
        floatParam("amp_env.decay_ms", "Amp Decay", "amp_env", "milliseconds", 1.0f, 10000.0f, 300.0f, 0.01f, 0.35f),
        floatParam("amp_env.sustain", "Amp Sustain", "amp_env", "normalized", 0.0f, 1.0f, 0.8f, 0.0001f),
        floatParam("amp_env.release_ms", "Amp Release", "amp_env", "milliseconds", 1.0f, 10000.0f, 200.0f, 0.01f, 0.35f),
        floatParam("mod_env.attack_ms", "Mod Attack", "mod_env", "milliseconds", 0.0f, 10000.0f, 1.0f, 0.01f, 0.35f),
        floatParam("mod_env.decay_ms", "Mod Decay", "mod_env", "milliseconds", 1.0f, 10000.0f, 400.0f, 0.01f, 0.35f),
        floatParam("mod_env.sustain", "Mod Sustain", "mod_env", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f),
        floatParam("mod_env.release_ms", "Mod Release", "mod_env", "milliseconds", 1.0f, 10000.0f, 160.0f, 0.01f, 0.35f),

        choiceParam("lfo.shape", "LFO Shape", "lfo", {"Sine", "Triangle", "SawUp", "SawDown", "Square", "SampleHold", "Noise"}, 3),
        choiceParam("lfo.rate_mode", "LFO Rate Mode", "lfo", {"Hz", "Sync"}, 1),
        floatParam("lfo.rate_hz", "LFO Rate", "lfo", "Hz", 0.01f, 40.0f, 2.0f, 0.0001f, 0.35f),
        choiceParam("lfo.sync_division", "LFO Sync Division", "lfo", {"1/16", "1/8", "1/8D", "1/4", "1/2", "1 bar"}, 3),
        floatParam("lfo.phase_degrees", "LFO Phase", "lfo", "degrees", 0.0f, 360.0f, 0.0f, 0.01f),
        choiceParam("lfo.gate_mode", "LFO Gate Mode", "lfo", {"Poly", "PolyOn", "Mono", "Song"}, 1),
        boolParam("lfo.mono", "LFO Mono", "lfo", false),
        floatParam("lfo.swing", "LFO Swing", "lfo", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f),

        boolParam("ramp.enabled", "Ramp Enabled", "ramp", false),
        choiceParam("ramp.mode", "Ramp Mode", "ramp", {"OneShot", "Loop", "Sync"}, 0),
        floatParam("ramp.delay_ms", "Ramp Delay", "ramp", "milliseconds", 0.0f, 10000.0f, 0.0f, 0.01f, 0.35f),
        floatParam("ramp.rise_ms", "Ramp Rise", "ramp", "milliseconds", 1.0f, 10000.0f, 1000.0f, 0.01f, 0.35f),
        choiceParam("ramp.curve", "Ramp Curve", "ramp", {"Linear", "Exponential", "Snappy"}, 0),

        floatParam("direct.filter_keytrack", "Filter Keytrack Direct", "direct", "normalized", -1.0f, 1.0f, 0.0f, 0.0001f),
        floatParam("direct.filter_lfo_semitones", "Filter LFO Direct", "direct", "semitones", -72.0f, 72.0f, 0.0f, 0.01f),
        floatParam("direct.filter_mod_env_semitones", "Filter Env Direct", "direct", "semitones", -72.0f, 72.0f, 0.0f, 0.01f),
        floatParam("direct.osc_lfo_semitones", "Osc LFO Direct", "direct", "semitones", -48.0f, 48.0f, 0.0f, 0.01f),
        floatParam("direct.osc_mod_env_semitones", "Osc Env Direct", "direct", "semitones", -48.0f, 48.0f, 0.0f, 0.01f),
        floatParam("direct.pulse_lfo", "Pulse LFO Direct", "direct", "normalized", -1.0f, 1.0f, 0.0f, 0.0001f),
        floatParam("direct.pulse_mod_env", "Pulse Env Direct", "direct", "normalized", -1.0f, 1.0f, 0.0f, 0.0001f),

        boolParam("fx.enabled", "FX Enabled", "fx", false),
        floatParam("fx.saturation_mix", "Saturation Mix", "fx", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f),
        floatParam("fx.delay_mix", "Delay Mix", "fx", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f),
        floatParam("fx.reverb_mix", "Reverb Mix", "fx", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f),
        floatParam("fx.chorus_mix", "Chorus Mix", "fx", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f),

        floatParam("macro.motion", "Motion", "macro", "normalized", 0.0f, 1.0f, 0.5f, 0.0001f),
        floatParam("macro.width", "Width", "macro", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f),
        floatParam("macro.drive", "Drive", "macro", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f),
        floatParam("macro.space", "Space", "macro", "normalized", 0.0f, 1.0f, 0.0f, 0.0001f),
    };

    const auto sources = sourceChoices();
    for (int slot = 1; slot <= 8; ++slot)
    {
        const auto prefix = "transmod." + std::to_string(slot) + ".";
        specs.push_back(boolParam(prefix + "enabled", "TransMod " + std::to_string(slot) + " Enabled", "transmod", false));
        specs.push_back(choiceParam(prefix + "source", "TransMod " + std::to_string(slot) + " Source", "transmod", sources, 0));
        specs.push_back(choiceParam(prefix + "scaler", "TransMod " + std::to_string(slot) + " Scaler", "transmod", sources, 0));
        specs.push_back(floatParam(prefix + "depth", "TransMod " + std::to_string(slot) + " Depth", "transmod", "normalized", -1.0f, 1.0f, 0.0f, 0.0001f));
    }

    return specs;
}
} // namespace

const std::vector<ParameterSpec>& getParameterSpecs()
{
    static const auto specs = buildSpecs();
    return specs;
}

const ParameterSpec* findParameterSpec(const std::string& id)
{
    const auto& specs = getParameterSpecs();
    const auto found = std::find_if(specs.begin(), specs.end(), [&id](const auto& spec) {
        return spec.id == id;
    });

    return found == specs.end() ? nullptr : &*found;
}

std::vector<std::string> validateParameterSpecs()
{
    std::vector<std::string> errors;
    std::set<std::string> ids;

    for (const auto& spec : getParameterSpecs())
    {
        if (spec.id.empty())
            errors.push_back("empty parameter id");

        if (!ids.insert(spec.id).second)
            errors.push_back("duplicate parameter id: " + spec.id);

        if (spec.name.empty())
            errors.push_back("empty parameter name: " + spec.id);

        if (spec.group.empty())
            errors.push_back("empty parameter group: " + spec.id);

        if (spec.kind == ParameterKind::Float)
        {
            if (spec.minimum >= spec.maximum)
                errors.push_back("invalid range: " + spec.id);

            if (spec.defaultValue < spec.minimum || spec.defaultValue > spec.maximum)
                errors.push_back("default out of range: " + spec.id);
        }

        if (spec.kind == ParameterKind::Choice)
        {
            if (spec.choices.empty())
                errors.push_back("choice parameter without choices: " + spec.id);

            if (spec.defaultChoice < 0 || spec.defaultChoice >= static_cast<int>(spec.choices.size()))
                errors.push_back("choice default out of range: " + spec.id);
        }
    }

    return errors;
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    for (const auto& spec : getParameterSpecs())
    {
        const juce::ParameterID parameterId { juce::String(spec.id), 1 };
        const juce::String name { spec.name };

        if (spec.kind == ParameterKind::Float)
        {
            juce::NormalisableRange<float> range { spec.minimum, spec.maximum, spec.interval, spec.skew };
            parameters.push_back(std::make_unique<juce::AudioParameterFloat>(parameterId, name, range, spec.defaultValue));
        }
        else if (spec.kind == ParameterKind::Bool)
        {
            parameters.push_back(std::make_unique<juce::AudioParameterBool>(parameterId, name, spec.defaultValue >= 0.5f));
        }
        else
        {
            juce::StringArray choices;
            for (const auto& choice : spec.choices)
                choices.add(choice);

            parameters.push_back(std::make_unique<juce::AudioParameterChoice>(parameterId, name, choices, spec.defaultChoice));
        }
    }

    return { parameters.begin(), parameters.end() };
}

const char* toString(ParameterKind kind) noexcept
{
    switch (kind)
    {
        case ParameterKind::Float: return "float";
        case ParameterKind::Bool: return "bool";
        case ParameterKind::Choice: return "choice";
    }

    return "unknown";
}
} // namespace synth
