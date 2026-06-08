# Cross-Repo Review

Current repo constraints:

- Use existing JUCE/CMake/APVTS patterns.
- Keep parameter IDs stable once exposed to host automation.
- Keep external I/O, AI orchestration, browsing, scanning, and reference analysis off the realtime audio thread.
- Generated reports, WAVs, bundles, and build artifacts remain under `build/` or `build-release/` unless intentionally promoted.
- UI work should be planned for Claude Code handoff but must still respect the engine/state contracts in this repo.

Files most likely to change in Phase 1:

- `src/plugin/ParameterRegistry.*`
- `src/dsp/SynthParameters.h`
- `src/dsp/SynthEngine.*`
- `src/voice/Voice.*`
- `src/voice/VoiceAllocator.*`
- `src/presets/PresetManager.*`
- `src/presets/PresetValidator.*`
- `src/validation/SynthRender.cpp`
- `tests/smoke/*`
- `docs/modern-sylenth-baseline.md`
- `docs/ARCHITECTURE.md`
- `docs/VALIDATION.md`
- `docs/PRESET_SCHEMA.md`

UI handoff files likely to change:

- `src/plugin/PluginEditor.*`
- new UI model/components under `src/plugin/` if introduced by the handoff.
- docs and QA screenshots/manual notes attached to the handoff ExecPlan.

