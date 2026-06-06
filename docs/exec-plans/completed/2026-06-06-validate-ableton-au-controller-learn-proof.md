---
title: Validate Ableton AU Controller Learn Proof
status: completed
summary: Prove the hosted AU editor can capture and persist a fresh MIDI Learn assignment in Ableton.
post_build_recap: Ableton Live 11 loaded the current sylenth-ai AU from `Audio Units > ParkerX > sylenth-ai`, routed a temporary CoreMIDI source into the AU track, captured CC74 from the hosted global MIDI Learn panel for `filter.resonance`, and wrote the expected global user controller-map sidecar. The temporary user sidecar was copied to ignored evidence and removed after the pass.
triggers:
  - Reviewing Ableton AU MIDI Learn capture proof.
  - Continuing host validation after AU seeded controller value-application proof.
  - Debugging hosted AU editor MIDI Learn behavior in Live.
---

# Validate Ableton AU Controller Learn Proof

This ExecPlan closes the AU in-editor MIDI Learn capture and sidecar persistence proof gap. It does not close AU global-panel Forget, AU/VST3 automation record/playback, per-control context-menu Learn, or controller/automation conflict behavior.

## Context

The earlier AU controller proof showed that a seeded global controller map loads in the AU and drives a continuous parameter in Ableton. AU-specific Learn capture still needed a fresh host proof because capturing the next CC from the hosted AU editor uses a separate UI/control path from loading an existing sidecar.

## Completed Work

- [x] Removed any existing `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json` before launching Ableton.
- [x] Created a temporary CoreMIDI source named `SylenthAI Codex AU Learn Source` before Ableton launch.
- [x] Launched Ableton after creating the source and verified Live logged it as `Track=true`.
- [x] Loaded the current `sylenth-ai` AU from `Audio Units > ParkerX > sylenth-ai` into a fresh Ableton `Untitled` set.
- [x] Verified the hosted MIDI CONTROL panel started with `MIDI learn ready`, `No MIDI CC mappings`, and footer `MIDI 0`.
- [x] Selected `FILTER / Resonance` in the hosted AU MIDI Learn parameter selector.
- [x] Armed Learn from the hosted AU editor and sent CC74 value `99` from the temporary CoreMIDI source.
- [x] Verified the hosted panel displayed `Mapped CC74 to filter.resonance` and `CC74 -> filter.resonance`, with footer `MIDI 1`.
- [x] Verified the user sidecar persisted `CC74 -> filter.resonance`.
- [x] Copied the proof sidecar and Ableton log excerpt to ignored local evidence, removed the temporary user-level sidecar, stopped the CoreMIDI source, and quit Live without saving the validation set.

## Validation

Ableton log evidence:

- `MidiInDevice [Name="SylenthAI Codex AU Learn Source", Track=true, Sync=false, Remote=false, MPE=false, MIDI Clock Sync Delay=0, Sync Type="MIDI Clock", MTC Frame Rate="All", MTC Start Offset=0]`
- `2026-06-06T15:09:19.460935: info: Au: Going to create: sylenth-ai`
- `2026-06-06T15:09:19.475730: info: Au: Created: sylenth-ai`

Persisted sidecar evidence:

```json
{
  "schema_version": 1,
  "mappings": [
    {
      "cc": 74,
      "parameter_id": "filter.resonance"
    }
  ]
}
```

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `au-learn-proof-live-fresh-launched.png`
- `au-learn-proof-au-loaded-no-map.png`
- `au-learn-proof-resonance-selected.png`
- `au-learn-proof-resonance-armed.png`
- `au-learn-proof-cc74-mapped.png`
- `midi-controller-map-au-learn-proof.json`
- `au-learn-proof-ableton-log-excerpt.txt`

## Findings

The hosted AU editor captured the next MIDI CC from Live after Learn was armed in the global MIDI CONTROL panel. The panel reported the expected mapping, the MIDI counter advanced, and the global user controller-map sidecar was written with the learned assignment.

This proof is intentionally AU-only and global-panel-only. Per-control context menus, host automation record/playback, and controller/automation conflict behavior remain separate validation surfaces.

## Remaining Gaps

- AU and VST3 automation record/playback.
- AU global-panel MIDI Forget proof in Ableton.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Retrospective

For AU Learn proof, create the virtual MIDI source before launching Ableton, start with no user sidecar, load the AU fresh, select the target parameter from the hosted MIDI CONTROL panel, arm Learn, send one CC, and verify both the visible assignment and the written sidecar before cleanup.
