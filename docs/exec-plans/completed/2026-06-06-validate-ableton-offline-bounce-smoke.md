---
title: Validate Ableton Offline Bounce Smoke
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Prove Ableton can export an offline bounce from the restored current VST3 set and record finite, non-silent WAV metrics.
post_build_recap: Ableton Live 11 exported WAV and MP3 artifacts from the restored current VST3 test set. The WAV artifact is stereo 44.1 kHz 16-bit PCM, 1.3750113378684807 seconds, non-silent, and non-clipping. Offline bounce versus realtime comparison remains open.
read_when:
  - Reviewing Ableton offline bounce proof.
  - Continuing bounce-versus-realtime validation.
  - Debugging host export or offline render behavior.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Validate Ableton Offline Bounce Smoke

This ExecPlan records the first current-build Ableton offline bounce proof after VST3 restore and transport smoke.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The remaining Ableton matrix needed proof that the restored current VST3 can participate in Ableton's offline export path. This slice drives Live's Export Audio/Video flow against the saved test set, captures local WAV/MP3 artifacts, and measures the WAV for format, duration, peak, RMS, silence, and clipping.

It remains a bounded host smoke slice. It does not claim a realtime-versus-offline comparison, AU export proof, automation, learned CC mapping, sample-rate/buffer-size changes, all-notes-off, panic, or hosted editor open/close coverage.

## Progress

- [x] Opened Ableton Export Audio/Video for the restored current VST3 set.
- [x] Exported master output with WAV PCM and MP3 enabled.
- [x] Normalized local artifact names under `build/reports/ableton/bounce/`.
- [x] Measured WAV format, duration, peak, RMS, non-silence, and clipping state.
- [x] Updated host validation notes and Program/index docs.

## Surprises & Discoveries

Ableton's export dialog exposes only shallow accessibility metadata, so the export flow used keyboard/default-button behavior and coordinate selection for the save dialog. The save panel appended format suffixes to the typed filename, producing temporary double extensions before the ignored build artifacts were renamed.

The export defaults produced both WAV and MP3 artifacts. The WAV is the durable validation artifact for metrics; the MP3 confirms Ableton's MP3 encoder path also completed but is not used for audio-quality proof.

## Decision Log

Decision: Count this as offline bounce artifact proof, not offline-versus-realtime comparison.
Rationale: The WAV is finite, non-silent, and non-clipping, but no realtime audio capture was recorded for waveform comparison in this slice.
Date: 2026-06-06.

Decision: Keep bounce artifacts under ignored `build/reports/ableton/bounce/`.
Rationale: Rendered audio is local evidence and should not be committed unless a human explicitly asks for an evidence snapshot.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. Ableton exported the restored current VST3 set through the offline render path. The WAV metrics prove non-silent, non-clipping stereo output at 44.1 kHz. The remaining host matrix is narrower but still needs realtime comparison, automation, learned CC mapping, preset recreation/modulation exercise, sample-rate/buffer-size changes, all-notes-off, panic, and hosted UI open/close while playing. AU transport run/stop proof was later covered by `2026-06-06-validate-ableton-au-transport-smoke.md`.

## Context and Orientation

Read first:

- `docs/host-validation/ableton-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-state-restore-smoke.md`
- `docs/exec-plans/completed/2026-06-06-validate-ableton-transport-device-smoke.md`

### In Scope

- Current VST3 Ableton offline export.
- WAV/MP3 artifact creation.
- WAV finite/non-silent/non-clipping metrics.
- Documentation of remaining bounce-versus-realtime comparison gap.

### Out Of Scope

- AU offline export.
- Realtime capture and waveform comparison.
- Automation record/playback.
- Learned MIDI CC mapping proof.
- Sample-rate and buffer-size changes.
- All-notes-off and panic proof.
- Hosted UI open/close proof.

## Plan of Work

Use the restored Ableton test set. Open File > Export Audio/Video, keep the default master render settings with WAV PCM enabled, save the render into `build/reports/ableton/bounce/`, normalize local artifact names, and run a small WAV metrics script to prove the render is finite, non-silent, and non-clipping.

## Milestones

Milestone 1 opens Ableton's Export Audio/Video flow.

Milestone 2 writes WAV and MP3 artifacts under the ignored build report directory.

Milestone 3 computes WAV metrics and records the bounded proof.

Milestone 4 narrows the remaining host matrix to bounce-versus-realtime comparison rather than generic bounce creation.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai

Prepare the output directory:

    mkdir -p build/reports/ableton/bounce

In Ableton Live 11 Suite, choose File > Export Audio/Video, keep master WAV PCM defaults, save under:

    build/reports/ableton/bounce/sylenth-ai-ableton-bounce-2026-06-06.wav

Normalize Ableton's double extensions if needed and compute WAV metrics:

    python3 - <<'PY'
    from pathlib import Path
    import audioop, json, math, wave
    p = Path('build/reports/ableton/bounce/sylenth-ai-ableton-bounce-2026-06-06.wav')
    with wave.open(str(p), 'rb') as w:
        channels = w.getnchannels()
        rate = w.getframerate()
        width = w.getsampwidth()
        frames = w.getnframes()
        data = w.readframes(frames)
    max_abs = audioop.max(data, width) if data else 0
    rms = audioop.rms(data, width) if data else 0
    max_possible = float(2 ** (8 * width - 1) - 1)
    metrics = {
        'channels': channels,
        'sample_rate': rate,
        'sample_width_bytes': width,
        'frames': frames,
        'duration_seconds': frames / rate if rate else 0,
        'peak': max_abs / max_possible if max_possible else 0,
        'rms': rms / max_possible if max_possible else 0,
        'rms_dbfs': 20 * math.log10(rms / max_possible) if rms > 0 and max_possible else None,
        'non_silent': rms > 0,
        'non_clipping': max_abs < max_possible,
    }
    Path('build/reports/ableton/bounce/sylenth-ai-ableton-bounce-2026-06-06.metrics.json').write_text(json.dumps(metrics, indent=2) + '\n')
    print(json.dumps(metrics, indent=2))
    PY

## Validation and Acceptance

Acceptance required:

- Ableton writes a WAV artifact from the restored current VST3 set.
- WAV artifact is stereo PCM at 44.1 kHz.
- WAV artifact is non-silent.
- WAV artifact is non-clipping.
- Export-setting screenshots document the dialog state used for the bounce.
- Remaining docs still list offline bounce versus realtime comparison as open.

### Test Commands

    file build/reports/ableton/bounce/*
    python3 - <<'PY'
    from pathlib import Path
    import json
    print(Path('build/reports/ableton/bounce/sylenth-ai-ableton-bounce-2026-06-06.metrics.json').read_text())
    PY

### Follow-Up Host Matrix

- AU and VST3 automation record/playback.
- AU learned CC mapping proof in Ableton. Follow-on VST3 learned-CC proof later passed.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.
- Hosted UI open/close while transport is running.

## Idempotence and Recovery

The bounce can be regenerated by deleting `build/reports/ableton/bounce/` and rerunning Ableton export. If the save panel appends duplicate format extensions, rename ignored build artifacts before recording evidence paths in durable docs.

## Artifacts and Notes

Local render artifacts are intentionally kept under ignored build paths:

- `build/reports/ableton/export-dialog-open.png`
- `build/reports/ableton/export-save-dialog.png`
- `build/reports/ableton/export-bounce-after-save.png`
- `build/reports/ableton/bounce/sylenth-ai-ableton-bounce-2026-06-06.wav`
- `build/reports/ableton/bounce/sylenth-ai-ableton-bounce-2026-06-06.mp3`
- `build/reports/ableton/bounce/sylenth-ai-ableton-bounce-2026-06-06.metrics.json`

The durable evidence is the metrics summary in `docs/host-validation/ableton-smoke.md`.

## Interfaces and Dependencies

This slice depends on Ableton Live 11 Suite, the installed local VST3 bundle, the existing `testing-synth.als` host test set, macOS System Events menu scripting, and Python's standard `wave`/`audioop` modules for local WAV inspection.
