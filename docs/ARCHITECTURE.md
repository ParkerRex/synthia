# Architecture

sylenth-ai is planned as a JUCE/CMake C++ instrument with AU, VST3, and standalone targets.

## Product Phases

Phase 1 rebuilds the Sylenth-style virtual analog instrument workflow for modern macOS/Ableton AU and VST3 use. The current single-core pluck engine is scaffolding toward a fuller A/B architecture, four oscillator slots, Sylenth-level preset/arp/effects workflow, and a modern original UI.

Phase 2 adds AI-assisted sound and arpeggio creation. Generated patches, chord movement, and arp ideas must compile down to ordinary editable parameters, presets, modulation routes, and sequencer state.

Phase 3 adds conversational VST control. Text requests and reference-sound analysis should produce reversible parameter, modulation, arp, and FX edits without touching the realtime audio thread.

Current scaffold:

- `CMakeLists.txt` configures JUCE `8.0.13` through CMake `FetchContent`, unless `SYLENTH_AI_JUCE_PATH` points at a local checkout.
- `SylenthAIPlugin` builds shared plugin code plus AU, VST3, and Standalone formats with host-facing product name `sylenth-ai`.
- `SylenthAIRender` is the command-line validation executable.
- `SylenthAISmokeTest` is the first CTest target.
- `SylenthAIContractTest` validates parameter registry, layer/oscillator-slot defaults, preset files, and APVTS state round-trip.
- `SylenthAIVoiceCoreTest` validates envelope, LFO reset, voice allocation, and engine note release.
- `SylenthAIDspCoreTest` validates oscillator, filter, ramp, glide, velocity glide, direct routes, TransMod scalers, voice/unison/random/performance modulation sources, layer slot rendering, and FX bypass, delay/tail, panic-clear, and reverb-state safety.
- `SylenthAIRenderCoreSuite` runs the standalone core render harness and writes disposable JSON/WAV artifacts under `build/reports/ctest-core`, including dry and wet factory pluck reports.

## Component Stack

1. `Plugin Shell`
   - JUCE processor/editor wrappers.
   - AU/VST3/standalone target metadata.
   - Host buses, MIDI, parameters, state chunks.

2. `Parameter Registry`
   - Stable parameter IDs.
   - Ranges, units, defaults, display conversion, smoothing, automation flags.
   - Preset and host-state migration anchors.

3. `Voice Allocator`
   - Mono, mono legato, poly, unison.
   - Sustain pedal, all-notes-off, panic.
   - Voice stealing and release draining.

4. `Voice Engine`
   - Per-voice oscillator, envelopes, LFO, ramp, glide, filter, pan, random-on-note.
   - Must preserve independent note-local motion for overlapping notes.

5. `DSP Modules`
   - Band-limited oscillator stack.
   - Mixer and gain compensation.
   - Nonlinear multimode filter.
   - Amp drive and stereo stage.
   - Optional FX.

6. `Modulation System`
   - Direct routes.
   - Eight TransMod-style slots.
   - Voice/unison/random/velocity/performance sources.

7. `Persistence`
   - Host state.
   - Factory/user presets.
   - Schema migrations.

8. `Validation Harness`
   - Standalone render fixtures.
   - Audio metrics.
   - Host smoke tests.

9. `AI Orchestration Layer`
   - Planned Phase 2/3 layer outside the realtime audio path.
   - Converts text prompts, randomization intent, chord/arp intent, or reference-sound analysis into validated parameter and preset edits.
   - Must publish changes through the normal control/preset path.

## Realtime Boundary

The audio thread may:

- read atomically published parameter state,
- process MIDI events,
- render DSP,
- update lock-free diagnostic counters,
- write output buffers.

The audio thread must not:

- allocate memory,
- block on locks,
- perform file or network I/O,
- call synchronous logging,
- parse presets,
- rebuild UI,
- scan directories,
- create or destroy heavyweight resources.

## Planned Source Layout

This is the expected code layout once implementation begins:

- `CMakeLists.txt`: top-level JUCE/CMake project.
- `src/plugin/`: processor, editor, parameters, state.
- `src/dsp/`: oscillator, filter, envelopes, LFO, ramp, amp, FX.
- `src/voice/`: allocator and per-voice render engine.
- `src/modulation/`: source evaluation, direct routes, TransMod slots.
- `src/presets/`: schema, migration, factory preset loader.
- `src/validation/`: standalone render/test helpers.
- `src/ai/`: planned Phase 2/3 prompt, generation, reference-analysis, and reversible-edit orchestration.
- `tests/`: unit and render tests.
- `presets/factory/`: factory presets.
- `fixtures/`: MIDI and render fixtures.
- `docs/`: project docs.

Existing scaffold files:

- `src/plugin/PluginProcessor.h`
- `src/plugin/PluginProcessor.cpp`
- `src/plugin/PluginEditor.h`
- `src/plugin/PluginEditor.cpp`
- `src/plugin/ParameterRegistry.h`
- `src/plugin/ParameterRegistry.cpp`
- `src/presets/PresetManager.h`
- `src/presets/PresetManager.cpp`
- `src/dsp/Envelope.h`
- `src/dsp/Envelope.cpp`
- `src/dsp/Lfo.h`
- `src/dsp/Lfo.cpp`
- `src/dsp/Ramp.h`
- `src/dsp/Ramp.cpp`
- `src/dsp/SynthEngine.h`
- `src/dsp/SynthEngine.cpp`
- `src/dsp/fx/FxChain.h`
- `src/dsp/fx/FxChain.cpp`
- `src/presets/PresetValidator.h`
- `src/presets/PresetValidator.cpp`
- `src/voice/Voice.h`
- `src/voice/Voice.cpp`
- `src/voice/VoiceAllocator.h`
- `src/voice/VoiceAllocator.cpp`
- `src/validation/SynthRender.cpp`
- `tests/smoke/SynthSmokeTest.cpp`
- `tests/smoke/SynthContractTest.cpp`
- `tests/smoke/SynthVoiceCoreTest.cpp`

## Parameter Strategy

Parameter IDs must be stable after public release. UI labels may change; IDs should not.

Recommended ID style:

- `voice.mode`
- `osc.stack_count`
- `osc.saw_level`
- `filter.cutoff_semitones`
- `direct.filter_lfo_semitones`
- `transmod.1.source`
- `macro.motion`

Each parameter needs:

- default,
- physical range,
- normalized conversion,
- display conversion,
- automation flag,
- smoothing policy,
- preset serialization flag.

Current registry status:

- 218 parameter IDs.
- Voice, A/B layer state, four oscillator-slot state, legacy oscillator, filter, amp, envelopes, LFO, ramp, direct modulation, FX, realtime/offline quality modes, macros, and eight TransMod-style slots with physical destination depths are represented.
- Host state uses `AudioProcessorValueTreeState` with schema metadata.

Current editor and preset status:

- `PluginEditor` is a compact, fixed-shell control surface modeled on the Sylenth workflow: a header (preset prev/next/load/save/duplicate, output meter, active voices, derived patch-load estimate, panic, and an always-visible SR/block/peak/MIDI/invalid/architecture diagnostics footer), a persistent Layer A/B selector exposing the `layer.*` mix state, and a `Sound` / `Modulation` / `Effects` tabbed workspace. The `Sound` tab keeps the live core controls visible without scrolling at the default window size (oscillator slots, core oscillator, filter, amp/mod envelopes, LFO, ramp, voice, amp/stereo, macros); `Modulation` holds the destination-grouped direct routes plus the eight TransMod slots; `Effects` is a per-module FX rack with master/quality. Controls are drawn with an original rotary/switch/combo look and grid-packed into stable cells with formatted value readouts. Render-boundary honesty is encoded in the UI: the legacy flat `osc.*` path that currently produces sound is badged `LIVE`, while the `layer.*` / `layer.N.osc.M.*` slot state is badged `STATE` and Layer B reads `STAGED - not yet rendered`, so editing staged state never implies it is audible.
- Editor controls are constructed against `ParameterRegistry` IDs and use APVTS attachments for sliders, combo boxes, and toggles so UI edits reach the same host-automatable parameters as presets and host state.
- `PresetManager` scans bundled factory presets with a source-directory development fallback, scans user presets from `~/Music/ParkerX/sylenth-ai/Presets`, also reads legacy `~/Music/ParkerX/Synth/Presets` presets during the rename transition, validates preset JSON before load, prepares defaults-plus-overrides APVTS state for one-shot replacement, maps canonical `mod_slots` objects into flat TransMod parameters, and writes schema-valid user preset JSON.
- Processor diagnostics expose sample rate, block size, active voices, MIDI event count, invalid sample count, peak, current preset, and binary architecture to the editor without filesystem or UI work on the audio thread.

Current layer and oscillator-slot status:

- `SynthParameters` has two `LayerParameters` records and two `LayerOscillatorParameters` records per layer: A1, A2, B1, and B2.
- Layer A defaults are enabled; Layer B defaults are disabled.
- Layer A oscillator 1 defaults to the primary enabled slot with one saw voice at full level. The other three slots default disabled with zero voices.
- `PluginProcessor` caches the namespaced `layer.*` atomics and publishes them into the realtime parameter snapshot.
- `SynthEngine::setParameters` clamps the layer and slot state at the audio boundary.
- Current audio rendering uses Layer A/B mix state and A1/A2/B1/B2 oscillator slots. Layer A oscillator 1 remains the compatibility gate for the legacy flat `osc.*` source; A2/B1/B2 render through preallocated oscillator stacks mapped from `layer.N.osc.M.*`. One filter, envelope pair, amp, modulation, and FX path are still shared until the later per-layer filter/envelope slice.

Current voice-core status:

- `SynthEngine` consumes note-on, note-off, all-notes-off, and all-sound-off from the plugin processor.
- `VoiceAllocator` supports basic poly allocation, note release, panic, and deterministic random-on-note values.
- `Envelope` and `Lfo` provide deterministic per-voice modulation primitives.
- `OscillatorStack` renders polyBLEP saw/pulse, deterministic noise, sub waveforms, stack detune, and hard sync.
- `Filter` renders semitone-domain L2/L4/B2/B4/H2/H4/Peak2/Notch2/Notch4 nonlinear responses with drive/resonance compensation and interpolated oversampling sub-steps.
- `Voice` applies direct pitch/pulse/cutoff routes, ramp, glide, velocity glide, TransMod scalers, synced or Hz LFO rates, per-voice/mono LFO behavior, amp envelope, amp drive, level, pan spread, unison spread, analog variation, performance MIDI sources, and macro influence.
- `FxChain` applies bypassable post-voice saturation, chorus, tempo-synced delay, and simple reverb. It allocates delay buffers during `prepare` and does not allocate in sample processing. `quality.realtime_mode` and `quality.offline_mode` select conservative processing variations without changing audio-thread resource allocation.
- `SylenthAIRender` can write oscillator, filter, modulation, voice, preset validation, dry-core factory pluck, wet factory pluck, LFO ablation, determinism, and core-suite summary reports using the requested preset and MIDI fixture.

## DSP Priorities

Highest priority:

- Phase 1 Sylenth rebuild roadmap and Ableton AU/VST3 proof.
- Per-layer filters/envelopes, layer mixer/master polish, and deeper oscillator-slot parity on top of the delivered layer/slot renderer.
- Preset browser, arpeggiator, effects, and modulation UX that support the Sylenth-level workflow.

Lower priority:

- Phase 2 AI generation,
- Phase 3 conversational/reference-sound editing,
- extended modulation processors beyond the Phase 1 workflow.
