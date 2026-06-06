---
title: Validate Ableton VST3 Hosted Editor Lifecycle
status: completed
summary: Prove the current VST3 hosted editor can open, close, and reopen in Ableton while transport runs.
post_build_recap: Ableton Live 11 created the current sylenth-ai VST3 after dragging the browser result into the MIDI track device area. Live opened the hosted `sylenth-ai/1-sylenth-ai` editor, and the editor closed and reopened from the device-header wrench while transport was running. The existing VST3 controller-state warning remains on the host-state watchlist but did not block the hosted editor lifecycle proof.
triggers:
  - Reviewing VST3 hosted editor lifecycle proof.
  - Continuing Ableton host validation after AU hosted editor reopen proof.
  - Debugging Ableton VST3 editor window behavior.
---

# Validate Ableton VST3 Hosted Editor Lifecycle

This ExecPlan closes the VST3 hosted editor lifecycle gap after the AU hosted editor reopen control pass.

## Context

The current Ableton matrix had AU hosted editor open/close/reopen proof, but VST3 hosted editor lifecycle was still open. The VST3 path also logs a controller-state warning on creation, so this slice separates hosted editor behavior from the host-state watchpoint.

## Completed Work

- [x] Created a fresh Ableton `Untitled` validation set.
- [x] Loaded the current `sylenth-ai` VST3 by dragging the top browser search result into the MIDI track device area.
- [x] Verified Ableton `Log.txt` recorded `Vst3: Going to create: sylenth-ai` and `Vst3: Created: sylenth-ai` at `2026-06-06T04:53:46`.
- [x] Confirmed the existing `Vst3: couldn't get controller state of sylenth-ai: not implemented` warning did not block creation or hosted editor display.
- [x] Captured hosted VST3 editor visibility.
- [x] Proved hosted VST3 editor open/close/reopen from Ableton's device-header wrench while transport runs.
- [x] Updated the remaining host-validation gap lists.

## Findings

Ableton's search result keyboard Return and double-click did not instantiate the VST3 in this UI state. Dragging the selected VST3 row into the MIDI track device area worked and produced the expected VST3 creation log lines.

The same CoreGraphics click target used for AU editor reopening, screen point `{750, 1105}` in this layout, hit the VST3 device-header wrench and reopened the hosted editor.

## Validation

Ableton evidence:

- VST3 create log lines:
  - `2026-06-06T04:53:46.797564: info: Vst3: Going to create: sylenth-ai`
  - `2026-06-06T04:53:46.892768: info: Vst3: Created: sylenth-ai`
- Watchpoint log line:
  - `2026-06-06T04:53:46.940881: info: Vst3: couldn't get controller state of sylenth-ai: not implemented`
- Window sequence:
  - hosted `sylenth-ai/1-sylenth-ai` editor visible after VST3 load,
  - closing the editor left only the main `Untitled` Live window,
  - with transport running, the device-header wrench opened the hosted editor,
  - closing it again left only `Untitled`,
  - repeating the wrench click reopened `sylenth-ai/1-sylenth-ai` while transport was still running.

Evidence screenshots are ignored build artifacts under `build/reports/ableton/`:

- `vst3-lifecycle-after-drag-vst3-to-device-area.png`
- `vst3-lifecycle-open-after-load.png`
- `vst3-lifecycle-running-editor-closed-before-open.png`
- `vst3-lifecycle-running-after-open.png`
- `vst3-lifecycle-running-after-close.png`
- `vst3-lifecycle-running-after-reopen.png`
- `vst3-lifecycle-stopped-cleanup.png`

## Remaining Gaps

- AU and VST3 automation record/playback.
- Learned CC mapping proof in Ableton.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Retrospective

VST3 hosted editor lifecycle works in Ableton once the VST3 device is instantiated. Keep using drag-to-device-area for this host when browser Return or double-click fails, and keep the VST3 controller-state warning separate from editor lifecycle validation.
