# Planning Brief 1: Build Synth End To End

This is the immutable first planning brief for the Synth clean-room pluck instrument Program.

## Program

Program ID: `synth-clean-room-pluck-instrument`

Program path: `docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/program.md`

## Source Requirements

Use these local source-of-truth docs:

- `SPEC.md`
- `CONTEXT.md`
- `docs/research/source-map.md`
- `docs/ARCHITECTURE.md`
- `docs/VALIDATION.md`
- `docs/PRESET_SCHEMA.md`
- `docs/research/source-map.md`

## Build Outcome

The end state is a clean-room JUCE/CMake software instrument that builds AU, VST3, and standalone targets on macOS, implements the dry-core Strobe-v1-like pluck architecture, exposes a usable editor, validates renders through a standalone harness, loads in Ableton, and has packaging/release readiness docs.

## Child Plan Split

Implement these slices in order:

1. Scaffold JUCE/CMake plugin foundation.
2. Build parameter, state, and preset contract.
3. Build voice, MIDI, envelope, and LFO core.
4. Build oscillator stack and mixer.
5. Build nonlinear filter, drive, and oversampling.
6. Build modulation matrix, ramp, and glide.
7. Build amp, stereo, analog, and factory pluck.
8. Build render validation harness and metrics.
9. Build editor UI and preset workflow.
10. Build onboard FX and quality modes.
11. Build Ableton host integration and packaging.
12. Build release hardening and documentation closeout.

## Boundary Rules

- Keep `SPEC.md` as product truth.
- Do not copy third-party code, UI, presets, names, or binary behavior.
- Do not add VST2, telemetry, accounts, or cloud features.
- Do not make the audio thread allocate, block, or perform I/O.
- Do not use onboard FX to pass dry-core validation.
- Keep every child plan independently validatable.

## First Slice Instruction

Start with `docs/exec-plans/active/2026-06-04-scaffold-juce-cmake-plugin-foundation.md`.

That slice must create the buildable project, local command surface, minimal plugin/standalone targets, initial test harness, and docs sync needed for later slices.
