---
title: Rename Project Identity To sylenth-ai
status: completed
created_at: 2026-06-05
completed_at: 2026-06-05
summary: Align the project, build, plugin, preset, script, docs, and remote identity with the renamed sylenth-ai project.
post_build_recap: Renamed host-facing CMake/product identity to sylenth-ai, added SYLENTH_AI_* build options with SYNTH_* compatibility aliases, changed bundle artifacts to SylenthAIPlugin_artefacts and sylenth-ai.component/vst3/app, moved new user preset writes to ~/Music/ParkerX/sylenth-ai/Presets while scanning the old Synth path, updated docs/scripts/host validation, and set origin to ParkerRex/sylenth-ai.
read_when:
  - Validating host scan, bundle names, install scripts, or CMake target names after the project rename.
  - Debugging preset paths after the rename.
  - Checking why C++ namespaces and source filenames still use synth/Synth names.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Rename Project Identity To sylenth-ai

This ExecPlan records the completed project-identity rename.

## Purpose / Big Picture

The repo path and product direction changed from `synth`/`Synth` to `sylenth-ai`. Host-facing identity needed a deliberate update before Ableton validation so AU/VST3 scans, bundle filenames, local install scripts, and docs point at the same product.

## Progress

- [x] Rename CMake project and public targets to `SylenthAI*`.
- [x] Rename host-facing plugin product and bundle output to `sylenth-ai`.
- [x] Add `SYLENTH_AI_*` CMake/cache options while preserving `SYNTH_*` compatibility aliases.
- [x] Update local bundle check/install/uninstall scripts.
- [x] Move new user preset writes to `~/Music/ParkerX/sylenth-ai/Presets`.
- [x] Preserve reads from legacy `~/Music/ParkerX/Synth/Presets`.
- [x] Update docs, host-validation notes, README, and AGENTS.
- [x] Update local `origin` remote to `https://github.com/ParkerRex/sylenth-ai.git`.
- [x] Validate CMake configure and generated target names.

## Decision Log

Decision: Do not rename the C++ namespace, `SynthParameters`, or source filenames in this slice.
Rationale: Those are internal implementation names and broad churn would add risk without improving host identity. The public product/bundle/docs identity is now `sylenth-ai`.
Date: 2026-06-05.

Decision: Accept old `SYNTH_*` CMake options as aliases.
Rationale: Existing scripts and local habits should keep working during the transition, while docs move to `SYLENTH_AI_*`.
Date: 2026-06-05.

Decision: Save new user presets under `~/Music/ParkerX/sylenth-ai/Presets` but scan legacy `~/Music/ParkerX/Synth/Presets`.
Rationale: The rename should not make existing local presets disappear.
Date: 2026-06-05.

Decision: Save host state under `SYLENTH_AI_STATE` but still accept `SYNTH_STATE`.
Rationale: New state chunks should match the product name while old sessions remain recoverable through the migration/default-merge path.
Date: 2026-06-05.

## Outcomes & Retrospective

Completed. Public project identity now aligns around `sylenth-ai`: CMake target family `SylenthAI*`, plugin product `sylenth-ai`, bundle ID `com.parkerx.sylenth-ai`, AU code `SyAI`, bundle files `sylenth-ai.component` and `sylenth-ai.vst3`, user preset write path `~/Music/ParkerX/sylenth-ai/Presets`, and README/host-validation commands using `SylenthAIRender`.

Archive docs and internal C++ names still contain `Synth` where they are historical or implementation-internal.

## Validation

Validation completed:

    cmake -S . -B build -DSYLENTH_AI_ENABLE_TESTS=ON
    cmake --build build --target help | rg "SylenthAI|sylenth-ai|SynthPlugin|SynthRender"
    bash -n scripts/check-plugin-bundles.sh scripts/install-local-plugins.sh scripts/uninstall-local-plugins.sh

Observed generated targets:

- `SylenthAIPlugin_All`
- `SylenthAIPlugin_AU`
- `SylenthAIPlugin_VST3`
- `SylenthAIPlugin_Standalone`
- `SylenthAIRender`
- `SylenthAIContractTest`
- `SylenthAIDspCoreTest`
- `SylenthAISmokeTest`
- `SylenthAIVoiceCoreTest`

The full universal CMake build was not rerun to completion during this slice because the prior rename invalidated the disposable build cache and JUCE universal object compilation was slow in this environment. The generated target list proves the renamed build graph is configured; host validation still needs a release build after the next Ableton pass.
