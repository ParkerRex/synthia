---
title: Build Sylenth Layer Oscillator Backbone
status: active
created_at: 2026-06-05
completed_at: null
summary: Establish Phase 1 A/B layer and four-oscillator-slot state contracts, migrations, and validation while preserving the current Layer A sound.
post_build_recap: null
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
- [ ] Inventory current parameter IDs and preset fields affected by layer/oscillator namespacing.
- [ ] Add layer and oscillator-slot state structs.
- [ ] Add registry entries or migration aliases for Layer A and future Layer B without breaking existing presets.
- [ ] Preserve current engine as Layer A and prove old preset equivalence.
- [ ] Add validation for layer/slot defaults, migration, and voice math.
- [ ] Update docs with the implemented contracts.

## Surprises & Discoveries

None yet. Record parameter migration, APVTS, preset schema, host automation, or CPU surprises here.

## Decision Log

Decision: Do the backbone before UI polish.
Rationale: Sylenth-style UI affordances need real A/B and oscillator-slot state. Fake controls would create brittle preset and automation semantics.
Date: 2026-06-05.

## Outcomes & Retrospective

Pending implementation.

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

- parameter report before/after
- preset validation report
- core suite report
- docs updates for layer/slot contracts

## Interfaces and Dependencies

This slice creates dependencies for:

- `2026-06-05-handoff-modern-sylenth-ui-shell.md`
- future Layer B/four-oscillator rendering plan
- future modulation route model
- future preset browser and arp UI handoff

