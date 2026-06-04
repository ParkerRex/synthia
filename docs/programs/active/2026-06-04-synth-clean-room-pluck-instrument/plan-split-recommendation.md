# Plan Split Recommendation

The Synth program should be implemented as twelve linked ExecPlans.

## Recommended Child ExecPlans

1. `docs/exec-plans/completed/2026-06-04-scaffold-juce-cmake-plugin-foundation.md`
2. `docs/exec-plans/completed/2026-06-04-build-parameter-state-and-preset-contract.md`
3. `docs/exec-plans/completed/2026-06-04-build-voice-midi-envelope-lfo-core.md`
4. `docs/exec-plans/active/2026-06-04-build-oscillator-stack-and-mixer.md`
5. `docs/exec-plans/active/2026-06-04-build-nonlinear-filter-drive-and-oversampling.md`
6. `docs/exec-plans/active/2026-06-04-build-modulation-matrix-ramp-and-glide.md`
7. `docs/exec-plans/active/2026-06-04-build-amp-stereo-analog-and-factory-pluck.md`
8. `docs/exec-plans/active/2026-06-04-build-render-validation-harness-and-metrics.md`
9. `docs/exec-plans/active/2026-06-04-build-editor-ui-and-preset-workflow.md`
10. `docs/exec-plans/active/2026-06-04-build-onboard-fx-and-quality-modes.md`
11. `docs/exec-plans/active/2026-06-04-build-ableton-host-integration-and-packaging.md`
12. `docs/exec-plans/active/2026-06-04-build-release-hardening-and-documentation-closeout.md`

## Why This Split

The split keeps each plan independently reviewable and testable. DSP correctness, host integration, UI, FX, and packaging each have different proof surfaces and should not be bundled into one uninspectable implementation pass.

## Current Slice

The scaffold, parameter/state, and voice-core slices are complete. Continue with `build-oscillator-stack-and-mixer`.

## Completion Rule

The program is complete only when all child ExecPlans have moved to `docs/exec-plans/completed/`, `SPEC.md` and docs reflect final behavior, release validation passes, and `program.md` contains an initiative retrospective.
