---
title: Validate Ableton Hosted UI Lifecycle Attempt
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Prove hosted AU editor close while transport runs and record the first reopen-after-close attempt in Ableton.
post_build_recap: Ableton Live 11 kept playback running while the hosted sylenth-ai AU editor was visible, and closing the hosted editor succeeded during transport. Reopen appeared to fail in this pass, but `2026-06-06-validate-ableton-hosted-au-editor-reopen-control.md` later proved that this was an automation-targeting miss rather than a plugin lifecycle bug.
read_when:
  - Reviewing hosted AU editor lifecycle proof.
  - Debugging Ableton plug-in editor reopen behavior.
  - Continuing the Phase 1 host validation matrix.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Validate Ableton Hosted UI Lifecycle Attempt

This ExecPlan records the first current-build hosted AU editor close/reopen attempt in Ableton after the AU transport proof.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The Ableton host matrix still required explicit hosted UI close/reopen while transport runs. The previous AU transport slice proved the hosted editor could be visible during playback, but did not close and reopen that editor.

This slice starts from the hosted `sylenth-ai/1-sylenth-ai` AU editor window, starts transport, closes the hosted editor while Live continues playback, and then attempts to reopen the hosted editor through Ableton's available plug-in window paths.

This is a completed validation attempt. Close while transport runs passed. A later control pass proved reopen after close with a precise CoreGraphics click on Ableton's device-header wrench.

## Progress

- [x] Captured hosted AU editor visible while transport was running.
- [x] Closed the hosted AU editor while transport was running.
- [x] Confirmed the main Ableton set stayed open and no standalone `sylenth-ai` process appeared.
- [x] Attempted reopen through the device header plug-in edit button, corrected-coordinate double click, and right-side device controls.
- [x] Attempted reopen through `View > Plug-In Windows`, global plug-in-window shortcuts, Key Map workflow, and a stopped-transport retry.
- [x] Updated host validation notes and Program/index docs with the pass/fail split.

## Surprises & Discoveries

Ableton's custom-drawn device strip does not expose useful button metadata through the accessibility tree. Coordinate targeting needed screenshot crops. An intermediate coordinate grid accidentally toggled the AU device activator; Ableton Undo restored the device before the final reopen attempts, and those intermediate screenshots are not used as evidence.

Ableton's `View > Plug-In Windows` menu item was disabled after the hosted editor was closed. The documented global plug-in-window shortcuts also did not restore the closed editor.

The device header wrench remained visible after close, but the clicks used in this pass did not hit the correct control reliably. A later CoreGraphics control pass hit the actual wrench hotspot and reopened the hosted editor.

## Decision Log

Decision: Count hosted editor close while transport runs as proven.
Rationale: The hosted editor window closed during playback, the main Ableton set stayed open, and no standalone `sylenth-ai` process was present.
Date: 2026-06-06.

Decision: Treat this reopen failure as superseded by the control pass.
Rationale: `2026-06-06-validate-ableton-hosted-au-editor-reopen-control.md` proved hosted AU editor reopen with the original resizable editor after correcting the Ableton device-header click target.
Date: 2026-06-06.

Decision: Do not treat coordinate-grid screenshots as validation evidence.
Rationale: One grid hit the AU device activator; the device was restored with Ableton Undo before the final evidence set.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed as a validation attempt that later proved useful for isolating a test-targeting issue. The current AU hosted editor can be visible during playback and can close while transport runs. Reopen appeared to fail through the tested host paths in this pass, but the later control pass proved hosted AU editor reopen with no source change.

## Context and Orientation

Read first:

- `docs/host-validation/ableton-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-au-transport-smoke.md`

### In Scope

- Hosted AU editor visibility during playback.
- Hosted AU editor close while playback continues.
- Reopen attempts using Ableton plug-in editor/window controls.
- Documentation of the first reopen attempt and its later superseding correction.

### Out Of Scope

- Fixing the reopen behavior. A later control pass proved no source fix was needed.
- AU or VST3 automation record/playback.
- Learned MIDI CC mapping proof.
- Current preset recreation or modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off and panic proof.

## Plan of Work

Use the saved Ableton test set and the current AU device. Start transport with the hosted editor open, capture proof, close the hosted editor, capture the running closed state, then attempt reopen through Ableton's plug-in window controls and record the actual result.

## Milestones

Milestone 1 captures hosted AU editor visibility while transport runs.

Milestone 2 closes the hosted editor while transport keeps running.

Milestone 3 attempts reopen through Ableton's plug-in edit/window paths.

Milestone 4 records the observed reopen-after-close result without overclaiming.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai

Capture the editor-open running state:

    screencapture -x build/reports/ableton/hosted-ui-lifecycle-running-open.png

Close the hosted editor window:

    osascript -e 'tell application "System Events" to tell process "Live" to click button 1 of window 1'

Capture the closed running state:

    screencapture -x build/reports/ableton/hosted-ui-lifecycle-running-closed.png

Attempt reopen through:

- AU device header plug-in edit button.
- Corrected-coordinate double click on the plug-in edit button.
- Right-side device controls.
- `View > Plug-In Windows`.
- `Cmd+Option+P`.
- `Cmd+Option+Control+P`.
- `Options > Edit Key Map` assignment attempt.
- Stopped-transport retry on the plug-in edit button.

## Validation and Acceptance

Acceptance for this attempt:

- Hosted AU editor is visible during playback.
- Hosted AU editor can be closed while playback continues.
- Process output does not include a standalone `sylenth-ai` process.
- Reopen attempts are recorded accurately as failed rather than counted as pass.
- Remaining docs no longer list hosted AU editor reopen after close as open after the control pass.

### Test Commands

    ps -p $(pgrep -f "Ableton|Live|sylenth-ai" | paste -sd, -) -o pid=,command=
    rg -n "sylenth-ai|Au:|Vst3:" "$HOME/Library/Preferences/Ableton/Live 11.0.12/Log.txt" | tail -20
    git diff --check

### Follow-Up Host Matrix

- VST3 hosted editor lifecycle proof was later resolved by `2026-06-06-validate-ableton-vst3-hosted-editor-lifecycle.md`.
- AU and VST3 automation record/playback.
- AU learned CC mapping proof in Ableton. Follow-on VST3 learned-CC, continuous value-application, Forget, and stepped-controller proof later passed.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Idempotence and Recovery

The close attempt can be regenerated by reopening or recreating the current AU, starting transport, and clicking the hosted editor window close button. If coordinate probing changes the AU device activator, use Ableton Undo immediately and do not count that screenshot as proof.

## Artifacts and Notes

Local evidence artifacts are intentionally kept under ignored build paths:

- `build/reports/ableton/hosted-ui-lifecycle-running-open.png`
- `build/reports/ableton/hosted-ui-lifecycle-running-closed.png`
- `build/reports/ableton/hosted-ui-lifecycle-keymap-mode-on.png`
- `build/reports/ableton/hosted-ui-lifecycle-stopped-reopen-attempt.png`
- `build/reports/ableton/hosted-ui-lifecycle-global-shortcut-attempts.png`

The durable evidence is summarized in `docs/host-validation/ableton-smoke.md`.

## Interfaces and Dependencies

This slice depends on Ableton Live 11 Suite, the installed local AU bundle, the existing `testing-synth.als` host test set, macOS System Events scripting, Ableton menu commands, and local Ableton log files.
