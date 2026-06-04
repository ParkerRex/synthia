---
title: Build Editor UI And Preset Workflow
status: active
created_at: 2026-06-04
completed_at: null
summary: Build the clean-room plugin editor, parameter bindings, modulation slot editing, diagnostics, and user preset workflow.
post_build_recap: null
read_when:
  - Implementing the Synth editor.
  - Changing preset picker, save, or modulation UI behavior.
  - Validating clean-room visual workflow.
program_id: synth-clean-room-pluck-instrument
planning_brief: docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/planning-brief-1.md
---

# Build Editor UI And Preset Workflow

This document is a living execution plan for the plugin editor and preset workflow.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The instrument needs a usable production UI that exposes the synth engine without copying historical panels or trade dress. After this slice, users should be able to load the plugin, edit oscillator/filter/envelope/LFO/modulation/amp controls, select and save presets, see diagnostics, and work with the factory pluck from a clean-room interface.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [ ] Design and implement the clean-room editor layout.
- [ ] Bind UI controls to the parameter registry.
- [ ] Implement preset picker/save/duplicate workflow.
- [ ] Implement TransMod slot editing.
- [ ] Add diagnostics view and UI smoke tests.

## Surprises & Discoveries

None yet. Record JUCE layout, scaling, accessibility, and host automation gesture issues here.

## Decision Log

Decision: The UI must be production-oriented, not a retro panel clone.
Rationale: The clean-room policy forbids copying protected layouts and the product should serve real workflow rather than nostalgia.
Date: 2026-06-04.

## Outcomes & Retrospective

Pending implementation.

## Context and Orientation

This slice depends on the parameter registry, preset system, dry-core engine, and validation harness. `SPEC.md` section 8.1 defines the required UI surfaces. `docs/CLEAN_ROOM.md` defines visual and naming boundaries.

### In Scope

This plan may update `src/plugin/PluginEditor.*`, add reusable UI controls, parameter attachments, preset browser components, modulation slot editors, macro controls, diagnostics, screenshot hooks, and UI tests.

### Out Of Scope

This plan does not add new DSP features, onboard FX algorithms beyond showing existing disabled controls if needed, signing, packaging, or final marketing assets.

## Plan of Work

Build the editor around the actual parameter registry. Create compact sections for header, voice, oscillator, filter, envelopes, LFO, ramp, TransMod, amp/stereo, macros, FX, and diagnostics. Use controls appropriate to each value type and make automation gestures correct.

Implement preset selection, save-as, duplicate, and validation errors. Ensure host state remains independent of external preset files.

Add screenshot/manual smoke support if automated UI tests are limited by JUCE tooling. Update docs with the editor structure and clean-room naming rules.

## Milestones

Milestone 1 creates the main editor shell and parameter bindings.

Milestone 2 adds preset workflow.

Milestone 3 adds TransMod and macro editing.

Milestone 4 adds diagnostics and UI validation.

## Concrete Steps

Work from the repo root:

    cd /Users/bazelrex/Developer/synth

Implement the editor. Build and run tests:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json

Manually launch the standalone target and verify the editor can load `Pluck Core 01`, edit controls, save a duplicate preset, and restore it.

## Validation and Acceptance

Acceptance requires no copied third-party visual identity, all main parameter groups visible, controls bound to real parameters, automation gestures supported where applicable, preset selection/save/duplicate working, invalid preset errors visible, diagnostics showing sample rate/block size/active voices, and standalone UI smoke proof.

### Test Commands

    cd /Users/bazelrex/Developer/synth
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json

Manual verification: open the standalone app, load `Pluck Core 01`, edit controls, save a duplicate preset, close, reopen, and restore the preset.

## Idempotence and Recovery

UI changes are safe to rerun through build/test. If preset save writes user files during testing, use a temporary preset directory and delete only that directory afterward.

## Artifacts and Notes

Expected proof artifacts:

- editor screenshots if captured
- preset validation report
- saved duplicate preset in a temporary test directory

## Interfaces and Dependencies

This slice consumes the parameter registry, preset system, modulation slots, validation harness, and dry-core DSP interfaces. Host packaging later depends on the editor being stable.

