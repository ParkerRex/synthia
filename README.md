# Synth

Synth is a clean-room software instrument project for a Strobe-v1-like, SH-101-inspired pluck engine aimed at modern macOS/Ableton use.

The core target is not an exact clone of any third-party plugin. The goal is the sound-design behavior that matters:

- one stackable oscillator
- saw, pulse, noise, and sub mixing
- mono/poly/unison voice allocation
- per-voice envelopes, LFO, ramp, glide, and random sources
- semitone-domain cutoff motion
- nonlinear filter drive and resonance interaction
- voice and unison spread
- TransMod-style modulation
- AU and VST3 delivery

## Current State

This repository has a working JUCE/CMake synth scaffold with the first dry-core DSP path.

Existing docs:

- `SPEC.md`: durable product specification.
- `CONTEXT.md`: project vocabulary and decision lanes.
- `docs/index.md`: documentation map.
- `docs/CLEAN_ROOM.md`: clean-room policy.
- `docs/ARCHITECTURE.md`: planned system architecture.
- `docs/VALIDATION.md`: validation and test strategy.
- `docs/PRESET_SCHEMA.md`: preset/state schema guidance.
- `docs/research/source-map.md`: historical and vendor source map.
- `docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/program.md`: end-to-end build Program.
- `docs/exec-plans/active/`: child implementation plans.

Implemented:

- `CMakeLists.txt`
- `src/plugin/PluginProcessor.*`
- `src/plugin/PluginEditor.*`
- `src/plugin/ParameterRegistry.*`
- `src/dsp/Envelope.*`
- `src/dsp/Filter.*`
- `src/dsp/Lfo.*`
- `src/dsp/OscillatorStack.*`
- `src/dsp/SynthEngine.*`
- `src/dsp/SynthParameters.h`
- `src/presets/PresetValidator.*`
- `src/voice/Voice.*`
- `src/voice/VoiceAllocator.*`
- `src/validation/SynthRender.cpp`
- `tests/smoke/SynthSmokeTest.cpp`
- `tests/smoke/SynthContractTest.cpp`
- `tests/smoke/SynthDspCoreTest.cpp`
- `tests/smoke/SynthVoiceCoreTest.cpp`
- `presets/factory/init.json`
- `presets/factory/pluck-core-01.json`

Current audio behavior renders the dry core: oscillator stack, nonlinear filter, direct envelope/LFO cutoff and pitch routes, amp drive, pan spread, analog variation, and the `Pluck Core 01` factory preset. Full TransMod slots, ramp, glide, UI workflow, FX, Ableton validation, and packaging remain active slices.

## Build Direction

The expected first implementation path is JUCE with CMake:

- AU for macOS.
- VST3 for macOS.
- Standalone target for deterministic validation.
- Universal binaries for `arm64` and `x86_64`.

`SPEC.md` owns the requirements. Update it before making behavior-changing code decisions.

Continue implementation with:

- `docs/exec-plans/active/2026-06-04-build-modulation-matrix-ramp-and-glide.md`

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

Run the smoke renderer:

```bash
./build/SynthRender --smoke --output build/reports/smoke.json
```

List parameters:

```bash
./build/SynthRender --list-parameters --output build/reports/parameters.json
```

Validate factory presets:

```bash
./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json
```

Run voice-core validation:

```bash
./build/SynthRender --voice-test --output build/reports/voice-core.json
```

Run oscillator and filter validation:

```bash
./build/SynthRender --osc-test --notes C1,C3,C5,C7 --output build/reports/oscillator.json
./build/SynthRender --filter-test --output build/reports/filter.json
```

Render the factory dry-core pluck:

```bash
./build/SynthRender --preset presets/factory/pluck-core-01.json --fixture fixtures/midi/overlap-pluck.mid --dry --output build/renders/pluck-core-01-dry.wav --report build/reports/pluck-core-01-dry.json
```

Current build artifacts:

- `build/SynthPlugin_artefacts/Standalone/Synth.app`
- `build/SynthPlugin_artefacts/AU/Synth.component`
- `build/SynthPlugin_artefacts/VST3/Synth.vst3`

The scaffold uses CMake `FetchContent` pinned to JUCE `8.0.13` by default. Set `SYNTH_JUCE_PATH=/path/to/JUCE` at configure time to use a local JUCE checkout.
