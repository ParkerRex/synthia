# Ableton Smoke Validation

Use this file when validating sylenth-ai on a Mac with Ableton installed. Commit filled-in notes only when they are useful project evidence; otherwise keep local scratch notes under `build/`.

Local install, uninstall, ad-hoc signing, AU validation, and Ableton rescan troubleshooting are documented in `docs/host-validation/local-install-troubleshooting.md`.

## Current Status Summary - 2026-06-07

Completed current-build proof:

- Release build, CTest, core suite, bundle checks, local install, and `auval -v aumu SyAI PkRx`.
- AU and VST3 scan/create/play plus AU and VST3 host state restore.
- VST3 transport run/stop, VST3 offline bounce artifact creation, and AU transport run/stop.
- AU and VST3 hosted editor open/close/reopen while transport runs.
- VST3 global MIDI Learn capture/persistence, continuous mapped value application, stepped mapped value application, and Forget.
- AU seeded MIDI controller map loading and continuous mapped value application.
- AU in-editor MIDI Learn capture and sidecar persistence.
- AU global-panel MIDI Forget and post-Forget CC non-application.
- VST3 all-notes-off, all-sound-off, and hosted Panic clearing sustained voices.
- AU all-notes-off, all-sound-off, and hosted Panic clearing sustained voices.
- AU sample-rate and buffer-size change handling.
- VST3 sample-rate and buffer-size change handling.
- Hosted VST3 `Arp Motion 01` factory preset editor-state inspection with visible modulation routes and FX state.
- Hosted AU `Arp Motion 01` factory preset editor-state inspection.
- Hosted AU and VST3 `Arp Motion 01` playback after preset load with active voices, MIDI count, output level, and Live meters.

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Rendered modulation-behavior comparison beyond route visibility.
- Offline bounce versus realtime comparison.

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
- VST3 learned-CC, continuous value-application, Forget, stepped-controller proof, AU seeded controller value proof, AU in-editor Learn capture, and AU global-panel MIDI Forget later passed.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- Transport stop proof later passed; AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.
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
- VST3 learned-CC, continuous value-application, Forget, stepped-controller proof, AU seeded controller value proof, AU in-editor Learn capture, and AU global-panel MIDI Forget later passed.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- Transport stop proof later passed; AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.
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
- VST3 learned-CC, continuous value-application, Forget, stepped-controller proof, AU seeded controller value proof, AU in-editor Learn capture, and AU global-panel MIDI Forget later passed.
- AU transport run/stop proof was later covered by `Ableton Current-Build AU Transport Smoke - 2026-06-06`.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.
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
- VST3 learned-CC, continuous value-application, Forget, stepped-controller proof, AU seeded controller value proof, AU in-editor Learn capture, and AU global-panel MIDI Forget later passed.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.
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
- This proves current-build AU create/play/stop behavior with a hosted editor window visible. At the time, it did not prove automation, learned CC mapping, preset/modulation exercise, offline-versus-realtime comparison, sample-rate/buffer changes, all-notes-off, panic, or explicit hosted editor close/reopen while transport was running. Follow-on validation later proved hosted AU editor open/close/reopen while transport runs, hosted AU/VST3 all-notes-off/all-sound-off/Panic clearing, hosted AU/VST3 sample-rate/buffer change handling, and hosted VST3 preset editor-state inspection.

Evidence screenshots are local build artifacts under `build/reports/ableton/`:

- `au-transport-audio-units-open.png`
- `au-transport-parkerx-expanded-3.png`
- `au-transport-au-return-load-attempt.png`
- `au-transport-running-with-editor.png`
- `au-transport-stopped-with-editor.png`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- VST3 learned-CC, continuous value-application, Forget, stepped-controller proof, AU seeded controller value proof, AU in-editor Learn capture, and AU global-panel MIDI Forget later passed.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.

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
- VST3 learned-CC, continuous value-application, Forget, stepped-controller proof, AU seeded controller value proof, AU in-editor Learn capture, and AU global-panel MIDI Forget later passed.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.

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
- VST3 learned-CC, continuous value-application, Forget, stepped-controller proof, AU seeded controller value proof, AU in-editor Learn capture, and AU global-panel MIDI Forget later passed.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.

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
- VST3 learned-CC, continuous value-application, Forget, stepped-controller proof, AU seeded controller value proof, AU in-editor Learn capture, and AU global-panel MIDI Forget later passed.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.

## Ableton Current-Build VST3 Controller Learn Proof - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: fresh Ableton `Untitled` validation set
- plugin format tested in this pass: VST3
- build under test: installed local `build-release-phase1-ableton` VST3 from the previous validation pass

Results:

- A temporary CoreMIDI source named `SylenthAI Codex CC Source` was created before Ableton launch.
- Ableton logged `MidiInDevice [Name="SylenthAI Codex CC Source", Track=true, Sync=false, Remote=false, MPE=false]` during startup.
- The current `sylenth-ai` VST3 was loaded by dragging the top browser result into the MIDI track device area. Ableton logged `Vst3: Going to create: sylenth-ai` and `Vst3: Created: sylenth-ai`.
- `All Ins` did not pass the first validation source into the plugin. Explicitly selecting `SylenthAI Codex CC Source` on track 1 made the hosted plugin footer MIDI count advance and moved the Ableton meters.
- In the hosted editor, the MIDI CONTROL panel selected `FILTER / Resonance`, armed Learn, captured incoming CC71, and displayed `Mapped CC71 to filter.resonance`.
- The user-level sidecar was absent before this proof, then the plugin wrote `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json` with:

```json
{"schema_version": 1, "mappings": [{"cc": 71, "parameter_id": "filter.resonance"}]}
```

- The proof sidecar was copied to `build/reports/ableton/midi-controller-map-vst3-proof.json`, then the temporary user-level sidecar was removed to avoid polluting future validation.
- A short Arrangement record attempt from the CC/note stream produced live MIDI input and learned-assignment persistence, but Arrangement view showed no captured clip or envelope. AU/VST3 automation record/playback remains open. Later VST3 continuous value-application and Forget/stepped proofs showed the persisted controller map driving continuous and choice parameters and being cleared from the hosted panel; later AU proofs showed a seeded AU controller map driving a continuous parameter and the hosted AU editor capturing a fresh learned CC.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `live-relaunched-with-virtual-midi-source.png`
- `controller-proof-vst3-loaded-with-virtual-source.png`
- `midi-input-menu-open.png`
- `midi-input-source-selected-enter.png`
- `midi-after-second-virtual-source.png`
- `midi-learn-vst3-resonance-mapped.png`
- `midi-controller-map-vst3-proof.json`
- `arrangement-after-editor-closed.png`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.

## Ableton Current-Build VST3 Continuous Controller Value Proof - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: fresh Ableton `Untitled` validation set
- plugin format tested in this pass: VST3
- build under test: installed local `build-release-phase1-ableton` VST3 from the previous validation pass

Results:

- The validation pass seeded `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json` with `CC71 -> filter.resonance` before plugin construction.
- A temporary CoreMIDI source named `SylenthAI Codex Value Source` was created before Ableton launch.
- Ableton logged `MidiInDevice [Name="SylenthAI Codex Value Source", Track=true, Sync=false, Remote=false, MPE=false]` during startup.
- The current `sylenth-ai` VST3 was loaded by dragging the top browser result into the MIDI track device area. Ableton logged `Vst3: Going to create: sylenth-ai` and `Vst3: Created: sylenth-ai`.
- The hosted MIDI CONTROL panel displayed `Loaded 1 MIDI CC mappings` and `CC71 -> filter.resonance`.
- With the hosted editor scrolled to the Filter section, CC71 value `0` showed continuous `Resonance 0.00`, CC71 value `127` showed `Resonance 1.00`, and CC71 value `0` returned the readout to `Resonance 0.00`.
- The proof sidecar was copied to `build/reports/ableton/midi-controller-map-vst3-value-proof.json`, then the temporary user-level sidecar was removed to avoid polluting future validation.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `value-proof-live-launched.png`
- `value-proof-vst3-loaded.png`
- `value-proof-editor-scrolled-filter.png`
- `value-proof-cc71-high-resonance.png`
- `value-proof-cc71-low-resonance.png`
- `midi-controller-map-vst3-value-proof.json`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.

## Ableton Current-Build VST3 Controller Forget And Stepped Proof - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: fresh Ableton `Untitled` validation set
- plugin format tested in this pass: VST3
- build under test: installed local `build-release-phase1-ableton` VST3 from the previous validation pass

Results:

- The validation pass seeded `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json` with `CC73 -> filter.mode` before plugin construction.
- A temporary CoreMIDI source named `SylenthAI Codex Stepped Source` was created before Ableton launch.
- Ableton logged `MidiInDevice [Name="SylenthAI Codex Stepped Source", Track=true, Sync=false, Remote=false, MPE=false]` during startup.
- The current `sylenth-ai` VST3 was loaded by dragging the top browser result into the MIDI track device area. Ableton logged `Vst3: Going to create: sylenth-ai` and `Vst3: Created: sylenth-ai`.
- The hosted MIDI CONTROL panel displayed `Loaded 1 MIDI CC mappings` and `CC73 -> filter.mode`.
- With the hosted editor scrolled to the Filter section, CC73 value `127` changed Filter Mode to `Notch4`, then CC73 value `0` changed it to `L2`.
- Reopening the hosted editor reset the view to the MIDI CONTROL panel. The panel selected `FILTER / Filter Mode`, then Forget displayed `Forgot MIDI CC for filter.mode` and `No MIDI CC mappings`.
- The post-Forget sidecar was copied to `build/reports/ableton/midi-controller-map-vst3-forget-stepped-after-forget.json` and contained `{"schema_version": 1, "mappings": []}`.
- After Forget, sending CC73 value `127` still advanced the hosted editor MIDI count, but Filter Mode remained `L2`.
- The temporary user-level sidecar and CC value file were removed, both temporary Swift CoreMIDI helper processes were stopped, and Live was quit without saving the validation set.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `forget-stepped-live-launched.png`
- `forget-stepped-vst3-loaded.png`
- `forget-stepped-filter-scrolled-low.png`
- `forget-stepped-filter-mode-high.png`
- `forget-stepped-filter-mode-low-after-high.png`
- `forget-stepped-editor-reopened-for-forget.png`
- `forget-stepped-filter-mode-selected-before-forget-keyboard.png`
- `forget-stepped-after-forget.png`
- `forget-stepped-after-forget-cc73-high-no-change.png`
- `forget-stepped-ableton-log-excerpt.txt`
- `midi-controller-map-vst3-forget-stepped-seeded.json`
- `midi-controller-map-vst3-forget-stepped-after-forget.json`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.

## Ableton Current-Build AU Controller Value Proof - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: fresh Ableton `Untitled` validation set
- plugin format tested in this pass: AU
- build under test: installed local current-build AU from the previous Ableton validation passes

Results:

- The validation pass seeded `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json` with `CC72 -> filter.resonance` before plugin construction.
- A temporary CoreMIDI source named `SylenthAI Codex AU Value Source` was created before Ableton launch.
- Ableton logged `MidiInDevice [Name="SylenthAI Codex AU Value Source", Track=true, Sync=false, Remote=false, MPE=false]` during startup.
- The current `sylenth-ai` AU was loaded from `Audio Units > ParkerX > sylenth-ai` into a fresh `Untitled` set. Ableton logged `Au: Going to create: sylenth-ai` at `2026-06-06T14:44:51.525961` and `Au: Created: sylenth-ai` at `2026-06-06T14:44:51.698979`.
- The hosted MIDI CONTROL panel displayed `Loaded 1 MIDI CC mappings` and `CC72 -> filter.resonance`.
- With the hosted editor scrolled to the Filter section, the initial state showed `Resonance 0.00`, CC72 value `127` changed the readout to `Resonance 1.00`, and CC72 value `0` returned it to `Resonance 0.00`.
- The temporary user-level sidecar was copied to ignored local evidence, then removed to avoid polluting future validation.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `au-value-proof-live-fresh-launched.png`
- `au-value-proof-au-loaded-after-return.png`
- `au-value-proof-filter-visible-before-cc-2.png`
- `au-value-proof-cc72-high-resonance.png`
- `au-value-proof-cc72-low-resonance.png`
- `midi-controller-map-au-value-proof-current.json`
- `au-value-proof-ableton-log-excerpt.txt`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.

## Ableton Current-Build AU Controller Learn Proof - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: fresh Ableton `Untitled` validation set
- plugin format tested in this pass: AU
- build under test: installed local current-build AU from the previous Ableton validation passes

Results:

- The validation pass started with no `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json`.
- A temporary CoreMIDI source named `SylenthAI Codex AU Learn Source` was created before Ableton launch.
- Ableton logged `MidiInDevice [Name="SylenthAI Codex AU Learn Source", Track=true, Sync=false, Remote=false, MPE=false]` during startup.
- The current `sylenth-ai` AU was loaded from `Audio Units > ParkerX > sylenth-ai` into a fresh `Untitled` set. Ableton logged `Au: Going to create: sylenth-ai` at `2026-06-06T15:09:19.460935` and `Au: Created: sylenth-ai` at `2026-06-06T15:09:19.475730`.
- The hosted MIDI CONTROL panel started with `MIDI learn ready`, `No MIDI CC mappings`, and footer `MIDI 0`.
- The panel selected `FILTER / Resonance`, armed Learn, then captured CC74 value `99` from the temporary source.
- The hosted panel displayed `Mapped CC74 to filter.resonance` and `CC74 -> filter.resonance`, with footer `MIDI 1`.
- The temporary user sidecar persisted `{"schema_version":1,"mappings":[{"cc":74,"parameter_id":"filter.resonance"}]}` and was copied to ignored local evidence before cleanup.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `au-learn-proof-live-fresh-launched.png`
- `au-learn-proof-au-loaded-no-map.png`
- `au-learn-proof-resonance-selected.png`
- `au-learn-proof-resonance-armed.png`
- `au-learn-proof-cc74-mapped.png`
- `midi-controller-map-au-learn-proof.json`
- `au-learn-proof-ableton-log-excerpt.txt`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.

## Ableton Current-Build AU Controller Forget Proof - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: fresh Ableton `Untitled` validation set
- plugin format tested in this pass: AU
- build under test: Release build `32638d5`, installed from `build-release-host-matrix`

Build/install validation before host proof:

- `cmake -S . -B build-release-host-matrix -DCMAKE_BUILD_TYPE=Release -DSYLENTH_AI_ENABLE_TESTS=ON -DSYLENTH_AI_JUCE_PATH=/Users/parkerrex/Developer/sylenth-ai/build/_deps/juce-src`
- `cmake --build build-release-host-matrix --config Release -j2`
- `ctest --test-dir build-release-host-matrix --output-on-failure`
- `./build-release-host-matrix/SylenthAIRender --suite core --output-dir build-release-host-matrix/reports/core`
- `scripts/check-plugin-bundles.sh build-release-host-matrix`
- `scripts/install-local-plugins.sh build-release-host-matrix Release`
- `auval -v aumu SyAI PkRx`

Results:

- The validation pass seeded `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json` with `CC72 -> filter.resonance` before plugin construction.
- A temporary CoreMIDI source named `SylenthAI Codex AU Forget Source` was created before Ableton launch.
- Ableton logged `MidiInDevice [Name="SylenthAI Codex AU Forget Source", Track=true, Sync=false, Remote=false, MPE=false]` during startup.
- The current `sylenth-ai` AU was loaded from `Audio Units > ParkerX > sylenth-ai` into a fresh `Untitled` set. Ableton logged `Au: Going to create: sylenth-ai` at `2026-06-06T21:05:08.955600` and `Au: Created: sylenth-ai` at `2026-06-06T21:05:10.064453`.
- Before Forget, sending CC72 value `127` changed the hosted Filter section readout to `Resonance 1.00`, and sending CC72 value `0` returned it to `Resonance 0.00`.
- The hosted MIDI CONTROL panel displayed `Loaded 1 MIDI CC mappings` and `CC72 -> filter.resonance`.
- The panel selected `FILTER / Resonance`, then Forget displayed `Forgot MIDI CC for filter.resonance` and `No MIDI CC mappings`.
- The post-Forget sidecar was copied to `build/reports/ableton/midi-controller-map-au-forget-proof-after-forget.json` and contained `{"schema_version": 1, "mappings": []}`.
- After Forget, sending CC72 value `127` still advanced the hosted editor MIDI count, but Filter Resonance remained `0.00`.
- The temporary user sidecar was removed, the CoreMIDI source was stopped, and Live was quit without saving the validation set.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `midi-controller-map-au-forget-proof-seeded.json`
- `midi-controller-map-au-forget-proof-after-forget.json`
- `au-forget-proof-ableton-log-excerpt.txt`
- `au-forget-proof-before-forget-cc72-high.png`
- `au-forget-proof-before-forget-cc72-low.png`
- `au-forget-proof-filter-resonance-selected.png`
- `au-forget-proof-after-forget-panel.png`
- `au-forget-proof-after-forget-cc72-high.png`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.
- AU and VST3 all-notes-off/all-sound-off and hosted Panic proof later passed.

## Ableton Current-Build VST3 All-Notes/Panic Proof - 2026-06-06

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: restored `testing-synth [testing-synth]` validation set
- plugin format tested in this pass: VST3
- build under test: installed local current-build VST3 from `build-release-host-matrix`; no plugin source changes were made during this proof
- MIDI source: temporary CoreMIDI source `SylenthAI Codex Panic Source`

Build/install validation before host proof:

- `cmake --build build-release-host-matrix --config Release -j2`
- `ctest --test-dir build-release-host-matrix --output-on-failure`
- `./build-release-host-matrix/SylenthAIRender --suite core --output-dir build-release-host-matrix/reports/core`
- `scripts/check-plugin-bundles.sh build-release-host-matrix`
- `scripts/install-local-plugins.sh build-release-host-matrix Release`
- `auval -v aumu SyAI PkRx`

Results:

- Release build, CTest, core suite, bundle check, local install, and `auval` passed before the host proof.
- Ableton restored the installed VST3 and logged ParkerX `sylenth-ai` v0.1.0.
- Ableton exposed `SylenthAI Codex Panic Source` as a track-enabled MIDI input.
- With two sustained notes held from the temporary source, the hosted editor showed `Voices 2/8`, MIDI input count advancing, active Ableton meters, and a finite output peak.
- Sending MIDI CC123 all-notes-off cleared the hosted editor to `Voices 0/8`.
- Holding two fresh notes and sending MIDI CC120 all-sound-off cleared the hosted editor to `Voices 0/8`.
- Holding two fresh notes and clicking the hosted editor `Panic` button cleared the hosted editor to `Voices 0/8`; the footer reported `Panic sent: voices and FX cleared`.
- The temporary MIDI source was sent CC123, CC120, note-offs, and then quit. Live remained silent. A clean quit request did not close Live within the short validation window, so it was left running rather than force-quit.
- A Live crash-report sidebar is visible in the screenshots because an earlier validation cleanup force-quit produced a crash report. It did not block this proof.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `panic-proof-vst3-ready-routed.png`
- `panic-proof-vst3-held-before-cc123.png`
- `panic-proof-vst3-after-cc123-all-notes-off.png`
- `panic-proof-vst3-held-before-cc120.png`
- `panic-proof-vst3-after-cc120-all-sound-off.png`
- `panic-proof-vst3-held-before-panic.png`
- `panic-proof-vst3-after-panic-button.png`
- `panic-proof-vst3-ableton-log-excerpt.txt`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.

## Ableton Current-Build AU All-Notes/Panic Proof - 2026-06-07

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: restored `testing-synth [testing-synth]` validation set
- plugin format tested in this pass: AU
- build under test: installed local current-build AU from `build-release-host-matrix`; no plugin source changes were made during this proof
- MIDI source: temporary CoreMIDI source `SylenthAI Codex Panic Source`

Results:

- Ableton loaded the current AU from `Audio Units > ParkerX > sylenth-ai` and logged `Au: Going to create: sylenth-ai` then `Au: Created: sylenth-ai`.
- Ableton exposed `SylenthAI Codex Panic Source` as a track-enabled MIDI input.
- With two sustained notes held from the temporary source, the hosted AU editor showed `Voices 2/8`, MIDI input count advancing, active Ableton meters, and a finite output peak.
- Sending MIDI CC123 all-notes-off cleared the hosted AU editor to `Voices 0/8`.
- Holding two fresh notes and sending MIDI CC120 all-sound-off cleared the hosted AU editor to `Voices 0/8`.
- Holding two fresh notes and clicking the hosted AU editor `Panic` button cleared the editor to `Voices 0/8`; the footer reported `Panic sent: voices and FX cleared`.
- The temporary MIDI source was sent CC123, CC120, note-offs, and then quit.
- A Live Preferences window appeared behind the hosted editor during the proof; it did not block the AU editor controls or invalidate the proof.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `au-panic-proof-audio-units-key-expanded.png`
- `au-panic-proof-parkerx-expanded.png`
- `au-panic-proof-after-au-load-return.png`
- `au-panic-proof-held-before-cc123.png`
- `au-panic-proof-after-cc123-all-notes-off.png`
- `au-panic-proof-held-before-cc120.png`
- `au-panic-proof-after-cc120-all-sound-off.png`
- `au-panic-proof-held-before-panic-live-activated.png`
- `au-panic-proof-after-panic-button-retry.png`
- `au-panic-proof-ableton-log-excerpt.txt`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- AU and VST3 sample-rate/buffer change proof later passed.

## Ableton Current-Build AU Sample-Rate/Buffer Proof - 2026-06-07

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: restored `testing-synth [testing-synth]` validation set
- plugin format tested in this pass: AU
- build under test: installed local current-build AU from `build-release-host-matrix`; no plugin source changes were made during this proof

Results:

- Baseline hosted AU diagnostics showed `SR 44100 Hz` and `Block 512`.
- Ableton Preferences > Audio changed the host sample rate from `44100` to `48000`.
- Ableton Preferences > Audio changed the host buffer size from `512 Samples` to `256 Samples`.
- Reopening the existing hosted AU editor showed diagnostics updated to `SR 48000 Hz` and `Block 256`.
- The hosted AU editor remained open, responsive, finite, and ready after the sample-rate and buffer-size changes.
- Ableton audio settings were restored to `44100` and `512 Samples` after the proof.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `sample-buffer-baseline.png`
- `sample-buffer-48000-selected.png`
- `sample-buffer-48000-256-selected.png`
- `sample-buffer-plugin-footer-48000-256-hotswap-cleared.png`
- `sample-buffer-restored-44100-512-confirmed-2.png`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.
- VST3 sample-rate and buffer-size change proof later passed.

## Ableton Current-Build VST3 Sample-Rate/Buffer Proof - 2026-06-07

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: restored `testing-synth [testing-synth]` validation set
- plugin format tested in this pass: VST3
- build under test: installed local current-build VST3 from `build-release-host-matrix`; no plugin source changes were made during this proof

Results:

- Loaded a fresh VST3 instance from `Plug-Ins > VST3 > ParkerX > sylenth-ai` onto track 2; Ableton logged `Vst3: Going to create: sylenth-ai` and `Vst3: Created: sylenth-ai`.
- Baseline hosted VST3 diagnostics showed `SR 44100 Hz` and `Block 512`.
- Ableton Preferences > Audio changed the host sample rate from `44100` to `48000`.
- Ableton Preferences > Audio changed the host buffer size from `512 Samples` to `256 Samples`.
- Reopening the hosted VST3 editor showed diagnostics updated to `SR 48000 Hz` and `Block 256`.
- The hosted VST3 editor remained open, responsive, finite, and ready after the sample-rate and buffer-size changes.
- Ableton audio settings were restored to `44100` and `512 Samples` after the proof.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `vst3-sample-buffer-vst3-parkerx-expanded.png`
- `vst3-sample-buffer-baseline.png`
- `vst3-sample-buffer-48000-selected.png`
- `vst3-sample-buffer-48000-256-selected.png`
- `vst3-sample-buffer-plugin-footer-48000-256.png`
- `vst3-sample-buffer-restored-44100-512-confirmed.png`
- `vst3-sample-buffer-ableton-log-excerpt.txt`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Hosted AU/VST3 Arp Motion 01 preset editor-state proof and hosted VST3 playback-after-load proof later passed; AU playback after preset load later passed; rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison.

## Ableton Current-Build VST3 Preset/Modulation Editor-State Proof - 2026-06-07

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: restored `testing-synth [testing-synth]` validation set
- plugin format tested in this pass: VST3
- build under test: installed local current-build VST3 from `build-release-host-matrix`; no plugin source changes were made during this proof

Results:

- Opened the hosted VST3 editor on track 2.
- The hosted preset menu listed `Factory / Arps - Arp Motion 01 [arp, chord]`.
- Selecting `Arp Motion 01` and clicking `Load` changed the hosted editor from Init to the factory patch and the footer reported `Loaded preset: Arp Motion 01`.
- The hosted Sound page showed patch-specific oscillator, filter, LFO, macro, and patch-load estimate state.
- The hosted Modulation page showed `ACTIVE ROUTES (2)`, with `Ramp -> Filter Cutoff x Velocity +12.0 st` and `LFO 1 -> Amp Level x Macro Motion +3.0 dB`; TransMod 1 and 2 were enabled with non-default sources/scalers.
- The hosted Effects page showed patch-specific FX state with global FX enabled, active Saturation, Delay, and Reverb, and quality controls visible.
- This pass did not prove AU preset loading, playback after the preset load, or modulation behavior in rendered Ableton audio. The later preset load/playback proof below closes AU preset editor-state loading and hosted VST3 playback after load only.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `preset-modulation-vst3-editor-init.png`
- `preset-modulation-preset-menu-open.png`
- `preset-modulation-arp-motion-selected-before-load.png`
- `preset-modulation-arp-motion-loaded-sound.png`
- `preset-modulation-arp-motion-modulation-page.png`
- `preset-modulation-arp-motion-effects-page.png`

Supporting standalone preset render validation also passed: `./build/SylenthAIRender --suite patch-recreation --output-dir build/reports/patch-recreation-ableton-preset-proof` wrote six reports under `build/reports/patch-recreation-ableton-preset-proof/`.

Remaining host-validation gaps after this VST3 editor-state proof:

- AU and VST3 automation record/playback.
- AU preset load, playback after preset load, and Ableton-side modulation behavior exercise.
- Offline bounce versus realtime comparison.

## Ableton Current-Build Preset Load/Playback Proof - 2026-06-07

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: restored `testing-synth [testing-synth]` validation set
- plugin formats tested in this pass: AU and VST3
- build under test: installed local current-build AU/VST3 from `build-release-host-matrix`; no plugin source changes were made during this proof

Results:

- Hosted VST3 on track 2 already had `Factory / Arps - Arp Motion 01 [arp, chord]` loaded.
- Launching the track 2 validation MIDI clip after the preset load drove the hosted VST3: the editor showed `1/8` voices, `Peak -15.9 dB`, `MIDI 212`, and active Live track/master meters.
- Opening the VST3 Modulation page during the same playback showed `ACTIVE ROUTES (2)` with `Ramp -> Filter Cutoff x Velocity +12.0 st` and `LFO 1 -> Amp Level x Macro Motion +3.0 dB`, while the editor showed `2/8` voices, `Peak -17.4 dB`, and `MIDI 832`.
- Stopping transport returned the hosted VST3 to `0/8` voices.
- Hosted AU on track 1 opened as `sylenth-ai/1-sylenth-ai`; its preset menu listed `Factory / Arps - Arp Motion 01 [arp, chord]`.
- Selecting and loading `Arp Motion 01` changed the AU editor from Init to the factory patch and the footer reported `Loaded preset: Arp Motion 01`.
- Track 1 AU playback after preset load remained open in this pass; the existing track 1 slot did not provide the same validation MIDI clip. The later AU preset playback proof below closes that gap with a temporary CoreMIDI source.
- This pass proves VST3 loaded-preset playback and route visibility during playback, not a rendered modulation-behavior comparison.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `vst3-preset-playback-after-load.png`
- `vst3-preset-playback-modulation-page.png`
- `vst3-preset-playback-stopped-cleanup-2.png`
- `au-preset-playback-menu-open.png`
- `au-preset-playback-arp-selected-before-load.png`
- `au-preset-playback-arp-loaded-sound.png`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Rendered modulation-behavior comparison beyond route visibility.
- Offline bounce versus realtime comparison.

## Ableton Current-Build AU Preset Playback Proof - 2026-06-07

Environment:

- machine: rex, MacBook Pro `MacBookPro18,2`, Apple M1 Max, 64 GB
- macOS version: 26.5 `25F71`
- Ableton version: Live 11 Suite `11.0.12 (2021-11-04_b232c5df34)`
- Live set: restored `testing-synth [testing-synth]` validation set
- plugin format tested in this pass: AU
- build under test: installed local current-build AU from `build-release-host-matrix`; no plugin source changes were made during this proof
- MIDI source: temporary CoreMIDI source `SylenthAI Codex AU Playback Source 2`

Results:

- Hosted AU on track 1 already had `Factory / Arps - Arp Motion 01 [arp, chord]` loaded.
- Track 1 was routed from `All Ins` with Monitor set to `In`.
- A temporary CoreMIDI source repeatedly sent note-ons into the hosted AU, then sent note-offs plus CC123 all-notes-off before exiting.
- During the note stream, the hosted AU editor showed `2/8` voices, `Peak -21.0 dB`, `MIDI 683`, and active Live track/master meters.
- After the same source pass, the hosted AU Modulation page showed `ACTIVE ROUTES (2)` with the Ramp and LFO routes and the MIDI counter advanced to `687`.
- This pass proves AU loaded-preset playback, not a rendered modulation-behavior comparison.

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `au-preset-playback-ready-for-midi-source.png`
- `au-preset-playback-midi-source-held-2.png`
- `au-preset-playback-modulation-page.png`

Remaining host-validation gaps:

- AU and VST3 automation record/playback.
- Rendered modulation-behavior comparison beyond route visibility.
- Offline bounce versus realtime comparison.

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
