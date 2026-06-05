---
title: Build Sylenth Layer Oscillator Backbone
status: completed
created_at: 2026-06-05
completed_at: 2026-06-05
summary: Establish Phase 1 A/B layer and four-oscillator-slot state contracts, migrations, and validation while preserving the current Layer A sound.
post_build_recap: Added A/B layer and A1/A2/B1/B2 oscillator-slot APVTS/preset state, realtime snapshots, engine-boundary clamping, legacy preset defaults, layer voice-cost validation, and docs while preserving the current flat osc audio path.
read_when:
  - Starting the Phase 1 Sylenth rebuild implementation.
  - Changing layer, oscillator-slot, parameter, preset, or host-state contracts.
  - Preparing dependencies for Claude Code UI handoff.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Build Sylenth Layer Oscillator Backbone

This document is a living execution plan for the first implementation slice of the Sylenth Lab Rebuild Program.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Phase 1 must become a real Sylenth-level instrument, not a skinned single-core synth. This slice creates the parameter, state, preset, and validation backbone for A/B layers and four oscillator slots while preserving the current sound path as Layer A. After this slice, UI work can reference real state contracts instead of fake A/B controls.

## Progress

- [x] 2026-06-05 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [x] 2026-06-05 EDT: Inventoried current flat `osc.*`, `filter.*`, envelope, modulation, amp, FX, macro, and TransMod parameter/preset contracts.
- [x] 2026-06-05 EDT: Added `LayerParameters` and `LayerOscillatorParameters` state for Layer A/B and A1/A2/B1/B2.
- [x] 2026-06-05 EDT: Added 62 namespaced `layer.*` registry entries while leaving old IDs stable.
- [x] 2026-06-05 EDT: Preserved current audio behavior by keeping the flat `osc.*` render path active and mapping Layer A defaults to the current primary sound path.
- [x] 2026-06-05 EDT: Added validation for layer/slot defaults, legacy preset load, saved preset serialization, and slot voice-cost math.
- [x] 2026-06-05 EDT: Updated README, architecture, preset schema, and modern Sylenth baseline docs with the implemented contracts.
- [x] 2026-06-05 EDT: Added host-state default-merge and current-render no-op regression coverage for the layer/slot backbone.

## Surprises & Discoveries

- The pre-change `SynthRender` report commands could not run until `build/` was configured in this checkout. After `cmake -S . -B build -DSYNTH_ENABLE_TESTS=ON`, post-change reports generated normally.
- The current audio path can remain untouched for this slice because the new layer/slot state is a host/preset/UI contract, not a rendered signal graph yet.

## Decision Log

Decision: Do the backbone before UI polish.
Rationale: Sylenth-style UI affordances need real A/B and oscillator-slot state. Fake controls would create brittle preset and automation semantics.
Date: 2026-06-05.

Decision: Keep the existing flat `osc.*` path as the current audio path and host-stable compatibility surface.
Rationale: Existing presets, automation, and render tests already depend on it. Namespaced `layer.*` state can prepare UI and future DSP without breaking current sound.
Date: 2026-06-05.

Decision: Do not add `layer.N.name` as an automatable parameter.
Rationale: APVTS parameters are numeric/boolean/choice sound-state controls. Custom layer names should be preset metadata or UI-local state if added later.
Date: 2026-06-05.

## Outcomes & Retrospective

Completed. The registry now reports 218 parameters. Layer A/B and four oscillator-slot state exist in APVTS, preset serialization, host-state default migration, realtime snapshots, and validation. Layer B is valid but disabled by default. Current rendering remains equivalent because the flat oscillator/filter/envelope/modulation path still drives audio, and regression coverage proves extreme layer/slot state changes do not alter the current render path.

Follow-up: the next engine slice should wrap the current voice path in a layer render unit, add Layer B/four-slot rendering, and add render tests for independent slots, layer mixer, solo/mute, and patch-cost behavior.

## Context and Orientation

Read first:

- `SPEC.md`
- `CONTEXT.md`
- `docs/modern-sylenth-baseline.md`
- `docs/ARCHITECTURE.md`
- `docs/PRESET_SCHEMA.md`
- `docs/VALIDATION.md`
- `src/plugin/ParameterRegistry.*`
- `src/dsp/SynthParameters.h`
- `src/dsp/SynthEngine.*`
- `src/voice/Voice.*`
- `src/presets/PresetManager.*`
- `src/presets/PresetValidator.*`
- `tests/smoke/*`

### In Scope

This slice may update parameter registry, DSP parameter snapshots, preset schema/migration, voice/layer data structures, validation reports, and focused tests.

### Out Of Scope

This slice does not implement the final polished UI, full Layer B rendering, full four-slot audio rendering, arp engine, FX rack expansion, AI generation, or conversational editing.

## Plan of Work

Start by inventorying current flat parameters and deciding which existing IDs remain host-stable aliases for Layer A. Introduce explicit layer and oscillator-slot state in the narrowest form needed for migration and future rendering.

Keep current audio behavior intact. The first implementation should map existing oscillator/filter/envelope/modulation state into Layer A defaults and leave Layer B disabled by default. Four oscillator slots may exist as state and preset schema before all four render independently.

Add validation before exposing UI. Tests should prove unique IDs, valid defaults, preset roundtrip, legacy preset migration, and Layer A render equivalence within tolerance.

## Milestones

Milestone 1: parameter and preset inventory.

Milestone 2: layer/slot state contracts and defaults.

Milestone 3: preset migration and legacy compatibility.

Milestone 4: Layer A equivalence and voice-math validation.

Milestone 5: docs sync and Claude Code UI dependency handoff notes.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/synth

Inspect current contracts:

    ./build/SynthRender --list-parameters --output build/reports/parameters-before-layer.json
    ./build/SynthRender --validate-presets presets/factory --output build/reports/presets-before-layer.json

Note: those pre-change commands require an existing `build/` tree. In this run, `build/` was configured after the failed pre-change attempt.

After implementation, run:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --suite core --output-dir build/reports/core
    ./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json

## Validation and Acceptance

Acceptance requires:

- Existing factory presets validate and load.
- Existing current-core renders remain equivalent within documented tolerance.
- Layer A defaults map to the current sound path.
- Layer B exists as disabled, valid state without affecting current renders.
- Four oscillator-slot state has stable defaults, validation, and migration path.
- Parameter and preset changes are documented in `docs/PRESET_SCHEMA.md` and `docs/ARCHITECTURE.md`.
- No UI plan claims implementation readiness beyond the state contracts actually delivered.

### Test Commands

    cd /Users/parkerrex/Developer/synth
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --suite core --output-dir build/reports/core
    ./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json

## Idempotence and Recovery

Preset migration must be deterministic and rerunnable. Unknown future fields should be preserved where possible. If host-automation aliases become risky, keep old IDs stable and add new namespaced fields only where they do not break existing behavior.

## Artifacts and Notes

Expected artifacts:

- `build/reports/parameters.json` with 218 parameters
- `build/reports/presets.json`
- `build/reports/core/`
- `build/reports/pluck-core-01-dry.json`
- docs updates for layer/slot contracts

## Interfaces and Dependencies

This slice creates dependencies for:

- `2026-06-05-handoff-modern-sylenth-ui-shell.md`
- future Layer B/four-oscillator rendering plan
- future modulation route model
- future preset browser and arp UI handoff
