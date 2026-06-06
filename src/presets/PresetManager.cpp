#include "PresetManager.h"

#include "PresetValidator.h"
#include "../dsp/SynthParameters.h"
#include "../plugin/ParameterRegistry.h"

#include <juce_core/juce_core.h>

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstring>
#include <random>
#include <string_view>
#include <system_error>

#if JUCE_MAC || JUCE_LINUX
#include <fcntl.h>
#include <unistd.h>
#endif

#if JUCE_MAC
#include <dlfcn.h>
#endif

#ifndef SYLENTH_AI_PROJECT_VERSION
#ifdef SYNTH_PROJECT_VERSION
#define SYLENTH_AI_PROJECT_VERSION SYNTH_PROJECT_VERSION
#else
#define SYLENTH_AI_PROJECT_VERSION "0.1.0"
#endif
#endif

namespace synth
{
namespace
{
bool isNumber(const juce::var& value)
{
    return value.isDouble() || value.isInt() || value.isInt64();
}

int choiceIndex(const ParameterSpec& spec, const juce::var& value)
{
    if (value.isString())
    {
        const auto selected = value.toString().toStdString();
        const auto found = std::find(spec.choices.begin(), spec.choices.end(), selected);
        if (found != spec.choices.end())
            return static_cast<int>(std::distance(spec.choices.begin(), found));
    }

    if (isNumber(value))
        return static_cast<int>(std::round(static_cast<double>(value)));

    return spec.defaultChoice;
}

float physicalValueFromPresetValue(const ParameterSpec& spec, const juce::var& value)
{
    if (spec.kind == ParameterKind::Bool)
        return value.isBool() && static_cast<bool>(value) ? 1.0f : 0.0f;

    if (spec.kind == ParameterKind::Choice)
        return static_cast<float>(std::clamp(choiceIndex(spec, value), 0,
                                             static_cast<int>(spec.choices.size()) - 1));

    if (isNumber(value))
    {
        const auto numeric = static_cast<float>(static_cast<double>(value));
        if (std::isfinite(numeric))
            return std::clamp(numeric, spec.minimum, spec.maximum);
    }

    return spec.defaultValue;
}

float physicalValueFromStateValue(const ParameterSpec& spec, const juce::var& value)
{
    if (!isNumber(value))
        return spec.kind == ParameterKind::Choice ? static_cast<float>(spec.defaultChoice) : spec.defaultValue;

    const auto numeric = static_cast<float>(static_cast<double>(value));
    if (!std::isfinite(numeric))
        return spec.kind == ParameterKind::Choice ? static_cast<float>(spec.defaultChoice) : spec.defaultValue;

    if (spec.kind == ParameterKind::Bool)
        return numeric >= 0.5f ? 1.0f : 0.0f;

    if (spec.kind == ParameterKind::Choice)
        return static_cast<float>(std::clamp(static_cast<int>(std::round(numeric)), 0,
                                             static_cast<int>(spec.choices.size()) - 1));

    return std::clamp(numeric, spec.minimum, spec.maximum);
}

juce::var valueForPreset(const ParameterSpec& spec, const juce::RangedAudioParameter& parameter)
{
    const auto physical = parameter.convertFrom0to1(parameter.getValue());
    if (spec.kind == ParameterKind::Bool)
        return physical >= 0.5f;

    if (spec.kind == ParameterKind::Choice)
    {
        const auto index = std::clamp(static_cast<int>(std::round(physical)), 0,
                                      static_cast<int>(spec.choices.size()) - 1);
        return juce::String(spec.choices[static_cast<std::size_t>(index)]);
    }

    return physical;
}

juce::ValueTree findOrCreateParameterTree(juce::ValueTree& state, const std::string& parameterId)
{
    const auto id = juce::String(parameterId);
    for (auto child : state)
    {
        if (child.hasType("PARAM") && child.getProperty("id").toString() == id)
            return child;
    }

    juce::ValueTree parameterTree { "PARAM" };
    parameterTree.setProperty("id", id, nullptr);
    state.appendChild(parameterTree, nullptr);
    return parameterTree;
}

void setParameterValue(juce::ValueTree& state, const ParameterSpec& spec, float physicalValue)
{
    auto parameterTree = findOrCreateParameterTree(state, spec.id);
    parameterTree.setProperty("value", physicalValue, nullptr);
}

void resetStateToDefaults(juce::ValueTree& state)
{
    for (const auto& spec : getParameterSpecs())
        setParameterValue(state, spec, spec.kind == ParameterKind::Choice
            ? static_cast<float>(spec.defaultChoice)
            : spec.defaultValue);
}

bool setKnownParameterValue(juce::ValueTree& state, const char* id, float physicalValue)
{
    const auto* spec = findParameterSpec(id);
    if (spec == nullptr)
        return false;

    setParameterValue(state, *spec, clampPhysicalParameterValue(*spec, physicalValue));
    return true;
}

void stampPreparedState(juce::ValueTree& state, const std::string& displayName)
{
    state.setProperty("schema_version", 1, nullptr);
    state.setProperty("plugin_version", SYLENTH_AI_PROJECT_VERSION, nullptr);
    state.setProperty("current_preset", juce::String(displayName), nullptr);
    state.setProperty("current_preset_path", "", nullptr);
}

float randomFloat(std::mt19937& rng, float minimum, float maximum)
{
    std::uniform_real_distribution<float> distribution(minimum, maximum);
    return distribution(rng);
}

int randomInt(std::mt19937& rng, int minimum, int maximum)
{
    std::uniform_int_distribution<int> distribution(minimum, maximum);
    return distribution(rng);
}

bool randomChance(std::mt19937& rng, float probability)
{
    return randomFloat(rng, 0.0f, 1.0f) < probability;
}

bool applyRandomizedPatchState(juce::ValueTree& state, std::uint32_t seed)
{
    std::mt19937 rng(seed);
    auto ok = true;
    auto set = [&state, &ok](const char* id, float value) {
        ok = setKnownParameterValue(state, id, value) && ok;
    };

    set("voice.mode", static_cast<float>(randomInt(rng, 1, 3)));
    set("voice.polyphony", static_cast<float>(randomInt(rng, 4, 16)));
    set("voice.unison_count", static_cast<float>(randomInt(rng, 1, 6)));
    set("voice.glide_ms", randomChance(rng, 0.35f) ? randomFloat(rng, 20.0f, 240.0f) : 0.0f);

    set("layer.1.enabled", 1.0f);
    set("layer.1.level_db", randomFloat(rng, -5.0f, 1.5f));
    set("layer.1.pan", randomFloat(rng, -0.25f, 0.25f));
    set("layer.1.osc.1.enabled", 1.0f);
    set("layer.1.osc.1.voices", 1.0f);
    set("layer.1.osc.1.level", 1.0f);
    set("osc.stack_count", static_cast<float>(randomInt(rng, 1, 5)));
    set("osc.stack_detune", randomFloat(rng, 0.0f, 0.62f));
    set("osc.pitch_semitones", static_cast<float>(randomInt(rng, -12, 12)));
    set("osc.fine_cents", randomFloat(rng, -15.0f, 15.0f));
    set("osc.saw_level", randomFloat(rng, 0.45f, 1.0f));
    set("osc.pulse_level", randomChance(rng, 0.65f) ? randomFloat(rng, 0.0f, 0.75f) : 0.0f);
    set("osc.noise_level", randomChance(rng, 0.25f) ? randomFloat(rng, 0.0f, 0.18f) : 0.0f);
    set("osc.sub_level", randomChance(rng, 0.45f) ? randomFloat(rng, 0.0f, 0.55f) : 0.0f);
    set("osc.pulse_width", randomFloat(rng, 0.2f, 0.8f));

    if (randomChance(rng, 0.45f))
    {
        set("layer.1.osc.2.enabled", 1.0f);
        set("layer.1.osc.2.voices", static_cast<float>(randomInt(rng, 1, 4)));
        set("layer.1.osc.2.waveform", static_cast<float>(randomInt(rng, 0, 3)));
        set("layer.1.osc.2.octave", static_cast<float>(randomInt(rng, -1, 1)));
        set("layer.1.osc.2.note", static_cast<float>(randomInt(rng, -7, 7)));
        set("layer.1.osc.2.fine_cents", randomFloat(rng, -12.0f, 12.0f));
        set("layer.1.osc.2.level", randomFloat(rng, 0.15f, 0.7f));
        set("layer.1.osc.2.detune", randomFloat(rng, 0.0f, 0.45f));
        set("layer.1.osc.2.stereo", randomFloat(rng, 0.0f, 0.6f));
        set("layer.1.osc.2.pan", randomFloat(rng, -0.35f, 0.35f));
    }

    set("filter.enabled", 1.0f);
    set("filter.mode", static_cast<float>(randomInt(rng, 0, 8)));
    set("filter.cutoff_semitones", randomFloat(rng, 42.0f, 122.0f));
    set("filter.resonance", randomFloat(rng, 0.02f, 0.68f));
    set("filter.drive", randomFloat(rng, 0.0f, 0.55f));
    set("filter.keytrack", randomFloat(rng, 0.2f, 0.8f));
    set("filter.oversampling", static_cast<float>(randomInt(rng, 0, 2)));

    set("amp_env.attack_ms", randomFloat(rng, 0.0f, 80.0f));
    set("amp_env.decay_ms", randomFloat(rng, 90.0f, 1800.0f));
    set("amp_env.sustain", randomFloat(rng, 0.15f, 0.95f));
    set("amp_env.release_ms", randomFloat(rng, 45.0f, 2400.0f));
    set("mod_env.attack_ms", randomFloat(rng, 0.0f, 180.0f));
    set("mod_env.decay_ms", randomFloat(rng, 80.0f, 2200.0f));
    set("mod_env.sustain", randomFloat(rng, 0.0f, 0.8f));
    set("mod_env.release_ms", randomFloat(rng, 40.0f, 1800.0f));

    set("lfo.shape", static_cast<float>(randomInt(rng, 0, 6)));
    set("lfo.rate_mode", static_cast<float>(randomInt(rng, 0, 1)));
    set("lfo.rate_hz", randomFloat(rng, 0.2f, 8.0f));
    set("lfo.sync_division", static_cast<float>(randomInt(rng, 0, 4)));
    set("lfo.phase_degrees", randomFloat(rng, 0.0f, 360.0f));
    set("direct.filter_lfo_semitones", randomChance(rng, 0.5f) ? randomFloat(rng, -24.0f, 24.0f) : 0.0f);
    set("direct.filter_mod_env_semitones", randomChance(rng, 0.5f) ? randomFloat(rng, -36.0f, 36.0f) : 0.0f);
    set("direct.osc_lfo_semitones", randomChance(rng, 0.3f) ? randomFloat(rng, -12.0f, 12.0f) : 0.0f);

    set("amp.drive", randomFloat(rng, 0.0f, 0.45f));
    set("amp.level_db", randomFloat(rng, -15.0f, -4.0f));
    set("amp.pan", randomFloat(rng, -0.2f, 0.2f));
    set("amp.pan_spread", randomFloat(rng, 0.0f, 0.65f));
    set("amp.unison_spread", randomFloat(rng, 0.0f, 0.8f));
    set("amp.analog", randomFloat(rng, 0.0f, 0.5f));

    set("macro.motion", randomFloat(rng, 0.2f, 0.8f));
    set("macro.width", randomFloat(rng, 0.0f, 0.8f));
    set("macro.drive", randomFloat(rng, 0.0f, 0.6f));
    set("macro.space", randomFloat(rng, 0.0f, 0.75f));

    const auto fxEnabled = randomChance(rng, 0.65f);
    set("fx.enabled", fxEnabled ? 1.0f : 0.0f);
    set("fx.saturation_enabled", fxEnabled && randomChance(rng, 0.6f) ? 1.0f : 0.0f);
    set("fx.saturation_mix", fxEnabled ? randomFloat(rng, 0.0f, 0.35f) : 0.0f);
    set("fx.saturation_drive", randomFloat(rng, 0.15f, 0.7f));
    set("fx.delay_enabled", fxEnabled && randomChance(rng, 0.45f) ? 1.0f : 0.0f);
    set("fx.delay_mix", fxEnabled ? randomFloat(rng, 0.0f, 0.28f) : 0.0f);
    set("fx.delay_feedback", randomFloat(rng, 0.08f, 0.55f));
    set("fx.reverb_enabled", fxEnabled && randomChance(rng, 0.45f) ? 1.0f : 0.0f);
    set("fx.reverb_mix", fxEnabled ? randomFloat(rng, 0.0f, 0.32f) : 0.0f);
    set("fx.reverb_decay", randomFloat(rng, 0.12f, 0.65f));
    set("quality.realtime_mode", 1.0f);
    set("quality.offline_mode", 2.0f);

    return ok;
}

void applyPresetParameter(juce::ValueTree& state,
                          const std::string& id,
                          const juce::var& value)
{
    const auto* spec = findParameterSpec(id);
    if (spec == nullptr)
        return;

    setParameterValue(state, *spec, physicalValueFromPresetValue(*spec, value));
}

void overlayParameterStateChild(juce::ValueTree& state, const juce::ValueTree& child)
{
    if (!child.hasType("PARAM"))
    {
        state.appendChild(child.createCopy(), nullptr);
        return;
    }

    const auto id = child.getProperty("id").toString().toStdString();
    const auto* spec = findParameterSpec(id);
    if (spec == nullptr)
    {
        state.appendChild(child.createCopy(), nullptr);
        return;
    }

    setParameterValue(state, *spec, physicalValueFromStateValue(*spec, child.getProperty("value")));
}

void applyModSlotDepth(juce::ValueTree& state,
                       int slotNumber,
                       const std::string& targetId,
                       float value,
                       const juce::String& depthDomain)
{
    const auto normalized = depthDomain == "Normalized";
    auto parameterId = std::string {};
    auto physicalValue = value;

    if (targetId == "osc.pitch_semitones")
    {
        parameterId = "transmod." + std::to_string(slotNumber) + ".osc_pitch_semitones";
        physicalValue = normalized ? std::clamp(value, -1.0f, 1.0f) * 48.0f : value;
    }
    else if (targetId == "osc.pulse_width")
    {
        parameterId = "transmod." + std::to_string(slotNumber) + ".pulse_width";
        physicalValue = std::clamp(value, -1.0f, 1.0f);
    }
    else if (targetId == "filter.cutoff_semitones")
    {
        parameterId = "transmod." + std::to_string(slotNumber) + ".filter_cutoff_semitones";
        physicalValue = normalized ? std::clamp(value, -1.0f, 1.0f) * 72.0f : value;
    }
    else if (targetId == "amp.level_db")
    {
        parameterId = "transmod." + std::to_string(slotNumber) + ".amp_level_db";
        physicalValue = normalized ? std::clamp(value, -1.0f, 1.0f) * 24.0f : value;
    }
    else if (targetId == "amp.pan")
    {
        parameterId = "transmod." + std::to_string(slotNumber) + ".pan";
        physicalValue = std::clamp(value, -1.0f, 1.0f);
    }

    if (const auto* spec = findParameterSpec(parameterId))
        setParameterValue(state, *spec, physicalValue);
}

void applyModSlotObject(juce::ValueTree& state, const juce::var& slotVar)
{
    if (!slotVar.isObject())
        return;

    const auto* slotObject = slotVar.getDynamicObject();
    if (slotObject == nullptr)
        return;

    const auto slotId = slotObject->getProperty(juce::Identifier("slot_id"));
    if (!slotId.isInt())
        return;

    const auto slotNumber = static_cast<int>(slotId);
    if (slotNumber < 1 || slotNumber > transModSlotCount)
        return;

    const auto prefix = "transmod." + std::to_string(slotNumber) + ".";
    applyPresetParameter(state, prefix + "enabled", slotObject->getProperty(juce::Identifier("enabled")));
    applyPresetParameter(state, prefix + "source", slotObject->getProperty(juce::Identifier("source")));

    const auto scaler = slotObject->getProperty(juce::Identifier("scaler"));
    applyPresetParameter(state, prefix + "scaler", scaler.isVoid() ? juce::var("None") : scaler);

    const auto depthDomain = slotObject->getProperty(juce::Identifier("depth_domain")).toString();
    const auto depths = slotObject->getProperty(juce::Identifier("depths"));
    if (!depths.isObject())
        return;

    if (const auto* depthObject = depths.getDynamicObject())
    {
        for (const auto& property : depthObject->getProperties())
        {
            if (!isNumber(property.value))
                continue;

            applyModSlotDepth(state, slotNumber, property.name.toString().toStdString(),
                              static_cast<float>(static_cast<double>(property.value)),
                              depthDomain);
        }
    }
}

std::filesystem::path bundleFactoryPresetDirectory()
{
#if JUCE_MAC
    Dl_info info {};
    if (dladdr(reinterpret_cast<const void*>(&bundleFactoryPresetDirectory), &info) != 0
        && info.dli_fname != nullptr)
    {
        const auto binary = std::filesystem::path { info.dli_fname };
        const auto resources = binary.parent_path().parent_path() / "Resources" / "factory";
        std::error_code error;
        if (std::filesystem::exists(resources, error) && !error)
            return resources;
    }
#endif

    return {};
}

std::filesystem::path withJsonExtension(std::filesystem::path path)
{
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });

    if (extension == ".json")
        return path;

    return path.replace_extension(".json");
}

std::unique_ptr<juce::DynamicObject> readPresetObject(const std::filesystem::path& path)
{
    const auto file = juce::File(juce::String(path.string()));
    const auto parsed = juce::JSON::parse(file);
    if (!parsed.isObject())
        return {};

    const auto* source = parsed.getDynamicObject();
    if (source == nullptr)
        return {};

    return std::unique_ptr<juce::DynamicObject>(source->clone());
}

std::string propertyString(const juce::DynamicObject& object, const char* propertyName)
{
    const auto value = object.getProperty(juce::Identifier(propertyName));
    return value.isString() ? value.toString().toStdString() : std::string {};
}

std::vector<std::string> propertyStringArray(const juce::DynamicObject& object, const char* propertyName)
{
    std::vector<std::string> values;
    const auto value = object.getProperty(juce::Identifier(propertyName));
    if (!value.isArray())
        return values;

    const auto* array = value.getArray();
    if (array == nullptr)
        return values;

    for (const auto& item : *array)
    {
        if (item.isString())
            values.push_back(item.toString().toStdString());
    }

    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
    return values;
}

std::string defaultBankForSource(PresetSource source)
{
    if (source == PresetSource::Factory)
        return "Factory";
    if (source == PresetSource::LegacyUser)
        return "Legacy User";
    return "User";
}

std::string defaultCategoryForSource(PresetSource source, const std::vector<std::string>& tags)
{
    if (!tags.empty())
        return tags.front();

    if (source == PresetSource::Factory)
        return "Factory";
    if (source == PresetSource::LegacyUser)
        return "Legacy";
    return "User";
}

std::string browserMetadataString(const juce::DynamicObject& object, const char* propertyName)
{
    const auto metadata = object.getProperty(juce::Identifier("metadata"));
    if (!metadata.isObject())
        return {};

    const auto* metadataObject = metadata.getDynamicObject();
    if (metadataObject == nullptr)
        return {};

    const auto browser = metadataObject->getProperty(juce::Identifier("browser"));
    if (browser.isObject())
    {
        if (const auto* browserObject = browser.getDynamicObject())
        {
            const auto value = browserObject->getProperty(juce::Identifier(propertyName));
            if (value.isString())
                return value.toString().toStdString();
        }
    }

    const auto fallback = metadataObject->getProperty(juce::Identifier(propertyName));
    return fallback.isString() ? fallback.toString().toStdString() : std::string {};
}

std::string lowercase(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

bool textContains(const std::string& value, const std::string& query)
{
    return lowercase(value).find(query) != std::string::npos;
}

bool tagMatches(const std::vector<std::string>& presetTags, const std::string& filterTag)
{
    const auto normalizedFilter = lowercase(filterTag);
    return std::any_of(presetTags.begin(), presetTags.end(), [&normalizedFilter](const auto& tag) {
        return lowercase(tag) == normalizedFilter;
    });
}

bool containsFavoriteKey(const std::vector<std::string>& favoriteKeys, const std::string& key)
{
    return std::find(favoriteKeys.begin(), favoriteKeys.end(), key) != favoriteKeys.end();
}

std::string normalizedFavoritePath(const std::filesystem::path& path)
{
    std::error_code error;
    auto normalized = std::filesystem::weakly_canonical(path, error);
    if (error)
    {
        error.clear();
        normalized = std::filesystem::absolute(path, error);
    }

    if (error)
        normalized = path;

    return normalized.generic_string();
}

std::vector<std::string> normalizedFavoriteKeys(std::vector<std::string> favoriteKeys)
{
    favoriteKeys.erase(std::remove_if(favoriteKeys.begin(), favoriteKeys.end(), [](const auto& key) {
        return key.empty();
    }), favoriteKeys.end());
    std::sort(favoriteKeys.begin(), favoriteKeys.end());
    favoriteKeys.erase(std::unique(favoriteKeys.begin(), favoriteKeys.end()), favoriteKeys.end());
    return favoriteKeys;
}

void hashBytes(std::uint64_t& hash, std::string_view bytes) noexcept
{
    for (const auto character : bytes)
    {
        hash ^= static_cast<std::uint8_t>(character);
        hash *= 1099511628211ull;
    }
}

void hashInteger(std::uint64_t& hash, std::int64_t value) noexcept
{
    for (auto byte = 0; byte < 8; ++byte)
    {
        hash ^= static_cast<std::uint8_t>((static_cast<std::uint64_t>(value) >> (byte * 8)) & 0xffu);
        hash *= 1099511628211ull;
    }
}

std::int64_t quantizedFingerprintValue(const ParameterSpec& spec, float physicalValue) noexcept
{
    physicalValue = clampPhysicalParameterValue(spec, physicalValue);
    if (spec.kind == ParameterKind::Bool)
        return physicalValue >= 0.5f ? 1 : 0;

    if (spec.kind == ParameterKind::Choice)
        return static_cast<std::int64_t>(std::round(physicalValue));

    const auto resolution = spec.interval > 0.0f ? spec.interval : 0.000001f;
    return static_cast<std::int64_t>(std::floor((physicalValue - spec.minimum) / resolution + 0.5f));
}

juce::var stateParameterValue(const juce::ValueTree& state, const std::string& parameterId)
{
    const auto juceId = juce::String(parameterId);
    for (const auto& child : state)
    {
        if (child.hasType("PARAM") && child.getProperty("id").toString() == juceId)
            return child.getProperty("value");
    }

    return {};
}

PresetStateFingerprint fingerprintRegistryValues(const std::vector<float>& physicalValues)
{
    PresetStateFingerprint fingerprint;
    fingerprint.valid = true;
    fingerprint.hash = 1469598103934665603ull;

    const auto& specs = getParameterSpecs();
    for (std::size_t index = 0; index < specs.size(); ++index)
    {
        const auto& spec = specs[index];
        if (!spec.presetSerialized)
            continue;

        hashBytes(fingerprint.hash, spec.id);
        hashInteger(fingerprint.hash,
                    quantizedFingerprintValue(spec, physicalValues[index]));
        ++fingerprint.comparedParameterCount;
    }

    return fingerprint;
}

juce::var currentParameterObject(const juce::AudioProcessorValueTreeState& parameters)
{
    auto object = std::make_unique<juce::DynamicObject>();
    for (const auto& spec : getParameterSpecs())
    {
        if (!spec.presetSerialized)
            continue;

        if (const auto* parameter = parameters.getParameter(juce::String(spec.id)))
            object->setProperty(juce::Identifier(spec.id), valueForPreset(spec, *parameter));
    }

    return juce::var(object.release());
}

juce::String choiceValue(const juce::AudioProcessorValueTreeState& parameters, const std::string& id)
{
    const auto* spec = findParameterSpec(id);
    const auto* parameter = parameters.getParameter(juce::String(id));
    if (spec == nullptr || parameter == nullptr || spec->choices.empty())
        return "None";

    const auto index = std::clamp(static_cast<int>(std::round(parameter->convertFrom0to1(parameter->getValue()))),
                                  0, static_cast<int>(spec->choices.size()) - 1);
    return juce::String(spec->choices[static_cast<std::size_t>(index)]);
}

float floatValue(const juce::AudioProcessorValueTreeState& parameters, const std::string& id)
{
    if (const auto* parameter = parameters.getParameter(juce::String(id)))
        return parameter->convertFrom0to1(parameter->getValue());

    return 0.0f;
}

bool boolValue(const juce::AudioProcessorValueTreeState& parameters, const std::string& id)
{
    return floatValue(parameters, id) >= 0.5f;
}

juce::var currentModSlotArray(const juce::AudioProcessorValueTreeState& parameters)
{
    auto slots = juce::Array<juce::var> {};

    for (int slot = 1; slot <= transModSlotCount; ++slot)
    {
        const auto prefix = "transmod." + std::to_string(slot) + ".";
        auto slotObject = std::make_unique<juce::DynamicObject>();
        slotObject->setProperty("slot_id", slot);
        slotObject->setProperty("enabled", boolValue(parameters, prefix + "enabled"));
        slotObject->setProperty("source", choiceValue(parameters, prefix + "source"));
        slotObject->setProperty("scaler", choiceValue(parameters, prefix + "scaler"));
        slotObject->setProperty("depth_domain", "Physical");

        auto depths = std::make_unique<juce::DynamicObject>();
        depths->setProperty("osc.pitch_semitones", floatValue(parameters, prefix + "osc_pitch_semitones"));
        depths->setProperty("osc.pulse_width", floatValue(parameters, prefix + "pulse_width"));
        depths->setProperty("filter.cutoff_semitones", floatValue(parameters, prefix + "filter_cutoff_semitones"));
        depths->setProperty("amp.level_db", floatValue(parameters, prefix + "amp_level_db"));
        depths->setProperty("amp.pan", floatValue(parameters, prefix + "pan"));
        slotObject->setProperty("depths", juce::var(depths.release()));

        slots.add(juce::var(slotObject.release()));
    }

    return slots;
}

juce::var currentMacroArray(const juce::AudioProcessorValueTreeState& parameters)
{
    struct Macro
    {
        const char* id;
        const char* displayName;
        const char* parameterId;
    };

    constexpr Macro macros[] = {
        { "motion", "Motion", "macro.motion" },
        { "width", "Width", "macro.width" },
        { "drive", "Drive", "macro.drive" },
        { "space", "Space", "macro.space" },
    };

    auto values = juce::Array<juce::var> {};
    for (const auto& macro : macros)
    {
        auto object = std::make_unique<juce::DynamicObject>();
        object->setProperty("id", macro.id);
        object->setProperty("display_name", macro.displayName);
        object->setProperty("value", floatValue(parameters, macro.parameterId));
        auto assignments = juce::Array<juce::var> {};
        if (std::string_view(macro.id) == "space")
        {
            auto delayAssignment = std::make_unique<juce::DynamicObject>();
            delayAssignment->setProperty("target_id", "fx.delay_mix");
            delayAssignment->setProperty("min", 0.0);
            delayAssignment->setProperty("max", 0.35);
            delayAssignment->setProperty("curve", "linear");
            assignments.add(juce::var(delayAssignment.release()));

            auto reverbAssignment = std::make_unique<juce::DynamicObject>();
            reverbAssignment->setProperty("target_id", "fx.reverb_mix");
            reverbAssignment->setProperty("min", 0.0);
            reverbAssignment->setProperty("max", 0.45);
            reverbAssignment->setProperty("curve", "linear");
            assignments.add(juce::var(reverbAssignment.release()));
        }

        object->setProperty("assignments", assignments);
        values.add(juce::var(object.release()));
    }

    return values;
}
} // namespace

std::string presetSourceId(PresetSource source)
{
    if (source == PresetSource::Factory)
        return "factory";
    if (source == PresetSource::LegacyUser)
        return "legacy_user";
    return "user";
}

std::string presetSourceLabel(PresetSource source)
{
    if (source == PresetSource::Factory)
        return "Factory";
    if (source == PresetSource::LegacyUser)
        return "Legacy User";
    return "User";
}

std::string presetFavoriteKey(PresetSource source, const std::string& presetId, const std::filesystem::path& path)
{
    auto keyId = presetId;
    if (keyId.empty())
        keyId = presetIdFromDisplayName(path.stem().string());

    const auto prefix = presetSourceId(source) + ":" + keyId;
    if (source == PresetSource::Factory)
        return prefix;

    return prefix + ":" + normalizedFavoritePath(path);
}

std::vector<PresetSummary> scanPresetDirectory(const std::filesystem::path& directory, bool factory)
{
    return scanPresetDirectory(directory, factory ? PresetSource::Factory : PresetSource::User, {});
}

std::vector<PresetSummary> scanPresetDirectory(const std::filesystem::path& directory,
                                               PresetSource source,
                                               const std::vector<std::string>& favoriteKeys)
{
    std::vector<PresetSummary> presets;
    std::error_code error;
    if (!std::filesystem::exists(directory, error) || error)
        return presets;

    std::filesystem::directory_iterator iterator { directory,
        std::filesystem::directory_options::skip_permission_denied,
        error };
    if (error)
        return presets;

    for (std::filesystem::directory_iterator end; iterator != end; iterator.increment(error))
    {
        if (error)
        {
            error.clear();
            continue;
        }

        const auto& entry = *iterator;
        if (!entry.is_regular_file(error) || error || entry.path().extension() != ".json")
        {
            error.clear();
            continue;
        }

        const auto validation = validatePresetFile(entry.path());
        if (!validation.passed())
            continue;

        auto object = readPresetObject(entry.path());
        if (object == nullptr)
            continue;

        PresetSummary summary;
        summary.path = entry.path();
        summary.id = propertyString(*object, "id");
        summary.displayName = propertyString(*object, "display_name");
        summary.author = propertyString(*object, "author");
        summary.description = propertyString(*object, "description");
        summary.tags = propertyStringArray(*object, "tags");
        summary.bank = browserMetadataString(*object, "bank");
        if (summary.bank.empty())
            summary.bank = defaultBankForSource(source);
        summary.category = browserMetadataString(*object, "category");
        if (summary.category.empty())
            summary.category = defaultCategoryForSource(source, summary.tags);
        summary.source = source;
        summary.factory = source == PresetSource::Factory;
        summary.favoriteKey = presetFavoriteKey(source, summary.id, summary.path);
        summary.favorite = containsFavoriteKey(favoriteKeys, summary.favoriteKey);
        presets.push_back(summary);
    }

    std::sort(presets.begin(), presets.end(), [](const auto& a, const auto& b) {
        if (a.source != b.source)
            return static_cast<int>(a.source) < static_cast<int>(b.source);
        if (a.bank != b.bank)
            return a.bank < b.bank;
        if (a.category != b.category)
            return a.category < b.category;
        return a.displayName < b.displayName;
    });

    return presets;
}

std::vector<PresetSummary> filterPresetSummaries(const std::vector<PresetSummary>& presets,
                                                 const PresetBrowserFilter& filter)
{
    auto query = lowercase(filter.searchText);
    std::vector<PresetSummary> filtered;

    for (const auto& preset : presets)
    {
        if (preset.source == PresetSource::Factory && !filter.includeFactory)
            continue;
        if (preset.source == PresetSource::User && !filter.includeUser)
            continue;
        if (preset.source == PresetSource::LegacyUser && !filter.includeLegacyUser)
            continue;
        if (filter.favoritesOnly && !preset.favorite)
            continue;
        if (!filter.bank.empty() && preset.bank != filter.bank)
            continue;
        if (!filter.category.empty() && preset.category != filter.category)
            continue;

        auto allTagsMatch = true;
        for (const auto& tag : filter.tags)
        {
            if (!tagMatches(preset.tags, tag))
            {
                allTagsMatch = false;
                break;
            }
        }
        if (!allTagsMatch)
            continue;

        if (!query.empty())
        {
            auto matchesSearch = textContains(preset.id, query)
                || textContains(preset.displayName, query)
                || textContains(preset.author, query)
                || textContains(preset.description, query)
                || textContains(preset.bank, query)
                || textContains(preset.category, query);

            if (!matchesSearch)
            {
                matchesSearch = std::any_of(preset.tags.begin(), preset.tags.end(), [&query](const auto& tag) {
                    return textContains(tag, query);
                });
            }

            if (!matchesSearch)
                continue;
        }

        filtered.push_back(preset);
    }

    return filtered;
}

PresetBrowserCatalog buildPresetBrowserCatalog(const std::vector<PresetSummary>& presets)
{
    PresetBrowserCatalog catalog;
    catalog.presets = presets;

    auto appendUnique = [](std::vector<std::string>& values, const std::string& value) {
        if (!value.empty())
            values.push_back(value);
    };

    for (const auto& preset : presets)
    {
        appendUnique(catalog.banks, preset.bank);
        appendUnique(catalog.categories, preset.category);
        for (const auto& tag : preset.tags)
            appendUnique(catalog.tags, tag);
    }

    auto normalize = [](std::vector<std::string>& values) {
        std::sort(values.begin(), values.end());
        values.erase(std::unique(values.begin(), values.end()), values.end());
    };

    normalize(catalog.banks);
    normalize(catalog.categories);
    normalize(catalog.tags);
    return catalog;
}

std::filesystem::path factoryPresetDirectory()
{
    const auto bundled = bundleFactoryPresetDirectory();
    if (!bundled.empty())
        return bundled;

    return "presets/factory";
}

std::filesystem::path defaultUserPresetDirectory()
{
    const auto music = juce::File::getSpecialLocation(juce::File::userMusicDirectory);
    return std::filesystem::path { music.getFullPathName().toStdString() } / "ParkerX" / "sylenth-ai" / "Presets";
}

std::filesystem::path legacyUserPresetDirectory()
{
    const auto music = juce::File::getSpecialLocation(juce::File::userMusicDirectory);
    return std::filesystem::path { music.getFullPathName().toStdString() } / "ParkerX" / "Synth" / "Presets";
}

std::filesystem::path defaultPresetFavoritesFile()
{
    return defaultUserPresetDirectory().parent_path() / "PresetFavorites.json";
}

std::string presetIdFromDisplayName(const std::string& displayName)
{
    std::string id;
    auto pendingDash = false;

    for (const auto character : displayName)
    {
        const auto lower = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        if ((lower >= 'a' && lower <= 'z') || (lower >= '0' && lower <= '9'))
        {
            if (pendingDash && !id.empty())
                id.push_back('-');
            id.push_back(lower);
            pendingDash = false;
        }
        else
        {
            pendingDash = true;
        }
    }

    return id.empty() ? "user-preset" : id;
}

std::vector<std::string> readFavoritePresetKeys(const std::filesystem::path& path)
{
    const auto file = juce::File(juce::String(path.string()));
    if (!file.existsAsFile())
        return {};

    const auto parsed = juce::JSON::parse(file);
    if (!parsed.isObject())
        return {};

    const auto* object = parsed.getDynamicObject();
    if (object == nullptr)
        return {};

    auto keys = std::vector<std::string> {};
    const auto favorites = object->getProperty(juce::Identifier("favorite_keys"));
    if (!favorites.isArray())
        return keys;

    if (const auto* array = favorites.getArray())
    {
        for (const auto& key : *array)
        {
            if (key.isString())
                keys.push_back(key.toString().toStdString());
        }
    }

    return normalizedFavoriteKeys(keys);
}

bool writeFavoritePresetKeys(const std::filesystem::path& path,
                             const std::vector<std::string>& favoriteKeys,
                             std::string& error)
{
    std::error_code createError;
    if (!path.parent_path().empty())
        std::filesystem::create_directories(path.parent_path(), createError);
    if (createError)
    {
        error = "could not create favorites directory: " + createError.message();
        return false;
    }

    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("schema_version", 1);

    auto keyArray = juce::Array<juce::var> {};
    for (const auto& key : normalizedFavoriteKeys(favoriteKeys))
        keyArray.add(juce::var(juce::String(key)));
    root->setProperty("favorite_keys", keyArray);

    const auto file = juce::File(juce::String(path.string()));
    if (!file.replaceWithText(juce::JSON::toString(juce::var(root.release()), true), false, false, "\n"))
    {
        error = "could not write favorites file: " + path.string();
        return false;
    }

    return true;
}

static bool writeNewTextFile(const std::filesystem::path& path, const juce::String& text, std::string& error)
{
#if JUCE_MAC || JUCE_LINUX
    const auto pathText = path.string();
    const auto descriptor = ::open(pathText.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (descriptor == -1)
    {
        error = errno == EEXIST
            ? "preset already exists: " + path.string()
            : "could not create preset file: " + std::string(std::strerror(errno));
        return false;
    }

    auto remaining = text.getNumBytesAsUTF8();
    auto* cursor = text.toRawUTF8();
    while (remaining > 0)
    {
        const auto written = ::write(descriptor, cursor, remaining);
        if (written < 0 && errno == EINTR)
            continue;

        if (written <= 0)
        {
            const auto writeError = errno;
            ::close(descriptor);
            std::error_code removeError;
            std::filesystem::remove(path, removeError);
            error = "could not write preset file: " + std::string(std::strerror(writeError));
            return false;
        }

        cursor += written;
        remaining -= static_cast<std::size_t>(written);
    }

    if (::close(descriptor) != 0)
    {
        const auto closeError = errno;
        std::error_code removeError;
        std::filesystem::remove(path, removeError);
        error = "could not close preset file: " + std::string(std::strerror(closeError));
        return false;
    }

    return true;
#else
    const auto tempPath = path.parent_path()
        / ("." + path.filename().string() + ".tmp-" + std::to_string(juce::Time::getMillisecondCounter()));
    const auto tempFile = juce::File(juce::String(tempPath.string()));
    if (!tempFile.replaceWithText(text, false, false, "\n"))
    {
        error = "could not write temporary preset file: " + tempPath.string();
        return false;
    }

    std::error_code copyError;
    const auto copied = std::filesystem::copy_file(tempPath, path, std::filesystem::copy_options::none, copyError);
    std::error_code removeError;
    std::filesystem::remove(tempPath, removeError);
    if (!copied)
    {
        error = copyError ? "could not create preset file: " + copyError.message()
                          : "preset already exists: " + path.string();
        return false;
    }

    return true;
#endif
}

bool setPresetFavorite(const std::filesystem::path& path,
                       const std::string& favoriteKey,
                       bool favorite,
                       std::string& error)
{
    auto keys = readFavoritePresetKeys(path);
    keys.erase(std::remove(keys.begin(), keys.end(), favoriteKey), keys.end());
    if (favorite && !favoriteKey.empty())
        keys.push_back(favoriteKey);

    return writeFavoritePresetKeys(path, keys, error);
}

PresetLoadResult preparePresetState(juce::AudioProcessorValueTreeState& parameters,
                                    const std::filesystem::path& path)
{
    PresetLoadResult result;
    const auto validation = validatePresetFile(path);
    if (!validation.passed())
    {
        result.message = "Preset load failed";
        if (!validation.errors.empty())
            result.message += ": " + validation.errors.front();
        return result;
    }

    auto object = readPresetObject(path);
    if (object == nullptr)
    {
        result.message = "Preset load failed: preset root object unavailable";
        return result;
    }

    auto state = parameters.copyState();
    resetStateToDefaults(state);

    const auto parameterVar = object->getProperty(juce::Identifier("parameters"));
    if (parameterVar.isObject())
    {
        if (const auto* parameterObject = parameterVar.getDynamicObject())
        {
            for (const auto& property : parameterObject->getProperties())
                applyPresetParameter(state, property.name.toString().toStdString(), property.value);
        }
    }

    const auto modSlots = object->getProperty(juce::Identifier("mod_slots"));
    if (modSlots.isArray())
    {
        if (const auto* slots = modSlots.getArray())
        {
            for (const auto& slot : *slots)
                applyModSlotObject(state, slot);
        }
    }

    result.loaded = true;
    result.displayName = propertyString(*object, "display_name");
    result.message = "Loaded preset: " + result.displayName;
    state.setProperty("schema_version", 1, nullptr);
    state.setProperty("plugin_version", SYLENTH_AI_PROJECT_VERSION, nullptr);
    state.setProperty("current_preset", juce::String(result.displayName), nullptr);
    result.fingerprint = fingerprintPresetState(state);
    result.state = state;
    return result;
}

PresetLoadResult prepareInitPresetState(juce::AudioProcessorValueTreeState& parameters)
{
    PresetLoadResult result;
    auto state = parameters.copyState();
    resetStateToDefaults(state);
    stampPreparedState(state, "Init");

    result.loaded = true;
    result.message = "Initialized preset";
    result.displayName = "Init";
    result.fingerprint = fingerprintPresetState(state);
    result.state = state;
    return result;
}

PresetLoadResult prepareRandomizedPresetState(juce::AudioProcessorValueTreeState& parameters,
                                              std::uint32_t seed)
{
    PresetLoadResult result;
    auto state = parameters.copyState();
    resetStateToDefaults(state);
    if (!applyRandomizedPatchState(state, seed))
    {
        result.message = "Randomize failed: preset parameter missing";
        return result;
    }

    const auto displayName = "Randomized " + std::to_string(seed);
    stampPreparedState(state, displayName);

    result.loaded = true;
    result.message = "Randomized preset: seed " + std::to_string(seed);
    result.displayName = displayName;
    result.fingerprint = fingerprintPresetState(state);
    result.state = state;
    return result;
}

PresetStateFingerprint fingerprintCurrentPresetState(const juce::AudioProcessorValueTreeState& parameters)
{
    const auto& specs = getParameterSpecs();
    std::vector<float> physicalValues(specs.size(), 0.0f);

    for (std::size_t index = 0; index < specs.size(); ++index)
    {
        const auto& spec = specs[index];
        auto value = spec.kind == ParameterKind::Choice
            ? static_cast<float>(spec.defaultChoice)
            : spec.defaultValue;

        if (const auto* parameter = parameters.getParameter(juce::String(spec.id)))
            value = parameter->convertFrom0to1(parameter->getValue());

        physicalValues[index] = clampPhysicalParameterValue(spec, value);
    }

    return fingerprintRegistryValues(physicalValues);
}

PresetStateFingerprint fingerprintPresetState(const juce::ValueTree& state)
{
    if (!state.isValid())
        return {};

    const auto& specs = getParameterSpecs();
    std::vector<float> physicalValues(specs.size(), 0.0f);

    for (std::size_t index = 0; index < specs.size(); ++index)
    {
        const auto& spec = specs[index];
        const auto value = stateParameterValue(state, spec.id);
        if (value.isVoid())
        {
            physicalValues[index] = spec.kind == ParameterKind::Choice
                ? static_cast<float>(spec.defaultChoice)
                : spec.defaultValue;
        }
        else
        {
            physicalValues[index] = physicalValueFromStateValue(spec, value);
        }
    }

    return fingerprintRegistryValues(physicalValues);
}

PresetDirtyState comparePresetDirtyState(const juce::AudioProcessorValueTreeState& parameters,
                                         const PresetStateFingerprint& baseline)
{
    PresetDirtyState state;
    state.current = fingerprintCurrentPresetState(parameters);
    state.baseline = baseline;
    state.dirty = !state.current.valid
        || !state.baseline.valid
        || state.current.comparedParameterCount != state.baseline.comparedParameterCount
        || state.current.hash != state.baseline.hash;
    return state;
}

PresetCompareSlot capturePresetCompareSlot(juce::AudioProcessorValueTreeState& parameters,
                                           const std::string& label)
{
    PresetCompareSlot slot;
    slot.label = label.empty() ? "Compare Slot" : label;
    slot.state = parameters.copyState();
    slot.captured = slot.state.isValid();
    slot.fingerprint = fingerprintPresetState(slot.state);
    return slot;
}

PresetLoadResult preparePresetCompareSlotState(juce::AudioProcessorValueTreeState& parameters,
                                               const PresetCompareSlot& slot)
{
    PresetLoadResult result;
    if (!slot.captured || !slot.state.isValid())
    {
        result.message = "Compare slot is empty";
        return result;
    }

    auto state = mergeParameterStateWithDefaults(parameters, slot.state);
    const auto displayName = slot.label.empty() ? std::string("Compare Slot") : slot.label;
    state.setProperty("schema_version", 1, nullptr);
    state.setProperty("plugin_version", SYLENTH_AI_PROJECT_VERSION, nullptr);
    state.setProperty("current_preset", juce::String(displayName), nullptr);
    state.setProperty("current_preset_path", "", nullptr);

    result.loaded = true;
    result.message = "Loaded compare slot: " + displayName;
    result.displayName = displayName;
    result.fingerprint = fingerprintPresetState(state);
    result.state = state;
    return result;
}

juce::ValueTree mergeParameterStateWithDefaults(juce::AudioProcessorValueTreeState& parameters,
                                                const juce::ValueTree& overrideState)
{
    auto state = parameters.copyState();
    resetStateToDefaults(state);
    state.copyPropertiesFrom(overrideState, nullptr);

    for (const auto& child : overrideState)
        overlayParameterStateChild(state, child);

    return state;
}

PresetLoadResult loadPresetIntoState(juce::AudioProcessorValueTreeState& parameters,
                                     const std::filesystem::path& path)
{
    auto result = preparePresetState(parameters, path);
    if (result.loaded)
        parameters.replaceState(result.state);

    return result;
}

bool writeCurrentPreset(const juce::AudioProcessorValueTreeState& parameters,
                        const std::filesystem::path& path,
                        const std::string& displayName,
                        std::string& error)
{
    PresetWriteOptions options;
    options.metadata.displayName = displayName;
    return writeCurrentPreset(parameters, path, options, error);
}

bool writeCurrentPreset(const juce::AudioProcessorValueTreeState& parameters,
                        const std::filesystem::path& path,
                        const PresetWriteOptions& options,
                        std::string& error)
{
    const auto destination = withJsonExtension(path);
    const auto safeName = options.metadata.displayName.empty() ? std::string("User Preset")
                                                               : options.metadata.displayName;
    std::error_code existsError;
    const auto destinationExists = std::filesystem::exists(destination, existsError);
    if (existsError)
    {
        error = "could not inspect preset destination: " + existsError.message();
        return false;
    }

    if (options.mode == PresetWriteMode::CreateNew && destinationExists)
    {
        error = "preset already exists: " + destination.string();
        return false;
    }

    std::error_code createError;
    if (!destination.parent_path().empty())
        std::filesystem::create_directories(destination.parent_path(), createError);
    if (createError)
    {
        error = "could not create preset directory: " + createError.message();
        return false;
    }

    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("schema_version", 1);
    root->setProperty("plugin_min_version", SYLENTH_AI_PROJECT_VERSION);
    root->setProperty("id", juce::String(presetIdFromDisplayName(safeName)));
    root->setProperty("display_name", juce::String(safeName));
    root->setProperty("author", juce::String(options.metadata.author.empty() ? "User"
                                                                             : options.metadata.author));
    root->setProperty("description", juce::String(options.metadata.description.empty()
        ? "User preset saved from the sylenth-ai editor."
        : options.metadata.description));
    const auto tagsToWrite = options.metadata.tags.empty()
        ? std::vector<std::string> { "user" }
        : options.metadata.tags;
    auto tags = juce::Array<juce::var> {};
    for (const auto& tag : tagsToWrite)
    {
        if (!tag.empty())
            tags.add(juce::var(juce::String(tag)));
    }
    if (tags.isEmpty())
        tags.add(juce::var("user"));
    root->setProperty("tags", tags);
    root->setProperty("parameters", currentParameterObject(parameters));
    root->setProperty("mod_slots", currentModSlotArray(parameters));
    root->setProperty("macros", currentMacroArray(parameters));

    auto metadata = std::make_unique<juce::DynamicObject>();
    metadata->setProperty("program", "sylenth_lab_rebuild");
    auto browser = std::make_unique<juce::DynamicObject>();
    browser->setProperty("bank", juce::String(options.metadata.bank.empty() ? "User"
                                                                            : options.metadata.bank));
    browser->setProperty("category", juce::String(options.metadata.category.empty() ? "User"
                                                                                    : options.metadata.category));
    browser->setProperty("source", "user");
    metadata->setProperty("browser", juce::var(browser.release()));
    root->setProperty("metadata", juce::var(metadata.release()));

    const auto presetJson = juce::JSON::toString(juce::var(root.release()), true);
    auto wrotePreset = false;
    if (options.mode == PresetWriteMode::CreateNew)
    {
        wrotePreset = writeNewTextFile(destination, presetJson, error);
    }
    else
    {
        const auto file = juce::File(juce::String(destination.string()));
        wrotePreset = file.replaceWithText(presetJson, false, false, "\n");
        if (!wrotePreset)
            error = "could not write preset file: " + destination.string();
    }

    if (!wrotePreset)
    {
        return false;
    }

    const auto validation = validatePresetFile(destination);
    if (!validation.passed())
    {
        error = "saved preset did not validate";
        if (!validation.errors.empty())
            error += ": " + validation.errors.front();
        return false;
    }

    return true;
}
} // namespace synth
