---
title: Build Randomize Render Validation
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Add standalone finite-audio proof for backend randomized preset state.
post_build_recap: Added `SylenthAIRender --randomize-test`, fixed-seed randomized APVTS preparation through `PresetManager`, conversion into standalone `SynthParameters`, as-prepared finite non-silent non-clipping render checks, strict seed-list parsing, core-suite integration, CTest coverage, and validation docs updates.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
read_when:
  - Changing the randomizer bounds or command model.
  - Planning Phase 2 AI patch generation validation.
  - Auditing `SylenthAIRender` suites or CTest coverage.
  - Reviewing whether generated patch state has standalone finite-audio proof.
---

# Build Randomize Render Validation

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The backend preset command model can prepare bounded seed-randomized APVTS state, but Phase 2 AI/randomization work also needs audio proof. This slice adds a renderer gate that asks the real preset manager for randomized state, converts the prepared parameter state into the standalone engine model, and renders fixed seeds against the overlap-pluck MIDI fixture.

## Progress

- [x] 2026-06-06 EDT: Added `--randomize-test` with optional comma-separated `--seeds`.
- [x] 2026-06-06 EDT: Reused `PresetManager::prepareRandomizedPresetState()` instead of duplicating randomization logic.
- [x] 2026-06-06 EDT: Added APVTS-state to `SynthParameters` conversion for prepared `PARAM` children.
- [x] 2026-06-06 EDT: Rendered fixed seeds through the existing standalone audio metrics path.
- [x] 2026-06-06 EDT: Added the randomize report to `--suite core` and CTest.
- [x] 2026-06-06 EDT: Updated validation, baseline, Program, and ExecPlan indexes.
- [x] 2026-06-06 EDT: Autoreview hardening: render randomized state as prepared instead of forcing FX on, and reject malformed seed lists.

## Surprises & Discoveries

- The existing preset render pass required note-local LFO spread for default preset renders. Randomized patches can legitimately choose mono/legato behavior, so this validation uses a narrower generated-state predicate: finite, non-silent, non-clipping output.
- Keeping the randomizer call inside `PresetManager` avoids a second implementation of generated patch bounds in the renderer.
- Randomized state must be rendered as prepared. Forcing wet FX in the validation harness can hide regressions in generated global-FX-bypassed states.

## Decision Log

Decision: Validate generated state by rendering fixed seeds, not by saving temporary preset JSON.
Rationale: Randomize is a command-state operation. The renderer should prove the prepared APVTS state can sound safely before any optional save/provenance workflow exists.
Date: 2026-06-06.

Decision: Add this to the core suite and as a standalone CTest.
Rationale: Randomization is now part of the normal sound-design workflow and should fail loudly if future parameter changes make generated state unsafe.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. `SylenthAIRender --randomize-test` now prepares seeds `1,42,12345,67890` by default, renders each seed with the overlap-pluck fixture, and writes a JSON report with prepared/rendered status, peak, RMS, nonzero sample count, invalid sample count, and per-seed pass/fail. The core suite includes `randomize.json`, and CTest includes `SylenthAIRandomizeRenderTest`.

The seed override is strict: malformed, partial, empty, or out-of-range tokens fail the command instead of silently falling back to default seed coverage.

## Context and Orientation

Relevant current code and docs:

- `src/validation/SynthRender.cpp`
- `src/presets/PresetManager.*`
- `CMakeLists.txt`
- `docs/VALIDATION.md`
- `docs/modern-sylenth-baseline.md`

### In Scope

- Standalone finite render proof for command-randomized state.
- Fixed seed list with optional CLI override.
- Strict validation for caller-supplied seed lists.
- Core-suite and CTest wiring.
- Docs and Program ledger updates.

### Out Of Scope

- UI randomize button.
- Saving randomized presets.
- AI generation prompts or provenance metadata.
- Arp/chord/modulation route randomization beyond current randomizer output.
- Ableton host proof for generated patches.

## Plan of Work

1. Add a renderer mode for randomized state.
2. Reuse the preset command model to prepare APVTS state.
3. Convert prepared state into `SynthParameters`.
4. Render fixed seeds with existing audio metrics.
5. Wire the mode into CTest and the core suite.
6. Update durable docs.

## Milestones

Milestone 1 added the renderer command and state conversion.

Milestone 2 added per-seed JSON reporting.

Milestone 3 wired CTest/core-suite validation.

Milestone 4 updated docs and ledgers.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --config Debug
    ./build/SylenthAIRender --randomize-test --output build/reports/randomize.json
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Validation and Acceptance

Acceptance requires:

- Every fixed seed prepares successfully through `PresetManager`.
- Every fixed seed renders through `SynthEngine` as prepared.
- Invalid sample count is zero.
- Peak remains below `1.0`.
- RMS is above `0.001`.
- Nonzero samples exceed one eighth of a second at the report sample rate.
- CTest and core suite include the randomize render proof.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    cmake --build build --config Debug
    ./build/SylenthAIRender --randomize-test --output build/reports/randomize.json
    ! ./build/SylenthAIRender --randomize-test --seeds 123x --output build/reports/randomize-invalid-seeds.json
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Idempotence and Recovery

The default seed list is deterministic. If a seed fails after a future parameter change, inspect the generated values and fix the randomizer bounds or DSP safety issue rather than weakening thresholds. If generated state later includes arp/chord or route randomization, extend this report with those generated-state assertions.

## Artifacts and Notes

The report is disposable under `build/reports/randomize.json` or the supplied output path. Core-suite output writes `randomize.json` under its output directory.

## Interfaces and Dependencies

This slice depends on `PresetManager::prepareRandomizedPresetState()`, the APVTS parameter registry, `SynthRender` preset-value conversion, `SynthEngine`, and the overlap-pluck MIDI fixture. It feeds Phase 2 AI patch generation validation.
