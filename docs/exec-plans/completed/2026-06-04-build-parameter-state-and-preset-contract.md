---
title: Build Parameter State And Preset Contract
status: complete
created_at: 2026-06-04
completed_at: 2026-06-04
summary: Add stable parameter IDs, host state serialization, preset JSON parsing, migrations, and factory preset storage.
post_build_recap: Added a 102-parameter registry, APVTS host-state layout, JSON preset validator, factory init/pluck presets, SynthRender parameter/preset commands, and contract tests.
read_when:
  - Implementing Synth parameter or preset behavior.
  - Changing host state serialization.
  - Adding or migrating factory presets.
program_id: synth-clean-room-pluck-instrument
planning_brief: docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/planning-brief-1.md
---

# Build Parameter State And Preset Contract

This document is a living execution plan for the stable parameter, host-state, and preset contract.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Every DSP and UI slice needs stable parameter IDs, units, defaults, smoothing rules, and serialization behavior. After this slice, Synth should have a central parameter registry, host state round-trip, JSON preset schema, migration hooks, and clean-room factory preset storage. Later work can add sound modules without inventing ad hoc parameters.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [x] 2026-06-04 EDT: Inspected scaffold files and confirmed no parameter/state/preset implementation existed.
- [x] 2026-06-04 EDT: Added `src/plugin/ParameterRegistry.*` with 102 stable parameter specs and JUCE APVTS layout creation.
- [x] 2026-06-04 EDT: Wired `SynthAudioProcessor` host state through `AudioProcessorValueTreeState`.
- [x] 2026-06-04 EDT: Added `src/presets/PresetValidator.*`, `SynthRender --list-parameters`, and `SynthRender --validate-presets`.
- [x] 2026-06-04 EDT: Added `presets/factory/init.json` and `presets/factory/pluck-core-01.json`.
- [x] 2026-06-04 EDT: Added `SynthContractTest` covering registry validation, APVTS state round-trip, and factory preset validation.
- [x] 2026-06-04 EDT: Ran build, CTest, parameter report, preset report, and smoke render successfully.

## Surprises & Discoveries

CTest initially failed because `SynthContractTest` ran from `build/` and could not see `presets/factory`. Setting CTest working directories to the repo root fixed the test path without hardcoding absolute paths.

Linking the validation/test targets against `juce_audio_processors` increased build time, but it keeps the parameter registry's APVTS layout creation shared with the plugin instead of duplicating layout logic.

## Decision Log

Decision: Parameter IDs are centralized before DSP and UI work.
Rationale: Stable IDs are hard to change after host automation and presets exist.
Date: 2026-06-04.

Decision: Preset files may be sparse in this slice.
Rationale: The full registry exists, but real DSP and UI are not implemented yet. Sparse clean-room factory presets keep the stored values meaningful while defaults remain in the registry.
Date: 2026-06-04.

Decision: Use JUCE `var`/`JSON` for preset validation.
Rationale: It avoids adding a new JSON dependency before the project has a stronger need.
Date: 2026-06-04.

## Outcomes & Retrospective

The parameter/state/preset slice is complete. Synth now has a broad v1 parameter inventory, APVTS host state, clean-room factory preset files, command-line parameter and preset reports, and a contract test that validates state round-trip and preset correctness.

Migration hooks are represented by schema versioning and validator structure, but no historical migration is needed yet because schema version 1 is the first schema.

## Context and Orientation

`docs/PRESET_SCHEMA.md` sketches the preset shape. `SPEC.md` section 4.1.2 defines the parameter entity, and section 8.4 defines the preset file contract. The scaffold slice should have created `src/plugin/`, `src/presets/`, `presets/factory/`, and tests.

### In Scope

This plan may add `src/plugin/ParameterRegistry.*`, `src/plugin/PluginState.*`, `src/presets/PresetSchema.*`, `src/presets/PresetMigration.*`, `src/presets/FactoryPresets.*`, JSON schema files, factory preset JSON files, and tests.

### Out Of Scope

This plan does not implement real oscillator/filter/modulation DSP, the editor UI, Ableton smoke tests, or final factory sound design. It may create placeholder parameters and presets needed for later slices.

## Plan of Work

Define the full v1 parameter inventory early enough for automation and presets. Each parameter must have an ID, group, name, unit, range, default, smoothing policy, automation flag, and preset serialization flag. Include parameters for voice, oscillator, mixer, filter, amp, envelopes, LFO, ramp, direct modulation, TransMod slots, macros, FX, and global settings.

Implement host state serialization using JUCE-supported state storage. The state must include schema version and all plugin settings required to restore without external preset files.

Implement JSON preset parsing and writing with explicit error classes. Add migrations even if only version 1 exists, so future changes have a place to go. Add `presets/factory/init.json` and `presets/factory/pluck-core-01.json` with clean-room safe display names.

Update `docs/PRESET_SCHEMA.md` if actual field names differ from the current sketch.

## Milestones

Milestone 1 adds the registry and tests for duplicate IDs, ranges, defaults, and display conversion.

Milestone 2 adds host state round-trip tests.

Milestone 3 adds preset parse/write/validate/migrate tests.

Milestone 4 adds clean-room factory preset files and docs sync.

## Concrete Steps

Work from the repo root:

    cd /Users/bazelrex/Developer/synth

Inspect actual scaffold files:

    find src tests presets -maxdepth 3 -type f | sort

Implement the registry and preset modules. Then run:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --list-parameters --output build/reports/parameters.json
    ./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json

If `SynthRender` does not yet support these commands, add them in this slice.

## Validation and Acceptance

Acceptance requires tests proving no duplicate parameter IDs, all default values are valid, host state round-trips, invalid presets fail safely, old schema fixture migration works, and factory presets validate.

Observed proof on 2026-06-04:

    ctest: 2/2 tests passed
    parameters.json: parameter_count 102, error_count 0, passed true
    presets.json: preset_count 2, error_count 0, warning_count 0, passed true
    smoke.json: invalid_samples 0, peak 0, passed true

### Test Commands

    cd /Users/bazelrex/Developer/synth
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --list-parameters --output build/reports/parameters.json
    ./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json

## Idempotence and Recovery

Parameter additions are additive until public release, but avoid churn. If a parameter ID changes during this slice, update presets and tests immediately. Invalid preset fixtures should remain in tests; do not delete them to make tests pass.

## Artifacts and Notes

Expected proof artifacts:

- `build/reports/parameters.json`
- `build/reports/presets.json`
- factory preset JSON files
- host state round-trip test output

## Interfaces and Dependencies

This slice creates the parameter and preset interfaces that DSP, UI, validation, and packaging slices depend on. Later plans must add new parameters through the registry rather than local constants.
