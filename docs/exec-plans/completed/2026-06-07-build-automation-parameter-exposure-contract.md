# Build Automation Parameter Exposure Contract

Status: completed 2026-06-07 EDT

## Goal

Add backend coverage that every registry parameter is actually exposed to the host-facing APVTS parameter layer.

## Scope

- Verify APVTS exposes every `ParameterRegistry` entry.
- Verify parameters marked automatable remain host-automatable.
- Verify APVTS parameter types match registry kind: float, bool, or choice.
- Verify every parameter accepts a host-notifying default write.
- Do not claim Ableton automation record/playback proof.

## Implementation

`SylenthAIContractTest` now constructs a minimal APVTS-backed processor and iterates over `getParameterSpecs()`.

For each parameter, the test verifies:

- `AudioProcessorValueTreeState::getParameter(id)` returns a parameter.
- `isAutomatable()` is true for automatable registry specs.
- The APVTS parameter dynamic type matches `ParameterKind`.
- `setValueNotifyingHost()` can round-trip the clamped default value.

## Validation

```bash
cmake --build build --config Debug --target SylenthAIContractTest
ctest --test-dir build --output-on-failure -R SylenthAIContractTest
```

## Residual Gaps

- Ableton AU/VST3 automation record/playback remains open.
- Offline bounce versus realtime comparison remains open.
