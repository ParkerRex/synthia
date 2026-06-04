# Cross-Repo Review

This review records lessons imported from the Worker planning system and constraints local to Synth.

## Worker Planning Lessons Applied

Worker separates durable Programs from executable plans. Synth follows that shape:

- `docs/programs/active/2026-06-04-synth-clean-room-pluck-instrument/` owns initiative memory.
- `docs/exec-plans/active/` owns slice implementation.
- `planning-brief-1.md` is immutable and referenced by each child ExecPlan.
- `current-planning-brief.txt` is a pointer, not provenance for already-written child plans.

Worker also treats active plans as living documents. Synth child ExecPlans must be updated as implementation discovers real build, JUCE, host, and DSP constraints.

## Synth-Specific Constraints

Synth is currently an empty documentation-first repo. The first implementation slice must create the build system before any test commands can be truly runnable.

There is no existing package manager or CI. Plans should name likely commands but allow the foundation slice to finalize exact build scripts.

The clean-room policy is stricter than a normal product build. Plans must avoid copied names, copied presets, copied UI, and binary reverse engineering.

Host validation depends on local DAW availability. Command-based proof should use standalone and plugin build checks where possible, then pair with explicit Ableton manual smoke steps when automation is not available.

## Watchpoints

- Do not let the final product name drift without updating bundle IDs, preset names, docs, and clean-room checks.
- Do not add FX before validating dry-core behavior.
- Do not skip standalone validation; it is the only reliable command-line render path.
- Do not add UI controls before the parameter registry owns stable IDs.
- Do not make Ableton proof a release-only task.

