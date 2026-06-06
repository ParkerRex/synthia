---
title: Validate Ableton AU Controller Value Proof
status: completed
summary: Prove a persisted AU MIDI CC mapping visibly drives one continuous mapped parameter in Ableton.
post_build_recap: Ableton Live 11 loaded the current sylenth-ai AU from `Audio Units > ParkerX > sylenth-ai` with a seeded CC72 to `filter.resonance` sidecar, enumerated a temporary CoreMIDI validation source, and visibly drove the hosted AU editor's Resonance value from `0.00` to `1.00` and back to `0.00`. The temporary user sidecar was copied to ignored evidence and removed after the pass.
triggers:
  - Reviewing Ableton AU controller value-application proof.
  - Continuing host validation after VST3 controller Learn/value/Forget proof.
  - Debugging mapped CC value application in Live.
---

# Validate Ableton AU Controller Value Proof

This ExecPlan closes the AU seeded controller-map loading and continuous value-application proof gap. It did not itself close AU in-editor MIDI Learn capture, which was later covered by `2026-06-06-validate-ableton-au-controller-learn-proof.md`; AU/VST3 automation record/playback remains open.

## Context

The VST3 controller proofs showed that Live can route a CoreMIDI source into the hosted plugin, that the global MIDI Learn panel can persist assignments, and that persisted mappings drive continuous and stepped parameters. AU-specific value application still needed a fresh host proof because AU construction, host MIDI routing, and editor behavior are separate surfaces from VST3.

## Completed Work

- [x] Seeded `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json` with `CC72 -> filter.resonance` before plugin construction.
- [x] Created a temporary CoreMIDI source named `SylenthAI Codex AU Value Source` before launching Ableton.
- [x] Launched Ableton after creating the source and verified Live logged it as `Track=true`.
- [x] Loaded the current `sylenth-ai` AU from `Audio Units > ParkerX > sylenth-ai` into a fresh Ableton `Untitled` set.
- [x] Verified Ableton logged `Au: Going to create: sylenth-ai` and `Au: Created: sylenth-ai` for this pass.
- [x] Verified the hosted MIDI CONTROL panel loaded `1 MIDI CC mappings` and displayed `CC72 -> filter.resonance`.
- [x] Scrolled the hosted editor to the Filter section.
- [x] Captured the initial `Resonance 0.00` state.
- [x] Sent CC72 value `127` and captured `Resonance 1.00`.
- [x] Sent CC72 value `0` and captured `Resonance 0.00`.
- [x] Copied the proof sidecar and Ableton log excerpt to ignored local evidence, removed the temporary user-level sidecar, stopped the CoreMIDI source, and quit Live without saving the validation set.

## Validation

Ableton log evidence:

- `MidiInDevice [Name="SylenthAI Codex AU Value Source", Track=true, Sync=false, Remote=false, MPE=false, MIDI Clock Sync Delay=0, Sync Type="MIDI Clock", MTC Frame Rate="All", MTC Start Offset=0]`
- `2026-06-06T14:44:51.525961: info: Au: Going to create: sylenth-ai`
- `2026-06-06T14:44:51.698979: info: Au: Created: sylenth-ai`

Persisted sidecar evidence:

```json
{"schema_version":1,"mappings":[{"cc":72,"parameter_id":"filter.resonance"}]}
```

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `au-value-proof-live-fresh-launched.png`
- `au-value-proof-au-loaded-after-return.png`
- `au-value-proof-filter-visible-before-cc-2.png`
- `au-value-proof-cc72-high-resonance.png`
- `au-value-proof-cc72-low-resonance.png`
- `midi-controller-map-au-value-proof-current.json`
- `au-value-proof-ableton-log-excerpt.txt`

## Findings

The AU processor loaded the seeded global user map on construction and the hosted editor displayed the assignment without re-learning it. Live delivered CC72 from the temporary validation source into the AU track through `All Ins`, the plugin footer MIDI count advanced from `0` to `2`, and the Filter section's Resonance readout followed CC72 from `0.00` to `1.00` and back to `0.00`.

This proof is intentionally AU-only and seeded-map-only. AU in-editor MIDI Learn capture was a separate host proof because this pass did not arm the AU Learn button and capture a fresh assignment inside the AU editor; that follow-on proof is now recorded in `2026-06-06-validate-ableton-au-controller-learn-proof.md`.

## Remaining Gaps

- AU and VST3 automation record/playback.
- AU global-panel MIDI Forget proof in Ableton.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Retrospective

For AU value-application proof, seed the controller sidecar before plugin construction, create the virtual MIDI source before launching Live, confirm the AU create log after loading from the Audio Units browser branch, and capture low/high/low visible value readouts in the hosted editor.
