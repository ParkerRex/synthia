---
title: Build Oscillator Stack And Mixer
status: active
created_at: 2026-06-04
completed_at: null
summary: Implement band-limited saw/pulse, noise, sub oscillator, hard sync, stack detune, and waveform mixer validation.
post_build_recap: null
read_when:
  - Implementing oscillator or mixer DSP.
  - Debugging aliasing, detune, pulse width, or sub tuning.
  - Preparing filter and pluck-core work.
program_id: synth-clean-room-pluck-instrument
planning_brief: docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/planning-brief-1.md
---

# Build Oscillator Stack And Mixer

This document is a living execution plan for the oscillator and pre-filter mixer.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The target oscillator is simple but must be clean, tunable, and stable. After this slice, Synth should render band-limited saw and pulse waves, noise, sub oscillator waveforms, stack count and detune, hard sync, pulse width, and gain-compensated waveform mixing.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [ ] Implement oscillator phase and band-limited saw/pulse.
- [ ] Implement noise and sub oscillator waveforms.
- [ ] Implement stack count, detune, and hard sync.
- [ ] Implement mixer/gain compensation.
- [ ] Add oscillator render metrics.

## Surprises & Discoveries

None yet. Record aliasing, sample-rate, and sync edge-case findings here.

## Decision Log

Decision: Use original band-limited oscillator code rather than naive saw/pulse.
Rationale: Naive waveforms alias under bright plucks, sync, drive, and high sample rates.
Date: 2026-06-04.

## Outcomes & Retrospective

Pending implementation.

## Context and Orientation

This slice depends on the voice/MIDI/envelope/LFO core and the parameter registry. It should add real pitch-producing audio before the filter exists. Use the clean-room policy: standard oscillator techniques are allowed, copied proprietary algorithms are not.

### In Scope

This plan may add `src/dsp/OscillatorStack.*`, `src/dsp/PolyBlep.*`, `src/dsp/Noise.*`, `src/dsp/Mixer.*`, oscillator parameters, fixtures, render tests, and validation CLI modes for oscillator analysis.

### Out Of Scope

This plan does not implement nonlinear filter, full modulation matrix, amp drive, UI, FX, or factory sound polish.

## Plan of Work

Implement saw and pulse using a band-limited technique such as polyBLEP or minBLEP. Implement phase accumulators per stack copy. Detune must be symmetric around center pitch and shaped for fine control near zero. Implement sub oscillator sine, triangle, saw, and pulse one to three octaves down. Implement hard sync with stable reset behavior and tests.

Implement the waveform mixer with saw, pulse, noise, and sub levels. Add gain compensation so stack count and waveform mixing do not create uncontrolled jumps.

Extend `SynthRender` with oscillator analysis commands that render fixed notes and write metric reports.

## Milestones

Milestone 1 renders tuned saw and pulse across sample rates.

Milestone 2 adds sub oscillator and noise.

Milestone 3 adds stack count, detune, and hard sync.

Milestone 4 adds mixer compensation and FFT-based validation.

## Concrete Steps

Work from the repo root:

    cd /Users/bazelrex/Developer/synth

Add oscillator modules and tests. Render note fixtures:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --osc-test --notes C1,C3,C5,C7 --output build/reports/oscillator.json

## Validation and Acceptance

Acceptance requires tuning within tolerance, pulse duty-cycle tests at multiple widths, sub octave accuracy, deterministic noise under seeded tests, stack detune symmetry, hard sync stability, and aliasing metrics recorded in `build/reports/oscillator.json`.

### Test Commands

    cd /Users/bazelrex/Developer/synth
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --osc-test --notes C1,C3,C5,C7 --output build/reports/oscillator.json

## Idempotence and Recovery

Oscillator tests should be deterministic. If FFT tolerances need adjustment, record why in this plan and `docs/VALIDATION.md`. Avoid changing parameter IDs after presets reference them.

## Artifacts and Notes

Expected proof artifacts:

- `build/reports/oscillator.json`
- rendered oscillator WAVs if the validation CLI writes them
- oscillator unit test output

## Interfaces and Dependencies

This slice creates `OscillatorStack` and mixer interfaces used by filter, modulation, amp, factory preset, and validation slices.

