# Research Source Map

This file records source material and confidence levels for the sylenth-ai spec. It is evidence support, not a replacement for `../../SPEC.md`.

## Current Verified Sources

### Host and Framework

Observed:

- Ableton documents macOS AU, VST2, and VST3 support, AUv2/AUv3 support as of Live 11.3, default plugin folders, Apple Silicon notes, and VST3 64-bit support.
  - https://help.ableton.com/hc/en-us/articles/209068929-Using-AU-and-VST-plug-ins-on-macOS
- JUCE CMake API documents plugin target formats including `Standalone`, `VST3`, and `AU`.
  - https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md
- Steinberg VST3 developer documentation describes `.vst3` macOS bundles and universal binary placement.
  - https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Locations%2BFormat/Plugin%2BFormat.html

### Modern Sylenth Phase 1

Observed:

- `../../Sylenth1Manual.pdf` is the local manual source for the Phase 1 rebuild. It documents Sylenth1's A/B parts, oscillator/filter/envelope/modulation/LFO behavior, polyphony and voices display, preset/program workflow, arpeggiator, master FX, sync behavior, MIDI control, and CPU guidance.
- `../../research/sylenth1-screenshots/SOURCE_INDEX.md` maps 25 local Sylenth UI screenshots to source pages and image URLs. These screenshots are accepted Phase 1 evidence for UI density, section grouping, preset views, skin/visual variation, arpeggiator/effects panels, routing diagrams, filter controls, oscillator/mix panels, top-panel workflow, and factory-bank browsing views.
- `../modern-sylenth-baseline.md` translates the manual and screenshot corpus into the current feature matrix, roadmap, and validation requirements.

Inferred:

- Phase 1 should recreate the Sylenth-level instrument workflow before adding broader synthesis families.
- The screenshot corpus is approved target evidence for visual parity: layout density, control grouping, panel rhythm, visual hierarchy, preset/LCD workflow, knob styling, and screenshot-to-screenshot comparison should drive UI decisions.

### AI Extension References

Inferred:

- Phase 2 should add AI-assisted patch randomization, sound generation, chord movement, and arpeggio generation in the spirit of fast composition helpers such as Xfer Records' Cthulhu, while storing results as normal editable preset and parameter state.
- Phase 3 should add conversational editing: text prompts and reference sounds produce reversible changes to synth parameters, modulation routes, arp/chord state, and FX.

### FXpansion / ROLI Lineage

Observed:

- FXpansion documents DCAM Synth Squad as a legacy product and describes the DCAM synths, including Strobe, at a high level.
  - https://www.fxpansion.com/products/dcamsynthsquad/
- FXpansion support documents DCAM Synth Squad as superseded by Cypher2/Strobe2 and no longer actively tested/developed.
  - https://www.fxpansion.com/support/faq/dcamsynthsquad/
- ROLI documents Strobe2 as inspired by SH-101, OB-1, and Pro-One, with a single super oscillator, stacked copies, TransMod, built-in effects, and 16 TransMod slots.
  - https://support.roli.com/support/solutions/articles/36000396689-strobe2-faqs
- ROLI states Strobe2 is not a drop-in replacement for Strobe v1 and that imported v1 presets can sound different due to engine differences.
  - https://support.roli.com/support/solutions/articles/36000396689-strobe2-faqs
- ROLI documents Strobe2/Cypher2 Universal Binaries and an Apple Silicon native oscillator issue where Rosetta is recommended for correct sound.
  - https://support.roli.com/support/solutions/articles/36000582347-cypher2-strobe2-compatibility-with-apple-silicon

## Manual-Derived Architecture From Supplied Brief

Observed from the supplied research brief:

- Strobe v1 is described as a single-oscillator analog-modelled performance synth inspired by Roland SH-series instruments, especially SH-09/SH-101, plus Oberheim OB-1 and SCI Pro-One.
- The oscillator section includes stack/detune, saw/pulse/noise, sub oscillator waveforms, hard sync, pulse width, and direct modulation.
- The filter section describes an OTA cascaded core with diode clipping in the feedback path, semitone cutoff, self-oscillation, drive/resonance interaction, and many multimode responses.
- The amp section describes overload/waveshaping, voice pan modulation, level, and an analog instability control.
- The modulation pages describe gateable modulators, LFO sync/Hz modes, phase reset on gating, mono versus per-voice behavior, TransMod, glide, velocity glide, voice/unison sources, and modulation scaling.

## Community/Historical Claims

Inferred:

- The legacy pluck engine scaffold should be treated as early-Strobe-v1-like background rather than the current product target.
- The legacy pluck signature is more likely per-voice cutoff movement plus filter/drive/stereo variation than wavetable synthesis.

Unknown:

- Exact identity of the historical white `SH-101`-style plugin.
- Exact binary, preset, parameter scaling, GUI implementation, or custom modifications.
- Exact filter mode and effect chain used in canonical tracks.

## Research Hygiene

- Keep historical claims in this file unless they become accepted product requirements in `SPEC.md`.
- Do not promote community claims to observed facts without stronger evidence.
- Do not use copied preset data or decompiled binary behavior.
- Prefer behavior targets and measurable ranges over mythology.
