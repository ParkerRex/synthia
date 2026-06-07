---
title: Validate Ableton Bounce Realtime Compare
status: completed
summary: Compare Ableton Master offline bounce against realtime Master resampling for the current host validation set.
post_build_recap: Ableton Live 11 exported a fresh 12-second Master offline bounce, recorded realtime Master output through track 3 Resampling, and produced a bounded host comparison report. The offline and aligned realtime renders are finite, non-silent, non-clipping, and energy-matched within 1 dB; strict waveform-null equivalence is not claimed.
triggers:
  - Reviewing Ableton bounce-versus-realtime proof.
  - Checking the bounded Ableton host sanity evidence before stricter offline/realtime proof.
---

# Validate Ableton Bounce Realtime Compare

This completed ExecPlan records the bounded Ableton offline/realtime host comparison captured on 2026-06-07.

## Scope

- No DSP, preset schema, parameter ID, or plugin source changes.
- Compare Ableton host offline export against Ableton realtime `Resampling`.
- Count this as bounded host proof, not strict waveform/null-test proof.
- Keep generated audio, screenshots, and JSON under ignored `build/reports/ableton/bounce-compare/`.

## Completed Work

- Exported a fresh Ableton Master offline bounce from start `1.1.1`, length `6.0.0`, at 44100 Hz, WAV 16-bit, with MP3 enabled.
- Routed audio track 3 to Ableton `Resampling`, armed it, and recorded realtime Master output.
- Copied the valid realtime AIF from `/Users/parkerrex/Desktop/testing-synth Project/Samples/Recorded/3-Audio 0001 [2026-06-07 051355].aif` into the ignored build evidence folder.
- Converted the offline WAV and realtime AIF to 16-bit analysis WAVs and aligned the offline bounce inside the realtime capture.
- Wrote `build/reports/ableton/bounce-compare/ableton-bounce-realtime-compare.json` with bounded comparison metrics and explicit strict-waveform status.

## Validation Results

- Offline WAV: stereo 44.1 kHz, 529200 frames, 12.0 seconds, peak `0.32830810546875`, RMS `0.036749536829718675`.
- Realtime AIF: stereo 44.1 kHz, 969728 frames, 21.99 seconds before alignment.
- Alignment offset: realtime frame `222209`, `5.03875283446712` seconds.
- Aligned realtime peak: `0.30682373046875`.
- Aligned realtime RMS: `0.03703281940458229`.
- RMS delta: `0.06669814839033678` dB.
- Peak delta: `-0.5878531810754004` dB.
- Waveform correlation: `0.6104798365036401`.
- Difference RMS: `0.033165678108847414`, `-29.586222372908438` dBFS.
- Bounded host comparison passed: both paths are finite, non-silent, non-clipping, and energy-matched within 1 dB.
- Strict waveform match passed: false.

## Findings

- Ghostty needed microphone permission before shell audio capture tools could open AVFoundation devices.
- CASTER virtual devices opened but did not receive system output from the current routing, so realtime proof used Ableton-internal `Resampling`.
- Ableton appended duplicate file extensions during export; local artifacts were normalized under `build/reports/ableton/bounce-compare/`.
- The first realtime capture required alignment because transport was not at a clean start position. The resulting proof is a bounded host comparison, not exact waveform equivalence.

## Remaining Gaps

- Stronger Ableton offline/realtime comparison remains open; this bounded pass can confirm signal health and rough energy matching but cannot reject all mismatched audio.
- Ableton audio-diff modulation capture remains unclaimed.
- Strict offline/realtime waveform-null equivalence remains unclaimed.
