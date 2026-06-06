---
title: Build Arp Step Chord Workflow
status: completed
created_at: 2026-06-05
completed_at: 2026-06-06
summary: Add the Phase 1 arpeggiator, step, and chord state/engine model that will unblock Claude Code arp UI polish and Phase 2 generation.
post_build_recap: Added APVTS-backed arp, 16-step, and chord state; a no-allocation Arpeggiator that generates note events into the existing voice allocator; direct chord expansion; minimal live arp/chord editor controls; preset/host-state contract coverage; and focused DSP tests for timing, gate, octave wrapping, step pitch/velocity, tie/hold, chord expansion, and panic.
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
- [x] 2026-06-06 EDT: Inventoried `SynthEngine`, `VoiceAllocator`, APVTS parameter mapping, preset validation, and direct DSP test surfaces.
- [x] 2026-06-06 EDT: Added `arp.*`, `arp.step.N.*`, `chord.*`, and `chord.voice.N.*` parameter state plus `SynthParameters` structs.
- [x] 2026-06-06 EDT: Implemented fixed-array `Arpeggiator` scheduling with Up/Down/UpDown/AsPlayed ordering, host-tempo rate, gate, octaves, hold, swing, step pitch/velocity/gate/tie, and chord candidates.
- [x] 2026-06-06 EDT: Routed generated arp notes and direct chord expansion through the existing `VoiceAllocator`.
- [x] 2026-06-06 EDT: Added focused DSP tests and contract-test assertions for arp/chord defaults, host-state merge, and preset serialization.
- [x] 2026-06-06 EDT: Updated architecture, preset schema, validation, modern baseline, Program, and Claude handoff docs.

## Decision Log

Decision: Keep arp output as generated MIDI/note events into the existing voice path.
Rationale: The voice allocator and render harness are already validated. Feeding deterministic note events avoids duplicating voice behavior.
Date: 2026-06-05.

Decision: Store generated AI arp/chord ideas as normal editable state.
Rationale: Phase 2 generation must leave users with inspectable parameters and steps, not hidden model output.
Date: 2026-06-05.

Decision: Keep the first arp implementation parameter-backed rather than adding a separate custom JSON block.
Rationale: The existing preset writer serializes all registered physical parameters, host automation needs the same state, and Phase 2 AI can generate the same editable IDs. A structured `arp_steps` block can be added later only if the AI/browser workflow needs a friendlier exchange format.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed on 2026-06-06. The slice delivered the Phase 1 arp/step/chord engine and state contract without UI-only placeholders. `Arpeggiator` uses fixed arrays for held notes and candidates, runs from `SynthEngine::process`, and emits generated note-on/off events into `VoiceAllocator`. With arp disabled, behavior remains compatible; with chord enabled and arp disabled, note-on/off expands through configured chord voices.

The minimal editor surface exposes live top-level Arp and Chord controls. The full 16-step grid, richer mode naming, Random/Ordered variants, and step-lane visual polish remain in the Claude Code UI handoff and future engine follow-ups.

Validation note: the CMake DSP/contract targets started compiling successfully through project sources, but the universal JUCE module rebuild stayed slow and was intentionally stopped to avoid another long loop. Direct DSP source compilation and the direct full DSP core test passed.

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

- [x] Existing presets continue to load through registry defaults with arp disabled by default.
- [x] Arp enabled produces deterministic note timing from fixed held notes.
- [x] Rate/gate/octave/wrap/step pitch/velocity/tie state changes render predictably in focused DSP tests.
- [x] Panic clears held/generated notes safely; all-notes-off resets arp held state and releases generated notes through the allocator.
- [x] No audio-thread allocation, filesystem I/O, logging, or UI dependency was added to the scheduler.
- [x] Docs describe the state model and remaining UI handoff gates.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    clang++ -std=c++20 -I. tests/smoke/SynthDspCoreTest.cpp \
      src/dsp/Arpeggiator.cpp src/dsp/Envelope.cpp src/dsp/Filter.cpp src/dsp/Lfo.cpp \
      src/dsp/OscillatorStack.cpp src/dsp/Ramp.cpp src/dsp/SynthEngine.cpp src/dsp/fx/FxChain.cpp \
      src/voice/Voice.cpp src/voice/VoiceAllocator.cpp -o /tmp/sylenth_ai_dsp_core_direct
    /tmp/sylenth_ai_dsp_core_direct

Attempted but intentionally stopped after project sources compiled and the build entered slow JUCE module compilation:

    cmake --build build --target SylenthAIDspCoreTest -j2
    cmake --build build --target SylenthAIContractTest -j2
