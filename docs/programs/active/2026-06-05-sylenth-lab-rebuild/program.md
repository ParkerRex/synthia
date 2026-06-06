---
program_id: sylenth-lab-rebuild
title: Sylenth Lab Rebuild
status: active
created_at: 2026-06-05
completed_at: null
summary: Coordinate the Phase 1 Sylenth-style rebuild, Phase 2 AI sound/arp generation, and Phase 3 conversational VST control roadmap.
post_build_recap: null
read_when:
  - Resuming the current sylenth-ai product roadmap.
  - Writing or revising child ExecPlans for the Sylenth rebuild.
  - Deciding whether a slice belongs to engine/state work, Claude Code UI handoff, AI generation, or host validation.
  - Checking the ordering between Phase 1, Phase 2, and Phase 3.
---

# Sylenth Lab Rebuild

This Program is the current initiative for sylenth-ai. It supersedes the older pluck-core Program as the product roadmap while preserving that work as the current engine scaffold.

This Program must be maintained in accordance with `docs/programs/PROGRAMS.md`.

## Purpose / Big Picture

sylenth-ai is a lab-built macOS/Ableton instrument. Phase 1 rebuilds the Sylenth1-style vintage VST experience for modern AU/VST3 hosts. Phase 2 adds AI-assisted sound, patch, chord movement, and arpeggio generation. Phase 3 adds conversational VST control and reference-sound recreation.

The first principle is sequencing: do not polish a fake Sylenth UI before the state model can support it. The Phase 1 engine and preset backbone must expose real A/B layers, four oscillator slots, layer mixer/master behavior, arpeggiator state, preset-browser state, FX rack state, and modulation routes. UI handoff plans can then be passed to Claude Code with precise APIs and screenshots as reference context.

## Program Inputs

Packet artifacts:

- `docs/programs/active/2026-06-05-sylenth-lab-rebuild/research-pass-sylenth-and-ai.md`
- `docs/programs/active/2026-06-05-sylenth-lab-rebuild/normalized-pass-phase-roadmap.md`
- `docs/programs/active/2026-06-05-sylenth-lab-rebuild/converged-decision-packet.md`
- `docs/programs/active/2026-06-05-sylenth-lab-rebuild/dependency-graph.md`
- `docs/programs/active/2026-06-05-sylenth-lab-rebuild/plan-split-recommendation.md`
- `docs/programs/active/2026-06-05-sylenth-lab-rebuild/cross-repo-review.md`
- `docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md`
- `docs/programs/active/2026-06-05-sylenth-lab-rebuild/current-planning-brief.txt`

Project truth surfaces:

- `SPEC.md`
- `CONTEXT.md`
- `docs/modern-sylenth-baseline.md`
- `docs/research/source-map.md`
- `docs/ARCHITECTURE.md`
- `docs/VALIDATION.md`
- `docs/PRESET_SCHEMA.md`
- `Sylenth1Manual.pdf`
- `research/sylenth1-screenshots/SOURCE_INDEX.md`

## Current State

The repo already has a buildable JUCE/CMake AU, VST3, and standalone instrument with a single-core subtractive pluck engine, APVTS parameter registry, preset validation, factory/user preset load-save-duplicate workflow, a scrollable editor, TransMod-style modulation, onboard FX, core render validation, and early Ableton smoke evidence.

Commit `075150d` created the current Sylenth lab roadmap, imported `Sylenth1Manual.pdf`, `Serum_Manual.pdf`, and the 25-image Sylenth screenshot corpus, and replaced the stale legacy doc references.

The current Program is not release hardening. It is a product expansion Program. The old pluck core is useful scaffolding but not the Phase 1 destination.

## Progress

- [x] 2026-06-05 EDT: Committed the Sylenth lab roadmap and local evidence corpus in `075150d`.
- [x] 2026-06-05 EDT: Created this Program packet for the current Phase 1/2/3 product roadmap.
- [x] 2026-06-05 EDT: Executed the first Phase 1 engine/state backbone: A/B layer state and A1/A2/B1/B2 oscillator-slot state, with Layer A mapped to the current sound path and Layer B disabled by default.
- [x] 2026-06-05 EDT: Completed the Claude Code modern UI shell handoff with fixed header, Layer A/B selector, Sound/Mod/FX workspace, render-boundary labels, screenshots, and passing Debug/CTest validation.
- [x] 2026-06-05 EDT: Executed Phase 1 layer rendering: Layer B, four oscillator slots, layer mute/solo/level/pan, and compatibility-preserving presets.
- [x] 2026-06-05 EDT: Renamed the host-facing project identity to sylenth-ai across CMake targets, bundle IDs, local install scripts, preset paths, docs, and Git remote.
- [x] 2026-06-06 EDT: Executed Phase 1 arp/step/chord workflow with parameter-backed state, fixed-array generated-note scheduling, direct chord expansion, minimal Arp/Chord editor controls, and focused DSP validation.
- [x] 2026-06-06 EDT: Executed Phase 1 preset browser/bank workflow with scan summaries, bank/category/tag metadata, favorites, search/filter state, and validation hooks.
- [x] 2026-06-06 EDT: Executed bounded arp/step/chord UI exposure with real APVTS bindings and opened the stacked UI PR.
- [x] 2026-06-06 EDT: Executed Phase 1 FX rack expansion with fixed-order rack state, distortion/phaser/EQ/compressor DSP, editor bindings, adversarial fixes, and validation.
- [x] 2026-06-06 EDT: Executed Phase 1 modulation route model and destination catalog over the existing TransMod engine, with adversarial fixes and validation.
- [ ] Hand off UI information architecture and visual polish plans to Claude Code after state contracts are ready.
- [ ] Execute Phase 1 Ableton validation against the Sylenth-level build.
- [ ] Execute Phase 2 AI sound and arpeggio generation.
- [ ] Execute Phase 3 conversational VST and reference-sound workflows.

## Decision Log

- 2026-06-05: Phase 1 is a Sylenth-level rebuild, not a generic pluck synth and not a UI-only skin.
- 2026-06-05: Engine/state contracts come before UI polish. UI handoff plans must state dependencies and are denoted for Claude Code.
- 2026-06-05: Keep generated AI output editable. AI workflows must produce normal preset, parameter, modulation, arp, and FX state rather than opaque audio-only results.
- 2026-06-05: Keep all AI and reference-analysis orchestration outside the realtime audio thread.
- 2026-06-05: Use `docs/modern-sylenth-baseline.md`, `Sylenth1Manual.pdf`, and the screenshot source index as the Phase 1 decision packet.

## Slice Ledger

Active child ExecPlans:

- `docs/exec-plans/active/2026-06-05-handoff-modulation-preset-arp-ui-polish.md`

Completed child ExecPlans:

- `docs/exec-plans/completed/2026-06-05-build-sylenth-layer-oscillator-backbone.md`
- `docs/exec-plans/completed/2026-06-05-build-sylenth-layer-b-and-four-osc-rendering.md`
- `docs/exec-plans/completed/2026-06-05-build-arp-step-chord-workflow.md`
- `docs/exec-plans/completed/2026-06-06-build-preset-browser-and-bank-workflow.md`
- `docs/exec-plans/completed/2026-06-05-handoff-modern-sylenth-ui-shell.md`
- `docs/exec-plans/completed/2026-06-05-rename-project-identity-to-sylenth-ai.md`
- `docs/exec-plans/completed/2026-06-06-build-fx-rack-expansion.md`
- `docs/exec-plans/completed/2026-06-06-build-modulation-route-model.md`

Planned child ExecPlans are listed in `plan-split-recommendation.md`.

## Next Slice

Product-order next slice: `docs/exec-plans/active/2026-06-05-handoff-modulation-preset-arp-ui-polish.md`.

Preset browser, arp/step/chord, FX rack, and modulation inspection state now exist. Claude Code can take a bounded visual polish pass over those ready surfaces; drag/drop modulation writing and per-route bypass/remove remain a later write-adapter/schema slice.

## Risks and Watchpoints

- UI work can drift into surface polish before the state model exists. Do not hand off UI implementation until dependencies in the UI ExecPlans are satisfied.
- Parameter IDs and preset fields can become expensive to change once Ableton automation or factory presets rely on them.
- Adding A/B layers and four oscillator slots multiplies voice count and CPU cost; validation must include patch-cost estimates and high-voice renders.
- Arp/chord generation must be deterministic under host tempo, offline bounce, transport stop, panic, and buffer-size changes.
- AI generation must never allocate or block on the audio thread.
- Reference-sound workflows need explicit local file handling and reversible edit reports before becoming user-facing.

## Outcomes & Retrospective

Pending implementation.

## Validation and Acceptance

Program-level acceptance requires:

- Phase 1 A/B layers, four oscillator slots, arp/step workflow, FX rack, preset browser, MIDI/controller workflow, UI shell, and modulation UX are implemented and validated.
- AU and VST3 load in Ableton, automate, bounce, save/reopen, and restore state with the Phase 1 build.
- Claude Code UI handoff plans are completed or explicitly closed with screenshots/manual QA notes.
- Phase 2 AI generation can create finite, valid, editable preset/arp/modulation state.
- Phase 3 conversational/reference workflows can produce reversible edits with clear reports.
- All child ExecPlans have retrospectives and this Program moves to completed only after no required slice remains.

## Artifacts and Notes

Program packet created on 2026-06-05 after commit `075150d` established the Sylenth lab roadmap and local evidence corpus.

Claude Code handoff plans are intentionally active but dependency-gated. They should be handed off by Parker only when their frontmatter and body gates are satisfied.

## Interfaces and Dependencies

Primary implementation dependencies:

- JUCE/CMake plugin shell and APVTS parameter registry.
- Current single-core DSP scaffold.
- Preset schema and migration hooks.
- `SylenthAIRender` validation harness.
- Ableton Live for AU/VST3 proof.
- Claude Code for UI implementation handoff where denoted in child ExecPlans.
