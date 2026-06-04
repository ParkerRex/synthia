---
title: Build Modulation Matrix Ramp And Glide
status: active
created_at: 2026-06-04
completed_at: null
summary: Implement direct modulation, eight TransMod-style slots, ramp, glide, velocity glide, and voice/unison/random modulation sources.
post_build_recap: null
read_when:
  - Implementing modulation routing.
  - Debugging voice variation, glide, ramp, or TransMod behavior.
  - Preparing factory pluck macros.
program_id: synth-clean-room-pluck-instrument
planning_brief: docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/planning-brief-1.md
---

# Build Modulation Matrix Ramp And Glide

This document is a living execution plan for full modulation routing.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The target pluck needs more than static envelope controls. After this slice, Synth should support direct key/LFO/envelope routes, eight TransMod-style slots, ramp, glide, velocity glide, random-on-note, voice index, unison index, macro sources, and deterministic modulation traces.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [ ] Implement modulation source registry.
- [ ] Implement direct modulation routes.
- [ ] Implement eight TransMod-style slots and scalers.
- [ ] Implement ramp, glide, and velocity glide.
- [ ] Add modulation trace tests.

## Surprises & Discoveries

None yet. Record control-rate versus audio-rate choices, smoothing issues, and parameter-domain mismatches here.

## Decision Log

Decision: Use eight TransMod-style slots for v1 profile.
Rationale: The spec targets Strobe-v1-like behavior first; 16 slots are an optional extension.
Date: 2026-06-04.

Decision: Modulation destinations must state physical or normalized depth domain.
Rationale: Cutoff, pitch, and time values need physical-domain behavior to stay musical.
Date: 2026-06-04.

## Outcomes & Retrospective

Pending implementation.

## Context and Orientation

This slice depends on voice/LFO/envelope, oscillator parameters, filter parameters, and preset state. `SPEC.md` sections 4.1.11 through 4.1.14 define modulation sources, TransMod slots, direct routes, and macros.

### In Scope

This plan may add `src/modulation/ModSource.*`, `src/modulation/DirectRoutes.*`, `src/modulation/TransMod.*`, `src/dsp/Ramp.*`, `src/voice/Glide.*`, source traces, and tests.

### Out Of Scope

This plan does not build the full UI editor for modulation slots, onboard FX modulation, MPE, microtuning, or Strobe2-style extended modulation processors.

## Plan of Work

Define a modulation source registry with polarity, range, update rate, and scope for each source. Implement direct routes from keytrack, LFO, and mod envelope to oscillator pitch, pulse width, and filter cutoff. Implement TransMod slots with source, optional scaler, and many destination depths.

Add ramp generator behavior and glide/velocity glide. Ensure modulation is deterministic for fixed inputs and random seeds. Add trace output so tests can verify per-voice source values.

Update preset schema handling for mod slots and macros if needed.

## Milestones

Milestone 1 proves modulation source evaluation.

Milestone 2 proves direct routes to oscillator, pulse width, and filter cutoff.

Milestone 3 proves TransMod one-source-to-many-destinations and scaler multiplication.

Milestone 4 proves ramp, glide, velocity glide, voice/unison/random source behavior.

## Concrete Steps

Work from the repo root:

    cd /Users/bazelrex/Developer/synth

Add modulation modules and tests. Run:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --modulation-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation.json

## Validation and Acceptance

Acceptance requires tests for every required source range, direct routes, TransMod scalers, per-voice random stability, voice/unison source spread, ramp timing, glide behavior, preset serialization of slots, and no modulation-induced invalid parameter values.

### Test Commands

    cd /Users/bazelrex/Developer/synth
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --modulation-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation.json

## Idempotence and Recovery

Modulation tests should inspect traces rather than relying only on listening. If a source changes polarity or range, update the registry, docs, presets, and tests together.

## Artifacts and Notes

Expected proof artifacts:

- `build/reports/modulation.json`
- modulation trace fixtures
- preset round-trip tests with TransMod slots

## Interfaces and Dependencies

This slice creates the modulation interfaces needed by the factory pluck, UI editor, preset workflow, validation harness, and future extensions.

