# Synth Workspace

This project is a clean-room software instrument workspace.

Read order for meaningful work:

1. `SPEC.md`
2. `CONTEXT.md`
3. `docs/index.md`
4. `docs/CLEAN_ROOM.md`
5. `docs/ARCHITECTURE.md`
6. `docs/VALIDATION.md`

## Source of Truth

- `SPEC.md` owns durable product requirements.
- `CONTEXT.md` owns local vocabulary.
- `docs/*` owns implementation guidance, evidence hygiene, validation, and schemas.
- Do not introduce product requirements in code comments or build files that are not reflected in `SPEC.md`.

## Clean-Room Rule

- Do not copy third-party source, binaries, GUI assets, presets, names, trade dress, or proprietary parameter data.
- Do not decompile, disassemble, patch, or inspect proprietary binaries.
- Use public documentation, licensed black-box listening/measurement, and original implementation only.
- Shipped names and UI must avoid implying affiliation with FXpansion, ROLI, Roland, SH-101, Deadmau5, or song titles unless legal approval exists.

## Audio Engineering Rules

- No allocation, blocking locks, filesystem I/O, network I/O, or synchronous logging on the realtime audio thread.
- DSP must guard NaN, infinity, denormals, runaway feedback, and invalid parameter states.
- Prefer deterministic tests for oscillator, filter, envelope, LFO, modulation, presets, and host restore.
- Treat Ableton AU/VST3 loading and state restore as required validation, not optional polish.

## Coding Defaults

- Use the repo's build system once scaffolded.
- Prefer JUCE/CMake unless the spec is revised.
- Keep parameter IDs stable and centralized.
- Add regression tests for every DSP bug that can be rendered or measured.
- Keep generated build artifacts out of source docs.

