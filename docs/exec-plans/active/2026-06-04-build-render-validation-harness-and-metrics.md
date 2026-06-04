---
title: Build Render Validation Harness And Metrics
status: active
created_at: 2026-06-04
completed_at: null
summary: Promote deterministic render fixtures, audio metrics, reports, and regression artifacts into a first-class validation harness.
post_build_recap: null
read_when:
  - Adding validation fixtures or render metrics.
  - Debugging audio regressions.
  - Preparing release proof for the dry core.
program_id: synth-clean-room-pluck-instrument
planning_brief: docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/planning-brief-1.md
---

# Build Render Validation Harness And Metrics

This document is a living execution plan for the render validation harness.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Synth needs evidence, not just compile success. After this slice, the repo should have deterministic MIDI fixtures, render commands, audio metric reports, golden/tolerance comparisons, and failure artifacts for oscillator, filter, modulation, factory pluck, and host-independent regression tests.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [ ] Consolidate `SynthRender` commands and report format.
- [ ] Add core MIDI fixtures and preset fixtures.
- [ ] Add FFT, timing, tuning, loudness, stereo, and invalid-sample metrics.
- [ ] Add golden tolerance reports.
- [ ] Update validation docs.

## Surprises & Discoveries

None yet. Record numerical library choices, tolerance tradeoffs, and deterministic rendering issues here.

## Decision Log

Decision: Render validation is a first-class slice even though earlier slices add focused tests.
Rationale: End-to-end audio behavior needs coherent reports and fixtures across DSP modules.
Date: 2026-06-04.

## Outcomes & Retrospective

Pending implementation.

## Context and Orientation

Earlier slices should have grown `SynthRender` organically. This slice turns it into a stable validation interface. `docs/VALIDATION.md` names required profiles and metrics.

### In Scope

This plan may add `src/validation/`, `fixtures/midi/`, `fixtures/presets/`, `tests/render/`, report schemas, metric utilities, render artifact directories, and docs.

### Out Of Scope

This plan does not add Ableton automation, UI testing, release packaging, or new synthesis features except where needed to expose metrics.

## Plan of Work

Define a JSON report schema for render validation. Implement metric collection for invalid samples, peak/RMS, tuning, LFO period, envelope timing, spectral centroid, stereo correlation, and loudness where feasible. Add fixed fixtures for oscillator, filter sweep, overlapping pluck, mono-LFO ablation, and per-voice-LFO render.

Make `SynthRender --suite core` run the main command-line validation set and write reports under `build/reports/`. Update `docs/VALIDATION.md` with actual report fields and command names.

## Milestones

Milestone 1 stabilizes the validation CLI command surface.

Milestone 2 adds render fixtures and reports.

Milestone 3 adds metric comparisons and failure artifacts.

Milestone 4 updates docs and active plan proof commands.

## Concrete Steps

Work from the repo root:

    cd /Users/bazelrex/Developer/synth

Implement the validation harness. Run:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --suite core --output-dir build/reports/core

Inspect the generated summary:

    ls build/reports/core

## Validation and Acceptance

Acceptance requires a single core validation command, JSON reports for each fixture, retained WAV artifacts for failures, deterministic pass/fail results on repeated runs, and docs that explain every metric used by release readiness.

### Test Commands

    cd /Users/bazelrex/Developer/synth
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --suite core --output-dir build/reports/core

## Idempotence and Recovery

Validation commands should overwrite reports in `build/` safely. Golden fixtures should be checked in only when intentionally accepted. If a metric is noisy, document the tolerance and why it is still useful.

## Artifacts and Notes

Expected proof artifacts:

- `build/reports/core/summary.json`
- per-fixture reports
- failure WAVs when applicable

## Interfaces and Dependencies

This slice creates the validation interface used by UI, FX, host integration, release hardening, and future DSP regression work.

