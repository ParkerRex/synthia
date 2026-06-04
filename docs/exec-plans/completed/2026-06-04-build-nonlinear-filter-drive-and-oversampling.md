---
title: Build Nonlinear Filter Drive And Oversampling
status: completed
created_at: 2026-06-04
completed_at: 2026-06-04
summary: Implement the semitone-domain nonlinear multimode filter, drive/resonance interaction, self-oscillation, and oversampling support.
post_build_recap: Added a clean-room nonlinear cascade-style filter with semitone cutoff mapping, L2/L4/B2/B4/H2/H4 outputs, drive/resonance feedback compensation, oversampling-factor processing hooks, filter unit tests, and `SynthRender --filter-test`.
read_when:
  - Implementing or tuning filter behavior.
  - Debugging cutoff movement, resonance, drive, self-oscillation, or oversampling.
  - Preparing the dry-core pluck preset.
program_id: synth-clean-room-pluck-instrument
planning_brief: docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/planning-brief-1.md
---

# Build Nonlinear Filter Drive And Oversampling

This document is a living execution plan for the nonlinear filter, drive, and oversampling path.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The filter is the main sound-shaping risk. After this slice, Synth should convert cutoff modulation in semitone space, run a stable nonlinear cascade-style filter, expose low-pass, band-pass, and high-pass modes, support drive with resonance compensation, and validate self-oscillation and oversampling behavior.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [x] 2026-06-04 EDT: Implemented semitone cutoff conversion and clamping.
- [x] 2026-06-04 EDT: Implemented nonlinear filter stages and mode outputs.
- [x] 2026-06-04 EDT: Implemented drive/resonance compensation behavior.
- [x] 2026-06-04 EDT: Added oversampling-factor processing path.
- [x] 2026-06-04 EDT: Added filter metrics and stability tests.

## Surprises & Discoveries

Resonant impulse energy is high but finite in BP/HP modes, so the report treats finite bounded response as stable instead of enforcing a low generic impulse-energy ceiling.

## Decision Log

Decision: Filter cutoff modulation must be accumulated in semitone space.
Rationale: Linear-Hz cutoff movement is musically wrong for the target pluck and conflicts with the spec.
Date: 2026-06-04.

Decision: A clean linear filter alone is not conformant.
Rationale: The spec requires nonlinear drive/resonance interaction and feedback clipping behavior.
Date: 2026-06-04.

## Outcomes & Retrospective

Completed. `build/reports/filter.json` validates semitone-to-Hz mapping, nine mode response energies, drive response changes, interpolated oversampling stability, and invalid-sample absence.

## Context and Orientation

This slice depends on oscillator/mixer output and the parameter registry. `SPEC.md` section 4.1.7 defines the filter entity. `docs/VALIDATION.md` requires cutoff mapping, drive/resonance behavior, and self-oscillation tests.

### In Scope

This plan may add `src/dsp/Filter.*`, `src/dsp/Oversampling.*`, filter parameter bindings, metrics, fixtures, and filter render tests. Required modes are L2, L4, B2, B4, H2, and H4.

### Out Of Scope

This plan does not implement the full compound Strobe-style mode family, TransMod, UI controls beyond parameter availability, FX, or factory pluck polish.

## Plan of Work

Implement cutoff as a semitone-domain value converted to Hz after direct modulation hooks exist or as a base value ready for later modulation. Clamp cutoff below Nyquist at the active oversampled sample rate.

Implement a stable nonlinear cascade with drive and feedback clipping. The algorithm does not need to claim exact third-party behavior, but it must satisfy the spec behavior: resonance, self-oscillation, drive saturation, and drive reducing effective resonance.

Add oversampling options used by the filter path. Make oversampling resource setup happen outside the realtime processing hot path.

Extend `SynthRender` to render filter sweeps, self-oscillation tests, and drive harmonic metrics.

## Milestones

Milestone 1 proves semitone cutoff conversion and clamping.

Milestone 2 proves filter modes and stability under sweeps.

Milestone 3 proves drive/resonance interaction and self-oscillation calibration.

Milestone 4 proves oversampling stability and documents CPU tradeoffs.

## Concrete Steps

Work from the repo root:

    cd /Users/bazelrex/Developer/synth

Add filter modules and tests. Render filter reports:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --filter-test --output build/reports/filter.json

## Validation and Acceptance

Acceptance requires cutoff mapping tests, no NaN/infinity during sweeps, distinct mode responses, self-oscillation tracking report, harmonic change under drive, resonance compensation behavior, and oversampling tests at 44.1, 48, and 96 kHz.

### Test Commands

    cd /Users/bazelrex/Developer/synth
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --filter-test --output build/reports/filter.json

## Idempotence and Recovery

Filter tuning may require iteration. Keep tests deterministic and record any tolerance changes. If an oversampling mode causes instability, disable that mode in defaults but keep a failing fixture or documented blocker.

## Artifacts and Notes

Expected proof artifacts:

- `build/reports/filter.json`
- filter sweep renders
- self-oscillation tuning data

## Interfaces and Dependencies

This slice creates the `Filter` and `Oversampling` interfaces used by modulation, amp/stereo, validation, UI, and quality-mode slices.
