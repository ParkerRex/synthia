# Synth

![Synth clean-room software instrument hero](docs/assets/readme-hero.png)

Synth is a clean-room macOS software instrument project focused on a fast, expressive pluck-synth core: oscillator stacking, mono/poly/unison allocation, per-voice modulation, semitone-domain filter motion, glide, ramp, and TransMod-style routing.

This is not a clone and does not ship third-party presets, UI assets, names, binaries, or proprietary parameter data. The implementation is original and uses public documentation, clean-room design, and deterministic validation.

## Current State

The repo builds a JUCE/CMake instrument scaffold with:

- AU, VST3, and standalone targets.
- A dry-core DSP path with oscillator, filter, envelopes, LFO, ramp, glide, velocity glide, amp drive, pan/spread, and performance MIDI sources.
- An 8-slot TransMod-style modulation layer with source/scaler routing and physical destination depths.
- Factory presets, preset schema validation, MIDI fixture rendering, and JSON report generation.
- Core validation for oscillator/filter behavior, modulation routing, voice allocation, render determinism, and preset loading.

The next required product validation step is host validation in Ableton: AU/VST3 scan, load, playback, automation, and state restore.

## Build

Configure:

```bash
cmake -S . -B build -DSYNTH_ENABLE_TESTS=ON
```

Build:

```bash
cmake --build build --config Debug
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

Build artifacts are written under:

- `build/SynthPlugin_artefacts/Standalone/Synth.app`
- `build/SynthPlugin_artefacts/AU/Synth.component`
- `build/SynthPlugin_artefacts/VST3/Synth.vst3`

The default build fetches JUCE `8.0.13`. Set `SYNTH_JUCE_PATH=/path/to/JUCE` during configure to use a local JUCE checkout.

## Validation

Run the full standalone core suite:

```bash
./build/SynthRender --suite core --output-dir build/reports/core
```

Focused validation commands:

```bash
./build/SynthRender --smoke --output build/reports/smoke.json
./build/SynthRender --list-parameters --output build/reports/parameters.json
./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json
./build/SynthRender --voice-test --output build/reports/voice-core.json
./build/SynthRender --osc-test --notes C1,C3,C5,C7 --output build/reports/oscillator.json
./build/SynthRender --filter-test --output build/reports/filter.json
./build/SynthRender --modulation-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation.json
```

Render the factory dry-core pluck:

```bash
./build/SynthRender \
  --preset presets/factory/pluck-core-01.json \
  --fixture fixtures/midi/overlap-pluck.mid \
  --dry \
  --output build/renders/pluck-core-01-dry.wav \
  --report build/reports/pluck-core-01-dry.json
```

Current core validation covers:

- finite output and non-clipping dry renders,
- oscillator tuning, pulse width, sub octave, stack detune, noise, and hard sync,
- semitone-domain filter mapping and nonlinear filter stability,
- ramp timing, glide, velocity glide, and mono/legato/unison allocation edge cases,
- direct modulation and TransMod source/scaler/destination behavior,
- top-level preset `mod_slots` schema loading and strict depth validation,
- deterministic render repeatability and LFO ablation metrics.

## Repository Map

- `SPEC.md`: durable product requirements.
- `CONTEXT.md`: project vocabulary and decision lanes.
- `docs/ARCHITECTURE.md`: implementation architecture.
- `docs/VALIDATION.md`: validation strategy and report contract.
- `docs/CLEAN_ROOM.md`: clean-room implementation rules.
- `src/dsp/`: DSP engine, oscillator, filter, envelopes, LFO, ramp, parameters.
- `src/voice/`: voice rendering and allocation.
- `src/plugin/`: JUCE processor/editor and parameter registry.
- `src/presets/`: preset schema validation.
- `src/validation/`: standalone render and report CLI.
- `tests/smoke/`: CTest smoke, contract, voice, and DSP coverage.
- `presets/factory/`: clean-room factory presets.
- `fixtures/`: MIDI and preset fixtures used by validation.

## Clean-Room Policy

Do not copy third-party source, binaries, GUI assets, presets, names, trade dress, or proprietary parameter data into this repository. Shipped names and UI should not imply affiliation with FXpansion, ROLI, Roland, SH-101, Deadmau5, or song titles unless legal approval exists.
