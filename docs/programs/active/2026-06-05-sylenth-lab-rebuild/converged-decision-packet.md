# Converged Decision Packet

Accepted direction:

- Build toward a real Sylenth-level Phase 1 architecture, not a UI-only resemblance.
- Use the current pluck core as scaffold until layer and oscillator-slot contracts replace or absorb it.
- Create engine/state ExecPlans before UI implementation handoff.
- Denote UI-focused ExecPlans for Claude Code and include dependencies, source references, expected deliverables, and QA artifacts.
- Keep generated AI output normal and editable: presets, parameter values, modulation routes, arp/chord lanes, and FX settings.
- Keep all AI and reference-sound workflows outside the realtime audio thread.

Rejected directions:

- Do not start with UI skinning before A/B layers and oscillator slots are modeled.
- Do not treat Phase 2 AI as audio-only sample generation.
- Do not let Phase 3 conversation mutate hidden state that users cannot inspect or undo.
- Do not move into broad wavetable/editor scope before Phase 1 Sylenth rebuild acceptance.

