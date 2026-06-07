# Validate Ableton AU Preset Playback Proof

Status: completed 2026-06-07 EDT

## Goal

Close the remaining AU playback-after-preset-load proof using the hosted AU instance that already loaded `Arp Motion 01`.

## Scope

- Use hosted AU on track 1 in the restored Ableton validation set.
- Use `Factory / Arps - Arp Motion 01 [arp, chord]` loaded in the hosted AU.
- Route a temporary CoreMIDI source into the AU through `All Ins` with Monitor set to `In`.
- Capture active hosted voice/output evidence.
- Do not claim host automation, offline-vs-realtime comparison, or rendered modulation-behavior comparison.

## Validation

Host proof:

- Hosted AU track 1 had `Arp Motion 01` loaded.
- Temporary CoreMIDI source `SylenthAI Codex AU Playback Source 2` repeatedly sent note-ons, then sent note-offs plus CC123 all-notes-off before exiting.
- During the note stream, the hosted AU editor showed `2/8` voices, `Peak -21.0 dB`, `MIDI 683`, and active Live track/master meters.
- The AU Modulation page showed `ACTIVE ROUTES (2)` and the expected Ramp/LFO routes after the source pass.

Evidence under `build/reports/ableton/`:

- `au-preset-playback-ready-for-midi-source.png`
- `au-preset-playback-midi-source-held-2.png`
- `au-preset-playback-modulation-page.png`

## Residual Gaps

- AU/VST3 automation record/playback remains open.
- Rendered modulation-behavior comparison beyond route visibility remains open.
- Offline bounce versus realtime comparison remains open.
