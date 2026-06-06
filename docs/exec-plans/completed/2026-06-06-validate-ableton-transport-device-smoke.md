---
title: Validate Ableton Transport Device Smoke
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Prove current VST3 transport run/stop in Ableton with only Live running, and correct prior standalone-window evidence.
post_build_recap: Ableton Live 11 restored the current sylenth-ai VST3, transport playback ran with active host meters, and transport stop halted playback without a visible host error. The separate standalone sylenth-ai process was closed before clean evidence capture. Hosted editor open/close remains unproven. Offline bounce artifact proof was later covered by `2026-06-06-validate-ableton-offline-bounce-smoke.md`.
read_when:
  - Reviewing Ableton transport proof.
  - Checking whether hosted editor proof exists.
  - Continuing automation, controller, bounce, panic, or UI open/close validation.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Validate Ableton Transport Device Smoke

This ExecPlan records the transport/device proof completed after AU/VST3 restore smoke and corrects a misleading visual-evidence issue.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The remaining Ableton matrix needed transport proof against the restored current VST3. While preparing that proof, the visible `sylenth-ai` editor window in earlier screenshots was identified as the standalone app process, not a hosted Ableton plug-in editor. This slice records clean transport run/stop proof with the standalone process closed and explicitly leaves hosted UI open/close as an open gap.

It remains a bounded host smoke slice. It does not claim automation, learned CC mapping, offline bounce, sample-rate/buffer-size changes, all-notes-off, panic, or hosted editor open/close coverage.

## Progress

- [x] Relaunched the saved Ableton test set and confirmed current VST3 restore in `Log.txt`.
- [x] Closed the separate standalone `sylenth-ai` process before clean transport capture.
- [x] Started Ableton transport from the keyboard shortcut and captured active host meters with the restored VST3 device present.
- [x] Stopped Ableton transport from the keyboard shortcut and captured the restored VST3 device still present.
- [x] Attempted to open the hosted editor from the visible Ableton device controls; no hosted `sylenth-ai` window or process appeared.
- [x] Corrected prior docs that treated the standalone app window as hosted-editor evidence.

## Surprises & Discoveries

The desktop screenshots from the current-build VST3 smoke pass included a visible `sylenth-ai` app process launched from `build/SylenthAIPlugin_artefacts/Standalone/sylenth-ai.app`. That app was not hosted by Ableton and must not be counted as hosted-editor proof.

The VST3 device remained visible in Ableton and played audio, but the visible device controls did not open a separate hosted editor window during this pass. The plug-in code reports `hasEditor() == true` and implements `createEditor()`, so the remaining issue may be Ableton interaction, host UI embedding behavior, or a host/editor integration bug. It needs a targeted UI-open validation slice.

## Decision Log

Decision: Correct the previous hosted-editor wording instead of carrying ambiguous evidence forward.
Rationale: A standalone app window proves standalone UI rendering, not hosted Ableton editor behavior.
Date: 2026-06-06.

Decision: Record transport run/stop as complete, but leave all-notes-off, panic, and hosted UI open/close open.
Rationale: Transport start/stop was visible and repeatable; the other checks require different host gestures or a reliable hosted-editor path.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. The current VST3 restored in Ableton, transport ran with active host meters, and transport stopped without a visible host error. At this point, the remaining host matrix was narrower but still important: automation, learned CC mapping, offline bounce, sample-rate/buffer-size changes, all-notes-off, panic, and hosted UI open/close while playing. Offline bounce artifact proof was later covered by `2026-06-06-validate-ableton-offline-bounce-smoke.md`.

## Context and Orientation

Read first:

- `docs/host-validation/ableton-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-current-build-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-state-restore-smoke.md`
- `src/plugin/PluginProcessor.h`
- `src/plugin/PluginProcessor.cpp`

### In Scope

- Current VST3 transport run proof.
- Current VST3 transport stop proof.
- Correction of standalone-window evidence.
- Documentation of hosted editor open/close as still unproven at this slice's completion.

### Out Of Scope

- AU transport proof.
- Automation record/playback.
- Learned MIDI CC mapping proof.
- Offline bounce.
- Sample-rate and buffer-size changes.
- All-notes-off and panic proof.
- Fixing or proving hosted editor open/close.

## Plan of Work

Use the restored Ableton test set, close the standalone `sylenth-ai` app process, start playback with Ableton focused, capture the running state, stop playback, and capture the stopped state. Inspect processes to confirm the standalone app is not running. If a hosted editor cannot be opened from the visible device controls, document that as a remaining validation gap rather than treating standalone evidence as host evidence.

## Milestones

Milestone 1 proves the restored VST3 set is running with only Ableton Live active.

Milestone 2 proves transport start with active meters.

Milestone 3 proves transport stop without a visible host error.

Milestone 4 corrects hosted-editor evidence wording in durable docs.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai

Open the restored test set:

    open -a "Ableton Live 11 Suite" "/Users/parkerrex/Desktop/testing-synth Project/testing-synth.als"

Close any standalone app process and verify only Live is running:

    osascript -e 'tell application id "com.parkerx.sylenth-ai" to quit' || true
    ps aux | rg -i "sylenth-ai|Live" | rg -v rg || true

Start and stop transport with Ableton focused, then capture screenshots under `build/reports/ableton/`.

## Validation and Acceptance

Acceptance required:

- Ableton log shows current VST3 restore for `sylenth-ai`.
- Process list shows Ableton Live running and no standalone `sylenth-ai` process during clean capture.
- Running screenshot shows active transport/meters with the restored VST3 device present.
- Stopped screenshot shows transport stopped with the restored VST3 device present.
- Hosted-editor attempt screenshots show no hosted `sylenth-ai` editor window or process after clicking the visible Ableton device controls.
- Docs no longer count standalone screenshots as hosted-editor proof.

### Test Commands

    tail -n 80 "$HOME/Library/Preferences/Ableton/Live 11.0.12/Log.txt"
    ps aux | rg -i "sylenth-ai|Live" | rg -v rg || true
    osascript -e 'tell application "Ableton Live 11 Suite" to activate'
    screencapture -x build/reports/ableton/transport-vst3-running-no-standalone.png
    screencapture -x build/reports/ableton/transport-vst3-stopped-no-standalone.png
    screencapture -x build/reports/ableton/host-editor-open-attempt.png
    screencapture -x build/reports/ableton/host-editor-open-attempt-2.png

### Follow-Up Host Matrix

- AU and VST3 automation record/playback.
- AU learned CC mapping proof in Ableton. Follow-on VST3 learned-CC, continuous value-application, Forget, and stepped-controller proof later passed.
- Resolved by `2026-06-06-validate-ableton-offline-bounce-smoke.md`: offline bounce artifact creation. Still open: offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.
- Hosted UI open/close while transport is running.

## Idempotence and Recovery

The test set can be reopened and reused. If transport state is unclear, press Space with Ableton focused and capture a fresh screenshot. If standalone UI appears, close it before using screenshots as host proof.

## Artifacts and Notes

Local screenshot artifacts are intentionally kept under ignored build paths:

- `build/reports/ableton/transport-vst3-running-no-standalone.png`
- `build/reports/ableton/transport-vst3-stopped-no-standalone.png`
- `build/reports/ableton/host-editor-open-attempt.png`
- `build/reports/ableton/host-editor-open-attempt-2.png`

The durable evidence is the log/process/screenshot summary in `docs/host-validation/ableton-smoke.md`.

## Interfaces and Dependencies

This slice depends on Ableton Live 11 Suite, the installed local VST3 bundle, the existing `testing-synth.als` host test set, macOS System Events keyboard scripting, and screenshot capture.
