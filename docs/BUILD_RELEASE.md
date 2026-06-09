# Build and Release Workflow

Use this workflow for Ableton profiling, host validation, and release-candidate bundle packaging.

For the full Ableton CPU profiling loop, use the repo skill at:

```text
.codex/skills/profile-synthia-ableton/SKILL.md
```

That skill documents AbletonMCP setup, the upstream MCP source, bar-65 playback, stale plugin-image checks, and the sample-based optimization loop.

## Build modes

- `Debug`: local debugging only. Do not use this for Ableton CPU numbers.
- `RelWithDebInfo`: optimized build with symbols. Use this for profiling and host validation.
- `Release`: optimized distribution candidate. Use this for packaged binaries after validation.

## Local Ableton profiling build

Build and install optimized AU/VST3 bundles into the current user's plugin folders:

```bash
scripts/build-release-bundles.sh --config RelWithDebInfo --install-local
```

Then restart or rescan Ableton and verify the host mapped the installed VST3 executable inode before trusting CPU data:

```bash
LIVE_PID=$(ps -axo pid=,args= | awk '/Contents\/MacOS\/Live/ && !/awk/ {print $1; exit}')
stat -f '%i %Sm %m %z %N' "$HOME/Library/Audio/Plug-Ins/VST3/Synthia.vst3/Contents/MacOS/Synthia"
lsof -p "$LIVE_PID" 2>/dev/null | rg -i 'Synthia|sylenth' || true
```

A valid Ableton profile must use the target validation set, start at bar 65, and show the same inode in both `stat` and `lsof`.

AbletonMCP setup source:

```text
https://github.com/ahujasid/ableton-mcp
```

Minimum setup:

1. Run the MCP server with `uvx ableton-mcp`.
2. Install `AbletonMCP_Remote_Script/__init__.py` as an Ableton MIDI Remote Script.
3. Restart Ableton.
4. In `Link, Tempo & MIDI`, set `Control Surface` to `AbletonMCP`, `Input` to `None`, and `Output` to `None`.
5. Use the skill's socket/session checks before profiling.

## Release-candidate package build

Build and package standalone, AU, and VST3 zip files:

```bash
scripts/build-release-bundles.sh --config Release --package
```

The script writes zips and `SHA256SUMS.txt` under `build/release-artifacts/`, then prints a draft `gh release create` command.

## Stale cache rule

Do not reuse a build directory whose `CMakeCache.txt` was created from another checkout. The release script fails fast when the cache points at a different source directory; choose a fresh `--build-dir` instead of copying stale artifacts.

## Current release blockers

- Decide tag/version policy beyond `0.1.0`.
- Decide signing and notarization requirements for public binaries.
- Add a CI release job that runs the release script and uploads artifacts to a draft GitHub release.
