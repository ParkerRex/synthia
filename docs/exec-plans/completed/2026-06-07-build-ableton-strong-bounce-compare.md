---
title: Build Ableton Strong Bounce Compare
status: completed
summary: Add a reusable Ableton offline/realtime audio comparison helper and use it to close the stronger host comparison gap.
post_build_recap: Added `scripts/compare-ableton-bounce-realtime.py`, a standard-library Python helper that aligns Ableton offline bounce and realtime resampling evidence by RMS envelope, checks per-channel content similarity with filtered-band envelope thresholds, and verifies same-loudness plus silent-channel negative controls fail. The existing Ableton bounce/resample artifacts passed the stronger comparison while strict waveform-null equivalence remains unclaimed.
triggers:
  - Reviewing Ableton offline/realtime host proof.
  - Checking whether any non-UI Phase 1 Ableton host-matrix item remains open.
---

# Build Ableton Strong Bounce Compare

This completed ExecPlan records the stronger Ableton offline/realtime comparison captured on 2026-06-07.

## Scope

- Add a reusable host-validation helper under `scripts/`.
- Reuse the existing ignored Ableton offline bounce and realtime resampling artifacts.
- Require content-match criteria that can reject mismatched audio, not just finite/non-silent/non-clipping loudness.
- Do not claim strict waveform/null-test equivalence.

## Completed Work

- Added `scripts/compare-ableton-bounce-realtime.py`.
- Implemented WAV and AIFF PCM loading, RMS-envelope alignment, per-channel filtered-band envelope metrics, content thresholds, and JSON reporting.
- Added a synthetic `--self-test` that must pass the positive control and fail wrong-offset, reversed, same-RMS sine, same-RMS noise, and silent-last-channel controls.
- Ran the helper against `build/reports/ableton/bounce-compare/ableton-offline-current.wav` and `build/reports/ableton/bounce-compare/ableton-realtime-resample-current.aif`.

## Validation Results

- Alignment offset: realtime frame `222208`.
- Mixed-envelope alignment correlation: `0.9855785842640679`.
- Per-channel content envelope correlation floor: `0.9432697509451544`.
- Per-channel RMS delta range: `-0.8406264635442906` to `0.8676234690114247` dB.
- Per-channel peak delta range: `-1.6826751105175193` to `0.3252204424193288` dB.
- Per-channel filtered-band correlation mean floor: `0.9177223458754987`.
- Minimum per-band correlation: `0.8982884460435024`.
- Per-channel frame filtered-band cosine mean floor: `0.8257845095752637`.
- Per-channel waveform correlations: `0.5984748756409872` and `0.5631472554905425`.
- Negative controls failed as expected: wrong offset, reversed realtime, same-RMS sine, same-RMS noise, and silent last channel.

Validation commands:

```bash
scripts/compare-ableton-bounce-realtime.py \
  --self-test \
  --output build/reports/ableton/bounce-compare/strong-compare-self-test.json

scripts/compare-ableton-bounce-realtime.py \
  --offline build/reports/ableton/bounce-compare/ableton-offline-current.wav \
  --realtime build/reports/ableton/bounce-compare/ableton-realtime-resample-current.aif \
  --output build/reports/ableton/bounce-compare/ableton-bounce-realtime-strong-compare.json
```

## Remaining Gaps

- No non-UI Phase 1 Ableton host-matrix item remains open.
- Ableton audio-diff modulation capture remains unclaimed.
- Strict offline/realtime waveform-null equivalence remains unclaimed.
