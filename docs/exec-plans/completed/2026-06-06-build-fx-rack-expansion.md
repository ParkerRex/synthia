---
title: Build FX Rack Expansion
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Add the Phase 1 fixed-order FX rack state, lightweight processors, validation, and docs needed before Claude Code FX rack polish.
post_build_recap: Added 14 FX rack parameters, lightweight distortion/phaser/EQ/compressor DSP, editor bindings, regression coverage, docs, visual standalone evidence, and adversarial fixes for AU hints, macro-space delay wetness, phaser bypass cleanup, and parameter-count docs.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
read_when:
  - Reviewing the Phase 1 fixed-order FX rack model.
  - Preparing Claude Code FX rack visual polish.
  - Auditing FX parameter, tail, bypass, or host-state behavior.
---

# Build FX Rack Expansion

This ExecPlan expands the existing practical FX chain into the Phase 1 fixed-order rack model. It must stay realtime-safe and validation-backed.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Phase 1 needs the Sylenth-style master FX workflow to be real before Claude Code polishes the rack UI. The existing chain already has global bypass, saturation, chorus, tempo delay, reverb, quality modes, wet validation, and tail reporting. This slice adds the missing fixed-order rack modules and state: richer distortion mode, phaser, EQ, and compressor, while preserving the current post-mix signal order and dry-core bypass proof.

## Progress

- [x] 2026-06-06 EDT: Created this Program-linked ExecPlan after preset browser and arp/step/chord UI slices.
- [x] 2026-06-06 EDT: Inventoried current FX parameters, processor path, preset/schema docs, and validation tests.
- [x] 2026-06-06 EDT: Added fixed-order rack parameter state and processor snapshot mapping.
- [x] 2026-06-06 EDT: Implemented lightweight realtime-safe phaser, EQ, compressor, and distortion-mode behavior.
- [x] 2026-06-06 EDT: Added focused DSP/contract validation for bypass, finite wet output, tail/cost invariants, and preset serialization.
- [x] 2026-06-06 EDT: Updated durable docs and Claude Code handoff gates.
- [x] 2026-06-06 EDT: Ran adversarial review, fixed all accepted findings, reran validation, and prepared the stacked PR.

## Surprises & Discoveries

- The adversarial pass caught that newly inserted FX parameters needed a higher AU `ParameterID` version hint than the older registry entries. The FX expansion parameters now use hint `3`, and the contract test checks the policy.
- `fxTailLengthSeconds()` already treated `macro.space` as delay wetness, but `FxChain::processDelay()` bypassed delay when raw `fx.delay_mix` was zero. The processor now computes the macro-expanded delay mix before bypass, and a regression test proves the render matches the tail report.
- The phaser has recursive state, so zero-tail behavior requires explicit cleanup when the module or global FX bypass turns off. The phaser now resets on bypass transitions, with a regression test for disable/re-enable silence.

## Decision Log

Decision: Keep a fixed rack order for Phase 1 and defer drag reorder.
Rationale: Reorder changes signal-flow semantics, tail behavior, and host-state migration. A fixed rack unblocks truthful UI polish with much lower validation risk.
Date: 2026-06-06.

## Outcomes & Retrospective

The slice landed a fixed-order rack model in code and docs:

- Rack order is distortion/saturation, phaser, chorus, EQ, delay, reverb, compressor.
- New APVTS-backed state covers `fx.distortion_mode`, `fx.phaser_*`, `fx.eq_*`, and `fx.compressor_*`.
- `PluginProcessor` snapshots the new state, `SynthRender` can parse it from presets, and `PluginEditor` exposes functional placeholder controls.
- `FxChain` keeps defaults bypassed/no-op so legacy presets and dry-core validation stay compatible.
- Focused DSP and contract tests cover disabled-rack dry equivalence, audible finite wet output, non-tail reporting, macro-space delay wetness, phaser bypass cleanup, host default merge, and FX AU version hints.

Validation passed:

- `git diff --check`
- `cmake --build build --target SylenthAIPlugin_Standalone -j2`
- `ctest --test-dir build --output-on-failure`
- `./build/SylenthAIRender --suite core --output-dir build/reports/core-fx-rack`
- `./build/SylenthAIRender --list-parameters --output build/reports/parameters-fx-rack.json`

Native standalone screenshot evidence was captured at `build/reports/ui/fx-rack-standalone.png`; build artifacts remain disposable and are not source-controlled. Ableton host proof was not run in this slice.

## Context and Orientation

Relevant current code:

- `src/dsp/fx/FxChain.*`
- `src/dsp/SynthParameters.h`
- `src/plugin/ParameterRegistry.cpp`
- `src/plugin/PluginProcessor.*`
- `src/plugin/PluginEditor.*`
- `tests/smoke/SynthDspCoreTest.cpp`
- `tests/smoke/SynthContractTest.cpp`
- `src/validation/SynthRender.cpp`

The current chain order is saturation, chorus, delay, reverb. This slice should expand that into a fixed master rack that remains cheap and deterministic:

- distortion/saturation
- phaser
- chorus/flanger
- EQ
- delay
- reverb
- compressor

### In Scope

- Stable APVTS parameters for the added rack modules and distortion mode.
- Realtime-safe processors that allocate only during `prepare`.
- Fixed-order module bypass and mix behavior.
- Tail reporting that remains driven by active time-based modules.
- Validation for dry bypass equivalence, finite wet output, and non-no-op behavior for added modules.
- Durable docs and UI handoff dependency updates.

### Out Of Scope

- Drag reorder.
- Full mastering-grade processors.
- External sidechain input.
- Claude Code visual polish.
- Host/Ableton automation proof beyond existing command validation.

## Plan of Work

1. Add parameter/spec state and raw pointer mapping.
2. Extend `FxParameters` and `FxChain` with bounded, deterministic processors.
3. Keep defaults conservative so legacy presets and dry-core validation do not change.
4. Bind the new parameters into the existing FX page as a functional placeholder rack.
5. Add DSP and contract tests before marking the FX handoff gate ready.
6. Update docs, then run adversarial review and final validation.

## Milestones

Milestone 1 added the stable FX rack state.

Milestone 2 implemented bounded processors and editor bindings.

Milestone 3 added regression coverage and durable docs.

Milestone 4 completed adversarial review, accepted fixes, validation, and PR handoff.

## Concrete Steps

Completed from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --target SylenthAIPlugin_Standalone -j2
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core-fx-rack
    ./build/SylenthAIRender --list-parameters --output build/reports/parameters-fx-rack.json

## Validation and Acceptance

Acceptance requires:

- Global FX bypass remains dry-equivalent.
- Disabled modules do not change output.
- Added modules produce measurable, finite wet changes when enabled.
- Delay/reverb tail reporting remains correct and does not include zero-mix modules.
- New parameters appear in the registry and host default-merge/preset validation path.
- UI binds to real APVTS parameters, even if final visual polish is deferred.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --target SylenthAIPlugin_Standalone -j2
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Idempotence and Recovery

If a processor destabilizes output or causes tail ambiguity, remove that processor and keep the parameter/model plan narrow. Do not weaken dry-core validation to make wet FX pass.

## Artifacts and Notes

- Stacked PR branch: `feat/fx-rack-expansion`.
- Standalone screenshot evidence: `build/reports/ui/fx-rack-standalone.png`.
- Generated reports: `build/reports/core-fx-rack/` and `build/reports/parameters-fx-rack.json`.
- Build outputs and reports are intentionally not source artifacts.

## Interfaces and Dependencies

This slice unblocks the FX portion of `docs/exec-plans/active/2026-06-05-handoff-modulation-preset-arp-ui-polish.md`. Modulation route UI remains separately gated.
