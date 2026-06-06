---
title: Build Modulation Route Model
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Expose a first-class modulation source and destination catalog plus route-view API over the existing TransMod engine.
post_build_recap: Added ModulationRouteModel source/destination catalogs, route and slot summaries, processor read API, contract coverage, and docs handoff updates; adversarial review found and the slice fixed legacy cutoff-depth cancellation visibility.
read_when:
  - Preparing modulation source tiles, halos, matrix sync, or AI edit reports.
  - Auditing how existing TransMod slots become UI-visible route rows.
  - Expanding modulation destinations without changing the audio thread contract.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
---

# Build Modulation Route Model

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Phase 1 modulation UX needs source tiles, target halos, and a synchronized matrix. The engine already has deterministic eight-slot TransMod DSP. This slice exposes that state as an explicit source catalog, destination catalog, and route view so UI and AI workflows can inspect real modulation routes without inventing hidden UI-only state.

## Progress

- [x] 2026-06-06 EDT: Created this ExecPlan from the Sylenth Lab Rebuild program dependency graph.
- [x] 2026-06-06 EDT: Added `src/modulation/ModulationRouteModel.*` with source metadata, destination metadata, route summaries, slot summaries, and a TransMod-to-route-view builder.
- [x] 2026-06-06 EDT: Added `SynthAudioProcessor::getModulationRouteView()` for control-side UI reads.
- [x] 2026-06-06 EDT: Added contract coverage for catalog contents, active route rows, scaler/source IDs, and legacy cutoff-depth contribution.
- [x] 2026-06-06 EDT: Ran adversarial review; fixed the discovered cancellation case where physical cutoff depth and legacy cutoff depth netted to zero and could hide present legacy state.
- [x] 2026-06-06 EDT: Passed `git diff --check`, focused contract build/test, standalone build, modulation render report, full CTest, and core render suite.

## Surprises & Discoveries

- Existing `transmod.N.depth` is a legacy normalized cutoff-depth field. The route view must surface it as a contributing parameter when it creates cutoff movement so future UI does not hide compatibility state.

## Decision Log

Decision: Keep the route model as a view over existing TransMod slots for this slice.
Rationale: The DSP and preset path already validate the eight-slot model. A view layer unblocks UI and AI edit reporting without destabilizing host automation IDs.
Date: 2026-06-06.

Decision: Do not add drag/drop, halos, bypass-per-destination, or new destination parameters in this slice.
Rationale: Those are UI and schema-expansion work. This slice only makes the current state inspectable and testable.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. `ModulationRouteModel` now exposes the current TransMod state as stable source/destination catalogs and explicit route rows for UI, validation, and future AI edit summaries. It keeps the existing audio engine and APVTS IDs intact, including visibility for legacy `transmod.N.depth` cutoff movement even when it cancels against physical cutoff depth.

Remaining work is UI/write-side behavior: drag/drop assignment, matrix editing, per-route bypass/remove, expanded destination creation, and visual halos should become a Claude Code handoff or a later write-adapter/schema slice.

## Context and Orientation

The current modulation engine lives in `SynthParameters::transMod`, `Voice::evaluateTransMod`, APVTS `transmod.N.*` parameters, preset `mod_slots`, and `SylenthAIRender --modulation-test`.

The route model is for non-realtime readers: editor rendering, matrix synchronization, Claude Code UI handoffs, validation reports, and future AI edit summaries. It must not be called from the audio thread.

### In Scope

- Modulation source catalog with ID, label, polarity, scope, update-rate, and anchor parameter where one exists.
- Modulation destination catalog for current physical TransMod destinations.
- Route summaries built from enabled TransMod slots and nonzero depths.
- Legacy `transmod.N.depth` cutoff compatibility visibility.
- Processor API for the editor to read the route view.
- Contract tests and docs updates.

### Out Of Scope

- New DSP behavior.
- More than eight TransMod slots.
- Per-destination bypass/remove state.
- Drag/drop UI, halos, hover inspector, or matrix drawer implementation.
- New modulatable target parameters beyond the current TransMod destination set.

## Plan of Work

Add a small model module under `src/modulation/`. Keep it dependency-light and deterministic. Wire it into common sources so plugin, renderer, and tests can use the same API. Add contract tests that prove route rows are generated only from enabled slots with real source and nonzero depth, including the legacy cutoff-depth path.

Update the UI handoff plan after validation so modulation polish can move from blocked to model-ready.

## Milestones

Milestone 1 adds source and destination catalogs.

Milestone 2 builds slot and route summaries from `TransModParameters`.

Milestone 3 exposes the processor read API.

Milestone 4 validates the route model and updates handoff/program docs.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai

Build and test:

    cmake --build build --target SylenthAIContractTest -j2
    ./build/SylenthAIContractTest
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --modulation-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation-route-model.json

## Validation and Acceptance

Acceptance requires the route model to expose all current sources, all current physical destinations, route summaries for enabled nonzero TransMod destinations, legacy cutoff-depth visibility, and no changes to rendered modulation behavior.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    cmake --build build --target SylenthAIContractTest -j2
    ./build/SylenthAIContractTest
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --modulation-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation-route-model.json

## Idempotence and Recovery

The route model is derived state. If a future UI patch writes route rows directly, reject it unless it compiles back to APVTS/preset TransMod fields. If a catalog entry changes, update tests and docs together.

## Artifacts and Notes

Expected proof artifacts:

- `build/reports/modulation-route-model.json`
- contract test output
- adversarial review notes

Build artifacts remain disposable and are not source-controlled.

## Interfaces and Dependencies

This slice consumes `SynthParameters::TransModParameters`, APVTS TransMod state, `PresetManager` mod-slot compatibility, and the validation harness. It produces the route model needed by the active Claude Code modulation UI handoff and future AI edit reports.
