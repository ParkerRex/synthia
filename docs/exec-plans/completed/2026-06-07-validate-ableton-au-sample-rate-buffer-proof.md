# Validate Ableton AU Sample-Rate And Buffer Proof

Status: completed 2026-06-07 EDT

## Goal

Prove the current installed AU in Ableton receives host sample-rate and buffer-size changes without losing the hosted editor or invalidating diagnostics.

## Scope

- Use the current local `sylenth-ai` AU installed from `build-release-host-matrix`.
- Use the restored Ableton validation set and existing AU instance.
- Change Ableton Preferences > Audio from `44100` / `512 Samples` to `48000` / `256 Samples`.
- Reopen the existing hosted AU editor and verify the plugin footer diagnostics update.
- Restore Ableton Preferences > Audio to `44100` / `512 Samples` after proof.
- Record durable host-validation notes only; keep screenshots and generated artifacts under ignored `build/` paths.

## Validation

Host proof:

- Baseline hosted AU footer showed `SR 44100 Hz` and `Block 512`.
- Ableton Preferences accepted `48000` for sample rate.
- Ableton Preferences accepted `256 Samples` for buffer size.
- Reopened hosted AU footer showed `SR 48000 Hz` and `Block 256`.
- Hosted AU editor remained responsive and finite after the host change.
- Ableton Preferences were restored to `44100` and `512 Samples`.

Evidence under `build/reports/ableton/`:

- `sample-buffer-baseline.png`
- `sample-buffer-48000-selected.png`
- `sample-buffer-48000-256-selected.png`
- `sample-buffer-plugin-footer-48000-256-hotswap-cleared.png`
- `sample-buffer-restored-44100-512-confirmed-2.png`

## Residual Gaps

- VST3 sample-rate and buffer-size proof remains open.
- AU/VST3 automation record/playback remains open.
- Ableton-side current preset recreation and modulation exercise remain open.
- Offline bounce versus realtime comparison remains open.
