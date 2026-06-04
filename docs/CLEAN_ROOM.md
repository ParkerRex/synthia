# Clean-Room Policy

Synth must be an original instrument. It may be inspired by publicly documented synthesis architecture and by legally obtained listening references, but it must not copy proprietary implementation, product identity, factory content, or visual trade dress.

## Allowed Inputs

- Public vendor documentation and manuals.
- Public host/plugin-format documentation.
- Public forum/community discussion, clearly marked as lower-confidence.
- Licensed black-box use of commercially obtained plugins for listening and measurement.
- Original DSP design and standard published synthesis techniques.
- Original presets created from scratch.
- Original UI designs that serve the same workflow without copying protected layouts or visual identity.

## Disallowed Inputs

- Decompiled, disassembled, patched, or memory-inspected proprietary binaries.
- Copied source code from proprietary products.
- Copied preset data, parameter dumps, factory names, or assets.
- Copied GUI panels, knob layouts, screenshots, logos, typography, trade dress, or visual skins.
- Product names or shipped factory names that imply endorsement or affiliation with FXpansion, ROLI, Roland, SH-101, Deadmau5, or specific songs.

## Evidence Classes

Use these labels in research docs and design notes:

- `Observed`: public/vendor/manual/host documentation or directly inspected local project material.
- `Inferred`: reasoned design conclusion from observed facts.
- `Unknown`: unresolved; do not encode as a fact.

## Naming Rules

Internal research aliases may use historical reference names when useful for discussion. Shipped user-facing content must not.

Examples:

- Internal alias: `FB_StrobeLike_Pluck_01`
- Shipped preset name: `Pluck Core 01`

The final product name and bundle identifier must be selected before public builds. They must be owned, distinctive, and legally reviewable.

## Measurement Rules

Black-box measurement is allowed only when:

- The reference product is lawfully obtained and run under its license.
- Measurement observes audio/MIDI/parameter behavior through normal user-facing controls.
- No private code, memory, files, or proprietary preset data are copied.
- Results are recorded as target ranges or behavioral notes, not as copied implementation.

## Documentation Rule

Historical claims must not be promoted from `Inferred` to `Observed` unless backed by stronger source material. Specs and code should target behavior, not mythology.

