---
title: Build Preset Command Model
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Add backend Init, Reset, and seedable Randomize preset commands as ordinary APVTS state preparation.
post_build_recap: Added `prepareInitPresetState()`, `prepareRandomizedPresetState()`, processor command APIs, bounded deterministic random patch generation, APVTS replacement through the existing preset path, and contract tests for default/init state, seed repeatability, bounds, and live-state isolation.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
read_when:
  - Wiring UI buttons for Init, Reset, or Randomize.
  - Adding AI-generated preset commands.
  - Auditing preset state replacement, randomization bounds, or host-state behavior.
  - Planning safe overwrite, dirty state, A/B compare, or preset provenance.
---

# Build Preset Command Model

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Phase 1 needs Sylenth-style preset commands that are real model actions, not decorative header buttons. This slice adds the backend command surface for Init, Reset, and bounded Randomize so later UI polish can bind explicit controls to safe state transitions.

The command model keeps generated patches editable. Init and Randomize produce normal APVTS parameter state, Reset reloads the current preset path when one exists, and no new runtime preset schema is introduced.

## Progress

- [x] 2026-06-06 EDT: Added init state preparation that resets APVTS state to registry defaults and stamps transient preset metadata.
- [x] 2026-06-06 EDT: Added deterministic seed-based random state preparation over bounded musical parameter ranges.
- [x] 2026-06-06 EDT: Added processor command APIs for initialize, reset, randomize, and randomize with explicit seed.
- [x] 2026-06-06 EDT: Reused the existing APVTS replacement path and panic request after command application.
- [x] 2026-06-06 EDT: Added contract coverage for init defaults, seed repeatability, random bounds, no live-state mutation before replace, and APVTS apply behavior.
- [x] 2026-06-06 EDT: Updated architecture, preset schema, baseline, completed ExecPlan index, and Program ledger docs.

## Surprises & Discoveries

- Randomization needed to start from registry defaults rather than mutate current state. That prevents stale values from the current preset from leaking into commands that intentionally skip parameters.
- Layer A oscillator 1 must be forced on in generated state because it gates the legacy `osc.*` compatibility path. Without that, a randomized patch could accidentally prepare a silent state even when the core oscillator values are valid.
- Seeded randomization is useful before UI exists because tests and future AI workflows can ask for reproducible command output.

## Decision Log

Decision: Prepare Init and Randomize as APVTS `ValueTree` states instead of adding preset JSON files or alternate command data.
Rationale: Host state, preset save, automation, UI, and AI should all see the same editable parameter state.
Date: 2026-06-06.

Decision: Keep randomization bounded and conservative.
Rationale: This is a Phase 1 musical command, not a chaos generator. It must avoid invalid audio, runaway FX, excessive gain, and impossible parameter combinations.
Date: 2026-06-06.

Decision: Leave UI controls, safe overwrite, dirty tracking, A/B compare, delete, and AI provenance out of this slice.
Rationale: Those features need separate UI and persistence rules. This slice only supplies model-ready command actions.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. `PresetManager` can now prepare Init and seeded randomized APVTS states without mutating live parameters. `SynthAudioProcessor` exposes command methods that apply prepared states, update current preset metadata, and request panic so old held voices/FX tails do not leak across commands. Reset reloads the current preset file when a path exists and falls back to Init for transient/generated states.

The randomizer covers the current sound-state subset: voice mode/polyphony/unison/glide, Layer A and optional A2, oscillator core, filter, envelopes, LFO/direct modulation, amp/macros, and conservative saturation/delay/reverb/quality fields. It does not randomize browser metadata, arp/chord lanes, full TransMod routes, delete/copy/paste, A/B state, or AI provenance.

## Context and Orientation

Relevant current code and docs:

- `src/presets/PresetManager.h`
- `src/presets/PresetManager.cpp`
- `src/plugin/PluginProcessor.h`
- `src/plugin/PluginProcessor.cpp`
- `tests/smoke/SynthContractTest.cpp`
- `docs/ARCHITECTURE.md`
- `docs/PRESET_SCHEMA.md`
- `docs/modern-sylenth-baseline.md`

### In Scope

- Backend Init command.
- Backend Reset command.
- Backend randomize command with explicit seed support.
- Conservative randomization bounds over current parameters.
- Contract tests for command state preparation and APVTS replacement.
- Durable docs and Program ledger updates.

### Out Of Scope

- Header buttons or visual UI polish.
- Safe overwrite prompts.
- Dirty state or original-state snapshots.
- Delete, insert, copy/paste, A/B compare, or metadata editing.
- Arp/chord/matrix randomization.
- AI prompt handling, reference-sound analysis, or generation provenance.
- Ableton-side command smoke.

## Plan of Work

1. Add pure preset-state preparation helpers for Init and Randomize.
2. Keep randomization deterministic with an explicit seed.
3. Wire processor command methods to the existing preset replacement boundary.
4. Add focused contract tests.
5. Update durable docs and run validation.

## Milestones

Milestone 1 added command-prepared APVTS states.

Milestone 2 exposed processor methods that later UI controls can call.

Milestone 3 added contract coverage for determinism, bounds, and state isolation.

Milestone 4 updated docs and Program ledgers.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --config Debug
    ./build/SylenthAIContractTest
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Validation and Acceptance

Acceptance requires:

- Init prepares registry-default APVTS state named `Init`.
- Preparing Init or Randomize does not mutate live parameters before replacement.
- Randomize with the same seed prepares identical representative parameter values.
- Randomize with a different seed changes representative state.
- Randomized stack count, filter cutoff, and compatibility A1 state stay within safe bounds.
- Prepared randomized state applies to APVTS parameters through `replaceState`.
- Full CTest and core standalone render validation pass.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    cmake --build build --config Debug
    ./build/SylenthAIContractTest
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Idempotence and Recovery

Init and Randomize are pure preparation steps until the processor applies the returned state. If a command fails, the current live state remains unchanged. If later randomization expands to arp, chord, FX, or modulation routes, add bounded state writes and tests for each new parameter family rather than weakening existing constraints.

Reset depends on the current preset path. If the current state has no file path because it is Init, Randomized, or otherwise transient, Reset intentionally falls back to Init.

## Artifacts and Notes

No source-controlled runtime artifacts are produced. Disposable validation reports remain under `build/reports/`.

Remaining follow-up: add UI controls for Init/Randomize/Reset, dirty state, safe overwrite, A/B compare, and measured finite render proof for broader AI/randomized patch classes.

## Interfaces and Dependencies

This slice depends on `PresetManager` default-state preparation, `ParameterRegistry` ranges/clamping, `SynthAudioProcessor` APVTS replacement, and `SynthContractTest`. It feeds Claude UI header/browser controls and future Phase 2 AI preset generation.
