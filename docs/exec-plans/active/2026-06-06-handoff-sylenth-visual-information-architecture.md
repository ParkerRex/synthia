---
title: Handoff Sylenth Visual Information Architecture
status: active
created_at: 2026-06-06
completed_at: null
summary: Claude Code handoff plan for reshaping the current model-backed editor into a denser Sylenth-informed visual information architecture without adding fake controls or DSP.
post_build_recap: null
read_when:
  - Preparing the next Claude Code UI pass after the roadmap truth audit.
  - Making the editor look and scan more like the supplied Sylenth screenshots.
  - Checking which visual changes are safe before modulation write adapters, preset dirty/randomize/reset, or per-layer filter parity exist.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Claude Code
handoff_status: ready_for_visual_ia_pass
---

# Handoff Sylenth Visual Information Architecture

This is a Claude Code handoff ExecPlan for the next visible product step: make the editor feel much closer to a Sylenth-level instrument while preserving the real state boundaries that exist today.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The current editor is a working model-backed shell, but it still reads like an internal scaffold more than a mature Sylenth-style synthesizer. This slice should use `Sylenth1Manual.pdf`, the 25-image screenshot corpus, and `docs/modern-sylenth-baseline.md` to reorganize and restyle the existing UI into a denser, faster, more hardware-like synth surface.

The goal is not to add backend features. The goal is to make the current real features look coherent: A/B layers, A1/A2/B1/B2 oscillator slots, shared filter/envelopes/LFO/ramp/macros, preset browser drawer, arp/step/chord, fixed FX rack, read-only modulation overview, MIDI Learn, meters, panic, diagnostics, and patch-load/voice feedback.

## Progress

- [x] 2026-06-06 EDT: Created this handoff plan after the roadmap truth audit commit `a30f970`.
- [x] 2026-06-06 EDT: Confirmed enough model-backed UI state exists for a visual IA pass: layer/slot rendering, preset browser, arp/step/chord, FX rack, read-only modulation overview, and global MIDI Learn.
- [ ] Hand off to Claude Code.
- [ ] Review returned patch for real bindings, layout fidelity, and no fake feature controls.
- [ ] Record screenshot/manual QA and update this plan with outcomes.

## Surprises & Discoveries

- Pending implementation.

## Decision Log

Decision: Start with visual information architecture before deeper backend feature adapters.
Rationale: The app now has enough real state to support a Sylenth-informed shell, and the current visible mismatch is blocking product judgment. Deeper interactions such as drag/drop modulation, preset dirty/randomize/reset, and per-control MIDI context menus still require later backend slices.
Date: 2026-06-06.

Decision: Treat this as a broad layout and visual-density pass, not a feature implementation pass.
Rationale: The fastest useful improvement is to make current controls scan like one integrated synth. Adding fake controls would make the roadmap less trustworthy.
Date: 2026-06-06.

## Outcomes & Retrospective

Pending implementation.

## Context and Orientation

Work from:

    cd /Users/parkerrex/Developer/sylenth-ai

Primary product references:

- `docs/modern-sylenth-baseline.md`
- `docs/programs/active/2026-06-05-sylenth-lab-rebuild/program.md`
- `docs/exec-plans/active/index.md`
- `docs/PRESET_SCHEMA.md`
- `docs/ARCHITECTURE.md`
- `Sylenth1Manual.pdf`
- `research/sylenth1-screenshots/SOURCE_INDEX.md`

Primary screenshot references:

- `research/sylenth1-screenshots/official-lennardigital-main-ui.jpg`
- `research/sylenth1-screenshots/kvraudio-sylenth1-v302-main-ui.png`
- `research/sylenth1-screenshots/vintagesynth-default-main-ui.jpg`
- `research/sylenth1-screenshots/strongmocha-base-ui-large.jpg`
- `research/sylenth1-screenshots/splice-full-ui-bright.png`
- `research/sylenth1-screenshots/splice-top-panel.png`
- `research/sylenth1-screenshots/splice-oscillator-mix-panel.png`
- `research/sylenth1-screenshots/splice-filter-controls-panel.png`
- `research/sylenth1-screenshots/splice-filter-section-panel.png`
- `research/sylenth1-screenshots/splice-modulation-play-modes-panel.png`
- `research/sylenth1-screenshots/splice-arpeggiator-effects-panel.png`
- `research/sylenth1-screenshots/splice-routing-chain-strip.png`
- `research/sylenth1-screenshots/strongmocha-presets-view.jpg`

Current implementation references:

- `src/plugin/PluginEditor.*`
- `src/plugin/PluginProcessor.*`
- `src/plugin/ParameterRegistry.*`
- `src/dsp/SynthParameters.h`
- `src/modulation/ModulationRouteModel.*`
- `src/presets/PresetManager.*`
- `src/dsp/Arpeggiator.*`
- `src/dsp/fx/FxChain.*`
- `tests/smoke/*`

Current real UI/state surfaces:

- Header: preset controls, output meter, active voices, patch-load estimate, panic, diagnostics.
- Layer surface: Layer A/B selector and parameter-backed enable, solo, mute, level, and pan.
- Oscillator slots: A1/A2/B1/B2 state and audible slot rendering, with A1 preserving the legacy flat `osc.*` compatibility source.
- Sound controls: current oscillator stack, filter, amp envelope, mod envelope, LFO, ramp, voice, amp/stereo, macros, MIDI Learn panel, preset browser drawer, and arp/step/chord row.
- Modulation controls: read-only route overview and TransMod slot controls over the existing route model.
- Effects controls: fixed-order rack for saturation/distortion, phaser, chorus, EQ, delay, reverb, compressor, master bypass, and quality.

Known missing backend/model support that must not be faked:

- Modulation drag/drop writing, halos, matrix editing, per-route bypass/remove, and expanded destination writing.
- Per-control right-click MIDI Learn/Forget context menus.
- Dirty state, init, randomize, reset original, safe overwrite, delete, and A/B compare.
- Rich preset metadata editing.
- LFO2 and editable movement graphs.
- Per-layer filters/envelopes, cross-routing, and final post-filter mixer/master parity.
- Full AU/VST3 automation proof.

### In Scope

- Reorganize the current editor so the first screen reads like a dense mature synthesizer rather than a tabbed internal control sheet.
- Use the screenshot corpus to drive section hierarchy: top preset/header strip, Part/Layer selector, oscillator/mixer/filter/envelope rhythm, modulation/play-mode area, arpeggiator/effects area, and performance/diagnostic feedback.
- Restyle existing JUCE controls in `PluginEditor.*` to be more coherent, compact, and Sylenth-informed while remaining original.
- Improve panel titles, grouping, spacing, value readouts, power states, badges, and visual rhythm.
- Prefer a default-size Sound page that exposes the core patching surface with minimal scrolling.
- Preserve responsive behavior at the existing compact plugin minimum.
- Capture screenshot evidence for Sound, Modulation, Effects, preset browser, and compact layout.
- Update this ExecPlan with what changed, validation commands, screenshots, and residual UI gaps.

### Out Of Scope

- Adding DSP, parameters, preset schema fields, or processor APIs.
- Adding UI controls for features that do not exist yet.
- Making modulation halos or drag/drop routes writable before the write-adapter/schema slice.
- Adding init/randomize/reset/A-B controls unless a real processor/preset API already exists and is safely wired.
- Adding per-layer Filter A/B controls or cross-routing controls before backend parity exists.
- Copying screenshot assets into the app or using literal raster screenshots as UI.
- Touching audio-thread code.

## Plan of Work

1. Read the roadmap truth first: `docs/modern-sylenth-baseline.md`, this plan, and `docs/exec-plans/active/2026-06-05-handoff-modulation-preset-arp-ui-polish.md`.
2. Inspect the current editor implementation in `src/plugin/PluginEditor.*` and identify which visible controls are already real APVTS bindings or derived diagnostics.
3. Review the screenshot corpus and write a short visual IA note in this plan's `Progress` or `Surprises & Discoveries`: what the Sylenth screenshots imply about hierarchy, density, grouping, and panel rhythm.
4. Implement a bounded UI-only patch, preferably confined to `src/plugin/PluginEditor.*`.
5. Keep all visible controls bound to existing parameters, processor methods, route views, browser APIs, or derived diagnostics.
6. Remove or reword misleading labels that imply missing backend behavior.
7. Build, test, run the render suite, and capture screenshots.
8. Update this plan with validation and residual gaps.

Design constraints:

- Dense, scan-friendly, production synth UI.
- No visible tutorial/explainer text in the app.
- Do not hide important state behind decorative cards.
- Avoid nested cards and oversized marketing-style sections.
- Text must fit in controls at normal and compact plugin sizes.
- Stable dimensions for module panels, knobs, switches, badges, and rows.
- Use source screenshots for product layout and density, while keeping the app's implementation and styling original.

## Milestones

Milestone 1: Reference and current-state audit.

- Read the docs, screenshot index, and `PluginEditor.*`.
- Record the visual IA interpretation in this plan.

Milestone 2: UI-only implementation.

- Reshape layout and visual styling in `PluginEditor.*`.
- Keep no-fake-control gates intact.

Milestone 3: Validation and screenshots.

- Run required commands.
- Capture normal and compact screenshots for key pages.

Milestone 4: Handoff closeout.

- Update this plan with screenshots, changed files, residual gaps, and validation results.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai

Inspect references:

    sed -n '1,260p' docs/modern-sylenth-baseline.md
    sed -n '1,220p' docs/exec-plans/active/2026-06-05-handoff-modulation-preset-arp-ui-polish.md
    sed -n '1,180p' research/sylenth1-screenshots/SOURCE_INDEX.md
    rg -n "Panel|addPanel|layer|presetBrowser|modulation|arp|fx|midi" src/plugin/PluginEditor.*

Suggested implementation scope:

- Start in `src/plugin/PluginEditor.h` and `src/plugin/PluginEditor.cpp`.
- Add small UI-only helpers only when they reduce complexity.
- Do not edit DSP, processor, preset schema, or parameter registry unless a compile error exposes a naming mistake.

Validation commands:

    git diff --check
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

Standalone screenshot target:

    open build/SylenthAIPlugin_artefacts/Standalone/sylenth-ai.app

Save screenshot evidence under:

    build/reports/ui/sylenth-visual-ia-sound.png
    build/reports/ui/sylenth-visual-ia-modulation.png
    build/reports/ui/sylenth-visual-ia-effects.png
    build/reports/ui/sylenth-visual-ia-browser.png
    build/reports/ui/sylenth-visual-ia-compact.png

## Validation and Acceptance

Acceptance requires:

- The editor looks materially closer to a Sylenth-level dense synth UI at first glance.
- Sound, Modulation, Effects, preset browser, and compact layouts have screenshot evidence.
- All visible controls bind to real APVTS parameters, processor APIs, route views, browser APIs, or derived diagnostics.
- Missing backend features are not represented as working controls.
- No new DSP, parameter IDs, preset schema, or audio-thread work.
- `git diff --check`, Debug build, CTest, and `SylenthAIRender --suite core` pass.
- No clipped labels, incoherent overlap, broken scrolling, or unclickable controls at the existing compact size.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

Manual verification: open the standalone app or installed plugin UI, switch through Sound/Modulation/Effects, open the preset browser, toggle relevant module power states, switch Layer A/B, exercise a few controls, and record screenshots/notes in this plan.

## Idempotence and Recovery

If a UI idea needs missing backend support, leave it out and record it as a dependency for a later ExecPlan. Do not introduce placeholder state or disabled-looking controls just to match the screenshots.

If the patch becomes too broad, reduce scope to layout, section hierarchy, typography/spacing, and existing control styling. The old layout can be recovered by reverting `PluginEditor.*` plus this plan's progress notes.

## Artifacts and Notes

Expected handoff deliverables:

- UI patch, preferably limited to `src/plugin/PluginEditor.*`.
- Updated progress/outcome notes in this ExecPlan.
- Screenshot evidence under `build/reports/ui/`.
- Residual gap list for backend-dependent UI features.

## Interfaces and Dependencies

Depends on completed slices:

- `docs/exec-plans/completed/2026-06-05-build-sylenth-layer-oscillator-backbone.md`
- `docs/exec-plans/completed/2026-06-05-build-sylenth-layer-b-and-four-osc-rendering.md`
- `docs/exec-plans/completed/2026-06-05-handoff-modern-sylenth-ui-shell.md`
- `docs/exec-plans/completed/2026-06-05-build-arp-step-chord-workflow.md`
- `docs/exec-plans/completed/2026-06-06-build-preset-browser-and-bank-workflow.md`
- `docs/exec-plans/completed/2026-06-06-build-fx-rack-expansion.md`
- `docs/exec-plans/completed/2026-06-06-build-modulation-route-model.md`
- `docs/exec-plans/completed/2026-06-06-build-midi-controller-workflow.md`

Produces:

- A visually credible Phase 1 editor baseline for Parker review.
- A clearer boundary for later backend-dependent UI ExecPlans: preset workflow, modulation write UX, LFO/graph work, and layer filter/envelope/master parity.
