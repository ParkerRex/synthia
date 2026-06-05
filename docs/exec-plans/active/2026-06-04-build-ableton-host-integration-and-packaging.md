---
title: Build Ableton Host Integration And Packaging
status: active
created_at: 2026-06-04
completed_at: null
summary: Prove AU/VST3 loading in Ableton, host state restore, universal binary output, install layout, and packaging/signing preparation.
post_build_recap: null
read_when:
  - Validating Synth in Ableton.
  - Changing install, package, signing, or plugin metadata behavior.
  - Preparing distributable macOS builds.
program_id: synth-clean-room-pluck-instrument
planning_brief: docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/planning-brief-1.md
---

# Build Ableton Host Integration And Packaging

This document is a living execution plan for host validation and macOS packaging.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Synth is not useful until it loads and restores correctly in Ableton. After this slice, the project should produce AU and VST3 bundles, verify universal architecture, copy/install them into test plugin folders, document Ableton smoke steps, prepare signing/notarization flow, and prove host state restore.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [x] 2026-06-05 EDT: Added local bundle-check/install scripts and an Ableton smoke-note template for handoff validation.
- [x] 2026-06-05 EDT: Verified release Standalone, AU, and VST3 bundles under `build-release`; all built as universal `x86_64 arm64` binaries with `com.parkerx.synth` metadata.
- [x] 2026-06-05 EDT: Updated local bundle-check/install scripts to resolve CMake config artifact directories such as `SynthPlugin_artefacts/Release`.
- [x] Verify AU/VST3 bundle outputs and metadata.
- [x] Add local development install scripts.
- [x] Add architecture/signing checks.
- [x] Run early Ableton AU and VST3 scan/load/play/reopen smoke validation.
- [ ] Run full Ableton automation and bounce validation after UI/preset workflow is in place.
- [ ] Document install, uninstall, and host troubleshooting.

## Surprises & Discoveries

2026-06-05: Ableton is available on the next development laptop, so this slice can begin as an early host-smoke pass before the final UI/FX slices. Full release acceptance still waits for UI, FX/quality, packaging, and release hardening.

2026-06-05: A `build-release` single-config CMake build writes plugin bundles under `build-release/SynthPlugin_artefacts/Release/`, while the first helper script versions assumed bundles lived directly under `SynthPlugin_artefacts/`. The scripts now resolve root and config-scoped layouts before checking or installing.

2026-06-05: Early Ableton Live 11 Suite smoke passed on macOS 26.5: VST3 appeared under `Plug-Ins > VST3 > ParkerX > Synth`, AU appeared after enabling Audio Units, both loaded on MIDI tracks, both produced audible output from MIDI notes, and both loaded again after Ableton quit/reopen of the saved set. Automation, bounce, preset recreation, and full behavior exercise remain for the UI/preset and later host-validation passes.

Record Ableton scan, plugin cache, code signing, architecture, and state-restore issues here.

## Decision Log

Decision: Ableton AU and VST3 proof is required before release hardening.
Rationale: The product target is Ableton/macOS, and host restore bugs are expensive to discover late.
Date: 2026-06-04.

## Outcomes & Retrospective

Pending implementation.

## Context and Orientation

Full host-release acceptance depends on a working plugin, editor, presets, validation harness, and FX/quality state. An early Ableton smoke pass can start now because AU/VST3 bundles, dry-core DSP, factory presets, and standalone validation exist.

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

    cd /Users/bazelrex/Developer/synth

Build release-like artifacts:

    cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSYNTH_ENABLE_TESTS=ON
    cmake --build build-release --config Release

Run command checks:

    ctest --test-dir build-release --output-on-failure
    ./build-release/SynthRender --suite core --output-dir build-release/reports/core
    scripts/check-plugin-bundles.sh build-release

Then perform the Ableton manual smoke checklist in `docs/host-validation/ableton-smoke.md`.

## Validation and Acceptance

Acceptance requires AU and VST3 bundles produced, architecture checks passing, plugin metadata report, local install script tested, Ableton AU and VST3 smoke validation recorded, host state restore proven, and uninstall/troubleshooting docs updated.

### Test Commands

    cd /Users/bazelrex/Developer/synth
    cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSYNTH_ENABLE_TESTS=ON
    cmake --build build-release --config Release
    ctest --test-dir build-release --output-on-failure
    ./build-release/SynthRender --suite core --output-dir build-release/reports/core
    scripts/check-plugin-bundles.sh build-release
    scripts/install-local-plugins.sh build-release

Manual verification: run the Ableton AU and VST3 smoke checklist and record results in `docs/host-validation/ableton-smoke.md` or a dated sibling note under `docs/host-validation/`.

## Idempotence and Recovery

Install scripts must support reinstalling the same build. If Ableton scan caches a broken plugin, document plugin cache reset steps instead of deleting unrelated user data. Signing/notarization failures should not block local unsigned development builds.

## Artifacts and Notes

Expected proof artifacts:

- plugin bundle check report
- Ableton smoke notes
- release build core validation reports
- install/uninstall docs

## Interfaces and Dependencies

This slice owns package scripts, install paths, plugin metadata checks, Ableton smoke validation, and signing/notarization preparation for release hardening.
