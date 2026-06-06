# Preset Schema

This document sketches the planned preset schema. `SPEC.md` remains the source of truth for required fields and behavior.

## Format

Presets should be JSON:

- deterministic key order when written by tools,
- human-inspectable,
- versioned,
- migratable,
- no copied third-party data.

Current factory presets:

- `presets/factory/init.json`
- `presets/factory/pluck-core-01.json`

Current user preset location:

- `~/Music/ParkerX/sylenth-ai/Presets`

The preset browser also scans the legacy `~/Music/ParkerX/Synth/Presets` path so local presets saved before the project rename remain visible. New user preset writes go to the `sylenth-ai` path.

The editor scans factory presets from bundled plugin resources when running from an installed AU, VST3, or Standalone bundle, falling back to the source `presets/factory` directory for development tools. User presets are scanned from the user preset location. Factory presets are treated as read-only; editor Save As and Duplicate write schema-valid user JSON presets.

Current validation command:

```bash
./build/SylenthAIRender --validate-presets presets/factory --output build/reports/presets.json
```

## Required Top-Level Fields

- `schema_version`: integer.
- `plugin_min_version`: semantic version string.
- `id`: stable lower-kebab-case preset ID.
- `display_name`: shipped user-facing name.
- `author`: string.
- `description`: string.
- `tags`: lowercase slug list.
- `parameters`: map of parameter ID to value.
- `mod_slots`: list of TransMod-style slot objects.
- `macros`: list of macro objects.
- `metadata`: optional object.

User preset writes currently include `metadata.program = "sylenth_lab_rebuild"`.

Phase 2 and Phase 3 may extend `metadata` with generation provenance, prompt text, seed, model/version identifiers, reference-analysis summaries, and reversible edit history. Those fields must not be required for normal audio rendering.

## Parameter Values

Parameter values should be stored in physical/display domains when stable and clear:

- semitones as numbers,
- milliseconds as numbers,
- dB as numbers,
- enums as strings,
- normalized values only when the parameter is intentionally abstract.

The parameter registry owns conversion between physical values and host-normalized values.

When a preset is loaded through the plugin editor, the processor resets APVTS parameters to registry defaults before applying preset overrides. This prevents values from a previous preset from leaking into presets that intentionally omit optional fields.

Current FX and quality fields are ordinary serialized parameters. `fx.enabled` is the global FX bypass; module bypasses use `fx.saturation_enabled`, `fx.delay_enabled`, `fx.reverb_enabled`, and `fx.chorus_enabled`. Delay sync is stored as an enum string such as `1/8`. Realtime and offline quality are stored as `quality.realtime_mode` and `quality.offline_mode`.

## Layer and Oscillator Slot State

Phase 1 now serializes a Sylenth-style A/B layer backbone as ordinary parameter state. Layer indices are numeric in IDs:

- `layer.1.*`: Layer A.
- `layer.2.*`: Layer B.

Layer fields:

- `layer.N.enabled`: boolean.
- `layer.N.level_db`: dB, `-48.0` through `12.0`, default `0.0`.
- `layer.N.pan`: normalized bipolar pan, `-1.0` through `1.0`, default `0.0`.
- `layer.N.solo`: boolean.
- `layer.N.mute`: boolean.

Each layer owns two oscillator-slot records, giving four slots total: `layer.1.osc.1`, `layer.1.osc.2`, `layer.2.osc.1`, and `layer.2.osc.2`.

Oscillator-slot fields:

- `layer.N.osc.M.enabled`: boolean.
- `layer.N.osc.M.voices`: integer-like voice count, `0` through `8`; `0` is a disabled-slot-compatible value.
- `layer.N.osc.M.waveform`: enum string: `Saw`, `Pulse`, `Noise`, or `Sub`.
- `layer.N.osc.M.octave`: integer-like octave offset, `-4` through `4`.
- `layer.N.osc.M.note`: integer-like semitone offset, `-12` through `12`.
- `layer.N.osc.M.fine_cents`: cents, `-100.0` through `100.0`.
- `layer.N.osc.M.level`: normalized level, `0.0` through `1.0`.
- `layer.N.osc.M.phase_degrees`: degrees, `0.0` through `360.0`.
- `layer.N.osc.M.detune`: normalized detune, `0.0` through `1.0`.
- `layer.N.osc.M.stereo`: normalized stereo spread, `0.0` through `1.0`.
- `layer.N.osc.M.pan`: normalized bipolar pan, `-1.0` through `1.0`.
- `layer.N.osc.M.retrigger`: boolean.
- `layer.N.osc.M.invert`: boolean.

Defaults preserve the current sound path:

- Layer A is enabled.
- Layer B is disabled.
- Layer A oscillator 1 is enabled with `voices = 1`, `waveform = Saw`, and `level = 1.0`.
- The other three oscillator slots are disabled with `voices = 0` and `level = 0.0`.

The existing flat `osc.*`, `filter.*`, envelope, modulation, amp, and FX parameters remain host-stable. Layer A oscillator 1 gates and mixes the legacy `osc.*` compatibility source; A2/B1/B2 render from `layer.N.osc.M.*` slot fields through the current oscillator stack foundation. Legacy presets that omit `layer.*` fields load by registry defaults; host states that predate these fields are merged over registry defaults before restore. Newly saved user presets include the layer and oscillator-slot fields.

Legacy presets are not fully inferred into equivalent oscillator-slot state yet. For example, a preset that renders from flat stack, pulse, sub, and detune parameters will still show the default Layer A slot backbone unless it explicitly stores layer fields. UI should treat Layer A oscillator 1 as the compatibility source for the flat oscillator path, not as a complete visualization of every legacy `osc.*` field.

Layer display names are not automatable parameters. If custom names are added later, they should live in preset metadata or UI-local state with an explicit migration rule.

## Mod Slot Shape

Example:

```json
{
  "slot_id": 1,
  "enabled": true,
  "source": "LFO",
  "scaler": "Macro1",
  "depth_domain": "Physical",
  "depths": {
    "filter.cutoff_semitones": 12.0,
    "amp.pan": 0.25
  }
}
```

Rules:

- `slot_id` must be 1 through 8 for the v1 profile.
- `source` is required when `enabled` is true.
- `scaler` may be `None`.
- Unknown destinations should be preserved during migration when possible.

## Macro Shape

Example:

```json
{
  "id": "motion",
  "display_name": "Motion",
  "value": 0.5,
  "assignments": [
    {
      "target_id": "direct.filter_lfo_semitones",
      "min": 4.0,
      "max": 24.0,
      "curve": "linear"
    }
  ]
}
```

The current factory `space` macro declares assignments to `fx.delay_mix` and `fx.reverb_mix`. Runtime processing also maps `macro.space` into delay and reverb wetness so the preset remains useful even before a richer macro-assignment engine exists.

## Migration Rules

- Schema versions are monotonic integers.
- Every migration must be tested.
- Unknown fields should be preserved when possible.
- Unknown parameter IDs should warn, not crash.
- Invalid numeric values may be clamped only during explicit migration.
- Host state must include enough data to restore without external preset files.

## Factory Preset Naming

Shipped factory presets must avoid unlicensed third-party marks.

Internal research names may exist in `metadata` during development, but release builds should omit or sanitize them.

The current factory preset display names are `Init` and `Pluck Core 01`.
