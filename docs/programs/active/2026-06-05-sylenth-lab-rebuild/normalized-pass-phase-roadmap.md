# Normalized Pass: Phase Roadmap

## Phase 1: Recreate Sylenth

Required tracks:

- Layer/state backbone: A/B parts, layer select, solo/mute, mixer, master output, host-state restore.
- Oscillator model: four slots across A/B, per-slot waveform, voices/unison, tuning, detune, phase/retrigger/invert, level, pan, and stereo spread.
- Per-part sound path: filters, amp envelopes, modulation envelopes, LFO routing, layer mixing, FX send/post-mix behavior.
- Modulation UX and routing: route model, source tiles, hover inspection, control halos, matrix sync.
- Preset workflow: bank/folder browsing, prev/next, search/tags, favorites, author/notes, dirty state, init/randomize/reset, safe save/overwrite.
- Arp/step/chord workflow: host-synced rate, mode, gate, octave/wrap, velocity, hold/tie, 16-step lanes, deterministic output.
- FX rack: distortion/saturation, phaser, chorus/flanger, EQ, delay, reverb, compressor, bypass, mix, tail reporting.
- UI shell and polish: original modern UI informed by screenshots/manual, implemented through Claude Code handoff after engine/state contracts exist.
- Ableton proof: AU/VST3 scan/load/play, automation, save/reopen, bounce, sample-rate/buffer-size changes, UI open/close while playing.

## Phase 2: AI Sound And Arp Generation

Required tracks:

- Bounded randomization that produces finite, musical, editable presets.
- Sound generation from text or intent into parameter/modulation/FX state.
- Chord movement and arpeggio generation inspired by composition-helper workflows.
- Generation provenance in preset metadata.
- Validation for seeds, bounds, finite renders, and editable state.

## Phase 3: Conversational VST Control

Required tracks:

- Text command parsing into reversible parameter diffs.
- Reference-sound analysis into synth/modulation/arp/FX edit proposals.
- User confirmation or A/B review before destructive replacement.
- Reported diffs explaining changed controls and musical intent.
- Strict separation from realtime audio processing.

