# Ableton Smoke Validation

Use this file when validating sylenth-ai on a Mac with Ableton installed. Commit filled-in notes only when they are useful project evidence; otherwise keep local scratch notes under `build/`.

Local install, uninstall, ad-hoc signing, AU validation, and Ableton rescan troubleshooting are documented in `docs/host-validation/local-install-troubleshooting.md`.

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
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSYLENTH_AI_ENABLE_TESTS=ON
cmake --build build-release --config Release
ctest --test-dir build-release --output-on-failure
./build-release/SylenthAIRender --suite core --output-dir build-release/reports/core
scripts/check-plugin-bundles.sh build-release
scripts/install-local-plugins.sh build-release
```

## Command Revalidation - 2026-06-05

Review-fix validation was run from a Release build on top of `9c27c07` with local working-tree fixes applied.

```bash
/opt/homebrew/bin/cmake --build build-release --config Release
/opt/homebrew/bin/ctest --test-dir build-release --output-on-failure
./build-release/SylenthAIRender --validate-presets presets/factory --output build-release/reports/presets-review.json
./build-release/SylenthAIRender --suite core --output-dir build-release/reports/core
scripts/check-plugin-bundles.sh build-release
scripts/install-local-plugins.sh build-release Release
auval -v aumu SyAI PkRx
```

Results:

- Release build, CTest, factory preset validation, and core suite passed.
- Bundle check passed and verified `Contents/Resources/factory/init.json` and `pluck-core-01.json` in Standalone, AU, and VST3 bundles.
- Installed AU and VST3 bundles under `~/Library/Audio/Plug-Ins`.
- `auval -v aumu SyAI PkRx` passed against the installed AU with AU Validation Tool `1.10.0`.
- No `vst3validator` or `pluginval` executable was available on this machine. At the time of this 2026-06-05 command-only pass, VST3 scan/load/play, automation, state restore, and bounce still required the Ableton manual checklist below.

## Command Revalidation - 2026-06-06

Current-build validation was run from branch `validate/phase1-ableton` on top of `a1ab086`.

```bash
cmake -S . -B build-release-phase1-ableton -DCMAKE_BUILD_TYPE=Release -DSYLENTH_AI_ENABLE_TESTS=ON -DSYLENTH_AI_JUCE_PATH=/Users/parkerrex/Developer/sylenth-ai/build/_deps/juce-src
cmake --build build-release-phase1-ableton --config Release -j2
ctest --test-dir build-release-phase1-ableton --output-on-failure
./build-release-phase1-ableton/SylenthAIRender --suite core --output-dir build-release-phase1-ableton/reports/core
scripts/check-plugin-bundles.sh build-release-phase1-ableton
scripts/install-local-plugins.sh build-release-phase1-ableton Release
scripts/uninstall-local-plugins.sh --dry-run
auval -v aumu SyAI PkRx
```

Results:

- Release configure/build passed with the existing local JUCE source path; a fresh build directory was used because `build-release` still had an old absolute source path from before the repo rename.
- CTest passed `5/5` tests, and `SylenthAIRender --suite core` wrote 11 reports.
- Bundle check passed for Standalone, AU, and VST3 universal `x86_64 arm64` artifacts with `com.parkerx.sylenth-ai` metadata and bundled factory presets.
- Local install copied and ad-hoc signed `~/Library/Audio/Plug-Ins/Components/sylenth-ai.component` and `~/Library/Audio/Plug-Ins/VST3/sylenth-ai.vst3`.
- Uninstall dry-run confirmed it would remove only the installed sylenth-ai AU/VST3 bundles and no legacy `Synth` bundles.
- `auval -v aumu SyAI PkRx` passed. It repeated the known non-fatal `Delay Feedback` maximum-value retention warning and ended with `AU VALIDATION SUCCEEDED`.
- No `vst3validator` or `pluginval` executable was available on this machine.

## Ableton Current-Build VST3 Smoke - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- build directory: `build-release-phase1-ableton`
- plugin format tested in this pass: VST3
- Ableton audio state visible during playback: 44100 Hz, 512-sample block

Results:

- Ableton initially showed the old `Synth` browser/device label because the running Live session had not rescanned since the project rename.
- A normal Preferences > Plug-Ins > Rescan updated `PluginScanner.txt`; Ableton found `sylenth-ai` at `~/Library/Audio/Plug-Ins/VST3/sylenth-ai.vst3` with `device-class-id` ending `?n=sylenth-ai`.
- Ableton's browser then showed `Plug-Ins > VST3 > ParkerX > sylenth-ai`.
- Double-clicking the browser entry created the current VST3. Ableton logged `Vst3: Going to create: sylenth-ai`, loaded ParkerX `sylenth-ai` v0.1.0 with class id `{ABCDEF01-9182-FAEB-506B-527853794149}`, reported 2427 parameters, and logged `Vst3: Created: sylenth-ai`.
- Playback against the existing MIDI clip ran with the created VST3 device present and Ableton track meters active.
- Correction from the 2026-06-06 transport/device pass: earlier screenshots from this smoke pass also showed a visible `sylenth-ai` standalone app window. That standalone window is not hosted-editor proof. Hosted editor open/close remains unproven in Ableton and stays in the remaining host-validation gaps.

Evidence screenshots are local build artifacts under `build/reports/ableton/`:

- `phase1-ableton-after-rescan-cgevent.png`
- `phase1-ableton-vst3-created.png`
- `phase1-ableton-vst3-transport-running.png`
- `phase1-ableton-vst3-transport-stopped.png`

Remaining host-validation gaps after this VST3 smoke pass:

- AU and VST3 automation record/playback.
- Learned CC mapping proof in Ableton.
- Current preset recreation and modulation exercise.
- Offline bounce comparison.
- Sample-rate and buffer-size changes.
- Transport stop/all-notes-off/panic proof.
- Hosted UI open/close while transport is running.

## Ableton Current-Build Restore Smoke - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: `/Users/parkerrex/Desktop/testing-synth Project/testing-synth.als`
- plugin formats tested in this pass: AU and VST3

Results:

- Ableton browser showed `Audio Units > ParkerX > sylenth-ai`; double-clicking the AU entry logged `Au: Going to create: sylenth-ai` at 02:29:54 and `Au: Created: sylenth-ai` at 02:29:56.
- Saving, quitting Ableton, and reopening the test set logged `Audio Unit: Going to restore: sylenth-ai` and `Audio Unit: Restored: sylenth-ai` at 02:30:57.
- A fresh VST3 instance was created after the AU restore pass. Ableton logged `Vst3: Going to create: sylenth-ai`, loaded ParkerX `sylenth-ai` v0.1.0, reported 2427 parameters, and logged `Vst3: Created: sylenth-ai` at 02:32:29.
- Saving, quitting Ableton, and reopening the test set again logged `Vst3: Going to restore: sylenth-ai`, loaded ParkerX `sylenth-ai` v0.1.0, reported 2427 parameters, and logged `Vst3: Restored: sylenth-ai` at 02:33:18.
- Post-restore transport playback from the clip start showed the Ableton transport running, the restored device present, active track meters, and 44100 Hz / 512-sample audio state.

Evidence screenshots are local build artifacts under `build/reports/ableton/`:

- `host-matrix-au-created.png`
- `host-matrix-reopen-restore.png`
- `host-matrix-vst3-track2-created.png`
- `host-matrix-reopen-vst3-restore.png`
- `host-matrix-post-restore-playback-from-start.png`

Remaining host-validation gaps after this restore smoke pass:

- AU and VST3 automation record/playback.
- Learned CC mapping proof in Ableton.
- Current preset recreation and modulation exercise.
- Offline bounce comparison.
- Sample-rate and buffer-size changes.
- Transport stop/all-notes-off/panic proof.
- Hosted UI open/close while transport is running.

## Ableton Current-Build Transport/Device Smoke - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: `/Users/parkerrex/Desktop/testing-synth Project/testing-synth.als`
- plugin format tested in this pass: VST3

Results:

- The test set restored the current VST3 again at 02:43:21. Ableton logged ParkerX `sylenth-ai` v0.1.0, 2427 parameters, and `Vst3: Restored: sylenth-ai`.
- The separate standalone `sylenth-ai` process was closed before the clean transport screenshots; only Ableton Live was running for the host proof.
- Keyboard transport start ran the restored VST3 clip with active track/device meters and 44100 Hz / 512-sample audio state visible.
- Keyboard transport stop halted playback with the restored VST3 device still present and no crash or visible host error.
- Attempting to open the hosted editor from the visible Ableton device controls did not create a hosted `sylenth-ai` window or process. Hosted UI open/close while transport runs remains unproven.

Evidence screenshots are local build artifacts under `build/reports/ableton/`:

- `transport-vst3-running-no-standalone.png`
- `transport-vst3-stopped-no-standalone.png`
- `host-editor-open-attempt.png`
- `host-editor-open-attempt-2.png`

Host-validation gaps after this slice:

- AU and VST3 automation record/playback.
- Learned CC mapping proof in Ableton.
- AU transport run/stop proof was later covered by `Ableton Current-Build AU Transport Smoke - 2026-06-06`.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.
- Hosted UI open/close while transport is running.

## Ableton Current-Build Offline Bounce Smoke - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: `/Users/parkerrex/Desktop/testing-synth Project/testing-synth.als`
- plugin format tested in this pass: VST3

Results:

- Export Audio/Video rendered the restored VST3 test set from the master output using Ableton's default export settings: WAV PCM enabled, MP3 enabled, 44100 Hz project/sample rate, 16-bit WAV.
- Ableton wrote ignored local artifacts under `build/reports/ableton/bounce/`.
- The WAV artifact is stereo 44.1 kHz 16-bit PCM, 60638 frames, 1.3750113378684807 seconds, peak `0.24182866908780176`, RMS `0.08581804864650411`, RMS dBFS `-21.32842729587825`, non-silent, and non-clipping.
- This proves offline bounce artifact creation and finite/non-silent output. It does not compare offline bounce against a captured realtime render.

Evidence artifacts are local build artifacts under `build/reports/ableton/bounce/`:

- `sylenth-ai-ableton-bounce-2026-06-06.wav`
- `sylenth-ai-ableton-bounce-2026-06-06.mp3`
- `sylenth-ai-ableton-bounce-2026-06-06.metrics.json`

Export-setting screenshots are local build artifacts under `build/reports/ableton/`:

- `export-dialog-open.png`
- `export-save-dialog.png`
- `export-bounce-after-save.png`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Learned CC mapping proof in Ableton.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.
- Hosted UI open/close while transport is running.

## Ableton Current-Build AU Transport Smoke - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: `/Users/parkerrex/Desktop/testing-synth Project/testing-synth.als`
- plugin format tested in this pass: AU
- Ableton audio state visible during playback: 44100 Hz, 512-sample block

Results:

- Ableton browser path `Audio Units > ParkerX > sylenth-ai` exposed the current AU entry. Moving selection to the AU child and pressing Return created the AU.
- Ableton logged `Au: Going to create: sylenth-ai` at `2026-06-06T03:34:09.979570` and `Au: Created: sylenth-ai` at `2026-06-06T03:34:10.005956`.
- AU creation opened a `sylenth-ai/1-sylenth-ai` editor window owned by Live. A process check during playback showed Ableton Live and no standalone `sylenth-ai` process.
- Transport start ran the MIDI clip with the AU editor showing active voices, moving peak level, MIDI count, 44100 Hz sample rate, 512-sample block size, and active Live meters.
- Transport stop halted playback with voices returning to `0`, peak returning to `-inf`, and no visible host crash.
- Live also logged `Vst3: couldn't get controller state of sylenth-ai: not implemented` at `2026-06-06T03:34:10.014440`. Keep that VST3 controller-state warning on the host-state watchlist; it did not block this AU transport smoke.
- This proves current-build AU create/play/stop behavior with a hosted editor window visible. It does not prove automation, learned CC mapping, preset/modulation exercise, offline-versus-realtime comparison, sample-rate/buffer changes, all-notes-off, panic, or explicit hosted editor close/reopen while transport is running. Follow-on hosted UI lifecycle validation later proved hosted AU editor open/close/reopen while transport runs.

Evidence screenshots are local build artifacts under `build/reports/ableton/`:

- `au-transport-audio-units-open.png`
- `au-transport-parkerx-expanded-3.png`
- `au-transport-au-return-load-attempt.png`
- `au-transport-running-with-editor.png`
- `au-transport-stopped-with-editor.png`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Learned CC mapping proof in Ableton.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Ableton Current-Build Hosted UI Lifecycle Attempt - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: `/Users/parkerrex/Desktop/testing-synth Project/testing-synth.als`
- plugin format tested in this pass: AU
- Ableton state at start: hosted `sylenth-ai/1-sylenth-ai` AU editor window visible, AU device active, no standalone `sylenth-ai` process

Results:

- Transport was started with the hosted AU editor visible; the editor remained open while Live playback continued.
- Closing button 1 of the hosted `sylenth-ai/1-sylenth-ai` window succeeded while transport was running. The only remaining Live window was `testing-synth  [testing-synth]`.
- After closing the hosted editor, Live continued running without a visible crash, and the process check showed Ableton Live plus Ableton Index, with no standalone `sylenth-ai` process.
- Reopen attempts did not restore the hosted editor: the AU device header plug-in edit button, corrected-coordinate double click, right-side device controls, `View > Plug-In Windows`, `Cmd+Option+P`, `Cmd+Option+Control+P`, Key Map assignment attempt, and a stopped-transport retry all left only the main Ableton set window visible.
- The latest Ableton log entries for `sylenth-ai` remained the 03:34 AU creation lines and the existing VST3 controller-state watchpoint; the reopen attempts did not add a new visible `sylenth-ai` Live log entry.
- Superseding correction from the hosted AU editor reopen control pass below: the failed reopen conclusion was caused by imprecise Ableton UI targeting. A precise CoreGraphics click on the actual device-header wrench reopened the hosted AU editor with the original resizable editor build, including while transport was running. No source change was required.

Evidence screenshots are local build artifacts under `build/reports/ableton/`:

- `hosted-ui-lifecycle-running-open.png`
- `hosted-ui-lifecycle-running-closed.png`
- `hosted-ui-lifecycle-keymap-mode-on.png`
- `hosted-ui-lifecycle-stopped-reopen-attempt.png`
- `hosted-ui-lifecycle-global-shortcut-attempts.png`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Learned CC mapping proof in Ableton.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Ableton Current-Build Hosted AU Editor Reopen Control - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: fresh Ableton `Untitled` validation set
- plugin format tested in this pass: AU
- build under test: `build-release-phase1-ableton`, rebuilt and installed after restoring the original resizable editor shell

Results:

- The fixed-size editor experiment was reverted before this control pass. `src/plugin/PluginEditor.cpp` again uses `setResizable(true, true)`, `setResizeLimits(1080, 760, 1800, 1320)`, and `setSize(1320, 940)`.
- Release build, CTest, bundle checks, local install, and `auval -v aumu SyAI PkRx` passed after the revert.
- Fresh Ableton launch plus browser search loaded the AU row for `sylenth-ai` and opened the hosted `sylenth-ai/1-sylenth-ai` editor window.
- With the editor closed, Ableton transport was started from the main Live window. The AU device stayed active while transport ran.
- A CoreGraphics click at screen point `{750, 1105}` hit the actual Ableton device-header wrench and opened the hosted AU editor while transport was running.
- Closing button 1 of the hosted `sylenth-ai/1-sylenth-ai` editor left only the main `Untitled` Live window. Repeating the same CoreGraphics wrench click reopened `sylenth-ai/1-sylenth-ai` while transport was still running.
- The durable result is that hosted AU editor open/close/reopen while transport runs is proven for the original resizable editor. The previous failed reopen attempt was an automation-targeting error, not a plugin lifecycle bug.

Evidence screenshots are local build artifacts under `build/reports/ableton/`:

- `resizable-control-after-au-load.png`
- `resizable-control-after-close.png`
- `resizable-control-after-reopen.png`
- `resizable-control-running-editor-closed-before-open.png`
- `resizable-control-running-after-open.png`
- `resizable-control-running-after-close.png`
- `resizable-control-running-after-reopen.png`
- `resizable-control-running-after-final-stop.png`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Learned CC mapping proof in Ableton.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Ableton Current-Build VST3 Hosted Editor Lifecycle - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: fresh Ableton `Untitled` validation set
- plugin format tested in this pass: VST3
- build under test: installed local `build-release-phase1-ableton` VST3 from the previous validation pass

Results:

- Ableton browser search exposed two `sylenth-ai` rows. The top VST3 row did not instantiate by Return or double-click in this UI state, but dragging the selected top row into the MIDI track device area loaded the VST3.
- Ableton logged `Vst3: Going to create: sylenth-ai` at `2026-06-06T04:53:46.797564` and `Vst3: Created: sylenth-ai` at `2026-06-06T04:53:46.892768`.
- Live also logged `Vst3: couldn't get controller state of sylenth-ai: not implemented` at `2026-06-06T04:53:46.940881`. Keep this on the VST3 host-state watchlist; it did not block VST3 create or hosted editor lifecycle behavior.
- VST3 creation opened a `sylenth-ai/1-sylenth-ai` editor window owned by Live. No standalone `sylenth-ai` process was used for this proof.
- Closing button 1 of the hosted editor left only the main `Untitled` Live window.
- With transport running, a CoreGraphics click at screen point `{750, 1105}` hit the Ableton device-header wrench and opened the hosted VST3 editor.
- Closing button 1 again left only the main `Untitled` Live window; repeating the same CoreGraphics wrench click reopened `sylenth-ai/1-sylenth-ai` while transport was still running.
- The durable result is that hosted VST3 editor open/close/reopen while transport runs is proven. This completes the current hosted editor lifecycle gap for both AU and VST3.

Evidence screenshots are local build artifacts under `build/reports/ableton/`:

- `vst3-lifecycle-after-drag-vst3-to-device-area.png`
- `vst3-lifecycle-open-after-load.png`
- `vst3-lifecycle-running-editor-closed-before-open.png`
- `vst3-lifecycle-running-after-open.png`
- `vst3-lifecycle-running-after-close.png`
- `vst3-lifecycle-running-after-reopen.png`
- `vst3-lifecycle-stopped-cleanup.png`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Learned CC mapping proof in Ableton.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Historical Ableton Setup - 2026-06-05

- [x] Enable Audio Units.
- [x] Enable VST3.
- [x] Rescan plug-ins after install.
- [x] Confirm `sylenth-ai` appeared as AU in the 2026-06-05 early smoke pass.
- [x] Confirm `sylenth-ai` appeared as VST3 in the 2026-06-05 early smoke pass.

## Historical AU Smoke - 2026-06-05

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

## Historical VST3 Smoke - 2026-06-05

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
VST3 should appear under Plug-Ins > VST3 > ParkerX > sylenth-ai after adding the user
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
