---
title: Build Arp Step Chord Workflow
status: active
created_at: 2026-06-05
completed_at: null
summary: Add the Phase 1 arpeggiator, step, and chord state/engine model that will unblock Claude Code arp UI polish and Phase 2 generation.
post_build_recap: null
read_when:
  - Implementing or reviewing arpeggiator, step sequencer, chord memory, host-sync timing, or generated arp state.
  - Preparing the arp UI handoff for Claude Code.
  - Designing Phase 2 AI arp/chord generation outputs.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Build Arp Step Chord Workflow

This ExecPlan is the next engine/state slice after layer rendering and project rename closeout.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Phase 1 needs a real arpeggiator/step/chord workflow before the UI can be polished. The model should be deterministic under host tempo, buffer slicing, transport state, note overlap, sustain, and offline render. It should also be ordinary preset/parameter state so Phase 2 AI can generate arp and chord ideas without creating opaque behavior.

## Progress

- [x] 2026-06-05 EDT: Created this Program-linked ExecPlan after closing layer rendering and the sylenth-ai rename.
- [ ] Inventory current MIDI scheduling, tempo, and validation harness support.
- [ ] Add arp/step/chord parameter and preset state.
- [ ] Implement deterministic arp event generation outside UI code.
- [ ] Route generated notes into the existing voice allocator without audio-thread allocation.
- [ ] Add render tests for host-sync timing, note order, gate, octave/wrap, hold/tie, and panic/transport stop.
- [ ] Update docs and Claude handoff gates.

## Decision Log

Decision: Keep arp output as generated MIDI/note events into the existing voice path.
Rationale: The voice allocator and render harness are already validated. Feeding deterministic note events avoids duplicating voice behavior.
Date: 2026-06-05.

Decision: Store generated AI arp/chord ideas as normal editable state.
Rationale: Phase 2 generation must leave users with inspectable parameters and steps, not hidden model output.
Date: 2026-06-05.

## Context and Orientation

Read first:

- `docs/modern-sylenth-baseline.md`
- `docs/ARCHITECTURE.md`
- `docs/PRESET_SCHEMA.md`
- `docs/VALIDATION.md`
- `src/dsp/SynthEngine.*`
- `src/voice/VoiceAllocator.*`
- `src/plugin/ParameterRegistry.*`
- `src/validation/SynthRender.cpp`
- `tests/smoke/SynthDspCoreTest.cpp`
- `tests/smoke/SynthContractTest.cpp`

## In Scope

- Arp enabled/bypass, mode, rate, sync, gate, octave range, wrap/order, hold, latch, swing where practical.
- Step lane state for pitch offset, velocity, gate/tie/hold.
- Chord memory/state sufficient for Phase 1 playback and Phase 2 generation handoff.
- Preset serialization, validation, and migration defaults.
- Deterministic render tests and docs.

## Out Of Scope

- Final Claude Code arp UI polish.
- AI generation implementation.
- Full Cthulhu-scale chord library authoring.
- MIDI learn UI.
- Arbitrary external MIDI file import.

## Validation and Acceptance

Acceptance requires:

- Existing presets continue to validate and render with arp disabled by default.
- Arp enabled produces deterministic note timing from a fixed chord/held-note fixture.
- Rate/gate/octave/wrap/step pitch/velocity/tie state changes render predictably.
- Panic/all-notes-off/transport stop clear held/generated notes safely.
- No audio-thread allocation, filesystem I/O, logging, or UI dependency.
- Docs describe the state model and remaining UI handoff gates.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --target SylenthAIDspCoreTest
    ./build/SylenthAIDspCoreTest
    cmake --build build --target SylenthAIContractTest
    ./build/SylenthAIContractTest

If the full JUCE build is slow in the local environment, focused source-level probes are acceptable during iteration, but closeout should record the exact limitation.
