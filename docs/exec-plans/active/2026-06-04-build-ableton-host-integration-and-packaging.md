---
title: Build Ableton Host Integration And Packaging
status: superseded
created_at: 2026-06-04
completed_at: null
summary: Prove AU/VST3 loading in Ableton, host state restore, universal binary output, install layout, and packaging/signing preparation.
post_build_recap: null
read_when:
  - Validating sylenth-ai in Ableton.
  - Changing install, package, signing, or plugin metadata behavior.
  - Preparing distributable macOS builds.
program_id: synth-pluck-core-foundation
planning_brief: docs/programs/completed/2026-06-04-synth-pluck-core-foundation/planning-brief-1.md
---

# Build Ableton Host Integration And Packaging

This document is a living execution plan for host validation and macOS packaging.

This plan is superseded by the Sylenth Lab Rebuild Program. Reuse host-validation details when the Phase 1 Sylenth build reaches Ableton validation readiness.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

sylenth-ai is not useful until it loads and restores correctly in Ableton. After this slice, the project should produce AU and VST3 bundles, verify universal architecture, copy/install them into test plugin folders, document Ableton smoke steps, prepare signing/notarization flow, and prove host state restore.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [x] 2026-06-05 EDT: Added local bundle-check/install scripts and an Ableton smoke-note template for handoff validation.
- [x] 2026-06-05 EDT: Verified release Standalone, AU, and VST3 bundles under `build-release`; all built as universal `x86_64 arm64` binaries with `com.parkerx.sylenth-ai` metadata.
- [x] 2026-06-05 EDT: Updated local bundle-check/install scripts to resolve CMake config artifact directories such as `SylenthAIPlugin_artefacts/Release`.
- [x] Verify AU/VST3 bundle outputs and metadata.
- [x] Add local development install scripts.
- [x] Add architecture/signing checks.
- [x] Run early Ableton AU and VST3 scan/load/play/reopen smoke validation.
- [x] 2026-06-05 EDT: Added `scripts/uninstall-local-plugins.sh`, local install troubleshooting docs, and default ad-hoc signing of installed AU/VST3 bundles for local host scanning.
- [x] 2026-06-05 EDT: Revalidated Release build, CTest, core suite, bundle checks, install signing, uninstall dry-run, and AU validation for the current FX/quality build.
- [x] 2026-06-06 EDT: Revalidated the current renamed build with `build-release-phase1-ableton`; Ableton rescanned `sylenth-ai.vst3`, created ParkerX `sylenth-ai` v0.1.0, opened the editor, and played MIDI with active voices/meters.
- [ ] Run full Ableton automation and bounce validation against the current UI/preset/FX build.
- [x] Document install, uninstall, and host troubleshooting.

## Surprises & Discoveries

2026-06-05: Ableton is available on the next development laptop, so this slice began as an early host-smoke pass before the final UI/FX slices. Full host validation can now run against the current UI/preset/FX build; release acceptance still waits for packaging and release hardening.

2026-06-05: A `build-release` single-config CMake build writes plugin bundles under `build-release/SylenthAIPlugin_artefacts/Release/`, while the first helper script versions assumed bundles lived directly under `SylenthAIPlugin_artefacts/`. The scripts now resolve root and config-scoped layouts before checking or installing.

2026-06-05: Early Ableton Live 11 Suite smoke passed on macOS 26.5: VST3 appeared under `Plug-Ins > VST3 > ParkerX > sylenth-ai`, AU appeared after enabling Audio Units, both loaded on MIDI tracks, both produced audible output from MIDI notes, and both loaded again after Ableton quit/reopen of the saved set. Automation, bounce, preset recreation, and full behavior exercise remain for the UI/preset and later host-validation passes.

2026-06-05: The install script now ad-hoc signs the installed per-user AU and VST3 bundles by default. This is local-development signing only and does not replace distribution signing, hardened runtime, notarization, or clean-machine verification.

2026-06-05: `auval -v aumu SyAI PkRx` passed against the installed AU after the FX/quality build. `auval` emitted a non-fatal warning for `Delay Feedback` maximum-value retention but ended with `AU VALIDATION SUCCEEDED`.

2026-06-06: The current renamed VST3 required a normal Ableton Preferences > Plug-Ins > Rescan before Live stopped showing the old `Synth` browser label from its stale scan state. After rescan, `PluginScanner.txt` found `sylenth-ai.vst3`; Ableton created `sylenth-ai`, reported 2427 parameters, and playback showed 4 editor voices plus active track meters.

Record Ableton scan, plugin cache, code signing, architecture, and state-restore issues here.

## Decision Log

Decision: Ableton AU and VST3 proof is required before release hardening.
Rationale: The product target is Ableton/macOS, and host restore bugs are expensive to discover late.
Date: 2026-06-04.

## Outcomes & Retrospective

Pending implementation.

## Context and Orientation

Full host-release acceptance depends on a working plugin, editor, presets, validation harness, and FX/quality state. The next validation pass should exercise the current AU/VST3 bundles with UI, preset restore, automation, wet/dry behavior, and bounce checks.

Ableton docs identify macOS plugin folders under `/Library/Audio/Plug-Ins/Components/` and `/Library/Audio/Plug-Ins/VST3/`, with per-user equivalents also available.

### In Scope

This plan may add package scripts, local install scripts, plugin metadata checks, architecture checks, signing/notarization preparation, Ableton smoke checklist, and host validation docs.

### Out Of Scope

This plan does not add new DSP features, redesign the UI, implement copy protection, publish a release, or guarantee automated Ableton testing if local automation is unavailable.

## Plan of Work

Confirm plugin metadata, bundle identifiers, manufacturer code, plugin code, category, MIDI input, stereo output, and tail length. Use `scripts/install-local-plugins.sh` to copy AU and VST3 bundles into per-user plugin folders for testing. Use `scripts/check-plugin-bundles.sh` for local bundle existence, executable, architecture, signing, and Info.plist checks.

Prepare signing and notarization commands but do not require production credentials unless available. Document local development signing separately from distribution signing.

Run Ableton smoke checks for AU and VST3: scan, load on MIDI track, play MIDI, tweak parameters, automate a control, save a set, close, reopen, confirm state restore, and export a short bounce.

## Milestones

Milestone 1 verifies bundle outputs and metadata.

Milestone 2 adds install/package scripts and architecture checks.

Milestone 3 proves Ableton AU and VST3 loading and state restore.

Milestone 4 documents packaging and troubleshooting.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai

Build release-like artifacts:

    cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSYLENTH_AI_ENABLE_TESTS=ON
    cmake --build build-release --config Release

Run command checks:

    ctest --test-dir build-release --output-on-failure
    ./build-release/SylenthAIRender --suite core --output-dir build-release/reports/core
    scripts/check-plugin-bundles.sh build-release
    scripts/install-local-plugins.sh build-release
    scripts/uninstall-local-plugins.sh --dry-run
    auval -v aumu SyAI PkRx

Then perform the Ableton manual smoke checklist in `docs/host-validation/ableton-smoke.md`.

## Validation and Acceptance

Acceptance requires AU and VST3 bundles produced, architecture checks passing, plugin metadata report, local install script tested, Ableton AU and VST3 smoke validation recorded, host state restore proven, and uninstall/troubleshooting docs updated.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSYLENTH_AI_ENABLE_TESTS=ON
    cmake --build build-release --config Release
    ctest --test-dir build-release --output-on-failure
    ./build-release/SylenthAIRender --suite core --output-dir build-release/reports/core
    scripts/check-plugin-bundles.sh build-release
    scripts/install-local-plugins.sh build-release
    scripts/uninstall-local-plugins.sh --dry-run
    auval -v aumu SyAI PkRx

Manual verification: run the Ableton AU and VST3 smoke checklist and record results in `docs/host-validation/ableton-smoke.md` or a dated sibling note under `docs/host-validation/`.

## Idempotence and Recovery

Install scripts must support reinstalling the same build. If Ableton scan caches a broken plugin, document plugin cache reset steps instead of deleting unrelated user data. Signing/notarization failures should not block local unsigned development builds.

## Artifacts and Notes

Expected proof artifacts:

- plugin bundle check report
- Ableton smoke notes
- release build core validation reports
- install/uninstall docs: `docs/host-validation/local-install-troubleshooting.md`

## Interfaces and Dependencies

This slice owns package scripts, install paths, plugin metadata checks, Ableton smoke validation, and signing/notarization preparation for release hardening.
