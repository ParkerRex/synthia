---
title: Build Preset Workflow UI Controls
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Bind the preset workflow model to visible dirty, Init, Random, Reset, and local A/B compare controls.
post_build_recap: Added a processor workflow snapshot, dirty baseline storage, compare-slot capture/recall APIs, a header dirty-state pill, a Sound-page Preset Workflow panel, standalone normal/compact screenshot evidence, and manual A Store to A Load control smoke.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
read_when:
  - Reviewing preset workflow UI bindings.
  - Reviewing the later metadata/safe-save UI dependency.
  - Checking what preset workflow controls are already visible.
---

# Build Preset Workflow UI Controls

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The preset workflow state model existed, but the editor still required visible, model-backed controls for the producer workflow: know whether a patch is clean or edited, initialize, randomize, reset, and compare A/B states without fake UI state.

## Progress

- [x] 2026-06-06 EDT: Attempted the Claude Code handoff with `claude -p --output-format stream-json --verbose`; Claude CLI was session-limited, so the slice was implemented locally.
- [x] 2026-06-06 EDT: Added `SynthAudioProcessor::PresetWorkflowSnapshot` for current preset metadata, dirty state, reset availability, and compare-slot readiness.
- [x] 2026-06-06 EDT: Added processor APIs for local A/B compare capture and recall, backed by existing `PresetManager` compare-slot helpers.
- [x] 2026-06-06 EDT: Kept dirty baselines as immutable registry-ordered fingerprints after load, save, init, randomize, compare recall, and host restore.
- [x] 2026-06-06 EDT: Added a header dirty-state pill and a Sound-page `PRESET WORKFLOW` panel with Init, Random, Reset, A Store, A Load, B Store, and B Load.
- [x] 2026-06-06 EDT: Captured normal, scrolled, compact, and after-A-store standalone screenshots under `build/reports/ui/`.
- [x] 2026-06-06 EDT: Ran manual control smoke: A Store captured the current state, updated the status line, and enabled A Load.
- [x] 2026-06-06 EDT: Updated README, architecture, validation, baseline, Program, and ExecPlan indexes.

## Surprises & Discoveries

- The standalone window clamps near 1080x788 for compact testing, but that still exercises the tight editor layout.
- The preset workflow panel sits below the arp/step/chord row, so compact/default-height users reach it by scrolling.
- Generated Init and Randomized states become the current clean baseline when applied; subsequent parameter edits show as dirty against that generated state.

## Decision Log

Decision: Keep A/B compare slots local and non-persistent.
Rationale: Compare slots are session workflow state, not preset sound contract.
Date: 2026-06-06.

Decision: Surface workflow commands in the Sound page instead of crowding the header.
Rationale: The header already carries preset navigation, load/save, metering, voices, patch cost, and panic. A full command row fits better near the browser and sequencer.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. The editor now exposes the preset workflow model with visible dirty status, Init/Random/Reset, and local A/B compare controls. The controls call processor APIs, not UI-only placeholders, and the processor updates dirty baselines consistently across preset load/save/command/host-restore paths.

## In Scope

- Header dirty-state indicator.
- Sound-page preset workflow panel.
- Init, Random, Reset buttons.
- A/B compare Store and Load controls.
- Processor workflow snapshot and compare-slot APIs.
- Standalone screenshot/manual-control smoke.
- Docs and ExecPlan updates.

## Out Of Scope

- Metadata editor.
- Safe overwrite prompt.
- Invalid-preset visible error surface.
- Bank import/export.
- Delete, insert, copy/paste.
- Persisted A/B compare slots.
- DSP, parameter registry, or preset schema changes.

## Validation and Acceptance

Acceptance required:

- Visible workflow controls bind to real processor/preset APIs.
- Dirty state is derived from current APVTS state versus immutable baseline fingerprint.
- A/B load buttons are disabled until their slot is captured.
- Compact and normal standalone layouts do not clip the command row.
- Existing build, tests, and core render suite continue to pass.

Validation run:

```bash
git diff --check
cmake --build build --config Debug
ctest --test-dir build --output-on-failure
./build/SylenthAIRender --suite core --output-dir build/reports/core
```

Manual UI proof:

- `build/reports/ui/preset-workflow-ui-tall.png`
- `build/reports/ui/preset-workflow-ui-workflow.png`
- `build/reports/ui/preset-workflow-ui-compact-workflow.png`
- `build/reports/ui/preset-workflow-ui-after-a-store.png`

## Follow-Up

- Metadata editing and explicit safe-save controls later landed in `2026-06-06-build-preset-safe-save-metadata-ui.md`.
- Add invalid-preset visible error rows.
- Decide whether workflow controls should move closer to the header after the final Sylenth-style layout pass.
