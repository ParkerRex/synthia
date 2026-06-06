---
title: Validate Ableton Hosted AU Editor Reopen Control
status: completed
summary: Correct the previous hosted-AU reopen finding and prove Ableton can open, close, and reopen the hosted AU editor while transport runs.
post_build_recap: The fixed-size editor experiment was reverted. The original resizable editor build passed Release build, CTest, bundle checks, local install, and AU validation. In Ableton Live 11, a precise CoreGraphics click on the actual device-header wrench opened, closed, and reopened the hosted AU editor while transport was running. No source change was required.
triggers:
  - Reviewing hosted AU editor lifecycle proof.
  - Debugging Ableton plug-in editor reopen behavior.
  - Continuing Ableton host validation after the hosted UI lifecycle attempt.
---

# Validate Ableton Hosted AU Editor Reopen Control

This ExecPlan corrects the prior reopen-after-close conclusion from `2026-06-06-validate-ableton-hosted-ui-lifecycle-attempt.md`.

## Context

The previous Ableton hosted UI lifecycle attempt proved that the hosted AU editor can close while transport runs, but it reported reopen as failed. Follow-up investigation found the likely weak point: Ableton's custom device header exposes poor accessibility metadata, and earlier coordinate-based clicks were not landing on the actual plug-in editor wrench.

A fixed-size editor experiment was tried and then rejected. The control test restored the original resizable editor shell before rebuilding and reinstalling, so the proof does not depend on a source change.

## Completed Work

- [x] Reverted the temporary fixed-size editor experiment.
- [x] Rebuilt the original resizable editor build in `build-release-phase1-ableton`.
- [x] Ran CTest and plugin bundle checks.
- [x] Installed the local AU/VST3 bundles and reran AU validation.
- [x] Relaunched Ableton into a fresh `Untitled` set and loaded the AU entry for `sylenth-ai`.
- [x] Proved the hosted AU editor opens, closes, and reopens from Ableton's device-header wrench.
- [x] Proved the hosted AU editor opens, closes, and reopens while Ableton transport is running.
- [x] Updated durable docs so the remaining host matrix no longer carries hosted AU editor reopen as a gap.

## Findings

Ableton's device header is custom-drawn and not reliably inspectable through System Events. The correct action target was the plug-in editor wrench in the device header. A CoreGraphics click at screen point `{750, 1105}` hit the real control in the current screen layout.

The original resizable editor shell is valid for this path. No JUCE editor lifecycle fix was needed.

## Validation

Commands:

```bash
/opt/homebrew/bin/cmake --build build-release-phase1-ableton --config Release -j2
ctest --test-dir build-release-phase1-ableton --output-on-failure
scripts/check-plugin-bundles.sh build-release-phase1-ableton
scripts/install-local-plugins.sh build-release-phase1-ableton Release
auval -v aumu SyAI PkRx
```

Ableton proof:

- Fresh `Untitled` Ableton set loaded the AU row for `sylenth-ai`.
- Hosted editor window appeared as `sylenth-ai/1-sylenth-ai`.
- Closing the hosted editor left only the main `Untitled` Live window.
- Clicking the actual device-header wrench reopened `sylenth-ai/1-sylenth-ai`.
- With transport running, the hosted AU editor opened, closed, and reopened from the same wrench target.

Evidence screenshots are ignored build artifacts under `build/reports/ableton/`:

- `resizable-control-after-au-load.png`
- `resizable-control-after-close.png`
- `resizable-control-after-reopen.png`
- `resizable-control-running-editor-closed-before-open.png`
- `resizable-control-running-after-open.png`
- `resizable-control-running-after-close.png`
- `resizable-control-running-after-reopen.png`
- `resizable-control-running-after-final-stop.png`

## Remaining Gaps

- AU and VST3 automation record/playback.
- AU learned CC mapping proof in Ableton. Follow-on VST3 learned-CC, continuous value-application, Forget, and stepped-controller proof later passed.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.
- VST3 hosted editor lifecycle proof was later resolved by `2026-06-06-validate-ableton-vst3-hosted-editor-lifecycle.md`.

## Retrospective

The earlier reopen failure was a test automation problem, not a product bug. For Ableton device-header clicks, use screenshot crops plus CoreGraphics mouse events rather than System Events `click at` or broad coordinate grids.
