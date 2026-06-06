---
title: Validate Ableton Current Build Smoke
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Revalidate the current renamed sylenth-ai Release build, install AU/VST3 bundles, prove AU validation, and prove Ableton VST3 scan/load/play with the renamed bundle.
post_build_recap: Fresh Release build, CTest, core suite, bundle checks, local install, uninstall dry-run, and AU validation passed. Ableton Live 11 rescanned the renamed VST3, created ParkerX sylenth-ai v0.1.0, and played an existing MIDI clip with active Ableton meters. At this slice's completion, full AU/VST3 automation, controller proof, hosted editor open/close, state restore, bounce, sample-rate, and buffer-size validation remained open; state restore was later covered by `2026-06-06-validate-ableton-state-restore-smoke.md`.
read_when:
  - Reviewing Ableton smoke proof after the sylenth-ai rename.
  - Continuing full Phase 1 host validation.
  - Debugging Ableton plugin cache or stale old Synth labels.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Validate Ableton Current Build Smoke

This ExecPlan records the bounded Ableton proof completed after the project identity changed to sylenth-ai and after the Phase 1 UI/state slices landed.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The repo needed current host evidence for the renamed AU/VST3 build, not stale proof from the pre-rename `Synth` bundle. This slice proves that the current Release build still packages, installs, validates as AU, scans as VST3 in Ableton, instantiates under the `sylenth-ai` name, and plays MIDI.

It intentionally does not claim the full Phase 1 Ableton matrix. At this slice's completion, automation, learned CC mapping, hosted UI open/close, save/reopen state restore, bounce, sample-rate, buffer-size, and panic/all-notes-off checks remained in the host-validation backlog. The follow-on restore slice later resolved current-build AU creation and AU/VST3 save/reopen restore.

## Progress

- [x] Built `build-release-phase1-ableton` from the current branch with the local JUCE source path.
- [x] Ran CTest and the core render suite.
- [x] Verified Standalone, AU, and VST3 bundle metadata, universal architecture, signing state, and factory resources.
- [x] Installed AU/VST3 bundles into per-user macOS plugin folders with ad-hoc signing.
- [x] Ran uninstall dry-run against the installed local bundles.
- [x] Ran `auval -v aumu SyAI PkRx`.
- [x] Triggered Ableton Preferences > Plug-Ins > Rescan and confirmed the scanner found `sylenth-ai.vst3`.
- [x] Created the renamed VST3 in Ableton and captured playback screenshots.
- [x] Updated `docs/host-validation/ableton-smoke.md`.

## Surprises & Discoveries

Ableton initially showed a stale `Synth` entry because the already-running Live session had not rescanned since the rename. Installed bundle metadata and `moduleinfo.json` were already `sylenth-ai`; a normal Ableton rescan updated the browser and scanner log without deleting cache files.

The Release configure used a fresh `build-release-phase1-ableton` directory. The previous `build-release` cache had stale absolute source paths from before the repo rename.

`auval` passed but repeated the known non-fatal `Delay Feedback` maximum-value retention warning.

The follow-on transport/device pass found that the visible `sylenth-ai` editor window in earlier screenshots was the standalone app, not a hosted Ableton plug-in editor. This smoke pass is therefore VST3 scan/create/play proof, not hosted-editor proof.

## Decision Log

Decision: Treat this as a bounded current-build smoke pass, not full Phase 1 host validation.
Rationale: VST3 scan/create/play was proven against the renamed current build, while current-build AU instantiation, automation, controller proof, hosted UI open/close, state restore, bounce, sample-rate, buffer-size, and panic behavior still needed separate host checks at this point. Current-build AU instantiation and AU/VST3 state restore were later covered by `2026-06-06-validate-ableton-state-restore-smoke.md`.
Date: 2026-06-06.

Decision: Use Ableton's normal rescan path before considering cache resets.
Rationale: The installed bundle metadata was already correct, and deleting host cache files would be broader than necessary.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed as a current-build smoke pass. Host packaging and basic Ableton VST3 scan/load/play behavior are healthy after the rename. The broader host behavior matrix still needed current-build AU instantiation, automation gestures, MIDI controller mapping, hosted UI open/close, state restore, offline bounce, sample-rate/buffer-size changes, and panic/transport behavior at this point. A later restore slice resolved current-build AU instantiation and AU/VST3 state restore.

## Context and Orientation

Read first:

- `docs/host-validation/ableton-smoke.md`
- `docs/host-validation/local-install-troubleshooting.md`
- `docs/exec-plans/active/2026-06-04-build-ableton-host-integration-and-packaging.md`
- `docs/programs/active/2026-06-05-sylenth-lab-rebuild/program.md`

### In Scope

- Current Release build validation.
- Local AU/VST3 install and uninstall dry-run.
- AU command validation.
- Ableton VST3 rescan, create, and playback proof.
- Documentation of remaining host-validation gaps.

### Out Of Scope For This Slice

- Distribution signing or notarization.
- Clean-machine installer validation.
- Current-build AU Ableton instantiation.
- Automation, MIDI controller mapping, hosted UI open/close, state restore, bounce, sample-rate, buffer-size, panic, or all-notes-off proof.

## Plan of Work

Use a fresh Release build directory to avoid stale source paths from the rename. Validate the command-line build and bundles, install the local AU/VST3 bundles, run AU validation, then drive Ableton through a normal plug-in rescan. After the renamed VST3 appears, create it on the existing MIDI track, run transport playback, and record the result in the host-validation notes.

## Milestones

Milestone 1 proves the current Release build and command-line validation.

Milestone 2 proves local bundle metadata, install, uninstall dry-run, and AU validation.

Milestone 3 proves Ableton VST3 rescan/create/play with the renamed bundle.

Milestone 4 records remaining full-host-matrix gaps.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai

Create and build the fresh Release directory:

    cmake -S . -B build-release-phase1-ableton -DCMAKE_BUILD_TYPE=Release -DSYLENTH_AI_ENABLE_TESTS=ON -DSYLENTH_AI_JUCE_PATH=/Users/parkerrex/Developer/sylenth-ai/build/_deps/juce-src
    cmake --build build-release-phase1-ableton --config Release -j2

Run command validation, install, and AU validation:

    ctest --test-dir build-release-phase1-ableton --output-on-failure
    ./build-release-phase1-ableton/SylenthAIRender --suite core --output-dir build-release-phase1-ableton/reports/core
    scripts/check-plugin-bundles.sh build-release-phase1-ableton
    scripts/install-local-plugins.sh build-release-phase1-ableton Release
    scripts/uninstall-local-plugins.sh --dry-run
    auval -v aumu SyAI PkRx

In Ableton Live 11 Suite, open Preferences > Plug-Ins, run Rescan, select `Plug-Ins > VST3 > ParkerX > sylenth-ai`, create the device on a MIDI track, run transport playback, and capture local screenshots under `build/reports/ableton/`.

## Validation and Acceptance

Acceptance for this bounded slice required:

- Current Release build passes CTest and core render suite.
- Bundle check verifies universal Standalone/AU/VST3 artifacts and resources.
- Local install and uninstall dry-run work against the current bundle names.
- AU validation succeeds with `auval -v aumu SyAI PkRx`.
- Ableton rescans the renamed VST3, creates `sylenth-ai`, and plays MIDI with active host meters.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake -S . -B build-release-phase1-ableton -DCMAKE_BUILD_TYPE=Release -DSYLENTH_AI_ENABLE_TESTS=ON -DSYLENTH_AI_JUCE_PATH=/Users/parkerrex/Developer/sylenth-ai/build/_deps/juce-src
    cmake --build build-release-phase1-ableton --config Release -j2
    ctest --test-dir build-release-phase1-ableton --output-on-failure
    ./build-release-phase1-ableton/SylenthAIRender --suite core --output-dir build-release-phase1-ableton/reports/core
    scripts/check-plugin-bundles.sh build-release-phase1-ableton
    scripts/install-local-plugins.sh build-release-phase1-ableton Release
    scripts/uninstall-local-plugins.sh --dry-run
    auval -v aumu SyAI PkRx

Manual Ableton evidence was captured under `build/reports/ableton/`.

### Follow-Up Host Matrix

- Resolved by `2026-06-06-validate-ableton-state-restore-smoke.md`: current-build AU instantiation and AU/VST3 save/reopen restore.
- AU and VST3 automation record/playback.
- AU learned CC mapping proof in Ableton. Follow-on VST3 learned-CC proof later passed.
- Offline bounce comparison.
- Sample-rate and buffer-size changes.
- Transport stop/all-notes-off/panic proof.
- Hosted UI open/close while transport is running.

## Idempotence and Recovery

The fresh build directory can be deleted and recreated. The local install script overwrites the per-user AU/VST3 bundles and ad-hoc signs the copied bundles. If Ableton shows stale names after install, run the normal Preferences > Plug-Ins > Rescan path first and inspect `~/Library/Preferences/Ableton/Live 11.0.12/PluginScanner.txt` before moving or deleting any Ableton cache files.

## Artifacts and Notes

Command-line reports and screenshots are local build artifacts and are not committed:

- `build-release-phase1-ableton/reports/core/`
- `build/reports/ableton/phase1-ableton-after-rescan-cgevent.png`
- `build/reports/ableton/phase1-ableton-vst3-created.png`
- `build/reports/ableton/phase1-ableton-vst3-transport-running.png`
- `build/reports/ableton/phase1-ableton-vst3-transport-stopped.png`

The durable evidence is the recorded command list, Ableton scanner log summary, and explicit remaining-gap list in `docs/host-validation/ableton-smoke.md`.

## Interfaces and Dependencies

This slice depends on the JUCE/CMake Release build, local macOS AU/VST3 plugin folders, `auval`, Ableton Live 11 Suite, and the local `scripts/check-plugin-bundles.sh`, `scripts/install-local-plugins.sh`, and `scripts/uninstall-local-plugins.sh` helpers.
