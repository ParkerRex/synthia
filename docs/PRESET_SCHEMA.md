# Preset Schema

This document sketches the planned clean-room preset schema. `SPEC.md` remains the source of truth for required fields and behavior.

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

- `~/Music/ParkerX/Synth/Presets`

The editor scans factory presets from bundled plugin resources when running from an installed AU, VST3, or Standalone bundle, falling back to the source `presets/factory` directory for development tools. User presets are scanned from the user preset location. Factory presets are treated as read-only; editor Save As and Duplicate write schema-valid user JSON presets.

Current validation command:

```bash
./build/SynthRender --validate-presets presets/factory --output build/reports/presets.json
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

## Parameter Values

Parameter values should be stored in physical/display domains when stable and clear:

- semitones as numbers,
- milliseconds as numbers,
- dB as numbers,
- enums as strings,
- normalized values only when the parameter is intentionally abstract.

The parameter registry owns conversion between physical values and host-normalized values.

When a preset is loaded through the plugin editor, the processor resets APVTS parameters to registry defaults before applying preset overrides. This prevents values from a previous preset from leaking into presets that intentionally omit optional fields.

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

The current factory preset display names are clean-room safe: `Init` and `Pluck Core 01`.
