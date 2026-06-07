# Validate Ableton AU All-Notes And Panic Proof

Status: completed 2026-06-07 EDT

## Goal

Prove the current installed AU in Ableton clears sustained voices through host MIDI all-notes-off, host MIDI all-sound-off, and the hosted editor Panic button.

## Scope

- Use the current local `sylenth-ai` AU installed from `build-release-host-matrix`.
- Load the AU from `Audio Units > ParkerX > sylenth-ai`.
- Route a temporary CoreMIDI source into Ableton.
- Capture before/after screenshots for CC123, CC120, and hosted Panic.
- Record durable host-validation notes only; keep screenshots, logs, and generated artifacts under ignored `build/` paths.

## Validation

Host proof:

- Ableton loaded the current AU and logged `Au: Going to create: sylenth-ai` then `Au: Created: sylenth-ai`.
- Ableton exposed `SylenthAI Codex Panic Source` as a track-enabled MIDI input.
- Two sustained notes drove the hosted AU editor to `Voices 2/8`.
- MIDI CC123 all-notes-off cleared the hosted AU editor to `Voices 0/8`.
- MIDI CC120 all-sound-off cleared the hosted AU editor to `Voices 0/8`.
- Hosted AU Panic cleared sustained voices from `Voices 2/8` to `Voices 0/8` and reported `Panic sent: voices and FX cleared`.
- The temporary MIDI source was silenced and quit after proof.

Evidence under `build/reports/ableton/`:

- `au-panic-proof-audio-units-key-expanded.png`
- `au-panic-proof-parkerx-expanded.png`
- `au-panic-proof-after-au-load-return.png`
- `au-panic-proof-held-before-cc123.png`
- `au-panic-proof-after-cc123-all-notes-off.png`
- `au-panic-proof-held-before-cc120.png`
- `au-panic-proof-after-cc120-all-sound-off.png`
- `au-panic-proof-held-before-panic-live-activated.png`
- `au-panic-proof-after-panic-button-retry.png`
- `au-panic-proof-ableton-log-excerpt.txt`

## Residual Gaps

- AU/VST3 automation record/playback remains open.
- Ableton-side current preset recreation and modulation exercise remain open.
- Offline bounce versus realtime comparison remains open.
- Sample-rate and buffer-size changes remain open.
