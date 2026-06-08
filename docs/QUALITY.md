# Quality Gates

This repo uses local fitness functions to keep the C++ codebase strict where mistakes are expensive: realtime audio, ownership, type boundaries, formatting, and render validation.

## Default Build Loop

Run the default quality gate before committing C++ or validation changes:

```bash
scripts/check-quality.sh
```

The default gate runs:

- `git diff --check`
- `scripts/check-cpp-fitness.py`
- CMake configure with tests and `compile_commands.json`
- Debug build
- CTest
- `SynthiaRender --suite core`

Build artifacts and reports stay under `build/`.

## Sanitizer Builds

Use sanitizer builds for focused memory/undefined-behavior sweeps:

```bash
cmake -S . -B build-asan -DSYNTHIA_ENABLE_TESTS=ON -DSYNTHIA_ENABLE_ASAN=ON
cmake --build build-asan --config Debug
ctest --test-dir build-asan --output-on-failure
```

```bash
cmake -S . -B build-ubsan -DSYNTHIA_ENABLE_TESTS=ON -DSYNTHIA_ENABLE_UBSAN=ON
cmake --build build-ubsan --config Debug
ctest --test-dir build-ubsan --output-on-failure
```

## C++ Fitness Rules

`scripts/check-cpp-fitness.py` is intentionally stricter than normal linting at the audio boundary.

Production code under `src/` rejects:

- `std::any`
- `dynamic_cast`
- unreviewed `void*`
- unreviewed `reinterpret_cast`
- unreviewed raw `new` / `delete`
- production exceptions and catch-all handlers

Realtime paths reject:

- filesystem and JUCE file/string work
- heap allocation calls
- container growth/mutation calls unless explicitly prepared and allowlisted
- locks
- synchronous logging
- thread creation or async work
- exceptions

Hot audio render/process functions additionally reject common CPU spikes:

- per-sample `std::pow`, `std::exp`, `std::log`, and `std::log10`
- per-sample `std::tanh`
- per-sample `std::sqrt` normalization that should be cached from voice/slot counts
- rounding/wrapping math that is not an explicitly allowlisted phase accumulator
- resource prepare/reset calls
- APVTS or `ValueTree` reads
- `std::function` / `std::bind` type-erased callbacks

These `audio-perf/*` checks are intentionally synth-specific. They scan named hot functions such as oscillator rendering, voice rendering, filter processing, FX processing, and the engine render loop rather than every utility function in `src/`.

Allowed boundary cases are narrow and documented in the script: JUCE state chunk APIs, JUCE plugin/editor factory returns, and one bundle-resource address lookup.

## Formatting

Check formatting:

```bash
scripts/check-cpp-format.sh
```

Check all tracked C++ formatting:

```bash
scripts/check-cpp-format.sh --all
```

Apply formatting:

```bash
scripts/check-cpp-format.sh --fix
```

The repo prefers Allman braces, 4-space indents, stable include order, and low-churn wrapping. The default format command checks changed C++ files only. Use `--all` for the dedicated historical formatting sweep.

## clang-tidy Sweep

The slower clang-tidy pass is available when doing a quality sweep:

```bash
scripts/check-quality.sh --with-tidy
```

or:

```bash
cmake -S . -B build -DSYNTHIA_ENABLE_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
scripts/check-cpp-tidy.sh
```

clang-tidy treats `bugprone-*` and `performance-*` diagnostics as errors. Readability checks are enabled to surface cleanup candidates, but the first strict sweep should fix or intentionally suppress the old-code backlog before making tidy mandatory in every fast edit loop.

To include changed-file formatting in the build loop:

```bash
scripts/check-quality.sh --with-format
```

To include the full formatting sweep:

```bash
scripts/check-quality.sh --all-format
```

When another agent owns a UI file, exclude it from sweep commands:

```bash
scripts/check-quality.sh --all-format --with-tidy \
  --exclude src/plugin/PluginEditor.cpp \
  --exclude src/plugin/PluginEditor.h
```
