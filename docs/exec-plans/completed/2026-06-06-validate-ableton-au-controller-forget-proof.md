---
title: Validate Ableton AU Controller Forget Proof
status: completed
summary: Prove the hosted AU global MIDI CONTROL panel can forget a persisted CC assignment and stop applying that CC in Ableton.
post_build_recap: Ableton Live 11 loaded the current sylenth-ai AU from `Audio Units > ParkerX > sylenth-ai` with a seeded CC72 to `filter.resonance` sidecar, showed CC72 driving Resonance before Forget, removed the mapping from the hosted global MIDI CONTROL panel, wrote an empty sidecar, and kept Resonance at `0.00` when CC72 high was sent afterward.
triggers:
  - Reviewing Ableton AU global-panel MIDI Forget proof.
  - Continuing host validation after AU seeded value and AU in-editor Learn proof.
  - Debugging stale controller mappings in the hosted AU editor.
---

# Validate Ableton AU Controller Forget Proof

This ExecPlan closes the AU global-panel MIDI Forget proof gap. It does not close AU/VST3 automation record/playback, per-control context-menu Learn/Forget, controller/automation conflict behavior, current preset recreation, modulation exercise, bounce-versus-realtime comparison, sample-rate/buffer-size changes, all-notes-off, or panic proof.

## Context

The AU value proof showed that a seeded global controller map loads in the AU and drives `filter.resonance`. The AU Learn proof showed that the hosted panel can capture a new mapping. AU Forget still needed a direct host proof because removing a mapping uses a separate UI/control path, persists a sidecar write, republishes realtime-safe controller indexes, and must stop later mapped CC application without blocking incoming MIDI.

## Completed Work

- [x] Built and installed the Release AU from `build-release-host-matrix`, then passed CTest, core render validation, bundle checks, and `auval -v aumu SyAI PkRx`.
- [x] Seeded `~/Music/ParkerX/sylenth-ai/MidiControllerMap.json` with `CC72 -> filter.resonance` before plugin construction.
- [x] Created a temporary CoreMIDI source named `SylenthAI Codex AU Forget Source` before launching Ableton.
- [x] Launched Ableton after creating the source and verified Live logged it as `Track=true`.
- [x] Loaded the current `sylenth-ai` AU from `Audio Units > ParkerX > sylenth-ai` into a fresh Ableton `Untitled` set.
- [x] Verified Ableton logged `Au: Going to create: sylenth-ai` and `Au: Created: sylenth-ai` for this pass.
- [x] Sent CC72 value `127` before Forget and captured the hosted Filter Resonance readout at `1.00`.
- [x] Sent CC72 value `0` before Forget and captured the hosted Filter Resonance readout at `0.00`.
- [x] Scrolled to the hosted MIDI CONTROL panel and verified it displayed `Loaded 1 MIDI CC mappings` and `CC72 -> filter.resonance`.
- [x] Selected `FILTER / Resonance`, clicked Forget, and verified the panel displayed `Forgot MIDI CC for filter.resonance` plus `No MIDI CC mappings`.
- [x] Verified the post-Forget sidecar persisted an empty `mappings` array.
- [x] Sent CC72 value `127` after Forget and captured the hosted Filter Resonance readout still at `0.00` while the MIDI counter advanced.
- [x] Copied the proof sidecars and Ableton log excerpt to ignored local evidence, removed the temporary user-level sidecar, stopped the CoreMIDI source, and quit Live without saving the validation set.

## Validation

Pre-host build/install validation:

```bash
cmake -S . -B build-release-host-matrix -DCMAKE_BUILD_TYPE=Release -DSYLENTH_AI_ENABLE_TESTS=ON -DSYLENTH_AI_JUCE_PATH=/Users/parkerrex/Developer/sylenth-ai/build/_deps/juce-src
cmake --build build-release-host-matrix --config Release -j2
ctest --test-dir build-release-host-matrix --output-on-failure
./build-release-host-matrix/SylenthAIRender --suite core --output-dir build-release-host-matrix/reports/core
scripts/check-plugin-bundles.sh build-release-host-matrix
scripts/install-local-plugins.sh build-release-host-matrix Release
auval -v aumu SyAI PkRx
```

Ableton log evidence:

- `MidiInDevice [Name="SylenthAI Codex AU Forget Source", Track=true, Sync=false, Remote=false, MPE=false, MIDI Clock Sync Delay=0, Sync Type="MIDI Clock", MTC Frame Rate="All", MTC Start Offset=0]`
- `2026-06-06T21:05:08.955600: info: Au: Going to create: sylenth-ai`
- `2026-06-06T21:05:10.064453: info: Au: Created: sylenth-ai`

Seeded sidecar evidence:

```json
{"schema_version":1,"mappings":[{"cc":72,"parameter_id":"filter.resonance"}]}
```

Post-Forget sidecar evidence:

```json
{"schema_version": 1, "mappings": []}
```

Evidence screenshots and local proof artifacts are ignored build outputs under `build/reports/ableton/`:

- `midi-controller-map-au-forget-proof-seeded.json`
- `midi-controller-map-au-forget-proof-after-forget.json`
- `au-forget-proof-ableton-log-excerpt.txt`
- `au-forget-proof-before-forget-cc72-high.png`
- `au-forget-proof-before-forget-cc72-low.png`
- `au-forget-proof-filter-resonance-selected.png`
- `au-forget-proof-after-forget-panel.png`
- `au-forget-proof-after-forget-cc72-high.png`

## Findings

The hosted AU global MIDI CONTROL panel removed the selected `filter.resonance` assignment, wrote an empty controller-map sidecar, and refreshed the panel to `No MIDI CC mappings`. After the removal, Live still delivered incoming MIDI from the temporary source into the AU, but the fixed controller mapping no longer applied CC72 to Resonance.

This proof is intentionally AU-only and global-panel-only. Per-control context menus and controller/automation conflict behavior remain separate validation surfaces.

## Remaining Gaps

- AU and VST3 automation record/playback.
- Current preset recreation and modulation exercise.
- Offline bounce versus realtime comparison.
- Sample-rate and buffer-size changes.
- All-notes-off/panic proof.

## Retrospective

For AU Forget proof, seed the controller sidecar before plugin construction, create the virtual MIDI source before launching Live, prove the mapping moves the visible parameter before Forget, remove it from the hosted MIDI CONTROL panel, verify the sidecar is empty, and send the same CC again to prove MIDI still arrives but no mapped parameter change occurs.
