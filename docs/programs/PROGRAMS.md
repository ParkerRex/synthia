# Program Contract

This contract is adapted from the Worker Program lane for this sylenth-ai project.

## Required Program Folder

Every active Program must be one folder:

- `docs/programs/active/<YYYY-MM-DD-program-id>/`

The dated folder suffix after `YYYY-MM-DD-` must match the `program_id` in `program.md`.

## Required Files

Each Program packet must contain:

- `program.md`
- at least one `research-pass-*.md`
- at least one `normalized-pass-*.md`
- `converged-decision-packet.md`
- `dependency-graph.md`
- `plan-split-recommendation.md`
- `cross-repo-review.md`
- at least one `planning-brief-<N>.md`
- `current-planning-brief.txt`

## Program Frontmatter

`program.md` must include:

- `program_id`
- `title`
- `status`
- `created_at`
- `completed_at`
- `summary`
- `post_build_recap`
- `read_when`

Use `status: active`, `completed_at: null`, and `post_build_recap: null` until the initiative is complete.

## Required Program Sections

Use these sections in order:

1. `Purpose / Big Picture`
2. `Program Inputs`
3. `Current State`
4. `Progress`
5. `Decision Log`
6. `Slice Ledger`
7. `Next Slice`
8. `Risks and Watchpoints`
9. `Outcomes & Retrospective`
10. `Validation and Acceptance`
11. `Artifacts and Notes`
12. `Interfaces and Dependencies`

## Completion Rule

Do not move a Program to `completed/` until all required child slices have landed, validation evidence exists, and `Outcomes & Retrospective` records what actually happened.
