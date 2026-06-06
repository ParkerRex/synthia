---
title: Build Patch Cost And Voice Math Model
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Move patch-load and active/max voice math out of the editor into a deterministic model-backed diagnostic with contract coverage.
post_build_recap: Added `PatchCostEstimate`, shared estimator helpers, processor diagnostic exposure, active/max voice header display, zero-level slot and A1 core-stack alignment with the renderer, and contract tests for default, high-cost, mono/unison/poly, FX, filter, and solo/mute cases.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
read_when:
  - Changing voice allocation, layer/oscillator-slot rendering, or patch cost.
  - Auditing header active/max voices or load estimate behavior.
  - Planning Ableton CPU calibration.
  - Preparing Claude UI work that displays cost or voice diagnostics.
---

# Build Patch Cost And Voice Math Model

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Phase 1 needs Sylenth-style voice/cost feedback that reflects the actual engine, not ad hoc UI math. The existing header displayed active voices and a patch-load estimate, but the estimate lived in `PluginEditor` and did not account for rendered A2/B1/B2 oscillator slots. This slice moves the estimate into a testable model derived from `SynthParameters`, exposes it through processor diagnostics, and keeps the UI as a display surface.

## Progress

- [x] 2026-06-06 EDT: Added `PatchCostEstimate` and estimator helpers in `src/dsp/SynthParameters.h`.
- [x] 2026-06-06 EDT: Aligned `oscillatorSlotVoiceCost()` with the render path by ignoring zero-level slots.
- [x] 2026-06-06 EDT: Exposed the estimate through `SynthAudioProcessor::DiagnosticsSnapshot`.
- [x] 2026-06-06 EDT: Replaced editor-local patch-cost math with the processor diagnostic estimate.
- [x] 2026-06-06 EDT: Updated the header voice chip to show active/max voices.
- [x] 2026-06-06 EDT: Added contract tests for default cost, high-cost patches, zero-level slots, A1 core stack count, solo/mute, mono-legato, unison, filter oversampling, and FX module factors.
- [x] 2026-06-06 EDT: Updated architecture, baseline, active index, Program, and superseded UI handoff notes.
- [x] 2026-06-06 EDT: Fixed autoreview-discovered undercount where the legacy A1 compatibility oscillator was gated by `layer.1.osc.1.voices` but rendered with `osc.stack_count`.

## Surprises & Discoveries

- `layerOscillatorVoiceCost()` counted enabled slots even when their level was zero, while `Voice::renderLayerOscillators()` skips zero-level slots. The estimator exposed that mismatch, so the helper now follows rendered cost.
- A1 is special: `layer.1.osc.1.voices` gates the legacy compatibility slot, but audible stack cost comes from `osc.stack_count`. The estimator now treats that slot as a rendered core stack instead of a normal layer slot voice count.
- The editor's earlier estimate used `voice.polyphony * voice.unison_count * osc.stack_count`; that was acceptable before real A2/B1/B2 rendering, but it became stale after the layer renderer landed.
- Patch cost is still an estimate, not measured CPU. The deterministic model is good enough for UI/AI warnings and testable behavior, but Ableton calibration remains a separate validation task.

## Decision Log

Decision: Put the estimator in `SynthParameters.h` rather than `PluginEditor`.
Rationale: Patch cost is derived product state that must be available to diagnostics, tests, UI, and future AI workflows without duplicating raw APVTS reads.
Date: 2026-06-06.

Decision: Keep the estimate derived and non-serialized.
Rationale: It is a function of sound state and local engine heuristics. Presets should serialize the underlying parameters, not a stale cost result.
Date: 2026-06-06.

Decision: Do not claim measured CPU accuracy.
Rationale: Host CPU depends on buffer size, sample rate, machine, compiler build, and Ableton conditions. The current model is deterministic load guidance until Ableton calibration exists.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. The current patch-cost model derives:

- note limit from voice mode and polyphony,
- unison voice count from voice mode and `voice.unison_count`,
- max active voices capped to the engine's 32-voice allocator,
- rendered oscillator-slot voices from active layer/slot state, with A1 counted by the live core `osc.stack_count`,
- filter multiplier from filter enable state and oversampling factor,
- FX multiplier from global FX enable state and enabled module count,
- load percent plus elevated/high/over-budget flags.

The editor now displays active/max voices and load percent from `DiagnosticsSnapshot`, so the UI no longer owns patch-cost math.

## Context and Orientation

Relevant current code and docs:

- `src/dsp/SynthParameters.h`
- `src/plugin/PluginProcessor.h`
- `src/plugin/PluginProcessor.cpp`
- `src/plugin/PluginEditor.cpp`
- `tests/smoke/SynthContractTest.cpp`
- `docs/ARCHITECTURE.md`
- `docs/modern-sylenth-baseline.md`

### In Scope

- Deterministic patch-cost and voice math helpers.
- Processor diagnostic exposure.
- Header display binding to the diagnostic estimate.
- Contract tests for representative edge cases.
- Docs and Program ledger updates.

### Out Of Scope

- Measured CPU profiling.
- Ableton CPU calibration.
- New quality modes or automatic degradation.
- UI visual polish beyond replacing the data source and active/max display.
- Audio-thread allocation or runtime measurement work.

## Plan of Work

1. Add a model-backed `PatchCostEstimate`.
2. Align layer/slot voice-cost helpers with the real render path.
3. Expose the estimate through processor diagnostics.
4. Replace editor-local math with diagnostic state.
5. Add focused contract coverage.
6. Update durable docs and run validation.

## Milestones

Milestone 1 added the deterministic cost model.

Milestone 2 wired diagnostics and UI display to the model.

Milestone 3 added contract tests for voice modes, slots, filter, FX, and thresholds.

Milestone 4 updated docs and Program ledgers.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --config Debug
    ./build/SylenthAIContractTest
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Validation and Acceptance

Acceptance requires:

- Default init estimate is deterministic and low cost.
- Zero-level oscillator slots are excluded from cost.
- Solo/mute layer behavior matches `layerOscillatorVoiceCost()`.
- Mono/mono-legato estimates do not multiply by polyphony or unison.
- Unison estimates use one note multiplied by unison voices.
- High polyphony/unison/slot/filter/FX settings produce high and over-budget flags.
- Editor reads patch cost from processor diagnostics, not local raw APVTS math.
- Full CTest and core standalone render validation pass.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    cmake --build build --config Debug
    ./build/SylenthAIContractTest
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Idempotence and Recovery

The estimate is a pure function of `SynthParameters`, so it is deterministic and rerunnable. If Ableton calibration later changes thresholds, update only the estimator constants/tests and keep the UI bound to diagnostics. If voice allocation changes beyond 32 voices, update the allocator cap in the model and tests together.

## Artifacts and Notes

No source-controlled runtime artifacts are produced. Disposable validation outputs remain under `build/reports/`.

Remaining follow-up: calibrate the load percent against Ableton CPU measurements at 44.1, 48, and 96 kHz with representative low/medium/high-cost patches.

## Interfaces and Dependencies

This slice depends on `SynthParameters`, layer/slot render semantics in `Voice`, `SynthAudioProcessor` diagnostics, and `SynthContractTest`. It feeds Claude UI polish, Ableton CPU validation, and future AI patch-generation bounds.
