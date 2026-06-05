# Validation Plan

Validation must prove the instrument behaves correctly as a plugin and as a sound engine. Passing unit tests alone is not enough; Ableton loading, host state, and audio renders are required.

## Roadmap Validation Phases

Phase 1 validation proves the Sylenth rebuild: AU/VST3 scan/load/play in Ableton, host state restore, automation, preset browsing, A/B architecture, oscillator/filter/envelope/modulation behavior, arpeggiator timing, FX tails, UI open/close while playing, and screenshot/manual QA against `docs/modern-sylenth-baseline.md`.

Phase 2 validation proves AI-assisted generation: randomize/generate commands must produce finite audio, valid preset state, bounded parameter values, deterministic or intentionally seeded results, and editable arp/chord/modulation data rather than opaque output.

Phase 3 validation proves conversational editing: text prompts and reference-sound workflows must produce reversible parameter diffs, preserve realtime safety, avoid invalid states, and include reports that explain which synth, modulation, arp, and FX controls changed.

Current scaffold commands:

```bash
cmake -S . -B build -DSYNTH_ENABLE_TESTS=ON
cmake --build build --config Debug
ctest --test-dir build --output-on-failure
./build/SynthRender --smoke --output build/reports/smoke.json
./build/SynthRender --list-parameters --output build/reports/parameters.json
./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json
./build/SynthRender --voice-test --output build/reports/voice-core.json
./build/SynthRender --osc-test --notes C1,C3,C5,C7 --output build/reports/oscillator.json
./build/SynthRender --filter-test --output build/reports/filter.json
./build/SynthRender --modulation-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation.json
./build/SynthRender --preset presets/factory/pluck-core-01.json --fixture fixtures/midi/overlap-pluck.mid --dry --output build/renders/pluck-core-01-dry.wav --report build/reports/pluck-core-01-dry.json
./build/SynthRender --preset presets/factory/pluck-core-01.json --fixture fixtures/midi/overlap-pluck.mid --wet --output build/renders/pluck-core-01-wet.wav --report build/reports/pluck-core-01-wet.json
./build/SynthRender --suite core --output-dir build/reports/core
```

The current smoke render is intentionally note-less and proves initialization, finite output, report writing, and command shape.

The current contract validation proves:

- 156 unique parameter IDs,
- valid defaults and ranges,
- APVTS state round-trip,
- two factory preset JSON files,
- no unknown preset parameter IDs,
- TransMod slot objects use valid slot IDs, source/scaler choices, depth domains, and destination IDs.

The current voice-core validation proves:

- envelope attack/release behavior,
- LFO phase reset,
- poly voice allocation and release,
- engine note-on/note-off voice lifecycle,
- audio render remains finite while voices are active.

The current DSP validation proves:

- oscillator tuning for C1, C3, C5, and C7 within the 5-cent target,
- high-band oscillator spectral metrics are recorded for C1, C3, C5, and C7,
- pulse duty cycle at 10, 25, 50, 75, and 90 percent widths,
- sub octave accuracy for -1, -2, and -3,
- deterministic noise, stack detune symmetry, and hard-sync finite output,
- semitone filter cutoff mapping,
- nonlinear filter mode response for L2, L4, B2, B4, H2, H4, Peak2, Notch2, and Notch4,
- finite resonant impulse behavior,
- drive changes filter response,
- `Pluck Core 01` dry render loads the requested preset and MIDI fixture, writes a finite non-clipping WAV/report, and records note-local LFO spread during overlapping notes.
- ramp timing, glide, velocity glide, direct keytrack/LFO/envelope routes, TransMod scaler multiplication to many physical destinations, performance MIDI sources, voice/unison/random source spread, and fixture trace ranges.
- top-level `mod_slots` preset schema objects are applied by render loading, including schema-only modulation fixtures that omit flat APVTS-style `transmod.*` parameters.
- FX bypass stays null-equivalent to dry rendering when globally bypassed, tempo-synced delay reports exact sample timing at test tempo, FX tail length is reported from the active FX parameters, and wet output remains finite, non-clipping, and measurably different from its dry reference.
- `SynthRender --suite core` runs the standalone smoke, parameter, preset, voice, oscillator, filter, modulation, dry pluck, wet pluck, LFO ablation, and determinism reports in one command.
- `SynthRenderCoreSuite` runs the core suite under CTest.

Preset render validation is expected to fail if the preset file is missing, the preset JSON is invalid, the MIDI fixture is missing, the fixture is not a valid MIDI file, or the fixture has no note events.

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

Current standalone core-suite artifacts:

- `summary.json`: aggregate pass/fail for all core reports.
- `pluck-core-01-dry.json`: dry factory pluck metrics plus WAV artifact path.
- `pluck-core-01-wet.json`: wet factory pluck metrics plus WAV artifact path, FX mode, delay division, tempo-synced delay samples, tail length, post-event render length, active quality mode, and wet-versus-dry difference metrics.
- `lfo-ablation.json`: compares per-voice LFO and mono LFO renders using note-local LFO spread and audio difference.
- `determinism.json`: renders the dry pluck twice and compares `max_abs_diff`, `rms_diff`, and `peak_delta` against fixed tolerances.
- `artifacts/*.wav`: retained dry/per-voice/mono render WAVs.
- `failures/*.wav`: written only when deterministic repeat comparison fails.

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

Implemented standalone metrics:

- `invalid_samples`: finite-output guard; must be `0`.
- `peak`: maximum absolute sample; dry pluck must remain below `1.0`.
- `rms`: linear RMS of stereo audio.
- `rms_dbfs`: `20 * log10(rms)`, used as a lightweight loudness proxy until LUFS exists.
- `crest_db`: peak-to-RMS relationship.
- `dc_offset`: average mono offset.
- `spectral_centroid_hz`: bounded-window DFT centroid for coarse spectral regression.
- `stereo_correlation`: left/right correlation, useful for spread regressions.
- `note_local_lfo_spread`: spread of per-voice LFO values while notes overlap.
- `fx_mode`, `delay_division_beats`, `tempo_synced_delay_samples`, `fx_tail_seconds`, `post_last_event_seconds`, `quality_mode`, `wet_dry_max_abs_diff`, `wet_dry_rms_diff`, and `wet_meaningful_passed`: wet-render proof for the onboard FX path.
- `mod_slot_schema_passed`: modulation harness check that canonical preset `mod_slots` objects are loaded into runtime TransMod slots.
- `max_abs_diff`, `rms_diff`, `peak_delta`: deterministic repeat comparison metrics.

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

Current report schema uses `schema_version: 1` and a per-report `suite` field. Core-suite reports live under the supplied output directory and are disposable build artifacts.

Failed renders should keep audio artifacts and JSON reports for inspection.

## First Validation Milestone

The first implementation milestone is not complete until:

- standalone renders the factory pluck fixture,
- oscillator/filter/envelope/LFO tests pass,
- AU loads in Ableton,
- VST3 loads in Ableton,
- Ableton state restore preserves the preset,
- per-voice LFO ablation test passes.
