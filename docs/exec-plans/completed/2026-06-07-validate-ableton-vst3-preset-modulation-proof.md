# Validate Ableton VST3 Preset Modulation Editor State

Status: completed 2026-06-07 EDT

## Goal

Prove Ableton can load a current factory preset into the hosted VST3 and expose the preset's modulation and FX editor state.

## Scope

- Use the current local `sylenth-ai` VST3 installed from `build-release-host-matrix`.
- Use the restored Ableton validation set and the hosted VST3 instance on track 2.
- Load `Arp Motion 01` from the hosted preset menu.
- Capture hosted Sound, Modulation, and Effects page evidence after load.
- Record durable host-validation notes only; keep screenshots and generated artifacts under ignored `build/` paths.

## Validation

Host proof:

- The hosted VST3 editor opened on track 2.
- The hosted preset menu listed `Factory / Arps - Arp Motion 01 [arp, chord]`.
- Selecting `Arp Motion 01` and clicking `Load` changed the hosted editor from Init to the factory patch.
- The hosted footer reported `Loaded preset: Arp Motion 01`.
- The hosted Sound page showed non-Init oscillator, filter, LFO, macro, and patch-load estimate state.
- The hosted Modulation page showed `ACTIVE ROUTES (2)` with Ramp and LFO sources and enabled TransMod 1/2 state.
- The hosted Effects page showed global FX enabled with active Saturation, Delay, and Reverb state.
- This pass did not prove AU preset loading, playback after the preset load, or modulation behavior in rendered Ableton audio.

Evidence under `build/reports/ableton/`:

- `preset-modulation-vst3-editor-init.png`
- `preset-modulation-preset-menu-open.png`
- `preset-modulation-arp-motion-selected-before-load.png`
- `preset-modulation-arp-motion-loaded-sound.png`
- `preset-modulation-arp-motion-modulation-page.png`
- `preset-modulation-arp-motion-effects-page.png`

Supporting renderer proof:

- `./build/SylenthAIRender --suite patch-recreation --output-dir build/reports/patch-recreation-ableton-preset-proof` wrote six patch-recreation reports.

## Residual Gaps

- AU/VST3 automation record/playback remains open.
- AU preset loading remains open.
- Playback after preset load remains open.
- Ableton-side modulation behavior exercise beyond editor-state inspection remains open.
- Offline bounce versus realtime comparison remains open.
