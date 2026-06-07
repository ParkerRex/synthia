# Build Offline Realtime Render Compare

Status: completed 2026-06-07 EDT

## Goal

Add deterministic standalone proof that realtime and offline quality modes both render safely and produce a bounded, meaningful quality-mode difference.

## Scope

- Add `SylenthAIRender --offline-realtime-compare-test`.
- Use `FX Space 01` because it enables the FX rack and sets realtime quality to Normal and offline quality to High.
- Record peak/RMS metrics for each render and audio-difference metrics between them.
- Add the comparison to the core suite and CTest.
- Do not claim Ableton bounce-versus-realtime proof.

## Validation

Observed local report values from `build/reports/offline-realtime-compare.json`:

- realtime quality: `normal`
- offline quality: `high`
- realtime peak: `0.238953`
- offline peak: `0.229923`
- max absolute difference: `0.0326109`
- RMS difference: `0.0036466`

Validation commands:

```bash
cmake -S . -B build -DSYLENTH_AI_ENABLE_TESTS=ON
cmake --build build --config Debug --target SylenthAIRender
./build/SylenthAIRender --offline-realtime-compare-test --output build/reports/offline-realtime-compare.json
ctest --test-dir build --output-on-failure -R SylenthAIOfflineRealtimeCompareTest
```

## Residual Gaps

- Ableton offline bounce versus realtime comparison remains open.
- AU/VST3 automation record/playback remains open.
