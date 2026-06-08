---
title: Rename Product Identity To Synthia
status: completed
created_at: 2026-06-08
completed_at: 2026-06-08
summary: Hard-rename the project, build, plugin, tests, runtime state, user paths, scripts, presets, and current docs from sylenth-ai/SylenthAI/SYLENTH_AI to Synthia with no compatibility layer.
post_build_recap: Renamed CMake/product identity to Synthia (`SynthiaPlugin`, `SynthiaRender`, `SYNTHIA_*` options), host bundles to `Synthia.component`/`Synthia.vst3` with bundle ID `com.parkerx.synthia` and AU code `SynA`, runtime state to `SYNTHIA_STATE`, user data to `~/Music/ParkerX/Synthia/`, removed `SYNTH_*`/`SYLENTH_AI_*`/`SYNTH_STATE`/legacy preset-path compatibility, renamed `docs/modern-synthia-baseline.md` and the active program folder, updated canonical docs/scripts, and validated Debug build, CTest 9/9, core render suite, preset validation, and bundle checks.
read_when:
  - Renaming the product identity from sylenth-ai to Synthia.
  - Changing CMake target names, plugin bundle metadata, AU subtype, install paths, or validation command names.
  - Auditing remaining Sylenth references after the Synthia rename.
program_id: synthia-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-synthia-lab-rebuild/planning-brief-1.md
---

# Rename Product Identity To Synthia

This document is a living execution plan for a hard green-field product rename.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The product identity is changing from `synthia` / `Synthia` / `SYNTHIA` to Synthia. This is not a compatibility migration. Treat Synthia as the product's first real public identity and remove old product names from live code, build configuration, tests, scripts, generated command references, preset metadata, runtime state tags, user-data paths, and current docs.

Only literal external-reference mentions of Sylenth1 may remain. The project still uses `Sylenth1Manual.pdf` and the screenshot corpus as Phase 1 reference material, but the software, repository docs, host identity, validation tools, and user-facing copy must say Synthia.

## Progress

- [x] 2026-06-08 EDT: Created this ExecPlan from a multi-agent read-only scan of source, build/package surfaces, docs/reference files, tests, fixtures, and generated artifact names.
- [x] 2026-06-08 EDT: Renamed build, plugin, executable, test, script, runtime, preset, and current-doc identities to Synthia; removed `SYNTH_*`, `SYLENTH_AI_*`, `SYNTH_STATE`, and legacy preset-path compatibility.
- [x] 2026-06-08 EDT: Reconfigured from source and validated that generated artifacts now use Synthia names (`Synthia.component`, `Synthia.vst3`, `Synthia.app`, `SynthiaRender`, `Synthia*Test`).
- [x] 2026-06-08 EDT: Audited remaining tracked `sylenth`/`Sylenth`/`SYLENTH` mentions; live code/docs now keep only approved Sylenth1 reference exceptions plus historical completed-plan filenames.

## Surprises & Discoveries

The scan found no deep C++ class, method, function, or namespace declarations named `Sylenth*`. The live code hits are mostly identity surfaces: CMake targets/options, plugin metadata, APVTS state names, validation executable names, test names, user-data paths, preset metadata, UI strings, and scripts.

Docs contain many more old-name hits than code. Canonical docs and active plans should be renamed as part of this slice. Historical completed Ableton proof that mentions `synthia` is stale under the new identity and must not be treated as current Synthia host proof.

Generated build trees contain thousands of old generated names. Do not edit `build/`, `build-release/`, `build-release-current/`, `build-release-host-matrix/`, or `build-release-phase1-ableton/` directly. Reconfigure/rebuild after source changes and then inspect the new generated output.

## Decision Log

Decision: No compatibility layer.
Rationale: Parker confirmed this is green field. Do not keep `SYLENTH_AI_*` CMake aliases, `SYNTHIA_STATE` restore support, legacy `~/Music/ParkerX/synthia` reads, old AU subtype `SynA`, old bundle ID, or old install paths.
Date: 2026-06-08.

Decision: Keep only true third-party reference mentions.
Rationale: The product direction still uses Sylenth1 as an external reference, but the project identity must not remain Sylenth-derived.
Date: 2026-06-08.

Decision: Regenerate host proof under Synthia before release.
Rationale: Old Ableton logs and proof artifacts are literal evidence for the former `synthia` host identity. They can remain as historical notes only if clearly stale; Synthia needs fresh AU/VST3 scan/load/play, automation, state restore, bounce, and UI lifecycle proof.
Date: 2026-06-08.

## Outcomes & Retrospective

Completed. Public product identity now aligns around Synthia: CMake target family `Synthia*`, plugin product `Synthia`, bundle ID `com.parkerx.synthia`, AU code `SynA`, bundle files `Synthia.component` and `Synthia.vst3`, APVTS/state tag `SYNTHIA_STATE`, user data under `~/Music/ParkerX/Synthia/`, and validation commands using `SynthiaRender` plus `SYNTHIA_*` CMake options.

Fresh Debug validation passed: `git diff --check`, configure/build, CTest 9/9, `SynthiaRender --suite core` (14 reports), factory preset validation, and `scripts/check-plugin-bundles.sh build Debug`.

Remaining tracked Sylenth mentions are limited to approved third-party reference material (`Sylenth1Manual.pdf`, screenshot corpus, Sylenth-rebuild product language in SPEC/CONTEXT), UI comments describing Sylenth1 visual reference targets, and historical completed ExecPlan filenames/evidence. Ableton host proof from the former `sylenth-ai` identity remains stale; fresh Synthia AU/VST3 host proof is still required before release.

## Context and Orientation

The current repo is rooted at `/Users/parkerrex/Developer/synthia` and still exposes the old name across CMake, scripts, docs, preset metadata, host paths, and UI copy. The implementation should work from the existing checkout and avoid editing disposable build outputs.

Use this canonical mapping:

- `synthia` -> `synthia`
- `Synthia` -> `Synthia`
- `SYNTHIA` -> `SYNTHIA`
- `SYNTHIA_STATE` -> `SYNTHIA_STATE`
- `synthia_lab_rebuild` -> `synthia_lab_rebuild`
- `com.parkerx.synthia` -> `com.parkerx.synthia`
- `SynA` -> `SynA`
- `SynthiaRender` -> `SynthiaRender`
- `SynthiaPlugin` -> `SynthiaPlugin`
- `Synthia*Test` -> `Synthia*Test`
- `~/Music/ParkerX/synthia/...` -> `~/Music/ParkerX/Synthia/...`

### In Scope

- `CMakeLists.txt` project name, cache options, compile definitions, target names, plugin target names, generated artifact root, product name, bundle ID, and AU subtype.
- Source strings and identifiers in `src/plugin`, `src/presets`, `src/midi`, and `src/validation`.
- Test expectations, CTest names, temporary paths, and fixture/factory preset metadata.
- Scripts under `scripts/` that configure, validate, install, uninstall, sign, or check bundles.
- Canonical docs, active docs, active index entries, host-validation templates, and current command examples.
- Tracked filenames/folders that carry old product identity, except approved external-reference assets.
- Repository remote/path notes where they are documented in source-controlled docs.

### Out Of Scope

- New DSP, new parameters, new preset schema features, UI redesign, AI features, or new host-validation automation.
- Editing generated build trees directly.
- Preserving old Ableton sessions, old state chunks, old preset sidecars, old MIDI maps, old CMake option aliases, or old installed bundles.
- Rewriting binary/PDF contents. The external manual may keep its literal filename.

## Plan of Work

Start by renaming source-of-truth build identity in `CMakeLists.txt`. Use one consistent Synthia naming family before touching downstream scripts or docs so every command has a single target name.

Rename runtime identity next: APVTS state tags, preset manager version macro, user preset folder, MIDI controller map folder, default preset descriptions, UI title/readout IDs, file chooser copy, and factory preset `program` metadata.

Update tests and scripts after the code identifiers are stable. Rename expected state tags, temp path strings, CTest target names, render executable names, bundle artifact roots, install paths, uninstall paths, ad-hoc signing environment variables, and AU validation commands.

Update docs last, except for any doc whose filename is referenced by code or scripts. Current canonical docs should describe Synthia as the product. Reference docs may still say Sylenth1 only when the sentence is explicitly about the third-party reference instrument, manual, screenshot corpus, legal/reference-use boundary, or historical comparison.

Run a clean source-controlled audit for old names. The final tracked `rg -i sylenth` result should be short and explainable. Every remaining hit must be one of the approved reference exceptions.

## Milestones

Milestone 1 completes the code/build identity rename and configures a fresh build with `SYNTHIA_*` options.

Milestone 2 completes runtime, presets, fixtures, tests, and scripts so `SynthiaRender`, `SynthiaPlugin`, and `Synthia*Test` are the only live names.

Milestone 3 updates canonical docs, active plans, and command references; old host proof is clearly marked stale or queued for regeneration.

Milestone 4 validates build/test/render/bundle checks and proves no unapproved tracked old-name hits remain.

Milestone 5, if Ableton is available, installs Synthia and records fresh Synthia AU/VST3 host proof.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/synthia

Capture the current old-name inventory before editing:

    rg -n --hidden --glob '!build*/**' --glob '!.git/**' -i 'sylenth' .
    find . -path './.git' -prune -o -path './build*' -prune -o -iname '*sylenth*' -print

Rename CMake identity:

    project(Synthia)
    SYNTHIA_ENABLE_TESTS
    SYNTHIA_ENABLE_COPY_AFTER_BUILD
    SYNTHIA_ENABLE_ASAN
    SYNTHIA_ENABLE_UBSAN
    SYNTHIA_JUCE_GIT_TAG
    SYNTHIA_JUCE_PATH
    SYNTHIA_COMMON_SOURCES
    SynthiaPlugin
    SynthiaPlugin_artefacts
    SynthiaRender
    SynthiaSmokeTest
    SynthiaContractTest
    SynthiaVoiceCoreTest
    SynthiaDspCoreTest
    SynthiaRenderCoreSuite
    SynthiaRandomizeRenderTest
    SynthiaModulationRouteRenderTest
    SynthiaOfflineRealtimeCompareTest
    SynthiaPatchRecreationSuite

Rename host identity:

    BUNDLE_ID "com.parkerx.synthia"
    PRODUCT_NAME "Synthia"
    PLUGIN_CODE SynA

Rename runtime and user data:

    SYNTHIA_PROJECT_VERSION
    SYNTHIA_STATE
    ~/Music/ParkerX/Synthia/Presets
    ~/Music/ParkerX/Synthia/PresetFavorites.json
    ~/Music/ParkerX/Synthia/MidiControllerMap.json
    synthia_lab_rebuild
    User preset saved from the Synthia editor.
    Save Synthia preset
    SYNTHIA

Rename scripts and validation commands:

    cmake -S . -B build -DSYNTHIA_ENABLE_TESTS=ON
    ./build/SynthiaRender --suite core --output-dir build/reports/core
    scripts/check-plugin-bundles.sh build
    scripts/install-local-plugins.sh build
    scripts/uninstall-local-plugins.sh --dry-run
    auval -v aumu SynA PkRx

Update canonical docs and active indexes:

    README.md
    AGENTS.md
    SPEC.md
    CONTEXT.md
    docs/index.md
    docs/ARCHITECTURE.md
    docs/VALIDATION.md
    docs/PRESET_SCHEMA.md
    docs/QUALITY.md
    docs/host-validation/local-install-troubleshooting.md
    docs/host-validation/ableton-smoke.md
    docs/exec-plans/active/index.md
    docs/programs/active/index.md
    docs/programs/active/2026-06-05-synthia-lab-rebuild/

Rename tracked product-identity paths where practical:

    docs/modern-synthia-baseline.md -> docs/modern-synthia-baseline.md
    docs/programs/active/2026-06-05-synthia-lab-rebuild/ -> docs/programs/active/2026-06-05-synthia-lab-rebuild/
    docs/exec-plans/*synthia*.md -> matching synthia filenames
    docs/exec-plans/*sylenth-*.md -> matching synthia filenames when the title is product identity, not third-party reference evidence

Leave these reference assets unless a later legal/docs cleanup intentionally moves them:

    Sylenth1Manual.pdf
    research/sylenth1-screenshots/**

After implementation, run the old-name audit again and classify every remaining hit:

    rg -n --hidden --glob '!build*/**' --glob '!.git/**' -i 'sylenth' .
    find . -path './.git' -prune -o -path './build*' -prune -o -iname '*sylenth*' -print

## Validation and Acceptance

Acceptance requires that all live build/code/test/script/product identity surfaces use Synthia, no old compatibility aliases remain, and the repo can configure, build, test, render, and bundle-check under the new names.

Remaining tracked Sylenth hits are allowed only when they are literal third-party reference mentions such as `Sylenth1Manual.pdf`, the screenshot corpus, legal/reference-use text, or source-map evidence about the reference instrument.

Host proof from the old name is stale. Before release, a follow-up host-validation pass must prove Synthia AU and VST3 scan/load/play, state restore, automation, bounce/realtime comparison, sample-rate/buffer changes, panic/all-notes-off, MIDI learn/forget, and hosted editor lifecycle.

### Test Commands

    cd /Users/parkerrex/Developer/synthia
    git diff --check
    cmake -S . -B build -DSYNTHIA_ENABLE_TESTS=ON
    cmake --build build --config Debug
    ctest --test-dir build --output-on-failure
    ./build/SynthiaRender --suite core --output-dir build/reports/core
    ./build/SynthiaRender --validate-presets presets/factory --output build/reports/presets.json
    ./build/SynthiaRender --suite patch-recreation --output-dir build/reports/patch-recreation
    scripts/check-plugin-bundles.sh build
    rg -n --hidden --glob '!build*/**' --glob '!.git/**' -i 'sylenth' .
    find . -path './.git' -prune -o -path './build*' -prune -o -iname '*sylenth*' -print

If Release/Ableton validation is in scope for the implementation pass:

    cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSYNTHIA_ENABLE_TESTS=ON
    cmake --build build-release --config Release
    ctest --test-dir build-release --output-on-failure
    ./build-release/SynthiaRender --suite core --output-dir build-release/reports/core
    scripts/check-plugin-bundles.sh build-release
    scripts/install-local-plugins.sh build-release
    auval -v aumu SynA PkRx

## Idempotence and Recovery

The rename should be repeatable from a clean checkout. Do not depend on existing `build*` directories. If stale generated artifacts confuse validation, create a fresh build directory rather than editing generated files.

If old installed `synthia` AU/VST3 bundles are present, remove them only through explicit uninstall commands or documented manual cleanup. The Synthia installer/uninstaller should target Synthia bundles; it does not need to preserve or migrate old installed bundles.

If the old-name audit reports many generated hits, confirm the command excluded `build*/` and `.git/` before editing anything.

## Artifacts and Notes

Expected implementation artifacts:

- Synthia-named CMake targets and generated bundles.
- Synthia-named validation executable and tests.
- Synthia-named user data paths and preset metadata.
- Updated docs and active plan references.
- An old-name audit with only approved reference exceptions.
- Fresh build reports under `build/reports/`.

Approved reference exceptions:

- `Sylenth1Manual.pdf`
- `research/sylenth1-screenshots/**`
- SPEC/legal/source-map lines that explicitly identify Sylenth1 as the external reference instrument.
- Historical completed proof logs only when clearly marked as old pre-Synthia evidence.

## Interfaces and Dependencies

This slice touches public host identity. AU/VST3 hosts will see Synthia as a new plugin because the bundle ID, product name, subtype, executable name, and generated plugin metadata change.

This slice touches local user-data paths. New presets, favorites, and MIDI maps should write under `~/Music/ParkerX/Synthia/`.

This slice touches validation command names. Any docs, scripts, or external notes that call `SynthiaRender`, `Synthia*Test`, `SYLENTH_AI_*`, or `auval -v aumu SynA PkRx` must move to the Synthia forms.

This slice does not change parameter IDs or DSP behavior. Stable APVTS parameter IDs such as `filter.cutoff_semitones`, `layer.1.osc.1.enabled`, and `transmod.1.source` remain unchanged unless they contain the old product name, which current scans did not find.
