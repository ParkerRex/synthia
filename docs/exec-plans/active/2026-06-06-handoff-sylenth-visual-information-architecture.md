---
title: Handoff Sylenth Visual Information Architecture
status: active
created_at: 2026-06-06
completed_at: null
summary: Claude Code handoff plan for reshaping the current model-backed editor into a Sylenth-faithful visual information architecture without adding fake controls or DSP.
post_build_recap: |
  Reshaped the editor into a Sylenth-faithful native JUCE surface in PluginEditor.{h,cpp} only.
  The latest bronze pass on master adds the warm bronze panel palette, side rails, chrome/charcoal knobs,
  cream tick rings, amber LED toggles, beveled module caption bars, recessed value readouts, blue LCD preset
  display, blue-LCD arp/step/chord grids, and vertical LED output meter while preserving real bindings.
  Earlier passes also moved the Sound page toward the synthesis hero, denser module grid, and honest Osc A1
  Tone naming. No DSP, parameters, schema, processor APIs, copied source assets, screenshot backplates, or fake
  controls were added. Validated with git diff --check, Debug build, CTest 9/9, and SylenthAIRender --suite core
  with 14 reports plus Sound/Modulation/Effects/Browser/compact screenshots under build/reports/ui/.
  PR #46 merged the bronze pass and the authorized visual-parity docs. The surface is closer to Sylenth,
  but additional Claude visual passes are expected for top-strip/LCD fidelity, one-screen density, browser
  integration, and deeper screenshot-to-screenshot polish.
read_when:
  - Preparing the next Claude Code UI pass after the roadmap truth audit.
  - Making the editor look and scan more like the supplied Sylenth screenshots.
  - Checking which visual changes are safe before modulation write adapters, preset dirty/randomize/reset, or per-layer filter parity exist.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Claude Code
handoff_status: bronze_visual_pass_merged_more_fidelity_passes_expected
---

# Handoff Sylenth Visual Information Architecture

This is a Claude Code handoff ExecPlan for the next visible product step: make the editor look and scan much closer to Sylenth while preserving the real state boundaries that exist today.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The current editor is a working model-backed shell, but it still reads like an internal scaffold more than a mature Sylenth synthesizer. This slice should use `Sylenth1Manual.pdf`, the 25-image screenshot corpus, and `docs/modern-sylenth-baseline.md` to reorganize and restyle the existing UI toward faithful Sylenth visual parity: dense module panels, top strip, A/B part workflow, preset/LCD rhythm, knob treatment, warm panel contrast, and compact hardware-like control grouping.

The goal is not to add backend features. The goal is to make the current real features look coherent: A/B layers, A1/A2/B1/B2 oscillator slots, shared filter/envelopes/LFO/ramp/macros, preset browser drawer, arp/step/chord, fixed FX rack, read-only modulation overview, MIDI Learn, meters, panic, diagnostics, and patch-load/voice feedback.

## Progress

- [x] 2026-06-06 EDT: Created this handoff plan after the roadmap truth audit commit `a30f970`.
- [x] 2026-06-06 EDT: Confirmed enough model-backed UI state exists for a visual IA pass: layer/slot rendering, preset browser, arp/step/chord, FX rack, read-only modulation overview, and global MIDI Learn.
- [x] 2026-06-06 EDT: Handed off to Claude Code.
- [x] 2026-06-06 EDT: Implemented the bounded visual IA pass in `src/plugin/PluginEditor.{h,cpp}` only: synthesis-first Sound page, functional-zone header ticks, clarified `Osc A1 Tone` naming, denser knob cells, knob end ticks, slimmer brand, shorter layer bar, selected-part accent. No DSP/parameters/schema/processor APIs added; no fake controls.
- [x] 2026-06-06 EDT: Validated `git diff --check`, Debug build, CTest (5/5), and `SylenthAIRender --suite core` (11 reports); captured Sound/Modulation/Effects/Browser/compact screenshots under `build/reports/ui/`.
- [x] 2026-06-07 EDT: Visual fidelity follow-up on branch `ui-polish-sylenth-fidelity` (UI-only, `PluginEditor.{h,cpp}`): Sylenth radial-tick rotary knobs, vertical ADSR `EnvelopePanel` (Amp/Mod env) with a derived read-only contour, `Osc A1 | Amp Env | Osc A2` + `Filter | Mod Env | LFO` Sylenth grid rows, caption-bar gradient/divider on every module, and taller knob cells. No DSP/parameters/schema/processor APIs added; no fake controls.
- [x] 2026-06-07 EDT: Validated `git diff --check` (clean), Debug build (clean), CTest (9/9), and `SylenthAIRender --suite core` (14 reports); captured `sylenth-fidelity-{sound,modulation,effects,browser,compact}.png` under `build/reports/ui/`.
- [x] 2026-06-07 EDT: Warm bronze Sylenth re-skin on branch `ui-polish-sylenth-bronze` (merged to master, per maintainer direction to adopt the approved Sylenth visual target): bronze metal palette + side rails, chrome/charcoal knobs with cream tick rings and white pointers, amber-lit LED toggles, beveled module caption bars, recessed value readouts, a blue LCD preset display (real preset name/source/bank/category + dirty), blue-LCD arp/step/chord grids, and a vertical LED output meter in the layer bar. Still UI-only in `PluginEditor.{h,cpp}`; no DSP/parameters/schema/processor APIs and no fake controls.
- [x] 2026-06-07 EDT: Validated the bronze pass: `git diff --check` clean, Debug build clean (no warnings), CTest 9/9, `SylenthAIRender --suite core` 14 reports; captured `sylenth-bronze-{sound,modulation,effects,browser,compact}.png` under `build/reports/ui/`.
- [x] 2026-06-07 EDT: PR #46 merged to `master` with `2416664 feat(editor): warm bronze Sylenth skin, blue LCD, and LED meter` and `9e64b73 docs: adopt Sylenth visual-parity direction`.
- [x] 2026-06-07 EDT: Post-merge `master` validation passed: `git diff --check`, `cmake --build build --config Debug`, `ctest --test-dir build --output-on-failure` (9/9), and `./build/SylenthAIRender --suite core --output-dir build/reports/core` (14 reports).
- [x] 2026-06-07 EDT: Top-strip / LCD / density / Browser fidelity pass on branch `ui-polish-sylenth-mainscreen` (UI-only, `PluginEditor.{h,cpp}`): added a dedicated `Browser` page and moved the preset workflow/metadata/browser/MIDI panels onto it so the Sound page holds the synthesis engine near one screen; rebuilt the blue LCD as a monospaced preset/program/diagnostics hub; unified the header+part strip and lit the active `PART` green; recessed knob value boxes. No DSP/parameters/schema/processor APIs and no fake controls; bronze palette preserved.
- [x] 2026-06-07 EDT: Validated this pass: `git diff --check` clean, Debug build clean, CTest 9/9, `SylenthAIRender --suite core` 14 reports, plus an adversarial multi-agent review of the diff; captured `sylenth-onescreen-{sound,modulation,effects,browser,compact}.png` under `build/reports/ui/`.
- [ ] Parker/Claude review of remaining visual fidelity gaps against the screenshot corpus.

## Surprises & Discoveries

### Visual IA interpretation (2026-06-06, from the screenshot corpus)

What the Sylenth1 screenshots imply about hierarchy, density, grouping, and panel rhythm:

- **One integrated top strip, not a marketing band.** `splice-top-panel.png` and the official UI put polyphony, voices, part select, solo, sync, and MIDI learn in a single dense performance strip. The current editor spends ~132px of header on a `SYLENTH-AI` / `CORE OSC ENGINE` brand block and splits performance state across two stacked bars.
- **Everything-visible module grid.** The official UI shows `OSC A1 | AMP ENV A | OSC A2` across the top, `FILTER + FILTER CONTROL + center LCD + MIXER` in the middle, and `MOD ENV 1/2 + LFO 1/2 + MISC 1/2` across the bottom — all on one fixed surface with no scrolling. The synthesis surface is the hero; utilities (the preset list) are a popover over the center LCD, never a leading panel.
- **Module = caption bar + tight knob cluster.** Each Sylenth panel is a titled frame with a small caption and a dense row of knobs, each with a short label beneath. Modules read as a rack because each has a clear identity and consistent rhythm.
- **Restrained warm colour, grouped by zone.** Sylenth is near-monochrome with warm brown/amber panel contrast. Identity comes from grouping, caption bars, labels, knob rhythm, and the central preset/LCD area. The UI should move toward that approved Sylenth target instead of preserving the previous non-Sylenth identity.

### Applied to this pass (UI-only, real bindings preserved)

- Lead the Sound page with the synthesis hero (`Osc A1 | Osc A2`, tone source, filter/envelopes/LFO, voice/amp/ramp/macros, arp/step/chord) and move the preset browser + MIDI utility panels to the bottom so the core patching surface is the first screen with minimal scrolling.
- Earlier passes used functional-zone header ticks so the grid read as a rack instead of a uniform form. Future passes should treat that palette as disposable and move the same grouping rhythm into the approved warm Sylenth visual language.
- Reword the vague legacy `osc.*` panel title to `Osc A1 Tone` so the A1 compatibility relationship is honest rather than a second unexplained "Oscillator".
- Tighten chrome (slimmer brand, shorter layer bar, denser knob cells, knob end ticks) for a more hardware-like density without clipping labels at the compact minimum.

## Decision Log

Decision: Treat the screenshot corpus as an approved visual parity target.
Rationale: The project direction is to recreate Sylenth first. Older notes about keeping the UI visually distinct or merely original caused Claude to preserve the wrong identity. Future UI polish should move toward the warm bronze Sylenth panel language and only stop where real backend/state bindings are missing.
Date: 2026-06-07.

Decision: Start with visual information architecture before deeper backend feature adapters.
Rationale: The app now has enough real state to support a Sylenth-informed shell, and the current visible mismatch is blocking product judgment. Deeper interactions such as drag/drop modulation, preset dirty/randomize/reset, and per-control MIDI context menus still require later backend slices.
Date: 2026-06-06.

Decision: Treat this as a broad layout and visual-density pass, not a feature implementation pass.
Rationale: The fastest useful improvement is to make current controls scan like one integrated synth. Adding fake controls would make the roadmap less trustworthy.
Date: 2026-06-06.

## Outcomes & Retrospective

### What changed (UI-only, bounded to `src/plugin/PluginEditor.{h,cpp}`)

- **Synthesis-first Sound page.** Reordered `layoutActivePage` so the Sound page leads with the synthesis hero — `Osc A1 | Osc A2` slots, the full-width tone source, `Filter | Amp Env | Mod Env | LFO`, and `Voice | Amp | Ramp | Macros` — then the arp/step/chord sequencer, with the preset browser and MIDI utility panels moved to the bottom. The core patching surface now fills the first screen with minimal scrolling, matching the Sylenth everything-visible grid.
- **Grouped-module identity.** Added a `paintModuleHeaderTick` helper and an early functional-zone palette. Every module — `Panel` plus the browser, MIDI, modulation-overview, and sequencer panels — now carries a header tick keyed to its zone, dimmed when a module's power toggle is off, so the surface reads as a rack instead of a uniform form. The palette itself is not a constraint; future polish should re-skin the same grouping with Sylenth's warm panel language.
- **Honest oscillator naming.** Retitled the vague legacy `osc.*` panel from `Oscillator` to `Osc A1 Tone` so the A1 compatibility relationship is explicit rather than a second unexplained oscillator. No binding changed; it still drives the flat `osc.*` parameters.
- **Hardware-like density.** Tightened `Panel` cell metrics (unit width 66→62, cell height 64→60, gaps), added faint min/max ticks to the rotary knob in the look-and-feel, slimmed the brand block (22pt→18pt, 154px→132px), shortened the layer bar (92→86), and added a selected-part accent underline beneath the active Layer button.

All visible controls remain bound to real APVTS parameters, processor APIs (`requestPanic`, MIDI learn, route view), browser APIs, or derived diagnostics. No DSP, parameters, preset schema, processor APIs, or audio-thread code were touched. No writable modulation halos/matrix, preset dirty/init/randomize/reset/A-B, LFO2, per-control MIDI menus, or Filter A/B were added.

### Validation (2026-06-06)

- `git diff --check`: clean.
- `cmake --build build --config Debug`: all targets built (exit 0).
- `ctest --test-dir build --output-on-failure`: 5/5 passed (smoke, contract, voice-core, dsp-core, render-core-suite).
- `./build/SylenthAIRender --suite core --output-dir build/reports/core`: wrote 11 reports.

### Screenshot evidence

Captured from the standalone (`build/SylenthAIPlugin_artefacts/Standalone/sylenth-ai.app`) at the default 1320×940 and the compact 1080×760 minimum:

- `build/reports/ui/sylenth-visual-ia-sound.png` — Sound page, synthesis hero with zone ticks.
- `build/reports/ui/sylenth-visual-ia-modulation.png` — Modulation overview, direct routes, eight TransMod slots.
- `build/reports/ui/sylenth-visual-ia-effects.png` — fixed-order FX rack with per-module ticks/badges.
- `build/reports/ui/sylenth-visual-ia-browser.png` — bottom of Sound page: preset browser + MIDI control.
- `build/reports/ui/sylenth-visual-ia-compact.png` — compact minimum size; both oscillators fit, header compacts, no clipping.

### Residual gaps (backend-dependent, intentionally not done)

- Writable modulation UX: drag/drop, halos on modulated controls, hover inspector, matrix editing, per-route bypass/remove — needs a route write adapter/schema.
- Preset workflow follow-up: delete, copy/paste, richer scanned-preset detail, and persisted A/B compare.
- Per-control right-click MIDI Learn/Forget context menus (global panel remains the bridge).
- LFO2, editable LFO/envelope movement graphs, and a filter response graph with modulation overlays.
- Per-layer Filter A/B, per-layer envelopes, cross-routing, and post-filter mixer/master parity.
- Full visualization/migration of legacy flat `osc.*` fields into the Osc A1 slot.
- Standalone JUCE UI automation for tab/scroll/learn flows (this pass used `screencapture` + peekaboo for manual QA).

### Visual fidelity follow-up (2026-06-07, branch `ui-polish-sylenth-fidelity`)

UI-only pass to make the surface read like a hardware Sylenth instrument rather than a flat dark form. All in `src/plugin/PluginEditor.{h,cpp}`; no DSP, parameters, schema, processor APIs, or fake controls.

- **Rotary knobs.** `SynthLookAndFeel::drawRotarySlider` now draws a radial tick ring (brighter min/max ticks), a top-lit gradient knob body, and a bright-capped accent pointer — the signature Sylenth knob read. Affects every knob.
- **Vertical ADSR envelopes.** New `EnvelopePanel` renders Amp Env and Mod Env as four vertical A/D/S/R faders bound to the same `amp_env.*` / `mod_env.*` APVTS parameters, with a read-only contour drawn above from the live fader values. The contour is a derived visualization, not new control state.
- **Sylenth grid rows.** Sound page top row is now `Osc A1 | Amp Env | Osc A2` and the shaping row is `Filter | Mod Env | LFO`, mirroring Sylenth's `OSC A1 | AMP ENV | OSC A2` hierarchy. The legacy `Osc A1 Tone` panel still spans full width below.
- **Module caption bars.** New `paintCaptionBar` helper gives every panel a soft top-down gradient header and a 1px base divider, so modules read as titled racks. Knob cells are slightly taller (60→64) so knobs read bigger.

Validation: `git diff --check` clean; Debug build clean; CTest 9/9; `SylenthAIRender --suite core` wrote 14 reports. Screenshots: `build/reports/ui/sylenth-fidelity-{sound,modulation,effects,browser,compact}.png` (default 1320×940 and compact 1080×760).

Residual UI gaps unchanged and still backend/scope-bound: writable modulation (drag/drop, halos, matrix, hover, per-route bypass/remove); LFO2 and editable LFO movement graph; filter response graph with modulation overlays (the envelope contour here is read-only, not drag-editable); per-control MIDI context menus; per-layer Filter A/B and master-stage parity; richer scanned-preset detail; reliable JUCE viewport scroll automation (browser shot used a CGEvent wheel helper over the page gutter).

### Bronze merged visual QA (2026-06-07, post-merge `master`)

Post-merge validation passed on `master`:

- `git diff --check`: clean.
- `cmake --build build --config Debug`: passed.
- `ctest --test-dir build --output-on-failure`: 9/9 passed.
- `./build/SylenthAIRender --suite core --output-dir build/reports/core`: wrote 14 reports.

Screenshot set reviewed:

- `build/reports/ui/sylenth-bronze-sound.png`
- `build/reports/ui/sylenth-bronze-modulation.png`
- `build/reports/ui/sylenth-bronze-effects.png`
- `build/reports/ui/sylenth-bronze-browser.png`
- `build/reports/ui/sylenth-bronze-compact.png`

What improved:

- Warm bronze/brown panel language, amber state lights, cream labels, and darker recessed fields now move the app away from the old dark/teal identity.
- The blue LCD preset strip gives the Sound page a Sylenth-like center/readout direction while staying bound to real preset/dirty metadata.
- Knobs, value readouts, caption bars, and power LEDs now read more like a hardware-style synth surface.
- Effects and modulation pages are coherent with the new skin and retain real enable/read-only route state.

Remaining visual fidelity gaps for the next Claude pass:

- Header/top strip still reads more like an app toolbar than Sylenth's integrated polyphony/voices/part/preset/MIDI-learn performance strip.
- The Sound page still scrolls at default and compact sizes; Sylenth's primary synth surface is much closer to all-visible.
- The LCD/preset area is not yet the central interaction hub for preset/program/panel context the way Sylenth uses it.
- The browser, preset workflow, save panel, and MIDI control are still bottom-page panels rather than integrated popover/center-panel workflows.
- Modulation and FX pages are visually skinned but still arranged like model panels; a later pass can push them closer to the reference routing/effects panel rhythm without adding fake controls.

### Top-strip / LCD / density / Browser pass (2026-06-07, branch `ui-polish-sylenth-mainscreen`)

UI-only follow-up in `src/plugin/PluginEditor.{h,cpp}` building on the merged bronze pass. No DSP, parameter IDs, schema, processor APIs, or fake controls were added; the bronze palette is preserved. This pass directly targets the four gaps the bronze QA flagged (top strip, one-screen density, LCD hub, browser integration).

What changed:

- **Browser is now its own page.** Added a fourth `Page::Browser` tab and moved the preset workflow, metadata/safe-save, preset browser, and MIDI-control panels off the bottom of the Sound page onto a dedicated Browser page, laid out as one integrated program workflow: a quick-action/compare strip on top, the program list as the dominant element, and the save-metadata + MIDI utilities paired below. Every real preset control is preserved and still bound — Init/Random/Reset, A/B compare store/load, Save New / Overwrite no-clobber, favorites, search/filter, and global MIDI Learn/Forget.
- **One-screen Sound density.** With the preset panels moved out, the Sound page is the synthesis engine only — `Osc A1 | Amp Env | Osc A2`, `Filter | LCD | Mixer`, `Mod Env | LFO | Voice | Amp`, `Osc A1 Tone`, `Ramp | Macros`, `Arp / Step / Chord` — so the core engine reads on one screen at default size with far less scrolling. Tightened panel cell metrics (unit width 70→66, cell height 74→70) and the inter-panel row gap (12→10).
- **LCD as the central readout hub.** Enlarged the blue LCD (preferredHeight 150→188 to fill the centre row) and rebuilt it as a monospaced backlit screen: preset name, a real `PROGRAM nnn / total` slot index derived from the preset-list position, source/bank/category detail, the dirty pill, and a live `VOICES / LOAD / SR` line fed straight from the diagnostics snapshot. Faint scanlines for the backlit read; no invented values or fake controls.
- **Integrated top strip.** Unified the header and part/layer bar into one continuous brushed-metal performance strip (a soft inset seam instead of a hard toolbar divider), trimmed chrome heights (header 66→60, layer bar 86→80, tabs 40→36), and relabelled the layer selector as `PART` with a green-lit active part and a green Part-Select underline.
- **Module rhythm.** Recessed value boxes beneath the rotary knobs (dark field tint, no outline) for the Sylenth tucked-value look.

Validation: `git diff --check` clean; `cmake --build build --config Debug` clean; `ctest --test-dir build --output-on-failure` 9/9; `./build/SylenthAIRender --suite core --output-dir build/reports/core` wrote 14 reports. The diff was also put through an adversarial multi-agent review (bindings, scope, correctness, layout): no P0/P1, no binding/scope violations, page dispatch and LCD real-state confirmed. Three P2s surfaced — the one introduced by this pass (the MIXER knobs missed the new recessed value-box styling) was fixed; the other two reflect intended/pre-existing behaviour and are recorded under residual gaps.

Screenshots (standalone, default 1320×940 and compact 1080×760):

- `build/reports/ui/sylenth-onescreen-sound.png`
- `build/reports/ui/sylenth-onescreen-modulation.png`
- `build/reports/ui/sylenth-onescreen-effects.png`
- `build/reports/ui/sylenth-onescreen-browser.png`
- `build/reports/ui/sylenth-onescreen-compact.png`

Residual gaps after this pass:

- The Sound page still scrolls below the synthesis engine to reach `Ramp | Macros` and the arp/step/chord grid at default height; the oscillator-slot panels render as three knob rows, taller than Sylenth's two-row oscillator block. Full all-visible parity would need a denser oscillator cell or a more compact arp grid.
- The standalone wrapper's `Options` menu bar and the `SYLENTH-AI` wordmark still sit above the strip (host-provided chrome and brand identity, not the instrument surface).
- The Browser is an in-window page, not a popover overlay over the synth, and the list is the real scanned preset set rather than a fixed 256-slot bank grid.
- The output LED meter lives in the Sound-page MIXER (the Sylenth-faithful location), so it is not visible on the Modulation/Effects/Browser pages; the level is still updated every tick.
- `amp.level_db` is surfaced by two real knobs visible together on the Sound page — the MIXER `MAIN` and the `Amp / Stereo` level. Both are genuine bindings to the same parameter shown in two contexts; left intact to preserve all real bindings rather than dropping a control.
- Backend-dependent gaps are unchanged and intentionally not done: writable modulation, LFO2/editable movement graphs, per-layer filter/master parity, per-control MIDI context menus, and 256-slot bank management (safety model not ready).

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
- Preset workflow controls: dirty state, init, randomize, reset original, local A/B compare, metadata fields, and explicit Save New / Overwrite actions.
- Modulation controls: read-only route overview and TransMod slot controls over the existing route model.
- Effects controls: fixed-order rack for saturation/distortion, phaser, chorus, EQ, delay, reverb, compressor, master bypass, and quality.

Known missing backend/model support that must not be faked:

- Modulation drag/drop writing, halos, matrix editing, per-route bypass/remove, and expanded destination writing.
- Per-control right-click MIDI Learn/Forget context menus.
- Delete/copy/paste preset actions, richer scanned-preset detail, and persisted A/B compare.
- LFO2 and editable movement graphs.
- Per-layer filters/envelopes, cross-routing, and final post-filter mixer/master parity.
- Additional automation proof only when new UI-visible parameters or workflows are added.

### In Scope

- Reorganize the current editor so the first screen reads like a dense mature synthesizer rather than a tabbed internal control sheet.
- Use the screenshot corpus to drive section hierarchy: top preset/header strip, Part/Layer selector, oscillator/mixer/filter/envelope rhythm, modulation/play-mode area, arpeggiator/effects area, and performance/diagnostic feedback.
- Restyle existing JUCE controls in `PluginEditor.*` to be more coherent, compact, and Sylenth-faithful.
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
- Adding bitmap screenshot backplates instead of implementing the UI in JUCE controls.
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
- Faithful Sylenth visual target: warm bronze/brown panels, blue LCD/preset center, caption bars, metal-like bevels, dark rotary caps, amber/green state lights, dense labels, and an integrated top strip.
- No visible tutorial/explainer text in the app.
- Do not hide important state behind decorative cards.
- Avoid nested cards and oversized marketing-style sections.
- Text must fit in controls at normal and compact plugin sizes.
- Stable dimensions for module panels, knobs, switches, badges, and rows.
- Use source screenshots as visual targets for layout, density, palette, knob rhythm, top strip, panel grouping, and preset/LCD behavior. Implement the result as native JUCE controls bound to real state.

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
