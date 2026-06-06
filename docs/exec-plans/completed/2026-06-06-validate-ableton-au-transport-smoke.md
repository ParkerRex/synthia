---
title: Validate Ableton AU Transport Smoke
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Prove Ableton can create the current sylenth-ai AU and run/stop transport with the hosted AU editor visible.
post_build_recap: Ableton Live 11 exposed `Audio Units > ParkerX > sylenth-ai`, created the current AU, opened a hosted `sylenth-ai/1-sylenth-ai` editor window, ran transport with active voices/meters, and stopped transport cleanly. At this slice's completion, explicit hosted editor close/reopen while transport ran remained open; later lifecycle/control passes proved hosted AU editor close and reopen.
read_when:
  - Reviewing current-build Ableton AU transport proof.
  - Continuing Ableton automation, controller, or hosted UI validation.
  - Debugging AU create/play/stop behavior in Live.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Validate Ableton AU Transport Smoke

This ExecPlan records current-build AU create/play/stop proof in Ableton after the VST3 restore, transport, and offline-bounce slices.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The Ableton matrix still needed current AU transport proof. Earlier host passes proved AU creation/restore and VST3 playback paths, but AU run/stop under the renamed `sylenth-ai` build remained open.

This slice navigated Ableton's plug-in browser to `Audio Units > ParkerX > sylenth-ai`, created the AU, verified Ableton-owned editor visibility, ran transport against the MIDI test set, stopped transport, and recorded the proof without a standalone `sylenth-ai` process.

It remains a bounded host smoke slice. It does not claim automation, learned CC mapping, preset recreation, modulation exercise, offline-versus-realtime comparison, sample-rate/buffer-size changes, all-notes-off, panic, or explicit hosted editor close/reopen while transport is running. Later lifecycle/control passes proved hosted AU editor close and reopen.

## Progress

- [x] Opened Ableton's Audio Units browser path for ParkerX.
- [x] Created the current `sylenth-ai` AU from `Audio Units > ParkerX > sylenth-ai`.
- [x] Confirmed Ableton log lines for AU creation.
- [x] Captured hosted AU editor visibility during transport playback.
- [x] Captured transport stop with voices returning to `0` and peak returning to `-inf`.
- [x] Updated host validation notes and Program/index docs.

## Surprises & Discoveries

Ableton's browser and plug-in controls exposed limited accessibility metadata, so the AU load path used keyboard navigation and Return once the `sylenth-ai` AU child was selected. This was more reliable than AX double-click attempts.

Live opened a `sylenth-ai/1-sylenth-ai` editor window for the AU. A process check during playback showed Ableton Live and no standalone `sylenth-ai` process, so this evidence is hosted AU UI visibility rather than standalone app leakage.

Live also logged `Vst3: couldn't get controller state of sylenth-ai: not implemented` immediately after AU creation. That warning belongs on the VST3 host-state watchlist, but it did not block AU create/play/stop behavior in this slice.

## Decision Log

Decision: Count this as AU transport run/stop proof with hosted editor visibility, not full hosted UI lifecycle proof.
Rationale: The AU editor was visible during playback and stopped state, but explicit close/reopen while transport was running was not exercised.
Date: 2026-06-06.

Decision: Keep screenshots under ignored `build/reports/ableton/`.
Rationale: Screenshots are local evidence artifacts and should not be committed unless a human explicitly asks for an evidence snapshot.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. Ableton created the current AU, opened a hosted editor window, ran MIDI playback with active voices and Live meters, then stopped transport cleanly. At this slice's completion, the remaining host matrix still needed automation, learned CC mapping, current preset recreation/modulation exercise, offline bounce versus realtime comparison, sample-rate/buffer-size changes, all-notes-off, panic, and hosted editor lifecycle proof. Later lifecycle/control passes proved AU and VST3 hosted editor close/reopen.

## Context and Orientation

Read first:

- `docs/host-validation/ableton-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-state-restore-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-transport-device-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-offline-bounce-smoke.md`

### In Scope

- Current AU browser visibility in Ableton.
- Current AU creation in Ableton.
- Transport start with active voices/meters.
- Transport stop with voices cleared.
- Hosted AU editor visibility during playback.
- Explicit process check that no standalone `sylenth-ai` process was providing the visible editor.

### Out Of Scope

- AU or VST3 automation record/playback.
- Learned MIDI CC mapping proof.
- Current preset recreation or modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off and panic proof.
- Hosted AU editor close while transport runs was later proven by `2026-06-06-validate-ableton-hosted-ui-lifecycle-attempt.md`; hosted AU editor reopen after close was later proven by `2026-06-06-validate-ableton-hosted-au-editor-reopen-control.md`; VST3 hosted editor lifecycle was later proven by `2026-06-06-validate-ableton-vst3-hosted-editor-lifecycle.md`.

## Plan of Work

Use the saved Ableton test set. Navigate the plug-in browser to `Audio Units > ParkerX > sylenth-ai`, create the AU, start transport, capture hosted editor and meter evidence, stop transport, capture stopped state, and record the remaining gaps accurately.

## Milestones

Milestone 1 exposes the current AU entry in Ableton's Audio Units browser.

Milestone 2 creates the AU and records Ableton creation logs.

Milestone 3 captures transport playback with hosted AU editor evidence.

Milestone 4 captures transport stop and narrows the remaining host matrix.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai

Confirm the local host proof is not accidentally using the standalone app:

    pgrep -af "sylenth-ai|Ableton|Live"

In Ableton Live 11 Suite, open the plug-in browser and navigate to:

    Audio Units > ParkerX > sylenth-ai

Create the AU by selecting the `sylenth-ai` child and pressing Return.

Record Live log evidence:

    rg -n "Au: (Going to create|Created): sylenth-ai|couldn't get controller state of sylenth-ai" ~/Library/Preferences/Ableton/Live*/Log.txt

Start transport with the MIDI clip loaded, then capture local screenshots:

    screencapture -x build/reports/ableton/au-transport-running-with-editor.png

Stop transport, then capture:

    screencapture -x build/reports/ableton/au-transport-stopped-with-editor.png

## Validation and Acceptance

Acceptance required:

- Ableton exposes `Audio Units > ParkerX > sylenth-ai`.
- Ableton logs AU create start and create completion for `sylenth-ai`.
- The AU editor window is hosted by Ableton, with no standalone `sylenth-ai` process visible in process output.
- Transport playback shows active voices/meters and 44100 Hz / 512-sample audio state.
- Transport stop returns AU voices to `0` and peak to `-inf` without a visible host crash.
- Docs leave hosted editor lifecycle proof open at this slice's completion. Hosted AU editor close while transport runs was later proven by `2026-06-06-validate-ableton-hosted-ui-lifecycle-attempt.md`; hosted AU editor reopen after close was later proven by `2026-06-06-validate-ableton-hosted-au-editor-reopen-control.md`; VST3 hosted editor lifecycle was later proven by `2026-06-06-validate-ableton-vst3-hosted-editor-lifecycle.md`.

### Test Commands

    pgrep -af "sylenth-ai|Ableton|Live"
    rg -n "Au: (Going to create|Created): sylenth-ai|couldn't get controller state of sylenth-ai" ~/Library/Preferences/Ableton/Live*/Log.txt
    git diff --check

### Follow-Up Host Matrix

- AU and VST3 automation record/playback.
- AU learned CC mapping proof plus VST3 host Forget/stepped controller proof in Ableton. Follow-on VST3 learned-CC and continuous value-application proof later passed.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.
- Hosted AU editor close while transport runs was later proven by `2026-06-06-validate-ableton-hosted-ui-lifecycle-attempt.md`; hosted AU editor reopen after close was later proven by `2026-06-06-validate-ableton-hosted-au-editor-reopen-control.md`; VST3 hosted editor lifecycle was later proven by `2026-06-06-validate-ableton-vst3-hosted-editor-lifecycle.md`.

## Idempotence and Recovery

The proof can be regenerated by opening the saved Ableton set, deleting the test plug-in device if needed, and recreating `Audio Units > ParkerX > sylenth-ai`. If Ableton browser selection drifts, use keyboard navigation from the visible `Audio Units` tree rather than relying on accessibility double-clicks.

## Artifacts and Notes

Local evidence artifacts are intentionally kept under ignored build paths:

- `build/reports/ableton/au-transport-audio-units-open.png`
- `build/reports/ableton/au-transport-parkerx-expanded-3.png`
- `build/reports/ableton/au-transport-au-return-load-attempt.png`
- `build/reports/ableton/au-transport-running-with-editor.png`
- `build/reports/ableton/au-transport-stopped-with-editor.png`

The durable evidence is summarized in `docs/host-validation/ableton-smoke.md`.

## Interfaces and Dependencies

This slice depends on Ableton Live 11 Suite, the installed local AU bundle, the existing `testing-synth.als` host test set, macOS System Events keyboard scripting, and local Ableton log files.
