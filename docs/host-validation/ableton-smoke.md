# Ableton Smoke Validation

Use this file when validating Synth on a Mac with Ableton installed. Commit filled-in notes only when they are useful project evidence; otherwise keep local scratch notes under `build/`.

## Environment

- date:
- machine:
- macOS version:
- Ableton version:
- repo commit:
- build directory:
- sample rate:
- buffer size:
- plugin format tested: AU / VST3

## Build Proof

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSYNTH_ENABLE_TESTS=ON
cmake --build build-release --config Release
ctest --test-dir build-release --output-on-failure
./build-release/SynthRender --suite core --output-dir build-release/reports/core
scripts/check-plugin-bundles.sh build-release
scripts/install-local-plugins.sh build-release
```

## Ableton Setup

- [ ] Enable Audio Units.
- [ ] Enable VST3.
- [ ] Rescan plug-ins after install.
- [ ] Confirm `Synth` appears as AU.
- [ ] Confirm `Synth` appears as VST3.

## AU Smoke

- [ ] Load AU on a MIDI track.
- [ ] Play notes and confirm finite audible output.
- [ ] Load or recreate `Pluck Core 01` state.
- [ ] Exercise mono, mono-legato, poly, unison, glide, velocity glide, ramp, and TransMod routing.
- [ ] Record and replay one parameter automation lane.
- [ ] Save the Live set.
- [ ] Close and reopen the Live set.
- [ ] Confirm preset/state restore.
- [ ] Export a short bounce.

Notes:

```text

```

## VST3 Smoke

- [ ] Load VST3 on a MIDI track.
- [ ] Play notes and confirm finite audible output.
- [ ] Load or recreate `Pluck Core 01` state.
- [ ] Exercise mono, mono-legato, poly, unison, glide, velocity glide, ramp, and TransMod routing.
- [ ] Record and replay one parameter automation lane.
- [ ] Save the Live set.
- [ ] Close and reopen the Live set.
- [ ] Confirm preset/state restore.
- [ ] Export a short bounce.

Notes:

```text

```

## Failures

For every host failure, record:

- plugin format:
- exact step:
- expected:
- actual:
- reproducibility:
- Ableton log path or relevant message:
- linked fix commit:
