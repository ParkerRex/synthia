---
title: Build Preset Browser Invalid Errors
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Surface invalid preset JSON files as visible non-loadable browser rows with validation messages.
post_build_recap: Added invalid preset rows to the processor preset list, rendered warning rows in the Sound-page browser, blocked invalid load/favorite/step actions, and updated roadmap docs.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
read_when:
  - Reviewing preset browser error handling.
  - Checking invalid preset UX.
  - Planning richer bank management or scanned-preset detail.
---

# Build Preset Browser Invalid Errors

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The preset scanner already validated JSON files, but invalid presets were silently skipped. This slice makes bad preset files visible in the browser so users can see which file is broken and why, without making invalid files loadable or favoritable.

## Progress

- [x] 2026-06-06 EDT: Added `valid` and `validationMessage` fields to `SynthAudioProcessor::PresetListItem`.
- [x] 2026-06-06 EDT: Added invalid `.json` rows for factory, user, and legacy-user preset directories using `validatePresetDirectory()`.
- [x] 2026-06-06 EDT: Rendered invalid browser rows with a warning marker, `INVALID` source label, and the first validation error.
- [x] 2026-06-06 EDT: Blocked invalid row load, favorite, double-click, and prev/next stepping behavior with status-line feedback.
- [x] 2026-06-06 EDT: Updated README, validation docs, baseline, Program, and ExecPlan indexes.
- [x] 2026-06-06 EDT: Attempted standalone screenshot proof with a temporary invalid user preset; JUCE viewport scrolling did not move reliably under automation, so full browser-bottom visual QA remains manual.

## Surprises & Discoveries

- `PresetSummary` should remain a valid-preset shape. Invalid rows belong in the processor/editor-facing `PresetListItem`, where UI status and load/favorite guards can enforce behavior.
- Combo and browser share the same `presetItems` vector, so invalid rows must preserve index mapping but be guarded at every command path.

## Decision Log

Decision: Keep invalid preset rows visible but non-actionable.
Rationale: The user needs error visibility, but invalid JSON cannot safely enter APVTS state or sidecar favorite state.
Date: 2026-06-06.

Decision: Show the first validator error inline.
Rationale: One concise row message is enough for triage; full validation reporting remains in CLI/report tooling.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. Invalid `.json` files now appear in the Sound-page preset browser with warning styling and status-line feedback. They cannot be loaded, favorited, or reached by prev/next stepping. Valid preset scanning, loading, favorites, and metadata hydration remain unchanged.

## In Scope

- Invalid preset rows for factory/user/legacy preset directories.
- Validation message surfacing in the browser row and status line.
- Load/favorite/step guards for invalid rows.
- Standalone manual-control smoke.
- Docs and ExecPlan updates.

## Out Of Scope

- Rich full-error detail panel.
- Preset repair tooling.
- Bank import/export.
- Delete, insert, copy/paste.
- Claude visual remapping against screenshot references.

## Validation and Acceptance

Acceptance requires:

- Invalid `.json` preset files are visible in the browser.
- Invalid rows show a validation message.
- Invalid rows cannot load, favorite, or become prev/next targets.
- Valid preset browser behavior is unchanged.
- Existing build, CTest, and core render suite continue to pass.

Validation run:

```bash
git diff --check
cmake --build build --config Debug
ctest --test-dir build --output-on-failure
./build/SylenthAIRender --suite core --output-dir build/reports/core
```

Manual UI note: create a temporary malformed `.json` preset in the user preset directory, open the standalone, scroll to the Sound-page preset browser, and confirm the invalid row shows the first validation error and cannot be loaded or favorited. Automation could not reliably scroll the JUCE viewport to this panel during this slice.

## Follow-Up

- Add richer scanned-preset detail when the browser detail surface is designed.
- Leave screenshot-driven visual hierarchy polish to the Claude UI handoff.
