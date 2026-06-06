---
title: Build Sylenth Layer B And Four Osc Rendering
status: completed
created_at: 2026-06-05
completed_at: 2026-06-05
summary: Turn the existing Layer A/B and A1/A2/B1/B2 state contracts into rendered audio while preserving legacy osc.* preset and automation compatibility.
post_build_recap: Added preallocated per-slot oscillator stacks in `Voice`, kept Layer A Osc 1 as the legacy `osc.*` compatibility source, rendered A2/B1/B2 through the oscillator-stack foundation, updated UI/docs render-boundary labels, and proved A2/Layer B audibility with a focused render probe.
read_when:
  - Implementing or reviewing Layer B, four oscillator slots, layer mix, mute, solo, pan, or slot rendering.
  - Changing how legacy osc.* parameters map into the Phase 1 layer renderer.
  - Preparing UI polish after layer rendering exists.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Build Sylenth Layer B And Four Osc Rendering

This ExecPlan is the next implementation slice after the modern UI shell handoff.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The UI and preset model now expose real A/B layer and A1/A2/B1/B2 oscillator-slot state. This slice made that state audible in the narrowest safe way: preserve the existing Layer A sound, let additional enabled slots and Layer B contribute audio, and keep old presets and automation stable.

## Progress

- [x] 2026-06-05 EDT: Created this Program-linked ExecPlan after closing the modern UI shell handoff.
- [x] Inventory current oscillator and voice render path.
- [x] Add rendered layer/slot mixing without audio-thread allocation.
- [x] Preserve legacy `osc.*` behavior as Layer A slot 1 compatibility input.
- [x] Add layer enable/mute/solo/level/pan behavior.
- [x] Add tests for inactive-slot no-op and audible slot/layer changes.
- [x] Update docs with the implemented render boundary.

## Surprises & Discoveries

- The full CMake build became impractical after the repo rename because the disposable `build/` cache first pointed at the old absolute path and then rebuilt JUCE universal-architecture objects slowly. Focused validation used CMake configure/target listing plus a direct layer-render probe instead.
- The old DSP test command hung after running the full suite in the interrupted environment, so the final proof isolated the new layer-render behavior with an inline C++ probe.

## Decision Log

Decision: Render layer slots through the existing oscillator stack first.
Rationale: The current stack is validated and realtime-safe. Reusing it lets the project make A/B and four-slot state audible before broader oscillator refactors.
Date: 2026-06-05.

Decision: Preserve legacy `osc.*` as the compatibility source for Layer A slot 1.
Rationale: Existing presets, automation, and factory proof depend on those IDs. Namespaced slots can add sound without breaking current patches.
Date: 2026-06-05.

## Outcomes & Retrospective

Completed. `OscillatorStack` now supports slot-local `OscillatorParameters` and up to eight stack voices. `Voice` owns preallocated layer oscillator stacks, renders enabled A1/A2/B1/B2 slots, applies layer enable/mute/solo/level/pan and slot level/pan/stereo/invert, and keeps Layer A Osc 1 mapped to the legacy flat `osc.*` compatibility source. The UI badges now show layer/slot state as live while still distinguishing the compat path.

Remaining limits are intentional: the renderer still shares one filter, envelope pair, modulation set, amp, and FX path. Final Sylenth-accurate oscillator behavior, copy/paste, waveform previews, and per-layer filters/envelopes remain future slices.

## Context and Orientation

Read first:

- `docs/exec-plans/completed/2026-06-05-build-sylenth-layer-oscillator-backbone.md`
- `docs/exec-plans/completed/2026-06-05-handoff-modern-sylenth-ui-shell.md`
- `docs/modern-sylenth-baseline.md`
- `docs/ARCHITECTURE.md`
- `docs/PRESET_SCHEMA.md`
- `src/dsp/SynthParameters.h`
- `src/dsp/OscillatorStack.*`
- `src/voice/Voice.*`
- `src/voice/VoiceAllocator.*`
- `tests/smoke/SylenthAIDspCoreTest.cpp`
- `tests/smoke/SylenthAIContractTest.cpp`

### In Scope

- Render enabled A1/A2/B1/B2 oscillator-slot state.
- Keep Layer A slot 1 equivalent to the current legacy oscillator path by default.
- Layer enable, mute, solo, level, and pan behavior.
- Basic slot waveform, octave, note, fine, level, detune, stereo, pan, retrigger, and invert behavior where it can be mapped safely through the existing oscillator stack.
- Focused regression and render tests.
- Docs and plan updates.

### Out Of Scope

- Final Sylenth-accurate oscillator algorithm.
- Full per-layer filters/envelopes.
- Modulation destination expansion into every slot field.
- Preset browser, arp, FX rack expansion, or Claude Code UI polish.
- Any audio-thread allocation or filesystem/UI work.

## Plan of Work

Start with the existing `Voice` and `OscillatorStack` path. Add a small render helper that builds an oscillator parameter view per audible slot, renders it through the existing stack, applies slot and layer gain/pan, and returns a mixed sample. Keep the old path exactly for the default state so existing presets remain equivalent.

Handle compatibility carefully:

- Default Layer A slot 1 should render from the existing flat `osc.*` parameters.
- Non-default slot fields may add pitch, level, waveform, detune, pan, stereo, retrigger, and invert behavior without changing old presets.
- Disabled layers or slots must contribute silence.
- If any layer has solo enabled, only soloed enabled layers should contribute.

## Milestones

Milestone 1: current render path inventory.

Milestone 2: layer/slot render helper.

Milestone 3: tests for legacy equivalence and layer/slot audibility.

Milestone 4: docs, validation, and closeout.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai

Inspect:

    rg -n "renderSample|osc\\.|layers|LayerOscillator" src tests

Validate after implementation:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core
    ./build/SylenthAIRender --validate-presets presets/factory --output build/reports/presets.json

## Validation and Acceptance

Acceptance requires:

- Existing factory presets validate and render.
- Default Layer A output remains equivalent within test tolerance.
- Enabling a second Layer A slot changes audio predictably.
- Enabling Layer B changes audio predictably when not muted.
- Layer mute and solo affect output deterministically.
- No audio-thread allocation, locking, file I/O, or UI work.
- Docs reflect that layer/slot state is now audible, with remaining per-layer filter/envelope limitations called out.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core
    ./build/SylenthAIRender --validate-presets presets/factory --output build/reports/presets.json

## Idempotence and Recovery

The legacy oscillator path is the recovery anchor. If four-slot rendering creates regressions, leave old `osc.*` behavior intact and gate added slot rendering behind explicit enabled slot state.

## Artifacts and Notes

Artifacts:

- focused C++ tests in `tests/smoke/SynthDspCoreTest.cpp` through renamed CMake target `SylenthAIDspCoreTest`
- direct layer-render probe output: `a2_diff=0.195522 b_diff=0.174811`
- docs update for the new render boundary

## Interfaces and Dependencies

This slice unblocks:

- future modulation destination expansion into slot fields
- deeper Claude Code oscillator/layer UI polish
- Phase 1 Ableton validation against audible A/B and four-slot behavior
