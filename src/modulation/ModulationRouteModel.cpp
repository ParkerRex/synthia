#include "ModulationRouteModel.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace synth
{
namespace
{
bool finiteNonZero(float value) noexcept
{
    return std::isfinite(value) && std::abs(value) > 0.000001f;
}

ModSource sanitizeSource(ModSource source) noexcept
{
    const auto value = static_cast<int>(source);
    if (value < static_cast<int>(ModSource::None) || value > static_cast<int>(ModSource::Macro4))
        return ModSource::None;

    return source;
}

float depthForDestination(const TransModSlotParameters& slot,
                          ModulationDestination destination) noexcept
{
    switch (destination)
    {
        case ModulationDestination::OscPitch:
            return std::clamp(slot.oscPitchSemitones, -48.0f, 48.0f);
        case ModulationDestination::PulseWidth:
            return std::clamp(slot.pulseWidth, -1.0f, 1.0f);
        case ModulationDestination::FilterCutoff:
            return std::clamp(slot.filterCutoffSemitones, -72.0f, 72.0f)
                   + std::clamp(slot.depth, -1.0f, 1.0f) * 72.0f;
        case ModulationDestination::AmpLevel:
            return std::clamp(slot.ampLevelDb, -24.0f, 24.0f);
        case ModulationDestination::Pan:
            return std::clamp(slot.pan, -1.0f, 1.0f);
    }

    return 0.0f;
}

bool hasDepthForDestination(const TransModSlotParameters& slot,
                            ModulationDestination destination) noexcept
{
    switch (destination)
    {
        case ModulationDestination::OscPitch:
            return finiteNonZero(slot.oscPitchSemitones);
        case ModulationDestination::PulseWidth:
            return finiteNonZero(slot.pulseWidth);
        case ModulationDestination::FilterCutoff:
            return finiteNonZero(slot.filterCutoffSemitones) || finiteNonZero(slot.depth);
        case ModulationDestination::AmpLevel:
            return finiteNonZero(slot.ampLevelDb);
        case ModulationDestination::Pan:
            return finiteNonZero(slot.pan);
    }

    return false;
}

std::vector<ModulationSourceInfo> buildSourceCatalog()
{
    return {
        { ModSource::None, "none", "None", ModulationPolarity::Unipolar, ModulationScope::Global,
          ModulationUpdateRate::Control, {} },
        { ModSource::Lfo, "lfo.1", "LFO 1", ModulationPolarity::Bipolar, ModulationScope::Voice,
          ModulationUpdateRate::Audio, "lfo.shape" },
        { ModSource::Ramp, "ramp", "Ramp", ModulationPolarity::Unipolar, ModulationScope::Voice,
          ModulationUpdateRate::Audio, "ramp.enabled" },
        { ModSource::ModEnv, "mod_env", "Mod Env", ModulationPolarity::Unipolar, ModulationScope::Voice,
          ModulationUpdateRate::Audio, "mod_env.attack_ms" },
        { ModSource::AmpEnv, "amp_env", "Amp Env", ModulationPolarity::Unipolar, ModulationScope::Voice,
          ModulationUpdateRate::Audio, "amp_env.attack_ms" },
        { ModSource::Keytrack, "keytrack", "Keytrack", ModulationPolarity::Bipolar, ModulationScope::Voice,
          ModulationUpdateRate::Note, "filter.keytrack" },
        { ModSource::Velocity, "velocity", "Velocity", ModulationPolarity::Unipolar, ModulationScope::Voice,
          ModulationUpdateRate::Note, {} },
        { ModSource::VelocityGlide, "velocity_glide", "Velocity Glide", ModulationPolarity::Unipolar,
          ModulationScope::Voice, ModulationUpdateRate::Audio, "voice.velocity_glide_ms" },
        { ModSource::PitchBend, "pitch_bend", "Pitch Bend", ModulationPolarity::Bipolar,
          ModulationScope::Global, ModulationUpdateRate::Control, {} },
        { ModSource::ModWheel, "mod_wheel", "Mod Wheel", ModulationPolarity::Unipolar,
          ModulationScope::Global, ModulationUpdateRate::Control, {} },
        { ModSource::Aftertouch, "aftertouch", "Aftertouch", ModulationPolarity::Unipolar,
          ModulationScope::Global, ModulationUpdateRate::Control, {} },
        { ModSource::VoiceUni, "voice_uni", "Voice Index +", ModulationPolarity::Unipolar,
          ModulationScope::Voice, ModulationUpdateRate::Note, {} },
        { ModSource::VoiceBi, "voice_bi", "Voice Index +/-", ModulationPolarity::Bipolar,
          ModulationScope::Voice, ModulationUpdateRate::Note, {} },
        { ModSource::UnisonUni, "unison_uni", "Unison Index +", ModulationPolarity::Unipolar,
          ModulationScope::Voice, ModulationUpdateRate::Note, {} },
        { ModSource::UnisonBi, "unison_bi", "Unison Index +/-", ModulationPolarity::Bipolar,
          ModulationScope::Voice, ModulationUpdateRate::Note, {} },
        { ModSource::RandomOnNote, "random_on_note", "Random", ModulationPolarity::Bipolar,
          ModulationScope::Voice, ModulationUpdateRate::Note, {} },
        { ModSource::Macro1, "macro.motion", "Macro Motion", ModulationPolarity::Unipolar,
          ModulationScope::Global, ModulationUpdateRate::Control, "macro.motion" },
        { ModSource::Macro2, "macro.width", "Macro Width", ModulationPolarity::Unipolar,
          ModulationScope::Global, ModulationUpdateRate::Control, "macro.width" },
        { ModSource::Macro3, "macro.drive", "Macro Drive", ModulationPolarity::Unipolar,
          ModulationScope::Global, ModulationUpdateRate::Control, "macro.drive" },
        { ModSource::Macro4, "macro.space", "Macro Space", ModulationPolarity::Unipolar,
          ModulationScope::Global, ModulationUpdateRate::Control, "macro.space" }
    };
}

std::vector<ModulationDestinationInfo> buildDestinationCatalog()
{
    return {
        { ModulationDestination::OscPitch, "osc.pitch", "Osc Pitch", "osc.pitch_semitones",
          "osc_pitch_semitones", "semitones", -48.0f, 48.0f },
        { ModulationDestination::PulseWidth, "osc.pulse_width", "Pulse Width", "osc.pulse_width",
          "pulse_width", "normalized", -1.0f, 1.0f },
        { ModulationDestination::FilterCutoff, "filter.cutoff", "Filter Cutoff", "filter.cutoff_semitones",
          "filter_cutoff_semitones", "semitones", -144.0f, 144.0f },
        { ModulationDestination::AmpLevel, "amp.level", "Amp Level", "amp.level_db",
          "amp_level_db", "dB", -24.0f, 24.0f },
        { ModulationDestination::Pan, "amp.pan", "Pan", "amp.pan",
          "pan", "normalized", -1.0f, 1.0f }
    };
}
} // namespace

const std::vector<ModulationSourceInfo>& modulationSourceCatalog()
{
    static const auto catalog = buildSourceCatalog();
    return catalog;
}

const std::vector<ModulationDestinationInfo>& modulationDestinationCatalog()
{
    static const auto catalog = buildDestinationCatalog();
    return catalog;
}

const ModulationSourceInfo* findModulationSourceInfo(ModSource source)
{
    source = sanitizeSource(source);
    const auto& catalog = modulationSourceCatalog();
    const auto found = std::find_if(catalog.begin(), catalog.end(), [source](const auto& info) {
        return info.source == source;
    });
    return found == catalog.end() ? nullptr : &*found;
}

const ModulationSourceInfo* findModulationSourceInfo(const std::string& id)
{
    const auto& catalog = modulationSourceCatalog();
    const auto found = std::find_if(catalog.begin(), catalog.end(), [&id](const auto& info) {
        return info.id == id;
    });
    return found == catalog.end() ? nullptr : &*found;
}

const ModulationDestinationInfo* findModulationDestinationInfo(ModulationDestination destination)
{
    const auto& catalog = modulationDestinationCatalog();
    const auto found = std::find_if(catalog.begin(), catalog.end(), [destination](const auto& info) {
        return info.destination == destination;
    });
    return found == catalog.end() ? nullptr : &*found;
}

const ModulationDestinationInfo* findModulationDestinationInfo(const std::string& id)
{
    const auto& catalog = modulationDestinationCatalog();
    const auto found = std::find_if(catalog.begin(), catalog.end(), [&id](const auto& info) {
        return info.id == id;
    });
    return found == catalog.end() ? nullptr : &*found;
}

std::string transModDepthParameterId(int slotNumber, const ModulationDestinationInfo& destination)
{
    if (slotNumber < 1 || slotNumber > transModSlotCount)
        return {};

    return "transmod." + std::to_string(slotNumber) + "." + destination.depthSuffix;
}

ModulationRouteView buildModulationRouteView(const TransModParameters& transMod)
{
    ModulationRouteView view;
    view.slots.reserve(transModSlotCount);
    view.activeRoutes.reserve(transModSlotCount * modulationDestinationCatalog().size());

    for (int slotIndex = 0; slotIndex < transModSlotCount; ++slotIndex)
    {
        const auto slotNumber = slotIndex + 1;
        const auto& slot = transMod.slots[static_cast<std::size_t>(slotIndex)];
        const auto source = sanitizeSource(slot.source);
        const auto scaler = sanitizeSource(slot.scaler);
        const auto* sourceInfo = findModulationSourceInfo(source);
        const auto* scalerInfo = findModulationSourceInfo(scaler);

        ModulationSlotSummary slotSummary;
        slotSummary.slotNumber = slotNumber;
        slotSummary.enabled = slot.enabled && source != ModSource::None;
        slotSummary.source = source;
        slotSummary.scaler = scaler;
        slotSummary.sourceId = sourceInfo != nullptr ? sourceInfo->id : "none";
        slotSummary.scalerId = scalerInfo != nullptr ? scalerInfo->id : "none";

        if (slotSummary.enabled)
        {
            for (const auto& destination : modulationDestinationCatalog())
            {
                const auto depth = depthForDestination(slot, destination.destination);
                if (!hasDepthForDestination(slot, destination.destination))
                    continue;

                ModulationRouteSummary route;
                route.slotNumber = slotNumber;
                route.source = source;
                route.scaler = scaler;
                route.destination = destination.destination;
                route.sourceId = slotSummary.sourceId;
                route.scalerId = slotSummary.scalerId;
                route.destinationId = destination.id;
                route.depthParameterId = transModDepthParameterId(slotNumber, destination);
                route.depthParameterIds.push_back(route.depthParameterId);
                if (destination.destination == ModulationDestination::FilterCutoff
                    && finiteNonZero(slot.depth))
                {
                    const auto legacyDepthId = "transmod." + std::to_string(slotNumber) + ".depth";
                    route.depthParameterIds.push_back(legacyDepthId);
                    if (!finiteNonZero(slot.filterCutoffSemitones))
                        route.depthParameterId = legacyDepthId;
                }
                route.depth = std::clamp(depth, destination.minimumDepth, destination.maximumDepth);
                route.enabled = true;

                slotSummary.routes.push_back(route);
                view.activeRoutes.push_back(std::move(route));
            }
        }

        view.slots.push_back(std::move(slotSummary));
    }

    return view;
}
} // namespace synth
