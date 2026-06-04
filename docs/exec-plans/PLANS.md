# ExecPlan Contract

This contract is adapted from the Worker ExecPlan lane for this Synth project.

An ExecPlan is a self-contained Markdown plan for one bounded implementation slice. Treat the reader as a capable engineer with no memory of prior chats.

## Storage

Active plans live under:

- `docs/exec-plans/active/`

Completed plans move to:

- `docs/exec-plans/completed/`

New plans must use dated filenames:

- `YYYY-MM-DD-<slug>.md`

## Frontmatter

Every ExecPlan must start with YAML frontmatter:

- `title`
- `status`
- `created_at`
- `completed_at`
- `summary`
- `post_build_recap`
- `read_when`

Program-linked ExecPlans must also include both:

- `program_id`
- `planning_brief`

`planning_brief` must point to an immutable `planning-brief-<N>.md`, not `current-planning-brief.txt`.

## Required Sections

Use these sections in order:

1. `Purpose / Big Picture`
2. `Progress`
3. `Surprises & Discoveries`
4. `Decision Log`
5. `Outcomes & Retrospective`
6. `Context and Orientation`
7. `Plan of Work`
8. `Milestones`
9. `Concrete Steps`
10. `Validation and Acceptance`
11. `Idempotence and Recovery`
12. `Artifacts and Notes`
13. `Interfaces and Dependencies`

`Context and Orientation` must include:

- `In Scope`
- `Out Of Scope`

`Validation and Acceptance` must include:

- `Test Commands`

## Living Plan Rule

Keep `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` current while implementing. If reality changes, update the plan before continuing.

