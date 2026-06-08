---
program_id: synth-pluck-core-foundation
title: Synth Pluck Core Foundation
status: superseded
created_at: 2026-06-04
completed_at: null
summary: Coordinate the end-to-end build of a lab-authored JUCE/CMake AU/VST3 pluck instrument from spec through release readiness.
post_build_recap: null
read_when:
  - Resuming the Synth build program.
  - Writing or revising child ExecPlans for the Synth plugin.
  - Checking slice order, source-use boundaries, or program-level validation.
  - Closing out the Synth release-readiness initiative.
---

# Synth Pluck Core Foundation

This Program is a historical initiative document for the earlier pluck-core scaffold. It is superseded by `docs/programs/active/2026-06-05-synthia-lab-rebuild/program.md`.

This Program must be maintained in accordance with `docs/programs/PROGRAMS.md`.

## Purpose / Big Picture

Synth should become a modern macOS/Ableton instrument for early-Strobe-v1-like analog plucks without copying third-party code, presets, names, UI, or binary behavior. When this Program is complete, a user should be able to build the repo, load the instrument in Ableton as AU or VST3, play the factory pluck, edit the sound in a lab-authored UI, save/restore presets, and run deterministic render validation.

The initiative matters because the target sound depends on several interacting systems: realtime plugin hosting, stable parameters, per-voice modulation, oscillator stack behavior, nonlinear filter drive, stereo voice spread, validation metrics, UI workflow, and host packaging. A Program keeps those slices aligned so the project does not drift into either a generic SH-101 clone or an unvalidated plugin shell.

## Program Inputs

Packet artifacts:

- `docs/programs/completed/2026-06-04-synth-pluck-core-foundation/research-pass-spec-and-docs.md`
- `docs/programs/completed/2026-06-04-synth-pluck-core-foundation/normalized-pass-build-tracks.md`
- `docs/programs/completed/2026-06-04-synth-pluck-core-foundation/converged-decision-packet.md`
- `docs/programs/completed/2026-06-04-synth-pluck-core-foundation/dependency-graph.md`
- `docs/programs/completed/2026-06-04-synth-pluck-core-foundation/plan-split-recommendation.md`
- `docs/programs/completed/2026-06-04-synth-pluck-core-foundation/cross-repo-review.md`
- `docs/programs/completed/2026-06-04-synth-pluck-core-foundation/planning-brief-1.md`
- `docs/programs/completed/2026-06-04-synth-pluck-core-foundation/current-planning-brief.txt`

Project truth surfaces:

- `SPEC.md`
- `CONTEXT.md`
- `docs/research/source-map.md`
- `docs/ARCHITECTURE.md`
- `docs/VALIDATION.md`
- `docs/PRESET_SCHEMA.md`
- `docs/research/source-map.md`

## Current State

The repository currently contains a JUCE/CMake AU, VST3, and standalone plugin scaffold with APVTS state, lab-authored factory presets, a usable lab-authored editor, user preset save/duplicate workflow, voice allocation, envelopes, LFO, ramp, glide, velocity glide, TransMod-style routing, oscillator stack, nonlinear filter, amp/stereo dry-core rendering, bypassable onboard FX, realtime/offline quality settings, tests, and validation reports.

The current planning brief is `planning-brief-1.md`. The planned product-order next slice is the Ableton host integration and packaging workflow because dry core, editor, preset workflow, FX, quality settings, and early Ableton host smoke now have validation evidence.

Ableton early host smoke passed on 2026-06-05. The remaining host-validation work is full automation and bounce validation with the FX/quality build.

## Progress

- [x] 2026-06-04 EDT: Created `SPEC.md` from the supplied research brief.
- [x] 2026-06-04 EDT: Added project orientation docs: `AGENTS.md`, `README.md`, `CONTEXT.md`, `docs/ARCHITECTURE.md`, `docs/VALIDATION.md`, `docs/PRESET_SCHEMA.md`, and `docs/research/source-map.md`.
- [x] 2026-06-04 EDT: Created local Program and ExecPlan lanes adapted from Worker planning contracts.
- [x] 2026-06-04 EDT: Created this Program packet and first immutable planning brief.
- [x] 2026-06-04 EDT: Wrote all initial child ExecPlans required for the end-to-end build.
- [x] 2026-06-04 EDT: Executed the scaffold foundation slice. The repo now builds AU, VST3, Standalone, `SynthRender`, and `SynthSmokeTest`; CTest and smoke render pass.
- [x] 2026-06-04 EDT: Executed the parameter/state/preset slice. The repo now has 102 registered parameters, APVTS host state, factory preset JSON, preset validation, and contract tests.
- [x] 2026-06-04 EDT: Executed the voice/MIDI/envelope/LFO slice. The repo now has envelope, LFO, voice allocation, engine MIDI handling, and voice-core validation while audio output remains silent.
- [x] 2026-06-04 EDT: Executed the oscillator/mixer slice. The repo now renders tuned polyBLEP saw/pulse, deterministic noise, sub waveforms, stack detune, sync, and oscillator metrics.
- [x] 2026-06-04 EDT: Executed the nonlinear filter slice. The repo now has semitone cutoff mapping, L2/L4/B2/B4/H2/H4 nonlinear filter modes, drive/resonance compensation, oversampling-factor processing, and filter metrics.
- [x] 2026-06-04 EDT: Executed the full modulation slice. The repo now has ramp, glide, velocity glide, direct keytrack/LFO/envelope routes, eight TransMod-style slots with scalers and physical destination depths, performance MIDI sources, preset slot validation, and `SynthRender --modulation-test`.
- [x] 2026-06-04 EDT: Executed the amp/stereo/factory pluck slice. The repo now renders `Pluck Core 01` dry to WAV/report with amp drive, pan spread, analog variation, and macro influence.
- [x] 2026-06-04 EDT: Executed the validation harness slice. The repo now has `SynthRender --suite core`, core-suite CTest coverage, per-fixture JSON reports, WAV artifacts, LFO ablation, deterministic repeat/tolerance comparison, and documented metrics.
- [x] 2026-06-05 EDT: Executed the UI/preset workflow slice. The repo now has a registry-bound lab-authored editor, factory/user preset load-save-duplicate workflow, TransMod slot editing, diagnostics, and standalone UI smoke evidence.
- [x] 2026-06-05 EDT: Executed the FX/quality slice. The repo now has bypassable saturation, tempo-synced delay, simple reverb, chorus, realtime/offline quality settings, FX UI controls, dry/wet render reports, and FX-focused CTest coverage.
- [ ] Execute the Ableton/packaging slice.
- [ ] Execute release hardening and move this Program to completed.

## Decision Log

- 2026-06-04: Use a Program because the build spans many validated slices and needs durable initiative memory above individual ExecPlans.
- 2026-06-04: Keep `SPEC.md` as product truth and planning docs as execution coordination. This prevents child plans from becoming competing specs.
- 2026-06-04: Start with JUCE/CMake AU, VST3, and standalone targets. This matches the current spec and gives both host delivery and deterministic validation.
- 2026-06-04: Split DSP work by dependency: voice/modulators, oscillator, filter, full modulation, amp/stereo. This keeps each sonic behavior testable.
- 2026-06-04: Put FX after dry-core validation. FX can improve production usability but must not hide a weak synth core.
- 2026-06-04: Treat Ableton proof as a planned host integration slice rather than a final afterthought.

## Slice Ledger

Completed child ExecPlans:

- `docs/exec-plans/completed/2026-06-04-scaffold-juce-cmake-plugin-foundation.md`
- `docs/exec-plans/completed/2026-06-04-build-parameter-state-and-preset-contract.md`
- `docs/exec-plans/completed/2026-06-04-build-voice-midi-envelope-lfo-core.md`
- `docs/exec-plans/completed/2026-06-04-build-oscillator-stack-and-mixer.md`
- `docs/exec-plans/completed/2026-06-04-build-nonlinear-filter-drive-and-oversampling.md`
- `docs/exec-plans/completed/2026-06-04-build-amp-stereo-analog-and-factory-pluck.md`
- `docs/exec-plans/completed/2026-06-04-build-modulation-matrix-ramp-and-glide.md`
- `docs/exec-plans/completed/2026-06-04-build-render-validation-harness-and-metrics.md`
- `docs/exec-plans/completed/2026-06-04-build-editor-ui-and-preset-workflow.md`
- `docs/exec-plans/completed/2026-06-04-build-onboard-fx-and-quality-modes.md`

Active child ExecPlans:

- `docs/exec-plans/active/2026-06-04-build-ableton-host-integration-and-packaging.md`
- `docs/exec-plans/active/2026-06-04-build-release-hardening-and-documentation-closeout.md`

## Next Slice

Planned product-order next slice: `docs/exec-plans/active/2026-06-04-build-ableton-host-integration-and-packaging.md`.

It is next because the dry core, editor, preset workflow, onboard FX, and quality modes now have command-line and standalone proof, while full Ableton automation/bounce validation still needs to be run against the current plugin build.

Host-validation follow-up: run full automation and bounce checks in `docs/exec-plans/active/2026-06-04-build-ableton-host-integration-and-packaging.md`.

## Risks and Watchpoints

- JUCE license and final product identity are still open questions and must be resolved before public distribution.
- Plugin metadata must be chosen carefully because AU plugin codes and bundle identifiers are expensive to change later.
- DSP slices can accidentally pass tests while missing the target sound if per-voice modulation and filter drive are delayed too long.
- UI work must not copy historical panels or third-party trade dress.
- Ableton validation may require manual local steps if automation is unavailable.
- The repo has no git history or CI yet, so early plans must establish validation conventions before relying on them.

## Outcomes & Retrospective

The Program is active. Core scaffold, parameter/preset state, voice/MIDI/envelope/LFO, oscillator/mixer, nonlinear filter, amp/stereo/factory pluck, full modulation, render validation harness, UI/preset workflow, and FX/quality slices have landed.

Superseded on 2026-06-05 by the Sylenth Lab Rebuild Program. Remaining Ableton/release-hardening ideas should be reintroduced under that Program when Phase 1 reaches host-validation readiness.

The retrospective should be filled only after all required child ExecPlans are complete and the release-readiness slice has produced final validation evidence.

## Validation and Acceptance

Program-level acceptance requires:

- AU, VST3, and standalone builds exist.
- The standalone target can render deterministic fixtures.
- The dry-core factory pluck passes render validation.
- AU and VST3 load in Ableton and restore state.
- The UI exposes oscillator, filter, envelope, LFO, modulation, amp/stereo, FX, macros, presets, and diagnostics.
- Factory presets are lab-authored.
- Packaging, signing/notarization prep, and release docs exist.
- All child ExecPlans have completed retrospectives and are moved to `docs/exec-plans/completed/`.
- This Program has `completed_at`, `post_build_recap`, and a final retrospective before moving to `docs/programs/completed/`.

## Artifacts and Notes

Planning packet created on 2026-06-04 from the current documentation-first workspace.

The Program intentionally does not choose final product name, bundle identifier, macOS minimum, or JUCE license. Those decisions are called out in child plans because they require implementation-time confirmation before public release.

## Interfaces and Dependencies

Program dependencies:

- `SPEC.md` for product requirements.
- `SPEC.md` and `docs/research/source-map.md` for evidence and naming safety.
- `docs/ARCHITECTURE.md` for source layout and realtime boundaries.
- `docs/VALIDATION.md` for test and host proof expectations.
- `docs/PRESET_SCHEMA.md` for preset shape.
- JUCE/CMake for the expected implementation path.
- Ableton Live for primary host validation.
