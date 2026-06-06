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

The repo already has a buildable JUCE/CMake AU, VST3, and standalone instrument with a Layer A/B-capable subtractive engine, A1/A2/B1/B2 oscillator-slot rendering, layer enable/mute/solo/level/pan state, APVTS parameter registry, preset validation, factory/user/legacy preset browser workflow, a scrollable tabbed editor, TransMod-style modulation route inspection, global MIDI Learn, onboard fixed-order FX, core render validation, and current Ableton smoke evidence.

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
- [x] 2026-06-06 EDT: Executed Phase 1 MIDI controller workflow with a global user CC map, realtime-safe learn capture, message-thread APVTS application, compact Sound-tab panel, and validation.
- [x] 2026-06-06 EDT: Executed current-build Ableton smoke proof after the rename: Release build, CTest, core suite, bundle checks, install/uninstall dry-run, AU validation, VST3 rescan/create, and MIDI playback passed.
- [x] 2026-06-06 EDT: Executed current-build Ableton state restore smoke: AU create/restore, VST3 create/restore, and VST3 post-restore playback with active meters passed.
- [x] 2026-06-06 EDT: Executed current-build Ableton transport/device smoke: corrected standalone-window evidence, proved VST3 transport run/stop with only Ableton running, and left hosted editor open/close unproven.
- [x] 2026-06-06 EDT: Executed current-build Ableton offline bounce smoke: exported WAV/MP3 from the restored VST3 set, measured non-silent non-clipping WAV output, and left bounce-versus-realtime comparison open.
- [x] 2026-06-06 EDT: Executed current-build Ableton AU transport smoke: created the AU from `Audio Units > ParkerX > sylenth-ai`, ran and stopped transport with the hosted AU editor visible, and left explicit hosted editor close/reopen unproven at that point.
- [x] 2026-06-06 EDT: Executed hosted UI lifecycle attempt: proved the hosted AU editor can close while transport is running; a later control pass corrected the failed-reopen conclusion as an automation-targeting miss.
- [x] 2026-06-06 EDT: Executed hosted AU editor reopen control: restored the original resizable editor, rebuilt/reinstalled, passed CTest/bundle checks/auval, and proved hosted AU editor open/close/reopen while transport runs with a precise CoreGraphics click on Ableton's device-header wrench.
- [x] 2026-06-06 EDT: Executed VST3 hosted editor lifecycle proof: dragged the current VST3 into a fresh Ableton set, verified VST3 create logs, and proved hosted VST3 editor open/close/reopen while transport runs.
- [x] 2026-06-06 EDT: Executed VST3 learned-CC capture/persistence proof: routed a temporary CoreMIDI source into Ableton, armed MIDI Learn for `filter.resonance`, captured CC71, persisted the expected controller-map sidecar, and kept AU/controller value-application plus automation replay open.
- [x] 2026-06-06 EDT: Executed VST3 continuous controller value-application proof: seeded the persisted CC71 map, routed a temporary CoreMIDI value source into Ableton, and captured hosted Resonance moving `0.00 -> 1.00 -> 0.00`.
- [x] 2026-06-06 EDT: Executed VST3 controller Forget/stepped proof: seeded CC73 to `filter.mode`, captured Filter Mode moving `L4 -> Notch4 -> L2`, clicked hosted Forget, verified an empty sidecar, and confirmed later CC73 high no longer changed Filter Mode.
- [x] 2026-06-06 EDT: Executed Phase 1 patch recreation suite with five additional lab-authored Factory presets, renderer support for preset-loaded arp/chord state, standalone WAV/JSON proof, and CTest coverage.
- [x] 2026-06-06 EDT: Executed modulation write adapter slice with route-write compilation to existing `transmod.N.*` APVTS fields, processor write/clear APIs, and contract tests for writes, invalid inputs, clamping, and slot clearing.
- [x] 2026-06-06 EDT: Executed patch cost and voice math model slice with a shared estimator, processor diagnostic exposure, header active/max voice display, and contract tests for default, high-cost, zero-level, solo/mute, mono/unison/poly, filter, and FX cases.
- [x] Model-ready UI handoff and first local polish passes exist for preset browser, arp/step/chord, FX rack, and read-only modulation inspection.
- [x] 2026-06-06 EDT: Created a Claude Code visual information architecture handoff to make the model-backed shell look materially closer to the Sylenth screenshot corpus without adding fake controls.
- [ ] Complete or explicitly close deeper UI visual/control polish follow-ups.
- [ ] Complete the remaining Phase 1 Ableton validation matrix against the Sylenth-level build.
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

- `docs/exec-plans/active/2026-06-06-handoff-sylenth-visual-information-architecture.md`
- `docs/exec-plans/active/2026-06-05-handoff-modulation-preset-arp-ui-polish.md`

Completed child ExecPlans:

- `docs/exec-plans/completed/2026-06-06-build-modulation-write-adapter-and-route-schema.md`
- `docs/exec-plans/completed/2026-06-06-build-patch-cost-and-voice-math-model.md`
- `docs/exec-plans/completed/2026-06-06-build-sylenth-patch-recreation-suite.md`
- `docs/exec-plans/completed/2026-06-05-build-sylenth-layer-oscillator-backbone.md`
- `docs/exec-plans/completed/2026-06-05-build-sylenth-layer-b-and-four-osc-rendering.md`
- `docs/exec-plans/completed/2026-06-05-build-arp-step-chord-workflow.md`
- `docs/exec-plans/completed/2026-06-06-build-preset-browser-and-bank-workflow.md`
- `docs/exec-plans/completed/2026-06-05-handoff-modern-sylenth-ui-shell.md`
- `docs/exec-plans/completed/2026-06-05-rename-project-identity-to-sylenth-ai.md`
- `docs/exec-plans/completed/2026-06-06-build-fx-rack-expansion.md`
- `docs/exec-plans/completed/2026-06-06-build-modulation-route-model.md`
- `docs/exec-plans/completed/2026-06-06-build-midi-controller-workflow.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-current-build-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-state-restore-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-transport-device-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-offline-bounce-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-au-transport-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-hosted-ui-lifecycle-attempt.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-hosted-au-editor-reopen-control.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-vst3-hosted-editor-lifecycle.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-vst3-controller-learn-proof.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-vst3-controller-value-proof.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-vst3-controller-forget-stepped-proof.md`

Planned child ExecPlans are listed in `plan-split-recommendation.md`.

## Next Slice

Product-order next Codex slice: complete the remaining Phase 1 Ableton host matrix against the current AU/VST3 build.

Preset browser, arp/step/chord, FX rack, modulation inspection/write adapter, model-backed patch cost, layer/slot rendering, and MIDI controller bridge state now exist. Claude Code can take bounded visual polish passes over those ready surfaces; per-route bypass/remove, per-control MIDI context menus, richer browser metadata editing, expanded modulation destinations, and per-layer filter/envelope/master parity remain later slices.

The remaining non-UI product proof is Ableton AU/VST3 automation record/playback, AU learned CC mapping/value application, Ableton-side current preset recreation, modulation exercise, bounce-versus-realtime comparison, sample-rate and buffer-size changes, all-notes-off, and panic. Scan/load/play/restore, VST3 transport, VST3 offline bounce artifact creation, AU transport, AU/VST3 hosted editor open/close/reopen while transport runs, VST3 controller Learn/value/Forget/stepped proof, and standalone patch recreation are already recorded.

## Risks and Watchpoints

- UI work can drift into surface polish before the state model exists. Do not hand off UI implementation until dependencies in the UI ExecPlans are satisfied.
- Parameter IDs and preset fields can become expensive to change once Ableton automation or factory presets rely on them.
- Adding A/B layers and four oscillator slots multiplies voice count and CPU cost; validation now has deterministic patch-cost estimates, but Ableton CPU calibration and high-voice renders remain release-risk proof.
- Arp/chord generation must be deterministic under host tempo, offline bounce, transport stop, panic, and buffer-size changes.
- AI generation must never allocate or block on the audio thread.
- Reference-sound workflows need explicit local file handling and reversible edit reports before becoming user-facing.

## Outcomes & Retrospective

Pending implementation.

## Validation and Acceptance

Program-level acceptance requires:

- Phase 1 A/B layers, four oscillator slots, arp/step workflow, FX rack, preset browser, MIDI/controller workflow, UI shell, and modulation UX are implemented and validated.
- AU and VST3 load in Ableton, automate, save/reopen, restore state, prove learned controller mapping for both formats, exercise current presets/modulation, export offline bounce artifacts, compare offline bounce against realtime render, handle sample-rate/buffer-size changes, pass all-notes-off/panic checks, and survive hosted UI close/reopen while playing with the Phase 1 build.
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
