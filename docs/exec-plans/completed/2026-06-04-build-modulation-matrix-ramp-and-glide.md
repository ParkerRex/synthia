---
title: Build Modulation Matrix Ramp And Glide
status: completed
created_at: 2026-06-04
completed_at: 2026-06-04
summary: Implement direct modulation, eight TransMod-style slots, ramp, glide, velocity glide, and voice/unison/random modulation sources.
post_build_recap: Implemented per-voice ramp, glide, velocity glide, source evaluation, eight TransMod slots with physical destination depths, preset slot validation, and standalone modulation traces.
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
- [x] 2026-06-04 EDT: Implemented modulation source enums and fixed eight-slot TransMod parameter storage in `SynthParameters`.
- [x] 2026-06-04 EDT: Implemented direct keytrack/LFO/envelope routes to oscillator pitch, pulse width, and filter cutoff.
- [x] 2026-06-04 EDT: Implemented eight TransMod-style slots with source, optional scaler, and physical destination depths for oscillator pitch, pulse width, filter cutoff, amp level, and pan.
- [x] 2026-06-04 EDT: Implemented per-voice ramp, glide, and velocity glide in `Voice`.
- [x] 2026-06-04 EDT: Added modulation CTest coverage and `SynthRender --modulation-test` JSON report output.

## Surprises & Discoveries

- The parameter registry already exposed ramp and generic TransMod source/scaler/depth controls, but the DSP snapshot ignored them. The slice added physical destination-depth parameters while retaining `transmod.N.depth` as a legacy normalized cutoff-depth control.
- Ableton validation is not available in this environment. The proof path for this slice is standalone fixture rendering plus CTest trace assertions; DAW loading remains in the host-integration ExecPlan.

## Decision Log

Decision: Use eight TransMod-style slots for v1 profile.
Rationale: The spec targets Strobe-v1-like behavior first; 16 slots are an optional extension.
Date: 2026-06-04.

Decision: Modulation destinations must state physical or normalized depth domain.
Rationale: Cutoff, pitch, and time values need physical-domain behavior to stay musical.
Date: 2026-06-04.

Decision: Store TransMod as fixed arrays in the audio snapshot.
Rationale: The audio thread can copy and evaluate all slots without allocation, locking, string lookups, or preset parsing.
Date: 2026-06-04.

Decision: Use standalone MIDI fixture and JSON traces for this slice instead of Ableton.
Rationale: Ableton is not installed in this environment; the user asked to defer that and use another validation route.
Date: 2026-06-04.

## Outcomes & Retrospective

Completed on 2026-06-04 EDT.

Implemented source evaluation for LFO, ramp, envelopes, keytrack, velocity, velocity glide, pitch bend, mod wheel, aftertouch, voice/unison indexes, random-on-note, and four macro sources. Direct routes now include keytrack, LFO, and mod-envelope movement for oscillator pitch, pulse width, and filter cutoff. The factory pluck preset lightly exercises ramp and TransMod parameters.

Validation evidence:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --modulation-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation.json
    ./build/SynthRender --osc-test --notes C1,C3,C5,C7 --output build/reports/oscillator.json
    ./build/SynthRender --filter-test --output build/reports/filter.json
    ./build/SynthRender --preset presets/factory/pluck-core-01.json --fixture fixtures/midi/overlap-pluck.mid --dry --output build/renders/pluck-core-01-dry.wav --report build/reports/pluck-core-01-dry.json

Residual risk: this slice proves standalone audio-engine behavior, not AU/VST3 behavior in a DAW. Host loading, automation gestures, and save/restore in Ableton remain deferred to the host-integration ExecPlan.

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
