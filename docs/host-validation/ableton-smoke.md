# Ableton Smoke Validation

Use this file when validating Synth on a Mac with Ableton installed. Commit filled-in notes only when they are useful project evidence; otherwise keep local scratch notes under `build/`.

## Environment

- date: 2026-06-05 EDT
- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- repo commit: `9bb2942`
- build directory: `build-release`
- sample rate: not captured
- buffer size: not captured
- plugin format tested: AU and VST3

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

- [x] Enable Audio Units.
- [x] Enable VST3.
- [x] Rescan plug-ins after install.
- [x] Confirm `Synth` appears as AU.
- [x] Confirm `Synth` appears as VST3.

## AU Smoke

- [x] Load AU on a MIDI track.
- [x] Play notes and confirm finite audible output.
- [ ] Load or recreate `Pluck Core 01` state.
- [ ] Exercise mono, mono-legato, poly, unison, glide, velocity glide, ramp, and TransMod routing.
- [ ] Record and replay one parameter automation lane.
- [x] Save the Live set.
- [x] Close and reopen the Live set.
- [x] Confirm preset/state restore.
- [ ] Export a short bounce.

Notes:

```text
AU scanned after enabling Audio Units and rescanning. AU loaded on a MIDI track,
opened the placeholder editor, played the test MIDI notes with audible output,
and still loaded after Ableton quit/reopen of the saved set.

Preset recreation, full modulation-mode exercise, automation, and bounce were
not completed in this early host smoke pass.
```

## VST3 Smoke

- [x] Load VST3 on a MIDI track.
- [x] Play notes and confirm finite audible output.
- [ ] Load or recreate `Pluck Core 01` state.
- [ ] Exercise mono, mono-legato, poly, unison, glide, velocity glide, ramp, and TransMod routing.
- [ ] Record and replay one parameter automation lane.
- [x] Save the Live set.
- [x] Close and reopen the Live set.
- [x] Confirm preset/state restore.
- [ ] Export a short bounce.

Notes:

```text
VST3 appeared under Plug-Ins > VST3 > ParkerX > Synth after adding the user
VST3 folder, rescanning, and ad-hoc signing the installed bundle. VST3 loaded
on a MIDI track, opened the placeholder editor, played the test MIDI notes with
audible output, and still loaded after Ableton quit/reopen of the saved set.

Preset recreation, full modulation-mode exercise, automation, and bounce were
not completed in this early host smoke pass.
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
