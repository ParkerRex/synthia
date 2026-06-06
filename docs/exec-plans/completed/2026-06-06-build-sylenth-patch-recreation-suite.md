---
title: Build Sylenth Patch Recreation Suite
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Add a curated Factory patch bank plus standalone render suite that proves Phase 1 patch recreation surfaces render finite, non-clipping, wet/dry-different audio.
post_build_recap: Added five lab-authored Factory presets, `SylenthAIRender --suite patch-recreation`, CTest coverage, arp/chord preset parsing in the renderer, and docs updates for preset inventory and validation.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
read_when:
  - Auditing the Phase 1 Factory patch bank.
  - Running standalone proof for Sylenth-style preset recreation.
  - Checking renderer support for preset-loaded arp and chord state.
  - Preparing Ableton current-preset recreation validation.
---

# Build Sylenth Patch Recreation Suite

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Phase 1 needs more than an Init patch and a single pluck. The product direction is to recreate the favorite vintage subtractive VST workflow first, then build AI patch/arp generation and conversational editing on top of normal editable synth state. This slice adds a small Factory patch set that exercises the current real surfaces and a repeatable standalone suite that proves those patches render.

## Progress

- [x] 2026-06-06 EDT: Created the patch-recreation slice from the Program's remaining product proof gaps.
- [x] 2026-06-06 EDT: Added five lab-authored Factory presets: `supersaw-stack-01`, `bass-wub-01`, `pad-wide-01`, `arp-motion-01`, and `fx-space-01`.
- [x] 2026-06-06 EDT: Extended `SynthRender` preset loading so `arp.*`, `arp.step.*`, `chord.*`, and `chord.voice.*` fields are applied in standalone renders.
- [x] 2026-06-06 EDT: Added `SylenthAIRender --suite patch-recreation` with WAV/report artifacts, wet-versus-dry checks, and summary output.
- [x] 2026-06-06 EDT: Added `SylenthAIPatchRecreationSuite` to CTest.
- [x] 2026-06-06 EDT: Updated durable validation, preset schema, baseline, active index, and Program ledger docs.

## Surprises & Discoveries

- The preset validator already accepted arp/chord parameters because they exist in the registry, but the standalone renderer's custom preset loader did not apply them. The patch suite would have made `Arp Motion 01` a false proof without adding that parser support.
- Mono-legato bass should not be forced through the note-local poly LFO proof. The suite renders `Bass Wub 01` as a mono-LFO case while keeping wet/dry FX proof enabled.

## Decision Log

Decision: Keep the patches lab-authored and role-based rather than trying to clone third-party presets.
Rationale: The suite is product proof for our current engine surfaces, not a bank import or licensed preset recreation.
Date: 2026-06-06.

Decision: Use the existing overlap-pluck MIDI fixture for this first suite.
Rationale: It already covers overlapping notes, velocities, and release/tail behavior, and it avoids adding a second fixture before the first patch bank proof exists.
Date: 2026-06-06.

Decision: Require meaningful wet-versus-dry difference for all patch-recreation cases.
Rationale: The current curated bank is intentionally FX-aware; this guards the fixed rack path while still preserving the core dry suite.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. The Factory bank now includes seven presets total, with six curated patch-recreation cases:

- `Pluck Core 01`
- `Supersaw Stack 01`
- `Bass Wub 01`
- `Pad Wide 01`
- `Arp Motion 01`
- `FX Space 01`

`SylenthAIRender --suite patch-recreation --output-dir build/reports/patch-recreation` rendered all six cases successfully. The suite writes per-patch WAV/JSON artifacts and a `summary.json` with pass/fail, peak/RMS, spectral centroid, note-local LFO spread where applicable, wet-versus-dry deltas, and an explicit `arp_chord_state_passed` assertion for `Arp Motion 01`.

## Context and Orientation

Relevant current code and docs:

- `presets/factory/*.json`
- `src/validation/SynthRender.cpp`
- `CMakeLists.txt`
- `docs/VALIDATION.md`
- `docs/PRESET_SCHEMA.md`
- `docs/modern-sylenth-baseline.md`

### In Scope

- Lab-authored Factory presets covering lead, bass, pad, arp/chord, FX, and the existing pluck.
- Standalone validation suite for finite, non-clipping, non-silent, wet/dry-different renders.
- Renderer preset-loading support for current arp and chord state.
- CTest and durable docs updates.

### Out Of Scope

- Copying third-party presets or names.
- Ableton-side preset recreation proof.
- Preset browser UI polish.
- AI generation, reference-sound analysis, or conversational editing.
- New DSP modules beyond using existing parameters.

## Plan of Work

1. Author a small Factory patch bank around current real Phase 1 surfaces.
2. Ensure the standalone renderer applies all parameter families the new patches rely on.
3. Add a patch-recreation suite that renders the cases, writes artifacts, and fails on invalid or weak output.
4. Wire the suite into CTest.
5. Update docs and Program ledgers.
6. Run build, tests, suite proof, and autoreview before PR handoff.

## Milestones

Milestone 1 added valid Factory preset JSON.

Milestone 2 added renderer support for preset-loaded arp and chord state.

Milestone 3 added the standalone patch-recreation suite and CTest hook.

Milestone 4 updated durable docs and validation proof.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --config Debug
    ./build/SylenthAIRender --validate-presets presets/factory --output build/reports/presets-patch-recreation.json
    ./build/SylenthAIRender --suite patch-recreation --output-dir build/reports/patch-recreation
    ctest --test-dir build --output-on-failure

## Validation and Acceptance

Acceptance requires:

- All Factory presets pass schema validation.
- The patch-recreation suite writes a report and WAV artifact for each curated patch.
- Every curated patch has finite, non-clipping, non-silent render output.
- Wet cases have meaningful wet-versus-dry difference.
- `Arp Motion 01` render loading explicitly asserts applied `arp.*` and `chord.*` state in `SynthRender`.
- The suite runs under CTest from the repo root.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --validate-presets presets/factory --output build/reports/presets-patch-recreation.json
    ./build/SylenthAIRender --suite core --output-dir build/reports/core
    ./build/SylenthAIRender --suite patch-recreation --output-dir build/reports/patch-recreation

## Idempotence and Recovery

The suite writes only disposable build artifacts under `build/reports/patch-recreation`. If a patch clips or fails wet proof, tune that patch's gain/FX values rather than weakening the suite thresholds. If arp/chord preset parsing changes, keep renderer, APVTS, and preset manager behavior aligned.

## Artifacts and Notes

Expected disposable artifacts:

- `build/reports/patch-recreation/summary.json`
- `build/reports/patch-recreation/*-wet.json`
- `build/reports/patch-recreation/artifacts/*-wet.wav`

The first local run produced `preset_count: 6`, `passed_count: 6`, and `failed_count: 0`.

## Interfaces and Dependencies

This slice depends on the existing parameter registry, preset validator, `SynthEngine`, FX chain, arp/chord engine, layer/oscillator-slot renderer, and overlap-pluck MIDI fixture. It feeds the remaining Ableton current-preset recreation work and future AI generation validation.
