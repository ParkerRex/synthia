# Documentation Index

This directory holds implementation-facing documentation for Synth. `../SPEC.md` remains the source of truth for product requirements.

## Start Here

- `../README.md`: project orientation.
- `../SPEC.md`: full product specification.
- `../CONTEXT.md`: vocabulary and decision lanes.
- `../AGENTS.md`: workspace rules for coding agents.

## Engineering Docs

- `ARCHITECTURE.md`: component model, planned code layout, realtime boundaries.
- `PRESET_SCHEMA.md`: preset and host-state shape.
- `VALIDATION.md`: tests, render fixtures, host checks, and acceptance metrics.
- `CLEAN_ROOM.md`: legal/evidence/naming rules.
- `PLANS.md`: routing for Programs and ExecPlans.
- `host-validation/`: Ableton and other host validation notes/templates.
  - `host-validation/local-install-troubleshooting.md`: local AU/VST3 install, uninstall, ad-hoc signing, AU validation, and Ableton rescan troubleshooting.
- `programs/`: active multi-slice initiative packets.
- `exec-plans/`: active and completed implementation-slice plans.

## Research

- `research/source-map.md`: vendor docs, host docs, manual-derived facts, inferred target behavior, and unknowns.

## Maintenance Rule

When docs disagree:

1. `SPEC.md` wins on product requirements.
2. `CLEAN_ROOM.md` wins on evidence and safety posture.
3. `docs/programs/active/*/program.md` wins on initiative-level progress and slice order.
4. `docs/exec-plans/active/*.md` wins on one-slice execution details while that slice is active.
5. `ARCHITECTURE.md`, `VALIDATION.md`, and `PRESET_SCHEMA.md` are implementation guidance and must be updated to match spec changes.
