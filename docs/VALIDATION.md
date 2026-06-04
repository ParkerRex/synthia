# Validation Plan

Validation must prove the instrument behaves correctly as a plugin and as a sound engine. Passing unit tests alone is not enough; Ableton loading, host state, and audio renders are required.

Current scaffold commands:

```bash
cmake -S . -B build -DSYNTH_ENABLE_TESTS=ON
cmake --build build --config Debug
ctest --test-dir build --output-on-failure
./build/SynthRender --smoke --output build/reports/smoke.json
./build/SynthRender --list-parameters --output build/reports/parameters.json
./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json
./build/SynthRender --voice-test --output build/reports/voice-core.json
```

The current smoke render is intentionally silent and only proves initialization, finite output, report writing, and command shape.

The current contract validation proves:

- 102 unique parameter IDs,
- valid defaults and ranges,
- APVTS state round-trip,
- two clean-room factory preset JSON files,
- no unknown preset parameter IDs.

The current voice-core validation proves:

- envelope attack/release behavior,
- LFO phase reset,
- poly voice allocation and release,
- engine note-on/note-off voice lifecycle,
- silent render remains finite while voices are active.

## Validation Profiles

### Core DSP

- oscillator aliasing,
- pulse duty cycle,
- sub oscillator tuning,
- stack detune symmetry,
- filter cutoff mapping,
- filter drive/resonance behavior,
- envelope timing,
- LFO sync and phase reset,
- per-voice modulation independence,
- TransMod source/scaler/destination behavior.

### Musical Render

Use fixed MIDI fixtures:

- 128 BPM,
- overlapping 1/8 and 1/16 pluck phrase,
- velocity variation from 70 to 120,
- dry core render,
- FX render,
- mono-LFO ablation,
- per-voice-LFO render.

Required checks:

- overlapping notes keep note-local cutoff motion in per-voice mode,
- mono LFO produces a measurably different movement profile,
- output is deterministic within tolerance,
- factory pluck does not clip at default level.

### Host Integration

Required Ableton checks:

- AU scan/load/play,
- VST3 scan/load/play,
- parameter automation record/playback,
- host state save/restore,
- offline bounce versus realtime render,
- buffer-size changes,
- sample-rate changes,
- transport stop and all-notes-off,
- UI open/close while playing.

Recommended additional hosts:

- Logic Pro for AU,
- Reaper for AU/VST3,
- Bitwig Studio for VST3.

### Performance

Measure:

- 44.1, 48, 96 kHz,
- 32, 64, 128, 512 sample buffers,
- 8, 16, 32 voices,
- unison 1, 2, 4, 8,
- oversampling off, 2x, 4x,
- UI open and closed,
- silence/release-tail denormal behavior.

## Metrics

Recommended targets:

- oscillator tuning: within 5 cents,
- self-osc filter tracking: within 10 cents after calibration,
- LFO sync period: within 1%,
- envelope decay/release timing: within 15%,
- regression loudness after gain match: within 0.5 LU,
- no NaN or infinity in any render,
- no denormal CPU spike on silence tails.

## Render Artifact Contract

Each validation render should record:

- fixture ID,
- preset ID,
- plugin version,
- sample rate,
- block size,
- tempo,
- seed,
- architecture,
- format or standalone runner,
- metric results,
- pass/fail summary.

Failed renders should keep audio artifacts and JSON reports for inspection.

## First Validation Milestone

The first implementation milestone is not complete until:

- standalone renders the factory pluck fixture,
- oscillator/filter/envelope/LFO tests pass,
- AU loads in Ableton,
- VST3 loads in Ableton,
- Ableton state restore preserves the preset,
- per-voice LFO ablation test passes.
