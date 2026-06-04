# Converged Decision Packet

This packet records the accepted direction for the Synth implementation program.

## Product Direction

Build Synth as a clean-room JUCE/CMake software instrument with AU, VST3, and standalone targets. The target is a Strobe-v1-like, SH-101-inspired analog pluck workflow, not a literal clone or brand replica.

## Architecture Direction

Use a layered architecture:

- host adapter,
- parameter/state contract,
- realtime voice engine,
- DSP modules,
- modulation system,
- preset persistence,
- UI editor,
- validation harness,
- packaging/release tooling.

The standalone target and validation harness are required, not afterthoughts.

## Slice Strategy

Split the work by dependency and proof surface:

1. foundation,
2. state contract,
3. voice/modulator core,
4. oscillator,
5. filter,
6. full modulation,
7. amp/stereo/factory pluck,
8. validation harness,
9. UI,
10. FX/quality,
11. host/packaging,
12. release hardening.

This order keeps tests close to the code that needs them and prevents UI or packaging work from obscuring DSP correctness.

## Boundary Decisions

- Do not add VST2 to v1.
- Do not add accounts, telemetry, cloud sync, or copy protection to v1.
- Do not copy third-party UI/preset/name/trade-dress assets.
- Do not implement FX before the dry core is musically validated.
- Keep Windows and Linux as optional extensions unless the spec changes.

## Validation Decision

Every slice must have a narrow command-based proof. The whole program is not complete until Ableton AU and VST3 loading, host state restore, deterministic render fixtures, DSP metrics, performance checks, and clean-room naming checks pass.

