---
title: Validate Ableton VST3 Controller Forget And Stepped Proof
status: completed
summary: Prove hosted VST3 global-panel Forget and stepped choice-parameter mapped CC playback in Ableton.
post_build_recap: Ableton Live 11 loaded the current sylenth-ai VST3 with a seeded CC73 to `filter.mode` sidecar, routed a temporary CoreMIDI validation source into the hosted track, and visibly drove the stepped Filter Mode choice from `L4` to `Notch4` and back to `L2`. The hosted MIDI CONTROL panel then selected `FILTER / Filter Mode`, Forget cleared the assignment, wrote an empty sidecar, and a post-Forget CC73 high value left Filter Mode at `L2`.
triggers:
  - Reviewing Ableton VST3 controller Forget proof.
  - Reviewing stepped MIDI CC value application in Live.
  - Continuing controller validation after VST3 continuous value proof.
---

# Validate Ableton VST3 Controller Forget And Stepped Proof

This ExecPlan closes the VST3 global-panel Forget and stepped mapped CC playback gap that remained after the continuous controller value proof. It does not close AU controller proof or AU/VST3 automation record/playback.

## Context

The previous VST3 controller value proof showed that a persisted CC71 assignment drove the continuous `filter.resonance` parameter. The remaining VST3 controller-specific gap was proving a choice parameter maps stepped CC values correctly in Ableton and that the hosted global MIDI CONTROL panel can Forget the mapping and persist that removal.

## Completed Work

- [x] Seeded `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json` with `CC73 -> filter.mode` before plugin construction.
- [x] Copied the seeded sidecar to `build/reports/ableton/midi-controller-map-vst3-forget-stepped-seeded.json`.
- [x] Created a temporary CoreMIDI source named `SylenthAI Codex Stepped Source`.
- [x] Launched Ableton after creating the source and verified Live logged it as `Track=true`.
- [x] Loaded the current `sylenth-ai` VST3 into a fresh Ableton `Untitled` set.
- [x] Verified the hosted MIDI CONTROL panel displayed `Loaded 1 MIDI CC mappings` and `CC73 -> filter.mode`.
- [x] Sent CC73 value `127` and captured Filter Mode changing to `Notch4`.
- [x] Sent CC73 value `0` and captured Filter Mode changing to `L2`.
- [x] Selected `FILTER / Filter Mode` in the hosted MIDI CONTROL panel and clicked Forget.
- [x] Verified the hosted UI displayed `Forgot MIDI CC for filter.mode` and `No MIDI CC mappings`.
- [x] Verified the sidecar became `{"schema_version": 1, "mappings": []}` and copied it to ignored local evidence.
- [x] Sent CC73 value `127` after Forget and captured Filter Mode remaining `L2`.
- [x] Removed the temporary user-level sidecar, stopped the CoreMIDI source, and quit Live without saving the validation set.

## Validation

Ableton log evidence:

- `MidiInDevice [Name="SylenthAI Codex Stepped Source", Track=true, Sync=false, Remote=false, MPE=false, MIDI Clock Sync Delay=0, Sync Type="MIDI Clock", MTC Frame Rate="All", MTC Start Offset=0]`
- `Vst3: Going to create: sylenth-ai`
- `Vst3: Created: sylenth-ai`

Seeded sidecar evidence:

```json
{"schema_version":1,"mappings":[{"cc":73,"parameter_id":"filter.mode"}]}
```

Post-Forget sidecar evidence:

```json
{"schema_version": 1, "mappings": []}
```

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

## Findings

The VST3 processor loaded the seeded global user map on construction and the hosted editor displayed the assignment without re-learning it. Live delivered the temporary stepped source into the VST3 track, and the Filter Mode choice followed CC73 from `L4` to `Notch4` to `L2`.

The hosted MIDI CONTROL panel's Forget flow persisted the empty assignment list and updated the visible assignment summary to `No MIDI CC mappings`. After Forget, another high CC73 value still reached the plugin, but `filter.mode` remained `L2`, proving the runtime mapping was removed.

This proof is intentionally VST3-only. AU learned-CC mapping and AU value application still need their own host proof.

## Remaining Gaps

- AU and VST3 automation record/playback.
- AU learned CC mapping and value-application proof in Ableton.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Retrospective

Use Accessibility for the hosted MIDI CONTROL parameter combo when synthetic mouse clicks are unreliable. Opening the combo exposes menu items directly, and selecting the next item by keyboard is reliable when the current item is adjacent to the target.
