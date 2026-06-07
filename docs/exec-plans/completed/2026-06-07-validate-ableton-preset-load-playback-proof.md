# Validate Ableton Preset Load Playback Proof

Status: completed 2026-06-07 EDT

## Goal

Narrow the remaining Ableton preset proof gap by validating current hosted preset loading in AU and playback after preset load in VST3.

## Scope

- Use the restored Ableton validation set.
- Use hosted AU on track 1 and hosted VST3 on track 2.
- Load or reuse `Factory / Arps - Arp Motion 01 [arp, chord]`.
- Capture hosted editor evidence only; keep screenshots under ignored `build/` paths.
- Do not claim host automation, offline-vs-realtime comparison, or rendered modulation-behavior comparison.

## Validation

Host proof:

- Hosted VST3 on track 2 had `Arp Motion 01` loaded.
- Launching the track 2 validation MIDI clip after the preset load drove the hosted VST3 to active voice/audio state: `1/8` voices, `Peak -15.9 dB`, `MIDI 212`, and active Live meters.
- Opening the hosted VST3 Modulation page during playback showed `ACTIVE ROUTES (2)` with the Ramp and LFO routes while voices, peak, MIDI count, and Live meters remained active.
- Stopping transport returned the hosted VST3 to `0/8` voices.
- Hosted AU on track 1 opened as `sylenth-ai/1-sylenth-ai`.
- Hosted AU preset menu listed `Factory / Arps - Arp Motion 01 [arp, chord]`.
- Loading `Arp Motion 01` changed the AU editor from Init to the factory patch and the footer reported `Loaded preset: Arp Motion 01`.

Evidence under `build/reports/ableton/`:

- `vst3-preset-playback-after-load.png`
- `vst3-preset-playback-modulation-page.png`
- `vst3-preset-playback-stopped-cleanup-2.png`
- `au-preset-playback-menu-open.png`
- `au-preset-playback-arp-selected-before-load.png`
- `au-preset-playback-arp-loaded-sound.png`

## Residual Gaps

- AU playback after preset load remains open.
- AU/VST3 automation record/playback remains open.
- Rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison remains open.
