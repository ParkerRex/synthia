# Research Pass: Spec And Project Docs

This pass records the durable inputs that define the Synth build program.

## Inputs Reviewed

- `SPEC.md`
- `CONTEXT.md`
- `docs/ARCHITECTURE.md`
- `docs/CLEAN_ROOM.md`
- `docs/VALIDATION.md`
- `docs/PRESET_SCHEMA.md`
- `docs/research/source-map.md`
- Worker planning contracts under `/Users/bazelrex/Developer/worker/docs/programs/` and `/Users/bazelrex/Developer/worker/docs/exec-plans/`

## Stable Findings

Synth is a clean-room software instrument for early-Strobe-v1-like analog plucks. The product target is behavior, not a literal third-party clone.

The first engineering risk is not the basic oscillator. The high-risk behavior is per-voice note-local modulation, semitone-domain filter movement, nonlinear filter drive/resonance interaction, and controlled stereo/voice variation.

The expected technology path is JUCE plus CMake with AU, VST3, and standalone targets. The standalone target is not optional because deterministic render validation needs a host-independent runner.

The plugin must be realtime-safe. No implementation slice may normalize allocations, locks, file I/O, network I/O, or synchronous logging on the audio thread.

The clean-room policy is a hard constraint across the whole program. Shipped code, names, UI, factory presets, and docs cannot copy or imply affiliation with third-party products, artists, or songs.

## Track Implications

The work should be split into separately validatable slices. A single giant "build the synth" plan would hide the DSP risk and make host validation too late.

The first slice must establish build and validation scaffolding. Later slices can then add sound-engine modules with tests instead of waiting for a full plugin before any proof exists.

The parameter registry and preset/state contract must land early because every DSP and UI slice depends on stable parameter IDs.

Host and packaging work should land after the core plugin and UI exist, but Ableton compatibility must still be planned from the start.

## Open Inputs

- Final product name and bundle identifier.
- Minimum supported macOS version.
- JUCE license posture.
- Whether Windows VST3 joins v1.
- Exact manual black-box reference materials approved for measurement.

