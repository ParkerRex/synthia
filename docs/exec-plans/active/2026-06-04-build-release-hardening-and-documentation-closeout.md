---
title: Build Release Hardening And Documentation Closeout
status: superseded
created_at: 2026-06-04
completed_at: null
summary: Close the program with performance validation, lab-authored review, final docs, release checklist, and Program retrospective.
post_build_recap: null
read_when:
  - Preparing sylenth-ai for a public or private release.
  - Closing the sylenth-ai build Program.
  - Auditing final validation, docs, or rebuild alignment.
program_id: synth-pluck-core-foundation
planning_brief: docs/programs/completed/2026-06-04-synth-pluck-core-foundation/planning-brief-1.md
---

# Build Release Hardening And Documentation Closeout

This document is a living execution plan for final hardening and Program closeout.

This plan is superseded by the Sylenth Lab Rebuild Program. Reuse release-hardening details after Phase 1 reaches feature and host-validation completeness.

This ExecPlan must be maintained in accordance with `docs/exec-plans/PLANS.md`.

## Purpose / Big Picture

After all build slices land, sylenth-ai still needs a final pass that proves the product is ready to hand off. This slice consolidates performance validation, rebuild alignment review, docs, release checklist, packaging proof, known limitations, and Program retrospective.

## Progress

- [x] 2026-06-04 EDT: Created this Program-linked ExecPlan from `planning-brief-1.md`.
- [ ] Run full validation matrix.
- [ ] Run performance and host checks.
- [ ] Perform rebuild alignment review.
- [ ] Finalize docs and release checklist.
- [ ] Update completed child plans and Program retrospective.

## Surprises & Discoveries

None yet. Record final blockers, deferred features, host limitations, and performance surprises here.

## Decision Log

Decision: Release hardening is a separate slice.
Rationale: Final proof spans docs, legal naming safety, host behavior, packaging, performance, and Program state, so it should not be hidden inside the packaging slice.
Date: 2026-06-04.

## Outcomes & Retrospective

Pending implementation.

## Context and Orientation

This slice depends on all prior child ExecPlans. It should not start until the plugin builds, renders, has UI, validates in Ableton, and has packaging scripts. The Program can move to completed only after this slice records final evidence.

### In Scope

This plan may update `README.md`, `SPEC.md`, `CONTEXT.md`, `docs/*`, release notes, validation reports, host validation docs, packaging docs, source-alignment audit notes, and Program/ExecPlan closeout sections.

### Out Of Scope

This plan does not add major new DSP, UI, or packaging features. It may fix release-blocking bugs discovered during validation, but non-blocking features should become future ExecPlans.

## Plan of Work

Run the full command validation suite, release build, plugin bundle checks, and Ableton smoke checklist. Add performance reports for sample rates, buffer sizes, voice counts, unison counts, oversampling, UI open/closed, and silence tails.

The current available standalone suite is `--suite core`. Add `--suite full` and `scripts/check-source-alignment.sh` during this slice before using them as release gates.

Audit shipped names, preset names, UI text, bundle IDs, and docs against `SPEC.md` and `docs/research/source-map.md`. Remove internal research aliases from release-facing artifacts.

Update docs to reflect actual commands, supported environments, known limitations, install/uninstall flow, preset behavior, validation reports, and future extension backlog.

Close child ExecPlans with retrospectives as they complete. Update `program.md` with final outcomes and move the Program only when no required slice remains.

## Milestones

Milestone 1 runs full validation and performance checks.

Milestone 2 completes lab-authored and release documentation review.

Milestone 3 records known limitations and future follow-up plans.

Milestone 4 completes Program and ExecPlan closeout.

## Concrete Steps

Work from the repo root:

    cd /Users/parkerrex/Developer/sylenth-ai

Run full validation:

    cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSYLENTH_AI_ENABLE_TESTS=ON
    cmake --build build-release --config Release
    ctest --test-dir build-release --output-on-failure
    ./build-release/SylenthAIRender --suite core --output-dir build-release/reports/core
    scripts/check-plugin-bundles.sh build-release

Run the Ableton smoke checklist and update the release docs.

## Validation and Acceptance

Acceptance requires full validation reports, performance report, Ableton smoke proof, source-alignment check passing, install/uninstall docs, known limitations, release checklist, all child plan closeouts, and Program retrospective.

### Test Commands

    cd /Users/parkerrex/Developer/sylenth-ai
    cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSYLENTH_AI_ENABLE_TESTS=ON
    cmake --build build-release --config Release
    ctest --test-dir build-release --output-on-failure
    ./build-release/SylenthAIRender --suite core --output-dir build-release/reports/core
    scripts/check-plugin-bundles.sh build-release

Manual verification: complete the Ableton AU/VST3 smoke checklist and record screenshots or notes in the host validation doc.

## Idempotence and Recovery

Release checks should be rerunnable. Generated reports belong under `build-release/` unless intentionally promoted. If a release blocker appears, fix it in the narrowest affected module and rerun the full suite.

## Artifacts and Notes

Expected proof artifacts:

- full validation report
- performance report
- Ableton smoke notes
- source-alignment audit result
- release checklist
- Program retrospective

## Interfaces and Dependencies

This slice consumes all project interfaces and produces the final release-readiness truth. It is the gate before moving the Program to `docs/programs/completed/`.
