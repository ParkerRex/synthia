# Synth Agent Guide

Synth is a JUCE/CMake software instrument for macOS Ableton validation. Keep agent work practical, test-backed, and aligned with the product docs.

## Read First

For meaningful work, read these in order:

1. `SPEC.md`
2. `CONTEXT.md`
3. `docs/index.md`
4. `docs/ARCHITECTURE.md`
5. `docs/VALIDATION.md`

Use the docs as the durable source of truth:

- `SPEC.md`: product requirements and behavioral contract.
- `CONTEXT.md`: vocabulary and decision lanes.
- `docs/ARCHITECTURE.md`: component boundaries and realtime boundary.
- `docs/VALIDATION.md`: validation profiles, metrics, and host proof.
- `docs/PRESET_SCHEMA.md`: preset and persistence schema details.

Do not bury product requirements in comments, build files, or tests unless the requirement already exists in `SPEC.md` or the relevant doc.

## Realtime Audio Rules

The audio thread may render DSP, process MIDI, read atomically published state, update lock-free diagnostics, and write output buffers.

The audio thread must not allocate, block on locks, do filesystem or network I/O, log synchronously, parse presets, scan directories, rebuild UI, or create/destroy heavyweight resources.

DSP changes must guard against NaN, infinity, denormals, runaway feedback, invalid parameter states, and uncontrolled gain jumps. Prefer deterministic tests for oscillator, filter, envelope, LFO, ramp, glide, modulation, presets, and host restore.

Keep parameter IDs stable and centralized in the parameter registry. UI labels may change; released automation/preset IDs must not be reused or casually renamed.

## Build And Test

Configure:

```bash
cmake -S . -B build -DSYNTH_ENABLE_TESTS=ON
```

Build:

```bash
cmake --build build --config Debug
```

Run CTest:

```bash
ctest --test-dir build --output-on-failure
```

Run the standalone core validation suite:

```bash
./build/SynthRender --suite core --output-dir build/reports/core
```

Focused render checks:

```bash
./build/SynthRender --smoke --output build/reports/smoke.json
./build/SynthRender --list-parameters --output build/reports/parameters.json
./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json
./build/SynthRender --voice-test --output build/reports/voice-core.json
./build/SynthRender --osc-test --notes C1,C3,C5,C7 --output build/reports/oscillator.json
./build/SynthRender --filter-test --output build/reports/filter.json
./build/SynthRender --modulation-test --fixture fixtures/midi/overlap-pluck.mid --output build/reports/modulation.json
```

Dry-core factory pluck render:

```bash
./build/SynthRender \
  --preset presets/factory/pluck-core-01.json \
  --fixture fixtures/midi/overlap-pluck.mid \
  --dry \
  --output build/renders/pluck-core-01-dry.wav \
  --report build/reports/pluck-core-01-dry.json
```

Build artifacts are disposable and live under `build/`. Keep generated reports, WAVs, bundles, and other build outputs out of source docs unless a human explicitly asks for evidence snapshots.

## Ableton Validation

Passing unit tests is not enough for product validation. Treat Ableton host proof as required for plugin-facing changes.

Required checks:

- AU scan, load, playback.
- VST3 scan, load, playback.
- Parameter automation record/playback.
- Host state save, close, reopen, and restore.
- Offline bounce compared with realtime render where practical.
- Buffer-size and sample-rate changes.
- Transport stop, all-notes-off, and panic behavior.
- UI open/close while audio is playing.

When installing local builds for Ableton, copy from:

- `build/SynthPlugin_artefacts/AU/Synth.component`
- `build/SynthPlugin_artefacts/VST3/Synth.vst3`

Record host failures with Ableton version, macOS version, plugin format, sample rate, buffer size, preset/state, and reproduction steps.

## Commit And Review Discipline

- Do not push unless explicitly asked.
- Keep commits focused and conventional, for example `docs: update agent guide`.
- Before committing, check `git status -sb` and make sure only intended files are staged.
- Do not revert or overwrite unrelated user or agent changes.
- Add regression coverage for DSP bugs when the behavior can be rendered or measured.
- For reviews, lead with bugs, realtime-safety risks, clean-room risks, host-state regressions, and missing validation.
- For user-visible behavior changes, update the relevant durable doc before or with code changes.
