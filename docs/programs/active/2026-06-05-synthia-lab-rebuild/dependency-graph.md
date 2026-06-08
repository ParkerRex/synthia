# Dependency Graph

1. Phase 1 backbone
   - Inputs: current parameter registry, preset schema, engine scaffold.
   - Outputs: `layer.*`, `layer.*.osc.*`, mixer/master, migration, render-preserving Layer A.

2. Phase 1 rendering expansion
   - Depends on: Phase 1 backbone.
   - Outputs: Layer B rendering, four oscillator slots, per-part filters/envelopes, voice math.

3. Phase 1 modulation route model
   - Depends on: backbone and destination catalog.
   - Outputs: route schema, source/destination catalog, matrix sync data.

4. Phase 1 preset browser and bank workflow
   - Depends on: preset metadata and migration updates.
   - Outputs: browser model, favorites/metadata, init/randomize/reset, safe writes.

5. Phase 1 arp/step/chord engine
   - Depends on: voice allocator and host tempo transport state.
   - Outputs: deterministic note scheduling, step lanes, velocity/hold/tie sources.

6. Phase 1 FX rack
   - Depends on: mixer/master and tail reporting.
   - Outputs: expanded rack modules, bypass/mix/order rules, wet/dry validation.

7. Claude Code UI handoff: shell/layout
   - Depends on: enough backbone state to avoid fake A/B UI.
   - Outputs: Sylenth-faithful shell, screenshots, manual QA.

8. Claude Code UI handoff: modulation/preset/arp polish
   - Depends on: route model, browser model, arp model.
   - Outputs: source tiles, halos, browser drawer, arp/step UI, FX rack polish.

9. Phase 1 Ableton validation
   - Depends on: engine, state, UI, presets, arp, FX.
   - Outputs: AU/VST3 automation, bounce, restore, sample-rate/buffer proof.

10. Phase 2 AI generation
    - Depends on: stable preset/parameter/modulation/arp schemas.
    - Outputs: bounded generation, provenance, validation.

11. Phase 3 conversational/reference control
    - Depends on: Phase 2 generation, reversible edit reports, stable UI affordances.
    - Outputs: prompt/reference workflows, diffs, confirmation, validation.
