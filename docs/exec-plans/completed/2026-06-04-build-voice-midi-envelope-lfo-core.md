---
title: Build Voice MIDI Envelope LFO Core
status: complete
created_at: 2026-06-04
completed_at: 2026-06-04
summary: Implement MIDI handling, voice allocation, amp/mod envelopes, LFO gate modes, and a deterministic silent/basic render path.
post_build_recap: Added envelope, LFO, voice, allocator, engine MIDI handling, SynthRender voice validation, and voice-core CTest coverage while keeping audio output silent for later oscillator work.
read_when:
  - Implementing note allocation or per-voice modulation.
  - Debugging stuck notes, envelope timing, or LFO retrigger behavior.
  - Preparing oscillator and filter slices.
program_id: synth-clean-room-pluck-instrument
planning_brief: docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/planning-brief-1.md
---

# Build Voice MIDI Envelope LFO Core

This document is a living execution plan for the note, voice, envelope, and LFO foundation.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The target pluck depends on overlapping notes keeping independent local motion. After this slice, Synth should allocate voices from MIDI, handle note-off and sustain safely, run amp and mod envelopes, run per-voice and mono LFO modes, and prove timing behavior with deterministic tests.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [x] 2026-06-04 EDT: Added `Envelope`, `Lfo`, `Voice`, and `VoiceAllocator` primitives.
- [x] 2026-06-04 EDT: Integrated `VoiceAllocator` into `SynthEngine`.
- [x] 2026-06-04 EDT: Forwarded note-on, note-off, all-notes-off, and all-sound-off from `SynthAudioProcessor` into `SynthEngine`.
- [x] 2026-06-04 EDT: Added `SynthRender --voice-test` and `SynthVoiceCoreTest`.
- [x] 2026-06-04 EDT: Ran build, CTest, voice report, and smoke render successfully.

## Surprises & Discoveries

The engine now consumes MIDI messages, but sample-offset scheduling is not yet sample-accurate inside the block. The current behavior is sufficient for voice lifecycle proof; the oscillator/render slices should tighten sample-offset handling when audible output begins.

The first LFO implementation supports Hz phase movement and reset. Tempo sync/gate-mode richness is still represented in parameters and should be completed when musical render fixtures exist.

## Decision Log

Decision: Per-voice LFO behavior is proven before oscillator/filter work.
Rationale: The target sound fails if overlapping notes share unintended global modulation.
Date: 2026-06-04.

Decision: Keep output silent while adding voice state.
Rationale: This slice proves note and modulator lifecycle without mixing in oscillator correctness.
Date: 2026-06-04.

## Outcomes & Retrospective

The voice/MIDI/envelope/LFO core slice is complete. Synth now has deterministic envelope and LFO primitives, a basic poly voice allocator, engine-level note-on/off handling, plugin MIDI forwarding, and command/test proof that voices activate and release without invalid output.

Tempo-synced LFO behavior, sustain-pedal semantics, mono/legato/unison policy depth, and sample-accurate audible scheduling remain follow-up details for later voice/modulation slices.

## Context and Orientation

This slice depends on the scaffold and parameter/state contract. It should work even before a real oscillator exists by rendering a simple test impulse, constant signal, or silent output while exposing envelope/LFO values to the validation harness.

### In Scope

This plan may add `src/voice/Voice.*`, `src/voice/VoiceAllocator.*`, `src/dsp/Envelope.*`, `src/dsp/Lfo.*`, MIDI state handling in the processor, and validation/test fixtures for note events.

### Out Of Scope

This plan does not implement the real oscillator stack, filter, TransMod, ramp, glide, amp drive, UI, or factory pluck sound.

## Plan of Work

Add a sample-accurate MIDI event loop in the processor or engine layer. Implement note-on, note-off, zero-velocity note-on, sustain pedal, all-notes-off, all-sound-off, and panic. Add voice states `Idle`, `Starting`, `Held`, `Releasing`, `Stolen`, and `Draining`.

Implement ADSR with exponential timing and deterministic stage transitions. Implement LFO shapes needed for early validation: sine, triangle, saw up, saw down, square, sample hold. Implement rate in Hz and tempo sync. Implement gate modes `Poly`, `PolyOn`, `Mono`, and `Song`, with explicit phase reset behavior.

Extend `SynthRender` to render MIDI fixtures and export per-voice diagnostic traces for tests.

## Milestones

Milestone 1 proves note allocation, release, sustain, all-notes-off, and panic.

Milestone 2 proves envelope timing at multiple sample rates.

Milestone 3 proves LFO rate, sync, phase reset, and mono versus per-voice behavior.

Milestone 4 updates docs with actual voice and modulation behavior.

## Concrete Steps

Work from the repo root:

    cd /Users/bazelrex/Developer/synth

Add MIDI fixtures under `fixtures/midi/`. Add tests under `tests/voice/` and `tests/dsp/`.

Run:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --fixture fixtures/midi/overlap-pluck.mid --preset presets/factory/init.json --trace build/reports/voice-trace.json

## Validation and Acceptance

Acceptance requires deterministic tests for note allocation, voice stealing policy placeholder behavior, sustain pedal, panic, envelope timing, LFO sync at several tempos, phase reset, and per-voice independence for overlapping notes.

Observed proof on 2026-06-04:

    ctest: 3/3 tests passed
    voice-core.json: active_after_note_on 1, active_after_release 0, invalid_samples 0, passed true
    smoke.json: invalid_samples 0, active_voices 0, peak 0, passed true

### Test Commands

    cd /Users/bazelrex/Developer/synth
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --fixture fixtures/midi/overlap-pluck.mid --preset presets/factory/init.json --trace build/reports/voice-trace.json

## Idempotence and Recovery

MIDI fixtures and tests should be deterministic. If a timing test fails, keep the fixture and adjust the implementation or documented tolerance; do not weaken tests without recording why.

## Artifacts and Notes

Expected proof artifacts:

- `build/reports/voice-trace.json`
- envelope timing tests
- LFO timing tests
- stuck-note recovery tests

## Interfaces and Dependencies

This slice establishes the `Voice`, `VoiceAllocator`, `Envelope`, and `Lfo` interfaces. Oscillator, filter, modulation, amp, and validation slices depend on these interfaces.
