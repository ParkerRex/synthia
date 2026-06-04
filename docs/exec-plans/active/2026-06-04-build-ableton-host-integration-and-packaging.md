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
- [ ] Verify AU/VST3 bundle outputs and metadata.
- [ ] Add packaging/install scripts.
- [ ] Add architecture/signing checks.
- [ ] Run Ableton AU and VST3 smoke validation.
- [ ] Document install, uninstall, and host troubleshooting.

## Surprises & Discoveries

None yet. Record Ableton scan, plugin cache, code signing, architecture, and state-restore issues here.

## Decision Log

Decision: Ableton AU and VST3 proof is required before release hardening.
Rationale: The product target is Ableton/macOS, and host restore bugs are expensive to discover late.
Date: 2026-06-04.

## Outcomes & Retrospective

Pending implementation.

## Context and Orientation

This slice depends on a working plugin, editor, presets, validation harness, and FX/quality state. Ableton docs identify macOS plugin folders under `/Library/Audio/Plug-Ins/Components/` and `/Library/Audio/Plug-Ins/VST3/`, with per-user equivalents also available.

### In Scope

This plan may add package scripts, local install scripts, plugin metadata checks, architecture checks, signing/notarization preparation, Ableton smoke checklist, and host validation docs.

### Out Of Scope

This plan does not add new DSP features, redesign the UI, implement copy protection, publish a release, or guarantee automated Ableton testing if local automation is unavailable.

## Plan of Work

Confirm plugin metadata, bundle identifiers, manufacturer code, plugin code, category, MIDI input, stereo output, and tail length. Add scripts to copy AU and VST3 bundles into per-user plugin folders for testing. Add architecture checks with `lipo` or equivalent.

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

Then perform the Ableton manual smoke checklist documented by this slice.

## Validation and Acceptance

Acceptance requires AU and VST3 bundles produced, architecture checks passing, plugin metadata report, local install script tested, Ableton AU and VST3 smoke validation recorded, host state restore proven, and uninstall/troubleshooting docs updated.

### Test Commands

    cd /Users/bazelrex/Developer/synth
    cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSYNTH_ENABLE_TESTS=ON
    cmake --build build-release --config Release
    ctest --test-dir build-release --output-on-failure
    ./build-release/SynthRender --suite core --output-dir build-release/reports/core
    scripts/check-plugin-bundles.sh build-release

Manual verification: run the Ableton AU and VST3 smoke checklist and record results in `docs/host-validation/ableton-smoke.md` or the path this slice creates.

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

