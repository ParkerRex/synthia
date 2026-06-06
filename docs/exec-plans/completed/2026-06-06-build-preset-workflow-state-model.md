---
title: Build Preset Workflow State Model
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Add backend preset workflow helpers for metadata-aware writes, safe-save policy, dirty-state comparison, and A/B compare slots.
post_build_recap: Added `PresetWriteOptions`, create-new versus overwrite-existing save modes, metadata-aware preset serialization, registry-ordered state fingerprints, dirty-state comparison against immutable baselines, local A/B compare slot capture/prepare helpers, no-clobber create-only writes, and contract tests.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
read_when:
  - Reviewing preset dirty indicators or safe-save controls.
  - Reviewing preset metadata UI dependencies.
  - Adding A/B compare controls.
  - Reviewing preset save or command-state behavior.
---

# Build Preset Workflow State Model

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The preset browser and command model were real, but the UI roadmap still needed backend surfaces for safe-save policy, dirty indicators, metadata editing, and A/B compare. This slice adds those model helpers without adding visible UI or new preset JSON fields for transient UI state.

## Progress

- [x] 2026-06-06 EDT: Added `PresetWriteOptions` with metadata and create-new versus overwrite-existing policy.
- [x] 2026-06-06 EDT: Preserved the existing `writeCurrentPreset(..., displayName, ...)` API as an overwrite-compatible wrapper.
- [x] 2026-06-06 EDT: Added metadata-aware writes for display name, author, description, tags, bank, and category.
- [x] 2026-06-06 EDT: Added registry-ordered preset-state fingerprints for current APVTS state and prepared `ValueTree` state.
- [x] 2026-06-06 EDT: Added dirty-state comparison against immutable baseline fingerprints carried by load/init/randomize/compare-slot results.
- [x] 2026-06-06 EDT: Added local A/B compare slot capture and prepare-for-recall helpers.
- [x] 2026-06-06 EDT: Added contract tests for clean/dirty detection, create-only rejection, overwrite replacement, metadata serialization, and A/B slot prepare.
- [x] 2026-06-06 EDT: Updated validation, preset schema, baseline, Program, and ExecPlan indexes.
- [x] 2026-06-06 EDT: Patched autoreview findings: dirty baselines no longer alias shared APVTS `ValueTree` state, and create-new saves now use a no-clobber final write boundary.

## Surprises & Discoveries

- JUCE `AudioProcessorValueTreeState::copyState()` is non-const, so compare-slot capture correctly takes a mutable APVTS reference.
- Dirty comparison should ignore host bookkeeping properties like current preset name/path. Registry-ordered parameter fingerprints keep that comparison scoped to serialized sound state.
- `AudioProcessorValueTreeState::replaceState()` can keep shared `ValueTree` state mutable after replacement. Dirty indicators should store `PresetLoadResult::fingerprint` instead of reusing a load result's `ValueTree` as the baseline.
- A/B compare slots are best treated as local control-path snapshots. They should not become preset JSON fields unless a future workflow explicitly asks to persist compare slots.

## Decision Log

Decision: Fingerprint serialized parameter values in registry order.
Rationale: Dirty checks should be deterministic, independent of `ValueTree` child order, and scoped to real preset sound state.
Date: 2026-06-06.

Decision: Keep A/B compare slots out of preset JSON.
Rationale: Compare slots are local workflow state, not part of a patch's sound contract.
Date: 2026-06-06.

Decision: Keep overwrite behavior compatible by preserving the old write API.
Rationale: Existing Save As / Duplicate paths should not change behavior until UI safe-save prompts are wired.
Date: 2026-06-06.

Decision: Create-new preset writes must be no-clobber at the final filesystem write boundary.
Rationale: A preflight existence check alone can race with another creator; the create-new contract is only true if the final write refuses an existing destination.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. `PresetManager` now has the backend model that Claude/UI can bind to for dirty state, safe-save controls, richer metadata editing, and A/B compare. Dirty state compares the live APVTS state against an immutable fingerprint baseline, and create-new saves use no-clobber writes. The changes do not affect audio-thread behavior and do not add transient workflow fields to preset JSON.

## Context and Orientation

Relevant current code and docs:

- `src/presets/PresetManager.*`
- `tests/smoke/SynthContractTest.cpp`
- `docs/PRESET_SCHEMA.md`
- `docs/VALIDATION.md`
- `docs/modern-sylenth-baseline.md`

### In Scope

- Metadata-aware preset writes.
- Create-new versus overwrite-existing save policy with no-clobber create-new writes.
- Dirty-state fingerprinting and immutable-baseline comparison.
- A/B compare slot capture and prepare-for-recall helpers.
- Contract tests and docs.

### Out Of Scope

- Visible UI buttons, prompts, indicators, or metadata editor.
- Persisted A/B compare slots.
- Delete/insert/copy/paste bank workflows.
- AI provenance metadata.
- Ableton host validation.

## Plan of Work

1. Add preset write options and safe-save policy.
2. Add metadata-aware serialization.
3. Add deterministic state fingerprinting.
4. Add dirty-state comparison.
5. Add A/B compare state helpers.
6. Add contract tests.
7. Update durable docs.

## Milestones

Milestone 1 added save policy and metadata serialization.

Milestone 2 added dirty-state fingerprints and A/B state helpers.

Milestone 3 added contract coverage.

Milestone 4 updated docs and ledgers.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake --build build --config Debug
    ./build/SylenthAIContractTest
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Validation and Acceptance

Acceptance requires:

- Existing preset save API remains compatible.
- Create-new writes reject existing destinations without overwriting them, including at the final write boundary.
- Overwrite-existing writes replace existing destinations.
- Metadata write options serialize display name, author, description, tags, bank, and category.
- Dirty-state comparison is clean immediately after baseline load and dirty after a parameter edit, even if APVTS state is flushed after replacement.
- A/B compare slots capture and prepare distinct states without mutating live parameters first.
- Full CTest and core suite remain green.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    cmake --build build --config Debug
    ./build/SylenthAIContractTest
    ctest --test-dir build --output-on-failure
    ./build/SylenthAIRender --suite core --output-dir build/reports/core

## Idempotence and Recovery

The helper functions are deterministic. If dirty-state comparison becomes noisy, inspect parameter quantization and registry changes before weakening the comparison. If A/B recall fails after new parameter fields are added, verify `mergeParameterStateWithDefaults()` default-merges the new fields.

## Artifacts and Notes

This slice does not produce persistent runtime artifacts beyond temporary contract-test preset files under the OS temp directory.

## Interfaces and Dependencies

This slice depends on `AudioProcessorValueTreeState`, the parameter registry, preset validation, and existing preset write serialization. It fed the later metadata/safe-save UI, dirty indicators, and A/B compare controls.
