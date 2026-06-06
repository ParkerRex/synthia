---
title: Validate Ableton VST3 Controller Value Proof
status: completed
summary: Prove a persisted VST3 MIDI CC mapping visibly drives one continuous mapped parameter in Ableton.
post_build_recap: Ableton Live 11 loaded the current sylenth-ai VST3 with a seeded CC71 to `filter.resonance` sidecar, routed a temporary CoreMIDI validation source into the hosted track, and visibly drove the hosted editor's continuous Resonance value from `0.00` to `1.00` and back to `0.00`. The temporary user sidecar was copied to ignored evidence and removed after the pass.
triggers:
  - Reviewing Ableton VST3 controller value-application proof.
  - Continuing host validation after VST3 learned-CC capture/persistence proof.
  - Debugging mapped CC value application in Live.
---

# Validate Ableton VST3 Controller Value Proof

This ExecPlan closes the VST3 continuous controller value-application proof gap that remained after the learned-CC capture/persistence pass. It does not close AU controller proof, VST3 host Forget/stepped mapped CC playback, or AU/VST3 automation record/playback.

## Context

The previous VST3 controller proof showed that Live could deliver CC71 into the hosted VST3 MIDI Learn flow and that the plugin persisted `CC71 -> filter.resonance`. The adversarial review correctly required separate visible evidence that a persisted mapping drives the mapped continuous parameter value after plugin construction.

## Completed Work

- [x] Seeded `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json` with `CC71 -> filter.resonance` before plugin construction.
- [x] Created a temporary CoreMIDI source named `SylenthAI Codex Value Source`.
- [x] Launched Ableton after creating the source and verified Live logged it as `Track=true`.
- [x] Loaded the current `sylenth-ai` VST3 into a fresh Ableton `Untitled` set.
- [x] Verified the hosted MIDI CONTROL panel loaded `1 MIDI CC mappings` and displayed `CC71 -> filter.resonance`.
- [x] Scrolled the hosted editor to the Filter section.
- [x] Sent CC71 value `0` and captured `Resonance 0.00`.
- [x] Sent CC71 value `127` and captured `Resonance 1.00`.
- [x] Sent CC71 value `0` again and captured `Resonance 0.00`.
- [x] Copied the proof sidecar to ignored local evidence, removed the temporary user-level sidecar, stopped the CoreMIDI source, and quit Live without saving the validation set.

## Validation

Ableton log evidence:

- `MidiInDevice [Name="SylenthAI Codex Value Source", Track=true, Sync=false, Remote=false, MPE=false, MIDI Clock Sync Delay=0, Sync Type="MIDI Clock", MTC Frame Rate="All", MTC Start Offset=0]`
- `Vst3: Going to create: sylenth-ai`
- `Vst3: Created: sylenth-ai`

Persisted sidecar evidence:

```json
{"schema_version": 1, "mappings": [{"cc": 71, "parameter_id": "filter.resonance"}]}
```

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `value-proof-live-launched.png`
- `value-proof-vst3-loaded.png`
- `value-proof-editor-scrolled-filter.png`
- `value-proof-cc71-high-resonance.png`
- `value-proof-cc71-low-resonance.png`
- `midi-controller-map-vst3-value-proof.json`

## Findings

The VST3 processor loaded the seeded global user map on construction and the hosted editor displayed the assignment without re-learning it. Live delivered the temporary value source into the VST3 track, the plugin footer MIDI count advanced, and the Filter section's Resonance readout followed CC71 from `0.00` to `1.00` and back to `0.00`.

This proof is intentionally VST3-only and continuous-parameter-only. AU learned-CC mapping, AU value application, VST3 host Forget, and VST3 stepped mapped CC playback still need their own host proof.

## Remaining Gaps

- AU and VST3 automation record/playback.
- AU learned CC mapping and value-application proof in Ableton.
- VST3 host Forget and stepped-parameter mapped CC playback proof.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Retrospective

For value-application proof, seed the controller sidecar before plugin construction, create the virtual MIDI source before launching Live, and capture both low and high visible value readouts in the hosted editor.
