---
title: Build Amp Stereo Analog And Factory Pluck
status: completed
created_at: 2026-06-04
completed_at: 2026-06-04
summary: Complete the dry core with amp drive, voice/unison panning, analog variation, macros, and the first clean-room factory pluck preset.
post_build_recap: Added per-voice amp envelope application, amp drive, level, pan spread, analog pitch/pan variation, macro influence, preset loading into the validation engine, and a dry WAV/report render for `Pluck Core 01`.
read_when:
  - Building the dry-core pluck sound.
  - Tuning amp drive, stereo spread, analog variation, or macros.
  - Validating the first factory preset.
program_id: synth-clean-room-pluck-instrument
planning_brief: docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/planning-brief-1.md
---

# Build Amp Stereo Analog And Factory Pluck

This document is a living execution plan for completing the first musical dry-core instrument.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

After oscillator, filter, and modulation exist, Synth still needs the final per-voice amp/stereo behavior that makes the pluck feel alive. After this slice, the dry core should produce a clean-room factory preset named `Pluck Core 01`, with amp drive, pan spread, analog variation, macro assignments, and render proof that it works mostly dry.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [x] 2026-06-04 EDT: Implemented amp envelope application and amp drive.
- [x] 2026-06-04 EDT: Implemented base pan, voice spread, and unison-spread contribution.
- [x] 2026-06-04 EDT: Implemented subtle analog pitch/pan variation controls.
- [x] 2026-06-04 EDT: Loaded the clean-room factory pluck preset and macros into render validation.
- [x] 2026-06-04 EDT: Rendered dry-core musical proof.

## Surprises & Discoveries

The first dry preset render peaks at 0.452775 with RMS 0.0871166 and stereo correlation 0.625192, leaving gain headroom for later FX.

## Decision Log

Decision: Shipped factory preset display name is `Pluck Core 01`.
Rationale: The internal research alias is not safe as shipped user-facing content.
Date: 2026-06-04.

Decision: The dry core must be validated with FX bypassed.
Rationale: Delay/reverb can hide a weak synth core and the spec explicitly requires dry-core strength.
Date: 2026-06-04.

## Outcomes & Retrospective

Completed. `build/renders/pluck-core-01-dry.wav` and `build/reports/pluck-core-01-dry.json` prove the requested factory preset and MIDI fixture load, render finite non-clipping dry-core audio, and show note-local LFO spread during overlapping notes.

## Context and Orientation

This slice depends on oscillator, filter, voice/modulators, full modulation, and preset state. `SPEC.md` Appendix A defines the starting pluck profile and naming boundaries.

### In Scope

This plan may add `src/dsp/Amp.*`, stereo panning utilities, analog variation controls, macro assignments, factory preset tuning, dry-core render fixtures, and validation reports.

### Out Of Scope

This plan does not add delay/reverb/chorus FX, final UI polish, Ableton packaging, or exact third-party preset matching.

## Plan of Work

Implement amp envelope application, amp waveshaping, output level, and denormal protection. Add per-voice and per-unison panning with stable spread. Add analog variation controls for low-level noise, drift, and control slur, keeping defaults subtle.

Create or update `presets/factory/pluck-core-01.json` using clean-room names. Assign macros for Motion, Width, Drive, and Space, even if Space remains low/no-op until FX lands.

Render the fixed overlapping pluck fixture with FX disabled and write metrics for loudness, clipping, stereo correlation, and modulation movement.

## Milestones

Milestone 1 proves amp drive and gain staging.

Milestone 2 proves pan spread and analog variation.

Milestone 3 creates and validates `Pluck Core 01`.

Milestone 4 renders dry-core musical proof.

## Concrete Steps

Work from the repo root:

    cd /Users/bazelrex/Developer/synth

Add amp/stereo modules and preset tuning. Run:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --preset presets/factory/pluck-core-01.json --fixture fixtures/midi/overlap-pluck.mid --dry --output build/renders/pluck-core-01-dry.wav --report build/reports/pluck-core-01-dry.json

## Validation and Acceptance

Acceptance requires no clipping at default preset level, no NaN/infinity, stereo spread within documented range, macros serialized and restored, factory preset validation passing, and a dry render that shows note-local cutoff movement.

### Test Commands

    cd /Users/bazelrex/Developer/synth
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json
    ./build/SynthRender --preset presets/factory/pluck-core-01.json --fixture fixtures/midi/overlap-pluck.mid --dry --output build/renders/pluck-core-01-dry.wav --report build/reports/pluck-core-01-dry.json

## Idempotence and Recovery

Preset tuning is iterative. Keep previous render reports when changing gain or macro behavior so regressions are visible. Do not rename shipped preset IDs casually.

## Artifacts and Notes

Expected proof artifacts:

- `presets/factory/pluck-core-01.json`
- `build/renders/pluck-core-01-dry.wav`
- `build/reports/pluck-core-01-dry.json`

## Interfaces and Dependencies

This slice creates the dry-core musical baseline consumed by validation harness, UI, FX, host integration, and release readiness slices.
