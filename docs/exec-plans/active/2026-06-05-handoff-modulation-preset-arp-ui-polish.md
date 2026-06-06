---
title: Handoff Modulation Preset Arp UI Polish
status: active
created_at: 2026-06-05
completed_at: null
summary: Claude Code handoff plan for modulation visuals, preset browser, arp/step UI, and FX rack polish after feature models exist.
post_build_recap: null
read_when:
  - Preparing modulation, preset browser, arpeggiator, or FX rack UI work for Claude Code.
  - Checking UI handoff dependencies after route, browser, arp, or FX models land.
  - Reviewing Claude Code UI output.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Claude Code
handoff_status: blocked_until_feature_models_exist
---

# Handoff Modulation Preset Arp UI Polish

This is a Claude Code handoff ExecPlan for the UI surfaces that depend on later feature models. It should not be implemented until those models exist.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Phase 1 needs the workflows that made Sylenth fast, plus modern modulation/preset visibility. This handoff covers source tiles, modulation halos, matrix sync, preset browser, arp/step UI, and FX rack polish once the engine/state models are real.

## Progress

- [x] 2026-06-05 EDT: Created this UI handoff ExecPlan.
- [ ] Confirm modulation route model exists before modulation UI handoff.
- [ ] Confirm preset browser model exists before browser UI handoff.
- [x] 2026-06-06 EDT: Confirmed arp/step/chord engine and APVTS model exists before arp UI handoff.
- [ ] Confirm FX rack model exists before FX UI handoff.
- [ ] Hand off completed dependency slice to Claude Code.
- [ ] Record screenshot/manual QA.

## Surprises & Discoveries

None yet. Record missing route/browser/arp/FX models, component limitations, or handoff blockers here.

## Decision Log

Decision: Split deeper UI polish from the first shell handoff.
Rationale: Modulation, browser, arp, and FX UI all depend on feature models that do not exist yet; a separate handoff prevents premature placeholder UI.
Date: 2026-06-05.

## Outcomes & Retrospective

Pending implementation and handoff review.

## Context and Orientation

Do not hand this to Claude Code until the relevant dependency exists:

- Modulation polish requires a route model and destination catalog.
- Preset browser polish requires browser/search/favorite/metadata state.
- Arp UI polish can now bind to `arp.*`, `arp.step.N.*`, `chord.*`, and `chord.voice.N.*`; final step-grid polish still needs Claude Code implementation.
- FX rack polish requires expanded FX module state and tail/cost diagnostics.

Partial handoff is allowed only if the handoff prompt clearly limits scope to completed dependencies.

Primary references:

- `docs/modern-sylenth-baseline.md`
- `Sylenth1Manual.pdf`
- `Serum_Manual.pdf`
- `research/sylenth1-screenshots/SOURCE_INDEX.md`
- `research/sylenth1-screenshots/strongmocha-presets-view.jpg`
- `research/sylenth1-screenshots/bears-preset-tour-main-screen.jpg`
- `research/sylenth1-screenshots/splice-modulation-play-modes-panel.png`
- `research/sylenth1-screenshots/splice-arpeggiator-effects-panel.png`
- `research/sylenth1-screenshots/splice-routing-chain-strip.png`

Current code references:

- `src/plugin/PluginEditor.*`
- `src/plugin/ParameterRegistry.*`
- `src/presets/PresetManager.*`
- future route, browser, arp, and FX model files.

### In Scope

- Source tiles for LFOs, envelopes, velocity, keytrack, mod wheel, aftertouch, random, voice, unison, macros, and future arp/step sources.
- Drag/drop route creation where supported by the model.
- Modulation halos/arcs, polarity display, hover inspection, context menu remove/bypass/reset/source/MIDI learn actions.
- Matrix drawer synchronized with direct manipulation.
- Preset browser drawer with factory/user filters, tags, favorites, author, notes, search, validation errors, prev/next, dirty state, init/randomize/reset, save/overwrite.
- Arp/step UI with mode, rate, gate, octave/wrap, hold, step pitch, velocity, tie/hold lanes, and host-sync state.
- FX rack with module power/bypass, mix, compact controls, cost, tail hints, and fixed signal order unless reorder is explicitly implemented.

### Out Of Scope

- Implementing missing route/browser/arp/FX engine behavior.
- Adding AI generation UI before Phase 2.
- Touching realtime DSP.
- Hiding state that should be inspectable in presets or parameters.

## Plan of Work

Use this as a handoff plan in parts. When a feature model lands, prepare a scoped Claude Code prompt for that surface only. Avoid combining unrelated polish if only one dependency is ready.

Design constraints:

- Keep controls dense and scan-friendly.
- Text and values must fit at compact plugin sizes.
- Draw modulation state from the route model; do not duplicate hidden UI-only routes.
- Browser scan and preset operations must not run on the audio thread.
- Use source screenshots/manual for workflow expectations, not literal copying.

## Milestones

Milestone 1 confirms which feature model is ready for handoff.

Milestone 2 hands only the ready surface to Claude Code.

Milestone 3 reviews binding and synchronization behavior.

Milestone 4 records screenshot/manual QA and updates this plan's progress.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai

Before each partial handoff, verify the relevant model exists:

    rg -n "mod_route|browser|favorite|arp\\.|fx\\." src docs/PRESET_SCHEMA.md docs/modern-sylenth-baseline.md

After Claude Code returns a patch, run:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

Then record screenshots/manual QA for the handed-off surface.

## Validation and Acceptance

Acceptance requires:

- UI controls bind to real route/browser/arp/FX state.
- Matrix and halos/source tiles stay synchronized.
- Preset browser actions roundtrip through preset/state APIs.
- Arp UI edits produce deterministic render changes in the relevant engine tests.
- FX rack bypass/mix state is visible and matches render validation.
- Screenshot/manual QA notes for each handed-off surface.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

Manual verification: open the standalone target or plugin UI, exercise the handed-off surface, and record screenshots/notes in this plan or a linked QA note.

## Idempotence and Recovery

If Claude Code implements UI-only state that diverges from route/browser/arp/FX models, reject that portion and return to the dependency gate. Partial handoffs should be independently reversible.

## Artifacts and Notes

Expected handoff deliverables:

- Patch to UI code and any required UI-only model adapters.
- Screenshot evidence for normal and compact sizes.
- Manual QA notes.
- List of missing engine/model fields, if any.

## Interfaces and Dependencies

Depends on future Phase 1 feature-model ExecPlans:

- modulation route model
- preset browser and bank workflow
- arp/step/chord engine
- FX rack expansion

Produces UI QA artifacts and polished surfaces for Phase 1 Ableton validation.
