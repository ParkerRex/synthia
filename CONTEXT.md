# Synth Context

Synth is a clean-room macOS/Ableton software instrument for early-Strobe-v1-like analog plucks.

## Core Vocabulary

- `Synth`: working project name, not necessarily final product name.
- `Spec`: `SPEC.md`, the durable product contract.
- `Clean-room`: original implementation based on public facts, accepted requirements, and legal black-box measurement.
- `Historical target`: the inferred early Strobe-v1-like / SH-101-inspired pluck workflow. It is not a confirmed binary identity.
- `Dry core`: oscillator, mixer, filter, amp, envelopes, LFO, ramp, glide, modulation, and voice spread with FX bypassed.
- `Pluck profile`: the factory sound-design target defined in `SPEC.md` Appendix A.
- `Voice`: one per-note synthesis state, including oscillator phases, envelopes, LFO, ramp, glide, filter, pan, and random source.
- `Unison voice`: one detuned/panned copy within a note allocation.
- `Stack`: multiple oscillator phase accumulators inside one voice, distinct from post-voice unison.
- `Direct modulation`: dedicated routes such as LFO/envelope/keytrack to oscillator pitch, pulse width, and filter cutoff.
- `TransMod-style slot`: one source, optional scaler, many destination depths.
- `Semitone cutoff`: cutoff modulation accumulated in musical semitone space before conversion to Hz.
- `Voice spread`: per-voice or per-unison pan, pitch, cutoff, envelope, or timing variation.

## Design Posture

The main sonic thesis:

- The oscillator is intentionally simple.
- The filter, drive, per-voice modulation, and voice spread are the high-risk/high-value areas.
- A static saw through a clean filter with a global LFO is not conformant.

The implementation posture:

- Build the dry core first.
- Validate note-local movement before adding polish FX.
- Keep UI clean-room and production-oriented rather than retro-copyist.

## Decision Lanes

- Hard product behavior: update `SPEC.md`.
- Engineering rationale and surfaces: update `docs/ARCHITECTURE.md`.
- Evidence, citations, historical confidence: update `docs/research/source-map.md`.
- Safety and naming constraints: update `docs/CLEAN_ROOM.md`.
- Test strategy and metrics: update `docs/VALIDATION.md`.
- Preset/persistence details: update `docs/PRESET_SCHEMA.md`.

