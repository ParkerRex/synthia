---
title: Build Onboard FX And Quality Modes
status: completed
created_at: 2026-06-04
completed_at: 2026-06-05
summary: Add bypassable saturation, delay, reverb, chorus, realtime/offline quality settings, and FX validation without weakening dry-core proof.
post_build_recap: Added a realtime-safe post-voice FX chain with saturation, tempo-synced delay, simple reverb, chorus, serialized quality modes, dry/wet render splitting, and validation coverage.
read_when:
  - Implementing onboard FX.
  - Changing oversampling or quality settings.
  - Validating dry versus wet render behavior.
program_id: synth-clean-room-pluck-instrument
planning_brief: docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/planning-brief-1.md
---

# Build Onboard FX And Quality Modes

This document is a living execution plan for optional FX and quality settings.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

FX make the instrument practical in production, but they must not hide a weak synth core. After this slice, Synth should have bypassable saturation, delay, reverb, and optional chorus, plus realtime/offline quality settings that are serialized, validated, and safe for the audio thread.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [x] 2026-06-05 EDT: Implemented FX module interfaces, global bypass, and module bypasses.
- [x] 2026-06-05 EDT: Added saturation, tempo-synced delay, simple reverb, and chorus processing after voice summing.
- [x] 2026-06-05 EDT: Added serialized realtime/offline quality settings with offline render quality selected in the standalone harness.
- [x] 2026-06-05 EDT: Added FX bypass, delay sync/tail, wet output safety, explicit `--wet`, and core-suite wet render validation.
- [x] 2026-06-05 EDT: Updated UI, factory preset, preset saving, validation docs, and architecture docs for FX behavior.

## Surprises & Discoveries

The standalone wet render uses the factory `1/8` delay sync at 128 BPM and reports `11250` delay samples at 48 kHz. FX report a 1.5 second tail for the current factory wet settings. Quality modes affect the reverb/chorus processing shape without reallocating resources on the audio thread.

## Decision Log

Decision: FX are bypassable and excluded from dry-core acceptance.
Rationale: The spec requires the core sound to work mostly dry.
Date: 2026-06-04.

## Outcomes & Retrospective

Completed on 2026-06-05. The FX chain is post-voice and bypassable; dry render validation explicitly forces FX off, while wet validation forces the preset FX path on. `SynthRender --suite core` now writes 11 reports including `pluck-core-01-wet.json`, and CTest covers FX bypass equivalence, delay sync/tail behavior, and wet finite-output safety.

## Context and Orientation

This slice depends on the dry core, parameter registry, UI, and validation harness. `SPEC.md` treats FX as optional but practical. Delay/reverb tails affect host tail length and packaging validation later.

### In Scope

This plan may add `src/dsp/fx/`, FX parameters, global FX bypass, delay tempo sync, simple reverb, saturation, optional chorus, quality settings, UI bindings, and render tests.

### Out Of Scope

This plan does not replace dry-core validation, add advanced mastering effects, add external sidechain audio input, or implement a full Strobe2-style FX suite.

## Plan of Work

Add a simple FX chain after voice summing. Implement global bypass and module bypasses. Ensure delay sync uses host tempo where available and a fallback tempo in standalone validation. Report plugin tail length when FX are enabled.

Add quality settings for realtime/offline oversampling or render quality. Resource changes must be prepared outside the realtime hot path.

Update the factory pluck macro `Space` to control delay/reverb mix only after dry-core preset validation already passes.

## Milestones

Milestone 1 adds FX interfaces and bypass behavior.

Milestone 2 adds saturation, delay, reverb, and optional chorus.

Milestone 3 adds quality settings and serialization.

Milestone 4 validates dry versus wet renders and CPU impact.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/synth

Implement FX and quality settings. Run:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --preset presets/factory/pluck-core-01.json --fixture fixtures/midi/overlap-pluck.mid --wet --output build/renders/pluck-core-01-wet.wav --report build/reports/pluck-core-01-wet.json
    ./build/SynthRender --suite core --output-dir build/reports/core

## Validation and Acceptance

Acceptance requires FX bypass null-equivalent behavior within tolerance, no invalid samples, correct delay sync at test tempos, documented tail length, serialized quality settings, dry-core validation still passing, and wet render reports.

### Test Commands

    cd /Users/parkerrex/Developer/synth
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --suite core --output-dir build/reports/core
    ./build/SynthRender --preset presets/factory/pluck-core-01.json --fixture fixtures/midi/overlap-pluck.mid --wet --output build/renders/pluck-core-01-wet.wav --report build/reports/pluck-core-01-wet.json

## Idempotence and Recovery

FX defaults should be conservative. If CPU or tail behavior is problematic, keep modules bypassed by default and record the reason rather than weakening dry-core tests.

## Artifacts and Notes

Expected proof artifacts:

- `build/renders/pluck-core-01-wet.wav`
- `build/reports/pluck-core-01-wet.json`
- updated core validation reports

## Interfaces and Dependencies

This slice creates FX and quality-mode interfaces consumed by UI, Ableton host validation, packaging, and release readiness.
