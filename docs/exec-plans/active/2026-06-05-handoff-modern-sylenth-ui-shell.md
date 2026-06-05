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
handoff_status: blocked_until_layer_state_exists
---

# Handoff Modern Sylenth UI Shell

This is a Claude Code handoff ExecPlan. It should be handed off only after the layer/oscillator backbone exposes real state for A/B layers and oscillator slots.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Build a modern original UI shell that reaches Sylenth-level speed and density without waiting for every later feature to be complete. The UI should be informed by `Sylenth1Manual.pdf` and `research/sylenth1-screenshots/`, but it must be a production-grade modern interface for Synth's actual state model.

## Progress

- [x] 2026-06-05 EDT: Created this UI handoff ExecPlan.
- [ ] Confirm layer/oscillator state dependencies are complete.
- [ ] Attach or reference before/after screenshots from current Synth UI.
- [ ] Hand off to Claude Code.
- [ ] Review Claude Code output for text fit, layout stability, and binding correctness.
- [ ] Record screenshot/manual QA.

## Surprises & Discoveries

None yet. Record missing parameters, layout constraints, JUCE component limitations, or handoff blockers here.

## Decision Log

Decision: Mark this plan for Claude Code but block implementation until the layer/oscillator backbone exists.
Rationale: The shell needs real A/B and oscillator-slot state; placeholder UI would create brittle assumptions.
Date: 2026-06-05.

## Outcomes & Retrospective

Pending implementation and handoff review.

## Context and Orientation

Do not hand this to Claude Code until:

- `2026-06-05-build-sylenth-layer-oscillator-backbone.md` has delivered real Layer A/Layer B and oscillator-slot contracts.
- Current parameter IDs, new layer IDs, and oscillator-slot IDs are documented.
- The UI can bind to real state instead of placeholder controls.

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

- `2026-06-05-build-sylenth-layer-oscillator-backbone.md`
- `src/plugin/ParameterRegistry.*`
- `src/plugin/PluginEditor.*`
- `docs/modern-sylenth-baseline.md`

Produces UI dependencies for later modulation, preset, arp, and FX polish plans.
