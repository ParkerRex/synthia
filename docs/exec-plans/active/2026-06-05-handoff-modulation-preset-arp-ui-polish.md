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
handoff_status: model_ready_for_preset_arp_fx_and_modulation_inspection
---

# Handoff Modulation Preset Arp UI Polish

This is a Claude Code handoff ExecPlan for the UI surfaces that depend on later feature models. It should not be implemented until those models exist.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Phase 1 needs the workflows that made Sylenth fast, plus modern modulation/preset visibility. This handoff covers source tiles, modulation halos, matrix sync, preset browser, arp/step UI, and FX rack polish once the engine/state models are real.

## Progress

- [x] 2026-06-05 EDT: Created this UI handoff ExecPlan.
- [x] 2026-06-06 EDT: Confirmed modulation route model exists for source/destination catalogs and route inspection over the current TransMod slots.
- [x] 2026-06-06 EDT: Confirmed preset browser/source/bank/category/tag/favorite/search model exists before browser UI handoff.
- [x] 2026-06-06 EDT: Confirmed arp/step/chord engine and APVTS model exists before arp UI handoff.
- [x] 2026-06-06 EDT: Confirmed fixed-order FX rack model exists before FX UI handoff.
- [x] 2026-06-06 EDT: Handed the arp/step/chord dependency slice to Claude Code with streaming headless output; the run produced no patch before it stalled.
- [x] 2026-06-06 EDT: Implemented the bounded arp/step/chord UI surface in `PluginEditor.*` with real APVTS bindings for arp controls, 16 arp steps, and 8 chord voices.
- [x] 2026-06-06 EDT: Recorded native standalone screenshot evidence that the sequencer row renders at a tall window size; bottom-lane scroll QA remains for host/manual polish.
- [x] 2026-06-06 EDT: Completed the FX rack model with fixed-order APVTS-backed distortion/saturation, phaser, chorus, EQ, delay, reverb, compressor, quality, and patch-cost state; captured native standalone FX tab screenshot evidence.
- [x] 2026-06-06 EDT: Verified the current Claude Code headless workflow from docs/local CLI help: use `claude -p` with `--output-format stream-json --verbose` for observable non-interactive UI handoff runs.
- [x] 2026-06-06 EDT: Handed preset/browser, arp/chord, and FX polish to Claude Code with screenshot references; stopped the run after it stayed in research mode and produced no patch.
- [x] 2026-06-06 EDT: Applied bounded local UI polish in `PluginEditor.cpp`: preset popup sections, ordered FX rack stage titles/badges, reserved title/badge spacing, and framed arp/chord grids.
- [x] 2026-06-06 EDT: Captured native standalone screenshot evidence at `build/reports/ui/preset-arp-fx-polish-sound.png`, `build/reports/ui/preset-arp-fx-polish-sound-paged.png`, and `build/reports/ui/preset-arp-fx-polish-effects.png`.
- [x] 2026-06-06 EDT: Passed `git diff --check`, standalone rebuild, CTest, and `SylenthAIRender --suite core` for the UI polish slice.
- [x] 2026-06-06 EDT: Route model handoff dependency landed in `ModulationRouteModel`: source catalog, destination catalog, active route summaries, and legacy cutoff-depth visibility. Drag/drop write adapters, per-route bypass/remove, and expanded destinations remain out of scope.
- [x] 2026-06-06 EDT: Handed a narrow modulation-inspection prompt to Claude Code with Sylenth screenshot references; stopped the run after it stayed in analysis/build-check mode without editing files.
- [x] 2026-06-06 EDT: Implemented a bounded read-only modulation overview in `PluginEditor.*` using `synth::modulationSourceCatalog()` and `SynthAudioProcessor::getModulationRouteView()`.
- [x] 2026-06-06 EDT: Captured native standalone Modulation tab screenshot evidence at `build/reports/ui/modulation-inspection-ui-polish.png`.
- [x] 2026-06-06 EDT: Passed `git diff --check`, standalone rebuild, CTest, and `SylenthAIRender --suite core` for the modulation inspection UI slice.
- [x] 2026-06-06 EDT: Ran an adversarial read-only pass; fixed hidden-tab route refresh churn and scaler-only source activity counts before commit.

## Surprises & Discoveries

- Claude Code headless streaming works with `claude -p --output-format stream-json --verbose`, but the first broad screenshot-heavy prompt over-researched and emitted no patch. Future Claude prompts for this repo should use tight file/edit constraints and bounded turns.
- A second tighter screenshot-referenced Claude handoff still spent the run reading and reasoning without patching. For UI polish, use Claude as a fast bounded reviewer/polisher, but keep a local implementation path ready when the patch does not appear quickly.
- A third narrow modulation-inspection Claude handoff with explicit two-file scope still stopped before patching. The practical loop is now: give Claude strict UI polish prompts, watch streaming output, terminate analysis-only runs quickly, and preserve local implementation as the shipping path.
- Native standalone screenshot automation confirmed the app opens after the editor patch. A taller window captured the new sequencer row itself, while PageDown and direct Accessibility scroll-bar automation did not move the JUCE viewport far enough to inspect the bottom lanes.

## Decision Log

Decision: Split deeper UI polish from the first shell handoff.
Rationale: Modulation, browser, arp, and FX UI all depend on feature models that do not exist yet; a separate handoff prevents premature placeholder UI.
Date: 2026-06-05.

## Outcomes & Retrospective

Partial arp/step/chord implementation passed build, CTest, adversarial review, and native standalone visual launch/row evidence. FX rack state is now model-backed and has a first local visual polish pass with ordered module badges. Modulation inspection polish now has a route model over the current TransMod state and a read-only overview panel that exposes source counts and active routes without inventing write behavior.

The next Claude Code pass can target deeper preset browser, arp/step/chord, and FX rack visual polish only. Modulation drag/drop, matrix editing, per-route bypass/remove, and route halos still need explicit write adapters or schema support before they become UI scope.

## Context and Orientation

Do not hand a surface to Claude Code until the relevant dependency exists:

- Modulation inspection polish can now bind to `ModulationRouteModel`, source/destination catalogs, `SynthAudioProcessor::getModulationRouteView()`, and current `transmod.N.*` APVTS fields. Drag/drop writing, per-route bypass/remove, and expanded destination creation still require a later write-adapter/schema slice.
- Preset browser polish can now bind to `PresetSummary`, `PresetBrowserFilter`, `PresetBrowserCatalog`, source/bank/category/tag fields, and sidecar favorite keys.
- Arp UI polish now has a bounded APVTS-backed editor surface for `arp.*`, `arp.step.N.*`, `chord.*`, and `chord.voice.N.*`; direct visual QA and any deeper Claude polish remain.
- FX rack polish can now bind to fixed-order distortion/saturation, phaser, chorus/flanger-style modulation, EQ, delay, reverb, compressor, master bypass, quality, tail, and patch-cost state.

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
