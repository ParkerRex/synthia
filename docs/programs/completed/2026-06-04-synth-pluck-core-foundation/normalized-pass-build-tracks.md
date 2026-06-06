# Normalized Pass: Build Tracks

This pass normalizes the research and spec into implementation tracks.

## Track 1: Foundation

Purpose: create the repo's buildable JUCE/CMake foundation, local conventions, and initial test runner.

Dependencies: none.

Outputs:

- `CMakeLists.txt`
- source/test directories
- minimal plugin processor/editor
- standalone target
- first smoke test

## Track 2: State Contract

Purpose: define stable parameter IDs, host state, preset JSON, migration hooks, and factory preset storage.

Dependencies: foundation.

Outputs:

- parameter registry
- host state serialization
- preset parser/writer
- migration tests

## Track 3: Core Voice And Modulators

Purpose: build MIDI note handling, voice allocation, envelopes, LFO gate modes, and basic render path.

Dependencies: foundation and parameter registry.

Outputs:

- voice allocator
- note-on/off behavior
- amp/mod envelopes
- LFO Hz/sync/gate behavior
- deterministic modulation tests

## Track 4: Oscillator And Mixer

Purpose: build the stackable oscillator, waveform mix, sub oscillator, hard sync, and loudness compensation.

Dependencies: voice core and parameter registry.

Outputs:

- band-limited saw/pulse
- noise and sub oscillator
- stack count/detune
- oscillator tests

## Track 5: Filter And Drive

Purpose: build the nonlinear filter core and oversampling path.

Dependencies: oscillator/mixer.

Outputs:

- L2/L4/B2/B4/H2/H4 filters
- semitone cutoff conversion
- drive/resonance interaction
- filter metrics

## Track 6: Full Modulation

Purpose: build direct modulation, TransMod-style slots, ramp, glide, velocity glide, random, voice, and unison sources.

Dependencies: voice/modulator foundation, oscillator, filter, state contract.

Outputs:

- direct route evaluation
- eight TransMod slots
- modulation source registry
- glide/ramp behavior

## Track 7: Amp, Stereo, And Factory Pluck

Purpose: complete the dry-core musical sound with amp drive, pan spread, analog variation, and factory pluck preset.

Dependencies: oscillator, filter, full modulation.

Outputs:

- amp/stereo stage
- analog instability controls
- `Pluck Core 01`
- dry-core musical render

## Track 8: Validation Harness

Purpose: make render fixtures, metrics, and reports first-class.

Dependencies: enough dry core to render.

Outputs:

- fixture runner
- audio metrics
- JSON reports
- regression artifacts

## Track 9: UI And Preset Workflow

Purpose: expose the instrument through a compact lab-authored editor and user preset workflow.

Dependencies: parameter/state contract and dry core.

Outputs:

- editor UI
- preset picker/save/duplicate
- modulation slot editing
- diagnostics view

## Track 10: FX And Quality

Purpose: add optional saturation/delay/reverb/chorus and quality/oversampling modes without hiding dry-core behavior.

Dependencies: dry core, state, validation.

Outputs:

- bypassable FX chain
- quality modes
- FX render tests

## Track 11: Host Integration And Packaging

Purpose: prove Ableton AU/VST3 loading, state restore, installation layout, signing/notarization prep, and universal builds.

Dependencies: plugin core and UI.

Outputs:

- AU/VST3 install validation
- Ableton smoke checklist
- package scripts
- architecture/signing checks

## Track 12: Release Hardening

Purpose: close the whole product loop with docs, performance, legal naming review, production validation, and deferred follow-up ledger.

Dependencies: all build tracks.

Outputs:

- production readiness report
- final docs sync
- performance report
- release checklist

