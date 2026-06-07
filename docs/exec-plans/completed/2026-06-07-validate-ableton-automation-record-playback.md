---
title: Validate Ableton Automation Record Playback
status: completed
summary: Prove AU and VST3 can record and replay host parameter automation in Ableton.
post_build_recap: Ableton Live 11 recorded `Layer A Level` automation on the hosted AU and VST3 tracks, saved a disposable set copy whose XML maps the changed envelopes back to each format's `Layer A Level` automation target, and replayed the automation with visible value changes in Live/plugin UI.
triggers:
  - Reviewing Ableton automation proof.
  - Continuing Phase 1 host validation.
---

# Validate Ableton Automation Record Playback

This completed ExecPlan records the Ableton AU/VST3 automation proof captured on 2026-06-07.

## Scope

- No DSP, preset schema, parameter ID, or plugin source changes.
- No save to the original validation set as durable source evidence.
- Generated Ableton set copies, proof JSON, and screenshots remain ignored build artifacts under `build/reports/ableton/`.

## Completed Work

- Recorded `Layer A Level` automation in Ableton Arrangement view on hosted AU track `1-sylenth-ai`.
- Parsed the copied Ableton set XML and verified the AU source `query:Plugins#Audio%20Units:ParkerX:sylenth-ai`, automation target `23778`, 7 envelope events, min value `0.8328332901`, and max value `0.9639999866`.
- Recorded `Layer A Level` automation in Ableton Arrangement view on hosted VST3 track `2-sylenth-ai`.
- Parsed the copied Ableton set XML and verified the VST3 source `query:Plugins#VST3:ParkerX:sylenth-ai`, automation target `24168`, 12 envelope events, min value `0.8000000119`, and max value `0.9759999514`.
- Replayed VST3 automation and captured visible `Layer A Level` movement from `0.00` to `10.56` with active hosted voices/meters.
- Replayed AU automation and captured visible Live device value movement from `1.97` to `9.84` and back to `1.97` while transport ran.

## Validation Artifacts

- `build/reports/ableton/automation/automation-envelope-proof.json`
- `build/reports/ableton/automation/testing-synth-copy-after-au-vst3-automation.als`
- `build/reports/ableton/automation-vst3-record-started.png`
- `build/reports/ableton/automation-vst3-after-record-pass.png`
- `build/reports/ableton/automation-vst3-playback-early.png`
- `build/reports/ableton/automation-vst3-playback-late.png`
- `build/reports/ableton/automation-au-playback-early.png`
- `build/reports/ableton/automation-au-playback-mid.png`
- `build/reports/ableton/automation-au-playback-late.png`

## Findings

- Ableton `Save a Copy` wrote the second copy into the Desktop validation project folder. The proof copy was moved into `build/reports/ableton/automation/`, and the Desktop duplicate was removed.
- VST3 appears as `PluginDevice` in the Ableton set XML, but the source context is `query:Plugins#VST3:ParkerX:sylenth-ai`; AU appears as `AuPluginDevice`.
- The generated proof report records both plugin formats and passed.

## Remaining Gaps

- Ableton offline bounce versus realtime comparison.
- Ableton audio-diff modulation capture remains unclaimed; standalone modulation-route render proof remains the rendered route behavior contract.
