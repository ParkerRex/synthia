---
title: Build MIDI Controller Workflow
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Add the Phase 1 MIDI learn bridge, global CC map sidecar, compact controller panel, and validation hooks needed before Ableton controller proof.
post_build_recap: Added a global MIDI controller map at ~/Music/ParkerX/sylenth-ai/MidiControllerMap.json, a Sound-tab MIDI Learn panel, realtime-safe CC capture through fixed atomics, message-thread APVTS application, map normalization/read-write helpers, docs, and contract coverage. Adversarial review found failed sidecar writes could desync in-memory state; fixed by staging writes before swapping and publishing assignments.
read_when:
  - Implementing or reviewing MIDI Learn, MIDI CC mappings, controller automation, or host controller validation.
  - Preparing per-control context menu polish for Claude Code.
  - Designing Phase 2 or Phase 3 prompt-driven control changes that must preserve normal APVTS automation.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Build MIDI Controller Workflow

This ExecPlan closes the Phase 1 controller bridge before the Ableton validation pass.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Sylenth-style control needs MIDI Learn/Forget before Phase 1 host proof is meaningful. The current engine already handles notes, pitchbend, sustain, all-notes-off, all-sound-off, mod wheel, aftertouch, and channel pressure, but it had no persistent CC-to-parameter assignment workflow.

This slice adds a minimal real controller workflow: learned CCs persist as global user-library state, incoming CCs are captured without audio-thread allocation or locks, and mapped values reach the same APVTS parameters used by UI, presets, and host automation.

## Progress

- [x] Created the `MidiControllerMap` helper for normalization, JSON read, and JSON write.
- [x] Added processor-level MIDI Learn, Forget, Cancel, assignment status, and message-thread application.
- [x] Added a compact Sound-tab MIDI Controller panel.
- [x] Added contract coverage for map normalization and sidecar read/write.
- [x] Fixed adversarially found write-failure rollback so in-memory assignments and published atomics change only after sidecar persistence succeeds.
- [x] Updated architecture, preset schema, validation, baseline, and Program docs.

## Decision Log

Decision: Store learned MIDI CC assignments in a global user sidecar.
Rationale: MIDI Learn is user controller preference state, not factory preset sound state. A sidecar keeps factory presets read-only and avoids adding 100+ automatable map parameters.
Date: 2026-06-06.

Decision: Apply mapped CC values from the message-thread timer.
Rationale: The audio thread may see incoming CCs, but it must not mutate APVTS parameters, write files, or lock map state.
Date: 2026-06-06.

Decision: Treat CC64, CC120, and CC123 as reserved.
Rationale: Sustain, all-sound-off, and all-notes-off already have performance/safety semantics and should not be repurposed by Learn.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. `PluginProcessor` now loads `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json`, publishes fixed atomic CC-to-parameter indexes, captures pending MIDI Learn through atomics, queues incoming mapped controller values, and applies values to APVTS parameters from its timer. The editor exposes a compact Sound-tab panel with parameter selection, Learn, Forget, Cancel, status, and current assignment summary.

The map sidecar normalizes invalid or conflicting entries and writes deterministic JSON. Learn/Forget stage candidate assignments, persist them, and only then swap in-memory state and publish atomics, so failed writes leave the previous runtime map intact. `SylenthAIContractTest` covers conflict handling and read/write roundtrip.

Per-control right-click Learn/Forget, default CC map data, UI polish, and Ableton controller smoke remain follow-up work.

## Context and Orientation

Read first:

- `docs/modern-sylenth-baseline.md`
- `docs/PRESET_SCHEMA.md`
- `docs/ARCHITECTURE.md`
- `src/midi/MidiControllerMap.*`
- `src/plugin/PluginProcessor.*`
- `src/plugin/PluginEditor.*`
- `tests/smoke/SynthContractTest.cpp`

## In Scope

- Global MIDI CC map sidecar.
- Learn, Forget, and Cancel workflow.
- Realtime-safe capture and message-thread parameter application.
- Compact UI bridge for the workflow.
- Contract tests and durable docs.

## Out Of Scope

- Per-control right-click context menus.
- Default Sylenth appendix CC map data.
- Host-specific Ableton controller proof.
- AI prompt-driven controller assignment.
- Preset-embedded MIDI maps.

## Validation and Acceptance

Acceptance requires:

- MIDI map normalization rejects invalid assignments and resolves CC/parameter conflicts deterministically.
- MIDI map JSON round-trips through the sidecar shape.
- Incoming mapped CCs do not allocate, lock, write files, or mutate APVTS on the audio thread.
- UI exposes Learn, Forget, Cancel, status, and current mappings.
- Docs describe the global map contract and remaining host-proof gaps.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    cmake --build build --target SylenthAIContractTest -j2
    ./build/SylenthAIContractTest
    cmake --build build --target SylenthAIPlugin_Standalone -j2
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core-midi-controller-workflow
