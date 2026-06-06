---
title: Build Modulation Route Render Proof
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Add standalone audio proof that a model-backed modulation route write changes sound and clear-slot restore returns to baseline.
post_build_recap: Added `SylenthAIRender --modulation-route-render-test`, route-edit application into standalone `SynthParameters`, baseline/routed/cleared render comparison, core-suite integration, CTest coverage, and validation docs updates.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
read_when:
  - Changing `ModulationRouteModel` write semantics.
  - Expanding modulation destinations.
  - Auditing `SylenthAIRender` suites or CTest coverage.
  - Planning modulation drag/drop, halos, or AI patch-edit commands.
---

# Build Modulation Route Render Proof

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The modulation route model already compiled UI-style route write requests into existing `transmod.N.*` parameters, but the roadmap still needed render proof that a created route changes sound. This slice adds a standalone validation mode that writes a route, renders the result, clears the slot, and proves the cleared render returns to baseline.

## Progress

- [x] 2026-06-06 EDT: Added `--modulation-route-render-test`.
- [x] 2026-06-06 EDT: Reused `ModulationRouteModel` to compile `macro.motion -> amp.level` into `transmod.3.*` edits.
- [x] 2026-06-06 EDT: Applied route edits into standalone `SynthParameters` through the parameter registry and preset-value conversion path.
- [x] 2026-06-06 EDT: Rendered baseline, routed, and cleared states against `Pluck Core 01` and `fixtures/midi/overlap-pluck.mid`.
- [x] 2026-06-06 EDT: Added route-audibility and clear-to-baseline thresholds to the JSON report.
- [x] 2026-06-06 EDT: Added the report to `--suite core` and CTest.
- [x] 2026-06-06 EDT: Updated validation, baseline, Program, and ExecPlan indexes.

## Surprises & Discoveries

- Slot 3 is unused in `Pluck Core 01`, which made it a clean target for route write and clear proof without disturbing existing factory route intent.
- The proof produces a large routed RMS delta while the clear path returns exactly to baseline in the current deterministic renderer.
- The route view reports all active factory routes plus the created route, so the acceptance checks target the specific new slot instead of expecting a single active route.

## Decision Log

Decision: Use `macro.motion -> amp.level` at `+6 dB` on slot 3.
Rationale: It is deterministic, audible, within current destination bounds, and avoids relying on UI state or Ableton automation.
Date: 2026-06-06.

Decision: Add this to the core suite and as a standalone CTest.
Rationale: Route writing is a model-backed editing surface for future UI and AI commands, so audio proof should fail with the normal validation set.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. `SylenthAIRender --modulation-route-render-test` compiles a route write through `ModulationRouteModel`, applies it to standalone `SynthParameters`, confirms the route view contains the expected slot 3 route, and renders baseline/routed/cleared audio. The report checks that routed audio differs above fixed thresholds and that clearing the slot restores baseline within deterministic tolerances.

The core suite now includes `modulation-route-render.json`, and CTest includes `SylenthAIModulationRouteRenderTest`.

## Context and Orientation

Relevant current code and docs:

- `src/validation/SynthRender.cpp`
- `src/modulation/ModulationRouteModel.*`
- `src/plugin/ParameterRegistry.*`
- `CMakeLists.txt`
- `docs/VALIDATION.md`
- `docs/modern-sylenth-baseline.md`

### In Scope

- Standalone route write audio proof.
- Clear-slot restore proof.
- JSON report fields for route edits, clear-slot edits, audio diffs, thresholds, and pass/fail.
- Core-suite and CTest wiring.
- Docs and Program ledger updates.

### Out Of Scope

- Modulation drag/drop UI.
- Matrix editing.
- Per-route bypass/remove.
- Expanded modulation destinations.
- Preset schema changes.
- Ableton host proof.

## Plan of Work

1. Add a renderer mode for route write proof.
2. Compile a route with `ModulationRouteModel`.
3. Apply the compiled edits to standalone parameters through the registry.
4. Render baseline, routed, and cleared states.
5. Compare audio diffs and write a report.
6. Wire the mode into CTest and the core suite.
7. Update durable docs.

## Milestones

Milestone 1 added the renderer command and route-edit application.

Milestone 2 added the baseline/routed/cleared audio comparison report.

Milestone 3 wired CTest/core-suite validation.

Milestone 4 updated docs and ledgers.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --config Debug
    ./build/SylenthAIRender --modulation-route-render-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation-route-render.json
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Validation and Acceptance

Acceptance requires:

- The route write compiles through `ModulationRouteModel`.
- The route edits apply through known parameter IDs.
- The route view contains slot 3 `macro.motion -> amp.level` with `+6 dB`.
- Baseline, routed, and cleared renders are finite, non-silent, and non-clipping.
- Routed audio differs from baseline above fixed max-abs and RMS thresholds.
- Cleared audio returns to baseline within fixed deterministic tolerances.
- CTest and core suite include the route render proof.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    cmake --build build --config Debug
    ./build/SylenthAIRender --modulation-route-render-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation-route-render.json
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Idempotence and Recovery

The proof is deterministic for the current renderer. If the routed diff fails, inspect route compilation and destination depth handling before weakening thresholds. If clear restore fails, inspect slot clearing and any future non-deterministic modulation sources introduced into the selected preset or fixture.

## Artifacts and Notes

The report is disposable under `build/reports/modulation-route-render.json` or the supplied output path. Core-suite output writes `modulation-route-render.json` under its output directory.

## Interfaces and Dependencies

This slice depends on `ModulationRouteModel`, the APVTS parameter registry, `SynthRender` preset-value conversion, `SynthEngine`, `Pluck Core 01`, and the overlap-pluck MIDI fixture. It feeds modulation drag/drop UI, halo/matrix synchronization, and future AI parameter-edit commands.
