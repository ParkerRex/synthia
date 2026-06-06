---
title: Build Modulation Write Adapter And Route Schema
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Add a model-backed modulation route write adapter that compiles one editable TransMod route into existing APVTS parameter edits without adding a new preset schema.
post_build_recap: Added route write and clear request models, processor control-thread APIs, contract tests for valid/invalid writes, and docs updates for the current single-destination slot replacement boundary.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
read_when:
  - Wiring modulation UI controls to real processor state.
  - Auditing TransMod write semantics.
  - Planning per-route bypass, remove, drag/drop, or expanded modulation destinations.
  - Separating current APVTS-backed modulation from future normalized route schema work.
---

# Build Modulation Write Adapter And Route Schema

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The modulation overview already had a read model, but UI and future AI editing need a safe write boundary before visual polish can become functional. This slice adds that boundary without inventing a second modulation state format: a route request compiles into the current `transmod.N.*` parameters, so host automation, presets, and audio-thread reads continue to use the existing APVTS-backed state.

## Progress

- [x] 2026-06-06 EDT: Added `ModulationRouteWriteRequest`, `ModulationRouteParameterEdit`, and `ModulationRouteWriteResult`.
- [x] 2026-06-06 EDT: Added model-level route write and slot clear compilers for the existing eight TransMod slots.
- [x] 2026-06-06 EDT: Added processor control-thread APIs: `writeModulationRoute()` and `clearModulationSlot()`.
- [x] 2026-06-06 EDT: Added contract coverage for valid writes, clamping, clear-slot behavior, and invalid input rejection.
- [x] 2026-06-06 EDT: Updated architecture, preset schema, baseline, active handoff, and Program ledger docs.
- [x] 2026-06-06 EDT: Fixed autoreview-discovered choice-parameter clamping so high-index sources and scalers survive processor-side application.

## Surprises & Discoveries

- The existing destination catalog has UI-facing ranges that do not always match the physical APVTS parameter range. The write adapter clamps against the parameter registry at compile/apply boundaries so writes cannot exceed real host parameter limits.
- A single slot can currently hold more than one nonzero destination depth in raw APVTS state. The new write adapter intentionally replaces a slot with one destination by clearing all slot depths before setting the selected destination.
- Choice-valued parameters cannot use the float `minimum`/`maximum` defaults for physical clamping. Source/scaler writes must clamp against `choices.size() - 1` or high-index sources such as RandomOnNote and Macro1 collapse to LFO.

## Decision Log

Decision: Compile writes to existing `transmod.N.*` parameter edits instead of adding `mod_route.*` parameters or a new preset schema now.
Rationale: The current plugin already persists and automates TransMod state. Reusing that contract avoids a schema fork and gives UI/AI code a safe boundary immediately.
Date: 2026-06-06.

Decision: Make route writes single-destination slot replacement.
Rationale: This maps cleanly to a Sylenth-style editable route row and avoids ambiguous per-destination bypass/remove semantics before a normalized route schema exists.
Date: 2026-06-06.

Decision: Reject zero-depth route writes and use `clearModulationSlot()` for removal.
Rationale: A zero-depth "route" is inactive state. Keeping removal explicit makes UI and future AI edits easier to reason about.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. The modulation model now supports both read and write use cases:

- `buildModulationRouteWrite()` validates source, scaler, destination, slot number, and finite nonzero depth.
- `buildModulationRouteWrite()` emits ordered parameter edits that enable the slot, set source/scaler, clear existing depths, and set the selected destination depth.
- `buildModulationSlotClear()` emits edits that disable a slot, reset source/scaler, and clear all depths.
- `SynthAudioProcessor::writeModulationRoute()` and `clearModulationSlot()` apply the compiled edits through JUCE parameters on the control thread.
- `clampPhysicalParameterValue()` centralizes bool, choice, and float physical clamping for the route model, processor apply path, and tests.

The result is enough for a non-placeholder modulation editor, drag/drop assignment prototype, and AI route-edit command adapter. It deliberately does not solve per-route bypass/remove or new destination families.

## Context and Orientation

Relevant current code and docs:

- `src/modulation/ModulationRouteModel.h`
- `src/modulation/ModulationRouteModel.cpp`
- `src/plugin/PluginProcessor.h`
- `src/plugin/PluginProcessor.cpp`
- `tests/smoke/SynthContractTest.cpp`
- `docs/ARCHITECTURE.md`
- `docs/PRESET_SCHEMA.md`
- `docs/modern-sylenth-baseline.md`

### In Scope

- Model-level write request and result types.
- Validation for source, scaler, destination, slot, and depth.
- Single-destination slot replacement compiled to existing `transmod.N.*` APVTS parameter edits.
- Explicit clear-slot operation.
- Processor methods that apply compiled edits through normal JUCE parameter mutation.
- Contract tests and durable docs updates.

### Out Of Scope

- UI implementation or visual polish.
- Drag/drop gestures, halos, or matrix editing widgets.
- Per-route bypass, per-route remove, or multiple independent destination rows inside one slot.
- Expanded destination catalog beyond current TransMod parameters.
- New normalized `mod_route.*` preset schema.
- Audio-thread mutation or filesystem/network work.

## Plan of Work

1. Extend the modulation route model with a small write request/result API.
2. Compile valid writes into existing TransMod parameter edits with explicit slot replacement.
3. Expose processor control-thread methods that apply those edits through APVTS parameters.
4. Add contract tests for write, clear, clamp, apply, and invalid input behavior.
5. Update architecture, preset schema, baseline, active ExecPlan, and Program ledger docs.
6. Run focused and full validation before PR handoff.

## Milestones

Milestone 1 added write request/result data structures and model compilers.

Milestone 2 added processor APIs for applying compiled writes to host-visible parameters.

Milestone 3 added smoke-contract tests proving write semantics and invalid input rejection.

Milestone 4 updated durable roadmap and schema docs to describe the current boundary and remaining gaps.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --config Debug
    ./build/SylenthAIContractTest
    ./build/SylenthAIRender --modulation-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation-write-adapter.json
    ctest --test-dir build --output-on-failure

## Validation and Acceptance

Acceptance requires:

- Valid route writes compile into deterministic parameter edits.
- Source/scaler choice values match the existing TransMod choice parameter IDs.
- Destination depths are clamped to actual parameter registry ranges.
- Existing depths in the target slot are cleared before the new destination depth is written.
- Clear-slot edits disable the slot and reset every slot depth.
- Invalid slot/source/scaler/destination/depth requests are rejected.
- Full CTest and core standalone render validation still pass.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    cmake --build build --config Debug
    ./build/SylenthAIContractTest
    ./build/SylenthAIRender --modulation-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation-write-adapter.json
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Idempotence and Recovery

The implementation writes no source artifacts at runtime. Validation outputs are disposable under `build/reports/`. If a future UI or AI adapter needs route removal, call `clearModulationSlot()` rather than writing a zero-depth route. If future schema work introduces normalized `mod_route.*` state, keep this adapter as the compatibility bridge until released presets and automation have a migration path.

## Artifacts and Notes

Expected disposable artifact:

- `build/reports/modulation-write-adapter.json`

The important product boundary is now explicit: route write semantics exist, but they are still constrained by the current eight-slot TransMod APVTS model.

## Interfaces and Dependencies

This slice depends on `ParameterRegistry` for stable parameter IDs and ranges, `ModulationRouteModel` for source/destination catalogs, `AudioProcessorValueTreeState` for host-visible parameter mutation, and the smoke contract test harness for APVTS round-trip checks. It feeds the next functional UI slice and future AI patch-edit command work.
