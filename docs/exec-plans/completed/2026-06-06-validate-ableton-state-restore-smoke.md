---
title: Validate Ableton State Restore Smoke
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Extend current-build Ableton proof with AU create/restore, VST3 restore, and VST3 post-restore playback evidence.
post_build_recap: Ableton Live 11 created the current sylenth-ai AU, restored it after saving and reopening the test set, then created and restored the current sylenth-ai VST3 after a second save/reopen pass. VST3 post-restore transport playback showed active meters at 44100 Hz / 512 samples. Automation, learned CC, bounce, sample-rate/buffer-size, panic, and hosted UI open/close checks remain open.
read_when:
  - Reviewing current-build Ableton state restore proof.
  - Continuing host automation, controller, bounce, or buffer-size validation.
  - Debugging AU/VST3 restore behavior in Ableton.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Validate Ableton State Restore Smoke

This ExecPlan records the state-restore extension after the current-build scan/load/play smoke pass.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The previous smoke pass proved current VST3 scan, creation, and playback after the rename. This slice extends host proof to current AU creation and AU/VST3 save/reopen restore in Ableton Live 11.

It remains a bounded host smoke slice. It does not claim automation, controller mapping, bounce, sample-rate/buffer-size, panic, or hosted UI open/close coverage.

## Progress

- [x] Created the current AU from `Audio Units > ParkerX > sylenth-ai`.
- [x] Saved and reopened the test set, then confirmed AU restore in Ableton logs.
- [x] Created a fresh current VST3 instance after the AU restore pass.
- [x] Saved and reopened the test set again, then confirmed VST3 restore in Ableton logs.
- [x] Ran VST3 post-restore transport playback from the clip start and captured active meter evidence.
- [x] Updated host validation notes and Program/index docs.

## Surprises & Discoveries

Ableton appeared to replace the current instrument rather than keeping AU and VST3 simultaneously on separate MIDI tracks during scripted browser interactions. The validation still produced useful evidence: AU restore and VST3 restore were proven in separate save/reopen passes against the same test set.

## Decision Log

Decision: Record AU and VST3 restore as separate current-build passes.
Rationale: The host log is authoritative, and separate restore passes avoid overclaiming simultaneous AU/VST3 state in one saved set.
Date: 2026-06-06.

Decision: Leave automation, controller, bounce, sample-rate, buffer-size, panic, and hosted UI open/close as the next host matrix slice.
Rationale: Those checks require more deliberate Ableton workflow automation and should not be hidden inside a scan/restore smoke note.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. Ableton Live 11 created and restored the current sylenth-ai AU, then created and restored the current sylenth-ai VST3. The VST3 restore path reported ParkerX `sylenth-ai` v0.1.0 and 2427 parameters. VST3 post-restore playback from the clip start showed active track meters.

## Context and Orientation

Read first:

- `docs/host-validation/ableton-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-current-build-smoke.md`
- `docs/exec-plans/active/2026-06-04-build-ableton-host-integration-and-packaging.md`

### In Scope

- Current-build AU creation.
- Current-build AU save/reopen restore.
- Current-build VST3 save/reopen restore.
- VST3 post-restore transport playback with active meters.
- Durable docs update.

### Out Of Scope

- Automation record/playback.
- Learned MIDI CC mapping proof in Ableton.
- Offline bounce.
- Sample-rate and buffer-size changes.
- Panic/all-notes-off proof.
- Hosted UI open/close while transport is running.

## Plan of Work

Use the existing Ableton test set. Create the current AU from the Audio Units browser path, save and reopen the set, then verify AU restore in `Log.txt`. Create the current VST3, save and reopen again, verify VST3 restore, and run transport playback from the clip start.

## Milestones

Milestone 1 proves current AU creation and restore.

Milestone 2 proves current VST3 creation and restore.

Milestone 3 proves VST3 post-restore playback still drives audio meters.

Milestone 4 records remaining host matrix gaps.

## Concrete Steps

Work from the running Ableton Live 11 test set:

    /Users/parkerrex/Desktop/testing-synth Project/testing-synth.als

Create AU:

    Plug-Ins > Audio Units > ParkerX > sylenth-ai

Save, quit Ableton, reopen the same set, and inspect:

    ~/Library/Preferences/Ableton/Live 11.0.12/Log.txt

Create VST3:

    Plug-Ins > VST3 > ParkerX > sylenth-ai

Save, quit Ableton, reopen the same set, inspect `Log.txt`, then run transport from the MIDI clip start and capture local screenshots under `build/reports/ableton/`.

## Validation and Acceptance

Acceptance required:

- Ableton logs current AU create and restore lines for `sylenth-ai`.
- Ableton logs current VST3 create and restore lines for `sylenth-ai`.
- VST3 restore reports ParkerX `sylenth-ai` v0.1.0 and 2427 parameters.
- VST3 post-restore transport playback shows active meters.
- Remaining host gaps are still listed.

### Test Commands

No new command-line build was required; this slice used the installed `build-release-phase1-ableton` AU/VST3 bundles from the immediately preceding smoke pass.

Relevant inspection command:

    tail -n 220 "$HOME/Library/Preferences/Ableton/Live 11.0.12/Log.txt"

### Follow-Up Host Matrix

- AU and VST3 automation record/playback.
- AU learned CC mapping proof in Ableton. Follow-on VST3 learned-CC, continuous value-application, Forget, and stepped-controller proof later passed.
- Offline bounce comparison.
- Sample-rate and buffer-size changes.
- Transport stop/all-notes-off/panic proof.
- Hosted UI open/close while transport is running.

## Idempotence and Recovery

The test set can be reopened and reused for host smoke proof. If Ableton prompts to save, save only the test set, not unrelated projects. If scan or restore names become stale, rerun the normal Ableton plug-in rescan and inspect `PluginScanner.txt` before touching cache files.

## Artifacts and Notes

Local screenshot artifacts are intentionally kept under ignored build paths:

- `build/reports/ableton/host-matrix-au-created.png`
- `build/reports/ableton/host-matrix-reopen-restore.png`
- `build/reports/ableton/host-matrix-vst3-track2-created.png`
- `build/reports/ableton/host-matrix-reopen-vst3-restore.png`
- `build/reports/ableton/host-matrix-post-restore-playback-from-start.png`

The durable evidence is the log summary in `docs/host-validation/ableton-smoke.md`.

## Interfaces and Dependencies

This slice depends on Ableton Live 11 Suite, the installed local AU/VST3 bundles, and the existing `testing-synth.als` host test set.
