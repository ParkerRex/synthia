# Programs

Programs coordinate work that spans several ExecPlans. A Program is not an implementation plan for one slice; it is the durable initiative memory above the slices.

Each active Program lives under:

- `docs/programs/active/<YYYY-MM-DD-program-id>/`

Each completed Program moves as a whole to:

- `docs/programs/completed/<YYYY-MM-DD-program-id>/`

Inside each Program folder:

- `program.md` is the canonical Program document.
- `research-pass-*.md` records stable research inputs.
- `normalized-pass-*.md` turns research into decision-ready tracks.
- `converged-decision-packet.md` records the accepted direction.
- `dependency-graph.md` records ordering and blocking dependencies.
- `plan-split-recommendation.md` defines the child ExecPlan split.
- `cross-repo-review.md` records repo/workspace constraints.
- `planning-brief-<N>.md` is the immutable execution brief for child plans.
- `current-planning-brief.txt` points at the current immutable brief.

Child ExecPlans must record both `program_id` and the exact `planning_brief` path they were written from.

