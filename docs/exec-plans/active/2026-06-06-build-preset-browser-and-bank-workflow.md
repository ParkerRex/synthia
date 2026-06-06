---
title: Build Preset Browser And Bank Workflow
status: active
created_at: 2026-06-06
completed_at: null
summary: Add the Phase 1 preset browser, bank/category metadata, favorites, search/filter state, and validation hooks needed before deeper Claude Code preset UI polish.
post_build_recap: null
read_when:
  - Implementing or reviewing preset browser, bank, category, favorite, author, tag, search, or preset metadata behavior.
  - Preparing preset browser UI polish for Claude Code.
  - Designing Phase 2 AI preset generation provenance and browser placement.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
handoff_target: Codex
---

# Build Preset Browser And Bank Workflow

This ExecPlan is the next engine/state slice after layer rendering and arp/step/chord closeout.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

Phase 1 needs a real preset workflow before the UI can be polished. The current preset list can load/save/duplicate JSON files, but the Sylenth-level workflow needs bank/category/tag metadata, factory/user grouping, favorites, search/filter state, and browser validation that future AI-generated presets can also use.

## Progress

- [x] 2026-06-06 EDT: Created this Program-linked ExecPlan after closing the arp/step/chord workflow.
- [ ] Inventory current `PresetManager`, preset JSON, editor combo, and validation behavior.
- [ ] Add bank/category/favorite/search metadata state without breaking existing factory presets.
- [ ] Update preset scan summaries and browser-facing models.
- [ ] Add validation coverage for metadata, legacy defaults, and saved user presets.
- [ ] Update docs and Claude Code preset-browser handoff gates.

## Decision Log

Decision: Keep browser metadata separate from realtime audio state.
Rationale: Bank/category/search/favorite behavior must not touch the audio thread and must remain useful for factory, user, and future AI-generated presets.
Date: 2026-06-06.

## Context and Orientation

Read first:

- `docs/modern-sylenth-baseline.md`
- `docs/PRESET_SCHEMA.md`
- `docs/ARCHITECTURE.md`
- `src/presets/PresetManager.*`
- `src/presets/PresetValidator.*`
- `src/plugin/PluginEditor.*`
- `tests/smoke/SynthContractTest.cpp`

## In Scope

- Preset bank/category/tag metadata.
- Favorite state or a durable local favorite marker.
- Search/filter state for browser-facing summaries.
- Factory/user/legacy grouping.
- Saved user preset metadata.
- Contract tests and docs.

## Out Of Scope

- Claude Code visual browser polish.
- AI generation implementation.
- Cloud sync or account-backed libraries.
- Audio-thread preset scanning or filesystem work.

## Validation and Acceptance

Acceptance requires:

- Existing factory presets remain valid.
- Saved user presets include browser metadata.
- Preset scanning returns enough structured metadata for a real browser UI.
- Favorites/search/filter state does not enter the realtime audio path.
- Docs describe the metadata contract and remaining Claude UI handoff gates.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    git diff --check
    cmake --build build --target SylenthAIContractTest
    ./build/SylenthAIContractTest

If the full JUCE build remains slow locally, use focused source-level or parser probes during iteration and record the exact limitation at closeout.
