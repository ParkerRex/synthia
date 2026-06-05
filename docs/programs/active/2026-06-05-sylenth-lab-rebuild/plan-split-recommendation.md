# Plan Split Recommendation

## Active Now

1. `2026-06-05-build-sylenth-layer-oscillator-backbone.md`
   - Owner: Codex.
   - Purpose: implement the state/model foundation for A/B layers and oscillator slots.

2. `2026-06-05-handoff-modern-sylenth-ui-shell.md`
   - Owner: Parker handoff to Claude Code.
   - Purpose: UI shell and visual system after state contracts exist.

3. `2026-06-05-handoff-modulation-preset-arp-ui-polish.md`
   - Owner: Parker handoff to Claude Code.
   - Purpose: modulation, browser, arp, and FX UI polish after feature models exist.

## Planned Phase 1 ExecPlans

- `build-sylenth-layer-b-and-four-osc-rendering`
- `build-sylenth-modulation-route-model`
- `build-sylenth-preset-browser-and-bank-workflow`
- `build-sylenth-arp-step-chord-engine`
- `build-sylenth-fx-rack-expansion`
- `build-sylenth-midi-controller-workflow`
- `validate-sylenth-phase-1-ableton`
- `closeout-sylenth-phase-1`

## Planned Phase 2 ExecPlans

- `build-ai-patch-randomization-and-generation`
- `build-ai-arp-and-chord-generation`
- `validate-ai-generation-bounds-and-provenance`

## Planned Phase 3 ExecPlans

- `build-conversational-vst-editing`
- `build-reference-sound-analysis-workflow`
- `validate-conversational-diffs-and-undo`

## Handoff Rule

Any UI-focused ExecPlan should include:

- `handoff_target: Claude Code` in frontmatter.
- Clear dependencies that must be satisfied before Parker hands it off.
- Screenshot/manual references.
- Expected screenshots or manual QA notes.
- Explicit no-go areas, especially realtime audio work and hidden state mutation.

