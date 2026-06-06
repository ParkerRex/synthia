---
title: Build Render Validation Harness And Metrics
status: completed
created_at: 2026-06-04
completed_at: 2026-06-04
summary: Promote deterministic render fixtures, audio metrics, reports, and regression artifacts into a first-class validation harness.
post_build_recap: Added `SynthRender --suite core`, per-fixture JSON reports, dry WAV artifacts, mono/per-voice LFO ablation, deterministic repeat comparison, CTest coverage, and validation docs.
read_when:
  - Adding validation fixtures or render metrics.
  - Debugging audio regressions.
  - Preparing release proof for the dry core.
program_id: synth-pluck-core-foundation
planning_brief: docs/programs/completed/2026-06-04-synth-pluck-core-foundation/planning-brief-1.md
---

# Build Render Validation Harness And Metrics

This document is a living execution plan for the render validation harness.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Synth needs evidence, not just compile success. After this slice, the repo should have deterministic MIDI fixtures, render commands, audio metric reports, golden/tolerance comparisons, and failure artifacts for oscillator, filter, modulation, factory pluck, and host-independent regression tests.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [x] 2026-06-04 EDT: Consolidated the core validation command as `SynthRender --suite core --output-dir build/reports/core`.
- [x] 2026-06-04 EDT: Added core-suite use of the overlap MIDI fixture, factory pluck preset, default dry render, per-voice LFO render, and mono-LFO ablation render.
- [x] 2026-06-04 EDT: Added richer render metrics: invalid samples, peak/RMS, RMS dBFS, crest, DC offset, spectral centroid, stereo correlation, tuning, timing, modulation trace, and note-local LFO spread.
- [x] 2026-06-04 EDT: Added deterministic repeat/tolerance report and retained failure WAV paths for repeatability failures.
- [x] 2026-06-04 EDT: Added `SynthRenderCoreSuite` to CTest and updated validation docs.

## Surprises & Discoveries

- Spectral centroid is implemented with a bounded in-process DFT window rather than a new FFT dependency. This keeps the validation tool dependency-light and deterministic while still catching large spectral regressions.
- Golden comparison is currently a deterministic repeat/tolerance report rather than a checked-in golden WAV. That is intentional for pre-release DSP churn; checked-in goldens can be added once the sound is deliberately frozen.

## Decision Log

Decision: Render validation is a first-class slice even though earlier slices add focused tests.
Rationale: End-to-end audio behavior needs coherent reports and fixtures across DSP modules.
Date: 2026-06-04.

Decision: Use standalone core-suite validation in place of Ableton for this slice.
Rationale: Ableton is unavailable in this environment, and the user explicitly asked to defer that path. Host validation remains in the host-integration ExecPlan.
Date: 2026-06-04.

Decision: Use deterministic repeat tolerance before checked-in golden WAVs.
Rationale: The dry-core sound is still changing; repeatability is useful now, while long-lived golden artifacts should wait until the target profile is stable.
Date: 2026-06-04.

## Outcomes & Retrospective

Completed on 2026-06-04 EDT.

The core suite writes `summary.json`, focused DSP reports, dry pluck report, WAV artifacts, LFO ablation report, and determinism tolerance report under the requested output directory.

Validation evidence:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --suite core --output-dir build/reports/core

Observed proof from `build/reports/core/summary.json`: 10 reports, 10 passed, 0 failed. The LFO ablation report showed per-voice LFO spread `1.60032`, mono spread `0`, and RMS diff `0.0384441`. The determinism report showed `max_abs_diff`, `rms_diff`, and `peak_delta` all `0` within tolerance.

Residual risk: this is still standalone validation, not DAW validation. AU/VST3 host behavior remains in the Ableton/host-integration ExecPlan.

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
