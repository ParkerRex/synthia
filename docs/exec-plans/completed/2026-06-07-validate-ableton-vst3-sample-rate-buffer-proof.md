# Validate Ableton VST3 Sample-Rate And Buffer Proof

Status: completed 2026-06-07 EDT

## Goal

Prove the current installed VST3 in Ableton receives host sample-rate and buffer-size changes without losing the hosted editor or invalidating diagnostics.

## Scope

- Use the current local `sylenth-ai` VST3 installed from `build-release-host-matrix`.
- Use the restored Ableton validation set and a freshly loaded VST3 instance from `Plug-Ins > VST3 > ParkerX > sylenth-ai`.
- Change Ableton Preferences > Audio from `44100` / `512 Samples` to `48000` / `256 Samples`.
- Reopen the hosted VST3 editor and verify the plugin footer diagnostics update.
- Restore Ableton Preferences > Audio to `44100` / `512 Samples` after proof.
- Record durable host-validation notes only; keep screenshots and generated artifacts under ignored `build/` paths.

## Validation

Host proof:

- Ableton logged `Vst3: Going to create: sylenth-ai` and `Vst3: Created: sylenth-ai`.
- Baseline hosted VST3 footer showed `SR 44100 Hz` and `Block 512`.
- Ableton Preferences accepted `48000` for sample rate.
- Ableton Preferences accepted `256 Samples` for buffer size.
- Reopened hosted VST3 footer showed `SR 48000 Hz` and `Block 256`.
- Hosted VST3 editor remained responsive and finite after the host change.
- Ableton Preferences were restored to `44100` and `512 Samples`.

Evidence under `build/reports/ableton/`:

- `vst3-sample-buffer-vst3-parkerx-expanded.png`
- `vst3-sample-buffer-baseline.png`
- `vst3-sample-buffer-48000-selected.png`
- `vst3-sample-buffer-48000-256-selected.png`
- `vst3-sample-buffer-plugin-footer-48000-256.png`
- `vst3-sample-buffer-restored-44100-512-confirmed.png`
- `vst3-sample-buffer-ableton-log-excerpt.txt`

## Residual Gaps

- AU/VST3 automation record/playback remains open.
- Ableton-side current preset recreation and modulation exercise remain open.
- Offline bounce versus realtime comparison remains open.
