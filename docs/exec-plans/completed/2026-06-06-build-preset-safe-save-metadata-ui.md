---
title: Build Preset Safe Save Metadata UI
status: completed
created_at: 2026-06-06
completed_at: 2026-06-06
summary: Add visible preset metadata fields plus explicit no-clobber Save New and Overwrite controls.
post_build_recap: Added a processor save overload for `PresetWriteOptions`, a Sound-page Preset Save panel with name/author/bank/category/tags/notes fields, explicit Save New and Overwrite actions, roadmap updates, and normal/compact standalone screenshot proof.
program_id: sylenth-lab-rebuild
planning_brief: docs/programs/active/2026-06-05-sylenth-lab-rebuild/planning-brief-1.md
read_when:
  - Reviewing preset metadata UI.
  - Checking safe-save behavior.
  - Planning invalid-preset error or bank-management UI.
---

# Build Preset Safe Save Metadata UI

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

The backend already had metadata-aware writes and create-new versus overwrite-existing modes. This slice exposes that model in the editor so users can set basic preset metadata and choose an explicit no-clobber Save New path or an explicit Overwrite path.

## Progress

- [x] 2026-06-06 EDT: Added a `SynthAudioProcessor::savePresetFile()` overload that accepts `synth::PresetWriteOptions`.
- [x] 2026-06-06 EDT: Preserved the legacy display-name save API by routing it through the new overload with overwrite-compatible defaults.
- [x] 2026-06-06 EDT: Added a `PRESET SAVE` panel to the Sound page with Preset name, Author, Bank, Category, Tags, Notes, Save New, and Overwrite controls.
- [x] 2026-06-06 EDT: Wired Save New to `PresetWriteMode::CreateNew` and Overwrite to `PresetWriteMode::OverwriteExisting`.
- [x] 2026-06-06 EDT: Synced the panel's preset-name field after load, init, reset, randomize, duplicate, compare recall, and successful saves.
- [x] 2026-06-06 EDT: Captured normal and compact standalone screenshot proof under `build/reports/ui/`.
- [x] 2026-06-06 EDT: Updated README, architecture, validation, baseline, Program, and ExecPlan indexes.
- [x] 2026-06-06 EDT: Patched autoreview finding: the metadata panel now hydrates author, description, bank, category, and tags from the active preset item before overwrite paths can serialize them.
- [x] 2026-06-06 EDT: Patched autoreview findings: metadata hydration is now file-path based only, and Overwrite targets the loaded preset file while rejecting factory/no-target overwrites.

## Surprises & Discoveries

- Keeping safe-save as two explicit buttons is simpler and safer than relying on a native file chooser overwrite warning. The create-new path uses the backend no-clobber write boundary; the overwrite path is intentionally separate and visibly destructive.
- The metadata panel sits below the preset workflow row, so compact/default-height users still reach it by scrolling.
- Preserving metadata on overwrite requires the editor-facing preset list to carry author and description, not just browser grouping fields.
- Display-name matching is unsafe for metadata hydration: generated no-path states such as Init can share names with scanned factory presets.

## Decision Log

Decision: Use a derived user-preset file path from the display name for metadata-panel saves.
Rationale: The panel is a fast preset workflow, not a full file browser. It writes to the canonical user preset directory and leaves arbitrary file-path save behavior to the existing Save As button.
Date: 2026-06-06.

Decision: Keep Save New and Overwrite as separate controls.
Rationale: Separate controls make no-clobber versus destructive replacement explicit and map directly to `PresetWriteMode`.
Date: 2026-06-06.

## Outcomes & Retrospective

Completed. The Sound page now exposes metadata fields and safe-save controls over the real preset writer. Save New refuses existing destinations through the create-new backend mode; Overwrite uses the explicit overwrite mode. No DSP, preset schema, or parameter IDs changed.

Autoreview caught correctness issues: name-only panel sync could make Overwrite serialize blank or stale non-name metadata, display-name fallback could hydrate generated Init from the factory Init preset, and Overwrite could target a display-name-derived file instead of the active preset file. The fix extends `PresetListItem` with author/description, hydrates metadata only from the current preset file path, and makes Overwrite target the loaded preset file while rejecting factory/no-target overwrites.

## In Scope

- Processor save overload for metadata-aware write options.
- Preset metadata fields for display name, author, bank, category, tags, and notes.
- Explicit Save New and Overwrite buttons.
- Standalone normal/compact screenshot proof.
- Docs and ExecPlan updates.

## Out Of Scope

- Invalid-preset visible error rows.
- Bank import/export.
- Delete, insert, copy/paste.
- Full metadata detail view for scanned factory presets.
- Arbitrary file path selection beyond the existing Save As button.
- DSP, parameter registry, or preset schema changes.

## Validation and Acceptance

Acceptance requires:

- Save New routes through `PresetWriteMode::CreateNew`.
- Overwrite routes through `PresetWriteMode::OverwriteExisting` against the loaded non-factory preset file.
- Metadata fields serialize through `PresetWriteOptions`.
- Overwrite preserves existing metadata fields unless the user edits them in the panel.
- Existing Save As and Duplicate behavior remain compatible.
- Normal and compact standalone screenshots show the panel without clipping.
- Existing build, tests, and core render suite continue to pass.

Validation run:

```bash
git diff --check
cmake --build build --config Debug
ctest --test-dir build --output-on-failure
./build/SylenthAIRender --suite core --output-dir build/reports/core
```

Manual UI proof:

- `build/reports/ui/preset-safe-save-metadata-ui-workflow.png`
- `build/reports/ui/preset-safe-save-metadata-ui-compact.png`

## Follow-Up

- Invalid-preset visible error rows later landed in `2026-06-06-build-preset-browser-invalid-errors.md`.
- Add richer bank-management workflows only when product scope requires them.
- Consider moving preset workflow/save controls closer to the header in a later visual layout pass.
