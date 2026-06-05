# Synth Context

Synth is a lab-built macOS/Ableton software instrument. Phase 1 rebuilds the Sylenth1 vintage VST experience for today's AU/VST3 hosts. Phase 2 adds AI-assisted sound and arpeggio generation. Phase 3 adds conversational VST control and reference-sound recreation.

## Core Vocabulary

- `Synth`: working project name, not necessarily final product name.
- `Spec`: `SPEC.md`, the durable product contract.
- `Lab`: the product context for rebuilding a vintage VST workflow and extending it with modern AI-native capabilities.
- `Phase 1`: recreate the Sylenth-style instrument workflow and sound-design surface as a modern macOS/Ableton AU/VST3.
- `Phase 2`: AI-assisted sound, patch, chord movement, and arpeggio generation, with generated output stored as normal editable synth state.
- `Phase 3`: conversational control where text prompts or reference sounds produce reversible synth, modulation, arp, and FX edits.
- `Modern Sylenth baseline`: `docs/modern-sylenth-baseline.md`, backed by `Sylenth1Manual.pdf` and `research/sylenth1-screenshots/`.
- `Historical Strobe research`: older supporting engine research that helped bootstrap the current pluck core. It is not the product destination.
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

The main product thesis:

- Recreate Sylenth first: fast A/B virtual analog workflow, recognizable oscillator/filter/modulation/arp/effects coverage, and high trust in Ableton.
- Then add AI where it improves musical speed: patch ideation, safe randomization, arpeggios/chord movement, and reference-sound matching.
- Then make the VST conversational so users can ask for edits instead of hunting through every parameter.

The implementation posture:

- Preserve the working dry core while Phase 1 grows toward the full Sylenth-level architecture.
- Validate every sound-path change with deterministic tests and Ableton host proof.
- Keep AI-generated output inspectable, reversible, and represented as ordinary synth/preset state.
- Keep UI production-oriented and fast rather than nostalgic for its own sake.

## Decision Lanes

- Hard product behavior: update `SPEC.md`.
- Phase roadmap and Sylenth feature baseline: update `docs/modern-sylenth-baseline.md`.
- Engineering rationale and surfaces: update `docs/ARCHITECTURE.md`.
- Evidence, citations, historical confidence: update `docs/research/source-map.md`.
- Evidence, safety, and naming constraints: update `SPEC.md`, `docs/research/source-map.md`, or the relevant implementation doc.
- Test strategy and metrics: update `docs/VALIDATION.md`.
- Preset/persistence details: update `docs/PRESET_SCHEMA.md`.
