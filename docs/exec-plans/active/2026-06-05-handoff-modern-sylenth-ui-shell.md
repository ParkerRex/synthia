---
title: Handoff Modern Sylenth UI Shell
status: active
created_at: 2026-06-05
completed_at: null
summary: Claude Code handoff plan for the Phase 1 modern Sylenth UI shell, layout system, visual hierarchy, and screenshot/manual QA.
post_build_recap: null
read_when:
  - Preparing UI work for Claude Code.
  - Designing the Phase 1 shell, layout, header, A/B layer surface, oscillator/filter/envelope panels, or visual style.
  - Checking whether UI implementation can start.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Claude Code
handoff_status: ready_for_handoff
---

# Handoff Modern Sylenth UI Shell

This is a Claude Code handoff ExecPlan. It should be handed off only after the layer/oscillator backbone exposes real state for A/B layers and oscillator slots.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Build a modern original UI shell that reaches Sylenth-level speed and density without waiting for every later feature to be complete. The UI should be informed by `Sylenth1Manual.pdf` and `research/sylenth1-screenshots/`, but it must be a production-grade modern interface for Synth's actual state model.

## Progress

- [x] 2026-06-05 EDT: Created this UI handoff ExecPlan.
- [x] 2026-06-05 EDT: Confirmed layer/oscillator state dependencies are complete through the completed backbone ExecPlan.
- [x] 2026-06-05 EDT: Handed off to Claude Code.
- [x] 2026-06-05 EDT: Implemented the Phase 1 modern shell in `src/plugin/PluginEditor.{h,cpp}` only (no DSP/registry/processor changes). Replaced the scroll-heavy section list with a fixed header + persistent Layer A/B selector + `Sound`/`Modulation`/`Effects` tabbed workspace, an original rotary/switch/combo look-and-feel, a grid-packing `Panel`, and a peak/clip output meter.
- [x] 2026-06-05 EDT: Reviewed output for text fit, layout stability, and binding correctness. Build is clean (Debug, no warnings) and `ctest` passes 5/5.
- [x] 2026-06-05 EDT: Recorded screenshot/manual QA at normal (1320x940) and compact-minimum (1080x760) sizes under `docs/host-validation/ui-qa-2026-06-05/`.

## Surprises & Discoveries

- Layer/oscillator state exists as APVTS and preset state, but Layer B and four-slot audio rendering are not implemented yet. The shell may bind to real state, but it must not present Layer B/four-slot rendering as complete sound behavior.
- Render-boundary honesty was resolved in the UI itself rather than with tutorial text: the core `osc.*` panel carries a `LIVE` badge, every `layer.N.osc.M.*` slot panel carries a `STATE` badge, Layer A reads `LIVE - core path`, and Layer B reads `STAGED - not yet rendered`. The header engine tag reads `CORE OSC ENGINE`. Editing staged slot/Layer-B state still serializes (useful for preparing presets) but the UI never implies it is audible.
- `juce::SliderParameterAttachment` overwrites `Slider::textFromValueFunction` in its constructor, so the formatter must be assigned after the attachment is created or value readouts fall back to raw parameter text.
- `juce::String(value, 0)` (zero decimal places) renders sub-unity magnitudes in scientific notation. Symmetric-range parameters whose normalized default round-trips to a tiny denormal (e.g. `osc.fine_cents`, `osc.pitch_semitones`) exposed this as `-2.2e-05`. Fixed by rounding integer-display units (`cents`, `degrees`, `percent`) with `roundToInt` and snapping sub-interval magnitudes (<1e-3) to a true zero for display only.
- Patch cost / "load est." is a UI-thread-derived estimate from the live rendering path (`voice.polyphony` x `voice.unison_count` x `osc.stack_count`, scaled by filter oversampling and enabled FX modules), not a measured CPU figure. The header voice count is the real diagnostic; the slot-derived `layerOscillatorVoiceCost` is intentionally not surfaced as a live number because those slots are staged.
- The default window (1320x940) shows the entire `Sound` surface without scrolling; below that the page content scrolls inside a viewport while the header/layer bar/tabs/footer stay fixed. No horizontal scroll and no clipped labels at the 1080x760 minimum.

## Decision Log

Decision: Mark this plan for Claude Code but block implementation until the layer/oscillator backbone exists.
Rationale: The shell needs real A/B and oscillator-slot state; placeholder UI would create brittle assumptions.
Date: 2026-06-05.

Decision: Mark this handoff ready after the state dependency landed.
Rationale: `layer.*` and `layer.N.osc.M.*` parameters are now registry-backed, preset-serialized, documented, and covered by tests. Remaining rendering limitations must be called out in the handoff prompt.
Date: 2026-06-05.

## Outcomes & Retrospective

Phase 1 modern shell implemented and verified. Scope held to `src/plugin/PluginEditor.{h,cpp}`; no DSP, registry, processor, or preset changes. All editable controls bind to real `ParameterRegistry`/APVTS parameters; no placeholder state was introduced.

Delivered:

- Header: preset prev/next, load, save-as, duplicate, editable name; output peak meter with clip latch; live active-voice count; derived patch-load estimate; panic; and an always-visible SR/block/peak/MIDI/invalid/architecture diagnostics footer.
- Persistent Layer A/B selector bound to `layer.N.enabled/level_db/pan/solo/mute`, with the `LIVE`/`STAGED` render-boundary pill.
- `Sound` tab: oscillator slots A1/A2 (or B1/B2 by selection, bound to `layer.N.osc.M.*`, badged `STATE`), the live core `osc.*` oscillator (badged `LIVE`), filter, amp/mod envelopes, LFO, ramp, voice, amp/stereo, and macros — all visible without scrolling at the default size.
- `Modulation` tab: destination-grouped direct routes (Osc Pitch / Pulse Width / Filter Cutoff) plus the eight TransMod slots.
- `Effects` tab: per-module rack (Saturation / Chorus / Delay / Reverb) plus master bypass and realtime/offline quality.

Validation:

- `cmake --build build --config Debug` clean (no warnings).
- `ctest --test-dir build --output-on-failure` -> 5/5 passed (`SynthContractTest` confirms parameter bindings/defaults are unchanged).
- Standalone visual QA captured at normal and compact-minimum sizes; see `docs/host-validation/ui-qa-2026-06-05/`.

QA notes / manual checks performed:

- Preset header: combo populated from `getPresetList()`; prev/next step and load apply; name field reflects current preset; Save As / Duplicate wired to existing processor APIs.
- Layer switching: A shows `LIVE - core path` and OSC A1/A2; B shows `STAGED - not yet rendered` and OSC B1/B2 (disabled by default, matching real Layer B defaults).
- Oscillator/filter/envelope editing: knobs/combos/toggles bind and display formatted values (`st`, `ct`, `dB`, `Hz`, `ms`, `%`, deg); no scientific-notation or `-0.0` artifacts.
- Panic button calls `requestPanic()`; diagnostics/meter update on a 15 Hz timer.
- No clipped labels, overlaps, or nested-card layouts at normal or compact sizes.

Blocked / not in scope (no work attempted, by design):

- Output meter only verified statically at `-inf` (no MIDI was injected in headless QA); live ballistics depend on audio input and should be confirmed during Ableton host validation.
- Per-slot / Layer B audio rendering, modulation halos/drag-drop, preset browser drawer, arp grid, and FX reorder remain owned by their separate plans.
- No init/randomize/reset header actions were added because the processor exposes no such API yet; only real preset load/save/duplicate are surfaced.

Evidence:

- `docs/host-validation/ui-qa-2026-06-05/ui-sound-page.png` (normal `Sound` surface)
- `docs/host-validation/ui-qa-2026-06-05/ui-modulation.png` (direct mod + TransMod)
- `docs/host-validation/ui-qa-2026-06-05/ui-effects.png` (FX module rack)
- `docs/host-validation/ui-qa-2026-06-05/ui-layer-b-staged.png` (Layer B `STAGED` state)
- `docs/host-validation/ui-qa-2026-06-05/ui-compact-min.png` (1080x760 minimum)

## Context and Orientation

This plan is ready to hand to Claude Code for the shell/state-bound UI. The handoff prompt must state:

- `docs/exec-plans/completed/2026-06-05-build-sylenth-layer-oscillator-backbone.md` delivered real Layer A/Layer B and A1/A2/B1/B2 oscillator-slot contracts.
- Current parameter IDs, new layer IDs, and oscillator-slot IDs are documented in `docs/PRESET_SCHEMA.md` and `docs/ARCHITECTURE.md`.
- The UI can bind to real state instead of placeholder controls.
- Current audio rendering still uses the legacy flat `osc.*` path; Layer B/four-slot rendering is a future engine slice.
- Legacy flat oscillator presets are not migrated into equivalent Layer A slot visuals yet, so the shell must not imply A1/A2/B1/B2 controls are the authoritative current render model.

Primary references:

- `docs/modern-sylenth-baseline.md`
- `Sylenth1Manual.pdf`
- `research/sylenth1-screenshots/SOURCE_INDEX.md`
- `research/sylenth1-screenshots/official-lennardigital-main-ui.jpg`
- `research/sylenth1-screenshots/kvraudio-sylenth1-v302-main-ui.png`
- `research/sylenth1-screenshots/splice-top-panel.png`
- `research/sylenth1-screenshots/splice-oscillator-mix-panel.png`
- `research/sylenth1-screenshots/splice-filter-controls-panel.png`
- `research/sylenth1-screenshots/splice-modulation-play-modes-panel.png`
- `research/sylenth1-screenshots/splice-arpeggiator-effects-panel.png`

Current code references:

- `src/plugin/PluginEditor.*`
- `src/plugin/ParameterRegistry.*`
- `src/plugin/PluginProcessor.*`
- `docs/ARCHITECTURE.md`

### In Scope

- Header with preset name, prev/next, save, init/randomize/reset, dirty state, panic, output meter, active voice count, patch cost, and diagnostics.
- Main A/B layer shell with layer tabs/select, solo/mute, layer cost, and master area.
- Oscillator, filter, envelope, mixer, and macro panels that bind to actual parameters.
- A layout system that supports compact plugin windows and avoids scroll-heavy patch editing.
- Stable dimensions for controls so labels, values, hover states, and dynamic content do not shift layout.
- Original visual style suitable for a modern VST.

### Out Of Scope

- Adding new DSP behavior.
- Inventing placeholder state not present in the parameter registry.
- Modulation halos/source tiles, preset browser drawer, arp grid, or FX rack polish covered by the separate UI handoff plan.
- Any audio-thread work.

## Plan of Work

Prepare a Claude Code handoff prompt from this plan once the dependency gate is clear. The handoff should tell Claude Code to inspect the current editor and parameter registry, use the screenshot corpus for information architecture, implement a modern original shell, and provide screenshot/manual QA evidence.

Design constraints:

- Do not use visible in-app tutorial text to explain how to use the synth.
- Use familiar controls and icons where practical.
- Text must fit on desktop and compact plugin sizes.
- Cards should not be nested; repeated modules may be framed, but page sections should not become floating marketing cards.
- The UI must be dense, clear, and production-focused.
- Use screenshot corpus for workflow density and grouping, not literal copying.

## Milestones

Milestone 1 confirms the layer/oscillator dependency gate.

Milestone 2 hands the plan to Claude Code.

Milestone 3 reviews the returned UI patch for binding correctness and layout stability.

Milestone 4 records screenshot/manual QA and closes the handoff.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/synth

Before handoff, verify that the dependency plan has landed:

    rg -n "layer\\.|osc\\." src/plugin src/dsp src/presets docs/PRESET_SCHEMA.md

After Claude Code returns a patch, run:

    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure

Then open the standalone/plugin UI and capture normal and compact-size screenshots or manual QA notes.

## Validation and Acceptance

Acceptance requires:

- All visible controls bind to real registry/APVTS parameters or documented derived diagnostics.
- Standalone/plugin UI screenshots at normal and compact sizes.
- Manual QA notes for preset header, layer switching, oscillator/filter/envelope editing, panic, meter, and diagnostics.
- No incoherent overlap or clipped labels.
- No UI work touches realtime DSP.

### Test Commands

    cd /Users/parkerrex/Developer/synth
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure

Manual verification: open the standalone target or plugin UI, exercise header/layer/oscillator/filter/envelope controls, and record screenshots/notes in this plan or a linked QA note.

## Idempotence and Recovery

If the handoff patch exposes placeholder controls or hidden state, reject that portion and return to the dependency gate. UI changes should be reversible without changing preset or host-state semantics.

## Artifacts and Notes

Expected handoff deliverables:

- Patch to UI code.
- Screenshot evidence.
- Notes on any missing parameters or bindings that blocked completion.
- Updated docs if control layout or workflow changes user-visible behavior.

## Interfaces and Dependencies

Depends on:

- `docs/exec-plans/completed/2026-06-05-build-sylenth-layer-oscillator-backbone.md`
- `src/plugin/ParameterRegistry.*`
- `src/plugin/PluginEditor.*`
- `docs/modern-sylenth-baseline.md`

Produces UI dependencies for later modulation, preset, arp, and FX polish plans.
