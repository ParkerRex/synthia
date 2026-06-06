---
title: Validate Ableton VST3 Controller Learn Proof
status: completed
summary: Prove the current VST3 learns an incoming CC in Ableton and persists the global MIDI controller map.
post_build_recap: Ableton Live 11 loaded the current sylenth-ai VST3, enumerated a temporary CoreMIDI validation source with Track enabled, routed that source into track 1, and the hosted plugin learned CC71 for `filter.resonance`. The plugin wrote the expected `MidiControllerMap.json` mapping and displayed the assignment in the hosted editor. Arrangement automation recording was attempted but did not capture a clip or envelope, and no visible post-learn parameter-value sweep artifact was captured, so automation record/playback and controller value-application proof remain open.
triggers:
  - Reviewing Ableton learned-CC controller proof.
  - Continuing host validation after VST3 hosted editor lifecycle proof.
  - Debugging MIDI Learn or controller-map persistence in Live.
---

# Validate Ableton VST3 Controller Learn Proof

This ExecPlan closes the VST3 learned-CC capture/persistence proof gap for the current hosted Ableton build. It does not close AU controller proof, visible post-learn controller value-application proof, or AU/VST3 automation record/playback.

## Context

The current implementation has a compact global MIDI Learn panel, a global user sidecar at `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json`, and message-thread APVTS updates for mapped CC values. Contract tests prove map normalization and sidecar roundtrip, but Ableton host proof was still open.

## Completed Work

- [x] Kept the current `build-release-phase1-ableton` VST3 installed in Live.
- [x] Created a temporary CoreMIDI source named `SylenthAI Codex CC Source`.
- [x] Restarted Ableton so Live enumerated that source at launch.
- [x] Verified Ableton `Log.txt` listed `MidiInDevice [Name="SylenthAI Codex CC Source", Track=true, Sync=false, Remote=false, MPE=false]`.
- [x] Loaded the current `sylenth-ai` VST3 on a fresh Live `Untitled` MIDI track.
- [x] Explicitly selected `SylenthAI Codex CC Source` as track 1's MIDI input after `All Ins` did not initially pass the source into the plugin.
- [x] Verified the hosted plugin footer MIDI count advanced and Ableton meters moved from the validation source.
- [x] Armed MIDI Learn for `FILTER / Resonance` in the hosted editor.
- [x] Captured incoming CC71 and persisted `filter.resonance` in `MidiControllerMap.json`.
- [x] Copied the proof sidecar to ignored local evidence and removed the temporary user-level sidecar after the pass.
- [x] Attempted a short Arrangement record pass from the CC/note stream; Arrangement view showed no captured clip or envelope, so automation record/playback remains open.

## Validation

Ableton log evidence:

- `MidiInDevice [Name="SylenthAI Codex CC Source", Track=true, Sync=false, Remote=false, MPE=false, MIDI Clock Sync Delay=0, Sync Type="MIDI Clock", MTC Frame Rate="All", MTC Start Offset=0]`
- `Vst3: Going to create: sylenth-ai`
- `Vst3: Created: sylenth-ai`

Persisted sidecar evidence:

```json
{"schema_version": 1, "mappings": [{"cc": 71, "parameter_id": "filter.resonance"}]}
```

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `live-relaunched-with-virtual-midi-source.png`
- `controller-proof-vst3-loaded-with-virtual-source.png`
- `midi-input-menu-open.png`
- `midi-input-source-selected-enter.png`
- `midi-after-second-virtual-source.png`
- `midi-learn-vst3-resonance-mapped.png`
- `midi-controller-map-vst3-proof.json`
- `arrangement-after-editor-closed.png`

## Findings

Live recognized the validation MIDI source on launch with `Track=true`, but `All Ins` did not pass the first CC stream into the hosted plugin. Explicitly selecting `SylenthAI Codex CC Source` on track 1 fixed routing, after which the plugin footer MIDI count advanced and MIDI Learn captured CC71.

The global MIDI Learn panel and controller-map persistence work inside the hosted VST3 editor. The temporary user sidecar was absent before the proof, written during the proof, copied to `build/reports/ableton/midi-controller-map-vst3-proof.json`, and removed afterward to avoid polluting future host runs.

The attempted Arrangement record pass produced live MIDI input, but Arrangement view did not show a captured clip or automation envelope. This pass also did not capture a visible post-learn parameter-value sweep artifact. Keep automation record/playback and controller value-application proof as separate host-validation slices.

## Remaining Gaps

- AU and VST3 automation record/playback.
- AU learned CC mapping proof in Ableton.
- Visible post-learn controller value-application proof.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Retrospective

For future controller validation, create the CoreMIDI source before launching Live, confirm it appears in `Log.txt`, and explicitly select it on the target MIDI track before arming MIDI Learn. Do not rely on `All Ins` for this validation path.
