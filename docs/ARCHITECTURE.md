# Architecture

Synth is planned as a JUCE/CMake C++ instrument with AU, VST3, and standalone targets.

Current scaffold:

- `CMakeLists.txt` configures JUCE `8.0.13` through CMake `FetchContent`, unless `SYNTH_JUCE_PATH` points at a local checkout.
- `SynthPlugin` builds shared plugin code plus AU, VST3, and Standalone formats.
- `SynthRender` is the command-line validation executable.
- `SynthSmokeTest` is the first CTest target.
- `SynthContractTest` validates parameter registry, preset files, and APVTS state round-trip.
- `SynthVoiceCoreTest` validates envelope, LFO reset, voice allocation, and engine note release.

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
- `tests/`: unit and render tests.
- `presets/factory/`: clean-room factory presets.
- `fixtures/`: MIDI and render fixtures.
- `docs/`: project docs.

Existing scaffold files:

- `src/plugin/PluginProcessor.h`
- `src/plugin/PluginProcessor.cpp`
- `src/plugin/PluginEditor.h`
- `src/plugin/PluginEditor.cpp`
- `src/plugin/ParameterRegistry.h`
- `src/plugin/ParameterRegistry.cpp`
- `src/dsp/Envelope.h`
- `src/dsp/Envelope.cpp`
- `src/dsp/Lfo.h`
- `src/dsp/Lfo.cpp`
- `src/dsp/SynthEngine.h`
- `src/dsp/SynthEngine.cpp`
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

- 102 parameter IDs.
- Voice, oscillator, filter, amp, envelopes, LFO, ramp, direct modulation, FX, macros, and eight TransMod-style slots are represented.
- Host state uses `AudioProcessorValueTreeState` with schema metadata.

Current voice-core status:

- `SynthEngine` consumes note-on, note-off, all-notes-off, and all-sound-off from the plugin processor.
- `VoiceAllocator` supports basic poly allocation, note release, panic, and deterministic random-on-note values.
- `Envelope` and `Lfo` provide deterministic per-voice modulation primitives.
- `OscillatorStack` renders clean-room polyBLEP saw/pulse, deterministic noise, sub waveforms, stack detune, and hard sync.
- `Filter` renders semitone-domain L2/L4/B2/B4/H2/H4/Peak2/Notch2/Notch4 nonlinear responses with drive/resonance compensation and interpolated oversampling sub-steps.
- `Voice` applies direct pitch/cutoff routes, synced or Hz LFO rates, per-voice/mono LFO behavior, amp envelope, amp drive, level, pan spread, unison spread, analog variation, and macro influence.
- `SynthRender` can write oscillator, filter, voice, preset validation, and dry-core factory pluck reports using the requested preset and MIDI fixture.

## DSP Priorities

Highest priority:

- full TransMod slot routing,
- ramp, glide, and velocity glide,
- richer deterministic render tests,
- UI/preset workflow,
- Ableton AU/VST3 proof.

Lower priority:

- onboard FX,
- full compound filter mode set,
- preset browser UI,
- extended modulation processors.
