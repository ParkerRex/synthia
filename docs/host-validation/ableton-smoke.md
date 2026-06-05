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

## Command Revalidation - 2026-06-05

Review-fix validation was run from a Release build on top of `9c27c07` with local working-tree fixes applied.

```bash
/opt/homebrew/bin/cmake --build build-release --config Release
/opt/homebrew/bin/ctest --test-dir build-release --output-on-failure
./build-release/SynthRender --validate-presets presets/factory --output build-release/reports/presets-review.json
./build-release/SynthRender --suite core --output-dir build-release/reports/core
scripts/check-plugin-bundles.sh build-release
scripts/install-local-plugins.sh build-release Release
auval -v aumu Syn1 PkRx
```

Results:

- Release build, CTest, factory preset validation, and core suite passed.
- Bundle check passed and verified `Contents/Resources/factory/init.json` and `pluck-core-01.json` in Standalone, AU, and VST3 bundles.
- Installed AU and VST3 bundles under `~/Library/Audio/Plug-Ins`.
- `auval -v aumu Syn1 PkRx` passed against the installed AU with AU Validation Tool `1.10.0`.
- No `vst3validator` or `pluginval` executable was available on this machine; VST3 scan/load/play, automation, state restore, and bounce still require the Ableton manual checklist below.

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
