# Dependency Graph

This file defines slice ordering for the Synth build program.

## Graph

```text
01 scaffold-juce-cmake-plugin-foundation
  -> 02 build-parameter-state-and-preset-contract
  -> 03 build-voice-midi-envelope-lfo-core
  -> 04 build-oscillator-stack-and-mixer
  -> 05 build-nonlinear-filter-drive-and-oversampling
  -> 06 build-modulation-matrix-ramp-and-glide
  -> 07 build-amp-stereo-analog-and-factory-pluck
  -> 08 build-render-validation-harness-and-metrics
  -> 09 build-editor-ui-and-preset-workflow
  -> 10 build-onboard-fx-and-quality-modes
  -> 11 build-ableton-host-integration-and-packaging
  -> 12 build-release-hardening-and-documentation-closeout
```

## Dependency Notes

The state contract follows the scaffold because parameter IDs, serialization, and preset schema need real project code to anchor.

The voice/MIDI/envelope/LFO core follows state because note handling and modulators need stable parameters and test fixtures.

Oscillator and filter are separated because oscillator tests can validate pitch, aliasing, stack, and mix behavior before nonlinear filter tuning complicates render metrics.

The full modulation slice follows oscillator and filter because TransMod destinations need real oscillator/filter/amp parameters to target.

The amp/stereo/factory-pluck slice follows full modulation because the target pluck needs voice/unison spread, amp drive, and macro behavior.

The validation harness becomes a standalone slice after enough dry core exists to render useful fixtures, but early slices must still add focused tests.

UI follows dry core and state contract so it can bind to real parameters instead of mock controls.

FX follows dry core validation so onboard space does not hide weak core synthesis.

Ableton host integration and packaging follow a usable plugin/UI, but host assumptions are carried from the foundation.

Release hardening closes all remaining docs, performance, packaging, and legal review.

