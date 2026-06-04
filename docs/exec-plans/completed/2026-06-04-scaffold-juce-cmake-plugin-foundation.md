---
title: Scaffold JUCE CMake Plugin Foundation
status: complete
created_at: 2026-06-04
completed_at: 2026-06-04
summary: Create the buildable JUCE/CMake foundation with AU, VST3, standalone, validation CLI, source layout, and first smoke tests.
post_build_recap: Created the JUCE/CMake scaffold, AU/VST3/Standalone plugin shell, silent engine, SynthRender smoke CLI, smoke test, and build documentation with green configure/build/test/render proof.
read_when:
  - Starting Synth implementation.
  - Resuming the foundation scaffold.
  - Validating the first buildable plugin shell.
program_id: synth-clean-room-pluck-instrument
planning_brief: docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/planning-brief-1.md
---

# Scaffold JUCE CMake Plugin Foundation

This document is a living execution plan for creating the first buildable Synth project.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The repo currently has product and architecture docs but no buildable code. After this slice, a developer should be able to configure the project with CMake, build AU/VST3/standalone targets, run a minimal validation CLI, and run the first smoke tests. This creates the foundation that every DSP, UI, preset, and host-integration slice depends on.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [x] 2026-06-04 EDT: Confirmed no local JUCE checkout was present and selected CMake `FetchContent` pinned to JUCE `8.0.13`, with `SYNTH_JUCE_PATH` available for local checkout override.
- [x] 2026-06-04 EDT: Added `CMakeLists.txt`, `.gitignore`, source directories, plugin shell, silent `SynthEngine`, `SynthRender`, and `SynthSmokeTest`.
- [x] 2026-06-04 EDT: Fixed configure by enabling the C language because JUCE builds C sources.
- [x] 2026-06-04 EDT: Fixed VST3 build by setting `JUCE_VST3_CAN_REPLACE_VST2=0`, matching the no-VST2 product boundary.
- [x] 2026-06-04 EDT: Ran configure, build, CTest, and `SynthRender --smoke` successfully.

## Surprises & Discoveries

JUCE requires C as well as CXX in the CMake `project()` declaration because JUCE modules include C sources. The first configure failed with missing `CMAKE_C_COMPILE_OBJECT`; adding `LANGUAGES C CXX` fixed it.

JUCE's VST3 wrapper fails compilation unless the VST2 replacement policy is explicit. Because this project does not ship VST2, `JUCE_VST3_CAN_REPLACE_VST2=0` is the correct scaffold choice.

`xcodebuild -version` fails because the active developer directory is Command Line Tools, not full Xcode. Despite that, CMake built Standalone, AU, and VST3 bundles with Apple clang from Command Line Tools. Later host/signing validation may still need full Xcode.

The linker prints `search path '/opt/homebrew/opt/bzip2/lib' not found`. This is a local environment warning and did not block the scaffold build.

## Decision Log

Decision: The foundation slice owns project structure and command shape, not real synthesis behavior.
Rationale: Later DSP plans need stable files, targets, and tests before implementing audio modules.
Date: 2026-06-04.

Decision: Pin default JUCE dependency to tag `8.0.13` using CMake `FetchContent`.
Rationale: No local JUCE checkout exists, and pinning the tag makes the scaffold repeatable while still allowing `SYNTH_JUCE_PATH` override.
Date: 2026-06-04.

Decision: Set `JUCE_VST3_CAN_REPLACE_VST2=0`.
Rationale: The spec excludes VST2, so the VST3 plugin should not advertise VST2 replacement compatibility.
Date: 2026-06-04.

## Outcomes & Retrospective

The foundation slice is complete. The repo now has a buildable JUCE/CMake scaffold with AU, VST3, and standalone plugin targets, a silent realtime-safe engine stub, a minimal clean-room editor, a `SynthRender` validation CLI, and a CTest smoke test.

This slice intentionally produces silence. The next slice should replace the placeholder state path with the real parameter registry, host state, and preset contract.

## Context and Orientation

The repo root is `/Users/bazelrex/Developer/synth`. `SPEC.md` requires AU, VST3, and standalone targets. `docs/ARCHITECTURE.md` defines the planned source layout. `docs/CLEAN_ROOM.md` forbids copied third-party code, presets, UI, and binary behavior.

Create a JUCE/CMake project that can build without relying on Projucer. The implementation may use a git submodule, vendored JUCE checkout, system package, or `FetchContent`, but it must document the chosen dependency path in `README.md`. If the network or JUCE license posture blocks an approach, stop and record the blocker rather than substituting another framework silently.

### In Scope

This plan may create `CMakeLists.txt`, `cmake/`, `src/plugin/`, `src/dsp/`, `src/voice/`, `src/modulation/`, `src/presets/`, `src/validation/`, `tests/`, `fixtures/`, `presets/factory/`, and minimal build/test documentation. It may add a minimal processor, editor, parameter stub, validation CLI, and smoke tests.

### Out Of Scope

This plan does not implement the oscillator, filter, full parameter registry, preset schema, modulation matrix, factory pluck, production UI, FX, signing, or Ableton validation.

## Plan of Work

Start by choosing and documenting the JUCE dependency path. Then create the CMake project with one plugin target named `Synth` and formats `AU`, `VST3`, and `Standalone`. Add a small validation executable named `SynthRender` that can render a short silence buffer or init patch through the same core engine stub. Add directories that match `docs/ARCHITECTURE.md` even if most modules are placeholders.

Implement `src/plugin/PluginProcessor.*` and `src/plugin/PluginEditor.*` with a safe silent processor, MIDI input enabled, stereo output, no audio input, and deterministic plugin metadata. Use temporary internal product identifiers if final naming is unresolved, but mark them in code/docs as pre-release values that must be revisited before public distribution.

Add `tests/smoke/` with the narrowest available C++ test harness. The tests should prove that the engine initializes, processes silence without NaN/infinity, and exposes expected build metadata.

Update `README.md`, `docs/ARCHITECTURE.md`, and `docs/VALIDATION.md` with the actual commands and target names chosen by this slice.

## Milestones

Milestone 1 creates the project skeleton. Success means CMake configures and the expected directories exist.

Milestone 2 builds the plugin shell. Success means AU, VST3, standalone, and `SynthRender` targets compile.

Milestone 3 adds smoke validation. Success means `ctest` passes and `SynthRender` produces a short report or render artifact without invalid samples.

Milestone 4 syncs docs. Success means the repo docs name the actual build commands and no plan or docs path points to a nonexistent target.

## Concrete Steps

Work from the repo root:

    cd /Users/bazelrex/Developer/synth

Inspect available tools:

    cmake --version
    xcodebuild -version

Create the source layout named in `docs/ARCHITECTURE.md`. Add the CMake project and minimal source files. Configure:

    cmake -S . -B build -DSYNTH_ENABLE_TESTS=ON

Build:

    cmake --build build --config Debug

Run tests:

    ctest --test-dir build --output-on-failure

Run the validation CLI:

    ./build/SynthRender --smoke --output build/reports/smoke.json

If the exact binary path differs by generator, update this plan and docs with the real path.

Actual artifact paths from the successful Debug build:

    build/SynthPlugin_artefacts/Standalone/Synth.app
    build/SynthPlugin_artefacts/AU/Synth.component
    build/SynthPlugin_artefacts/VST3/Synth.vst3

## Validation and Acceptance

Acceptance requires a clean configure, successful build, passing smoke tests, and a validation CLI report. The plugin shell must process audio buffers without NaN, infinity, denormals, allocation on the audio path, or host crashes in standalone.

Observed proof on 2026-06-04:

    ctest: 1/1 tests passed
    SynthRender: Smoke render passed: 4800 samples
    smoke.json: invalid_samples 0, peak 0, passed true

### Test Commands

    cd /Users/bazelrex/Developer/synth
    cmake -S . -B build -DSYNTH_ENABLE_TESTS=ON
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthRender --smoke --output build/reports/smoke.json

## Idempotence and Recovery

CMake configure and build should be rerunnable. If configuration breaks due to a bad cache, remove only `build/` and rerun configure. Do not delete source files or docs as cleanup. If JUCE dependency acquisition fails, leave a documented blocker and do not partially vendor unknown code.

## Artifacts and Notes

Expected proof artifacts:

- `build/reports/smoke.json`
- CMake configure transcript
- CTest output
- updated README build commands

## Interfaces and Dependencies

This slice establishes the `Synth` plugin target, `SynthRender` validation target, source directory layout, and test command shape that all later ExecPlans depend on.
