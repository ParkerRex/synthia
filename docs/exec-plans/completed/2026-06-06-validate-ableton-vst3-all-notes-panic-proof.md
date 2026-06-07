# Validate Ableton VST3 All-Notes And Panic Proof

Status: completed 2026-06-06 EDT

## Goal

Prove the current installed VST3 in Ableton clears sustained voices through host MIDI all-notes-off, host MIDI all-sound-off, and the hosted editor Panic button.

## Scope

- Use the current local `sylenth-ai` VST3 installed from `build-release-host-matrix`.
- Route a temporary CoreMIDI source into Ableton.
- Capture before/after screenshots for CC123, CC120, and hosted Panic.
- Record durable host-validation notes only; keep screenshots, logs, and generated artifacts under ignored `build/` paths.

## Validation

Build/install proof completed before host validation:

```bash
cmake --build build-release-host-matrix --config Release -j2
ctest --test-dir build-release-host-matrix --output-on-failure
./build-release-host-matrix/SylenthAIRender --suite core --output-dir build-release-host-matrix/reports/core
scripts/check-plugin-bundles.sh build-release-host-matrix
scripts/install-local-plugins.sh build-release-host-matrix Release
auval -v aumu SyAI PkRx
```

Host proof:

- Ableton restored the current VST3 and exposed `SylenthAI Codex Panic Source` as a track-enabled MIDI input.
- Two sustained notes drove the hosted editor to `Voices 2/8`.
- MIDI CC123 all-notes-off cleared the hosted editor to `Voices 0/8`.
- MIDI CC120 all-sound-off cleared the hosted editor to `Voices 0/8`.
- Hosted Panic cleared sustained voices from `Voices 2/8` to `Voices 0/8` and reported `Panic sent: voices and FX cleared`.
- The temporary MIDI source was silenced and quit after proof.

Evidence under `build/reports/ableton/`:

- `panic-proof-vst3-ready-routed.png`
- `panic-proof-vst3-held-before-cc123.png`
- `panic-proof-vst3-after-cc123-all-notes-off.png`
- `panic-proof-vst3-held-before-cc120.png`
- `panic-proof-vst3-after-cc120-all-sound-off.png`
- `panic-proof-vst3-held-before-panic.png`
- `panic-proof-vst3-after-panic-button.png`
- `panic-proof-vst3-ableton-log-excerpt.txt`

## Residual Gaps

- AU-specific all-notes-off, all-sound-off, and hosted Panic proof remains open.
- AU/VST3 automation record/playback remains open.
- Ableton-side current preset recreation and modulation exercise remain open.
- Offline bounce versus realtime comparison remains open.
- Sample-rate and buffer-size changes remain open.
