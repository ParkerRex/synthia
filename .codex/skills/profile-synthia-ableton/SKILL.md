---
name: profile-synthia-ableton
description: Profile and optimize the Synthia JUCE plugin in Ableton Live using AbletonMCP, current plugin-image verification, bar-65 playback start, CPU sampling, and iterative C++ performance fixes. Use when working in the Synthia repo on Ableton CPU/load profiling, multi-instance supersaw validation, stale VST3/AU cache checks, MCP transport control, or sub-50-percent process CPU optimization without reducing synth capability.
---

# Profile Synthia in Ableton

Use this skill for the Synthia Ableton profiling loop. The goal is real host evidence, not synthetic comfort checks.

## How to use this skill

Use this skill when the task is to build Synthia for Ableton, set up AbletonMCP transport control, prove Ableton loaded the current plugin binary, or profile/optimize the bar-65 multi-instance validation set.

Follow this order:

1. Stop any playback left over from a previous profile.
2. Set up or verify AbletonMCP.
3. Build and locally install an optimized Synthia bundle.
4. Restart or rescan Ableton so it maps the new plugin image.
5. Verify the installed VST3 inode equals the inode mapped by Ableton.
6. Position playback at bar 65, beat `256.0`.
7. Start playback, capture CPU/sample evidence, stop playback.
8. Optimize only the measured hot path, then rebuild and repeat.

## What this skill covers

- Build optimized Synthia AU/VST3 bundles for host validation.
- Install the current VST3/AU into the user's macOS plugin folders.
- Set up AbletonMCP so Codex can start/stop playback and inspect the Live set.
- Verify Ableton is mapping the current installed Synthia binary, not a stale cached plugin image.
- Start the validation set at bar 65 and capture process CPU plus native stack samples.
- Use the newest valid sample to guide C++ optimization without reducing synth capability.

## Non-negotiables

- Do not count a CPU profile unless Ableton maps the same Synthia binary inode as the currently installed plugin.
- Do not reduce synth capability to win CPU: no hard voice caps, no stack-count caps, no lower default quality, no weakening presets.
- Do not profile a Debug bundle for CPU decisions. Use `RelWithDebInfo` for profiling and `Release` for distribution candidates.
- Start playback at bar 65 for the validation loop. Earlier bars are not representative for this set.
- Verify the playhead after positioning it. The setter response can be stale; trust a follow-up session read.
- Use AbletonMCP for transport when available.
- Preserve the user's current Ableton set; do not rebuild the test arrangement unless explicitly asked.

## Upstream AbletonMCP source

Primary upstream repo:

```text
https://github.com/ahujasid/ableton-mcp
```

Upstream states the project has two parts:

- `AbletonMCP_Remote_Script`: Ableton MIDI Remote Script that opens the socket server inside Live.
- `MCP_Server`: Python MCP server, usually run with `uvx ableton-mcp`.

Use the upstream repo for the remote script file and use the PyPI/uvx package for the MCP server. Do not vendor the upstream MCP repo into Synthia.

Upstream prerequisites:

- Ableton Live 10 or newer.
- Python 3.8 or newer.
- `uv` package manager.

Install `uv` on macOS when missing:

```bash
brew install uv
```

## Set up AbletonMCP in Codex and Ableton

Install or configure the MCP server so Codex has an `AbletonMCP` server entry whose command is:

```bash
uvx ableton-mcp
```

If editing a Codex MCP config manually, the server definition should be equivalent to this command/args/env shape:

```json
{
  "mcpServers": {
    "AbletonMCP": {
      "command": "uvx",
      "args": ["ableton-mcp"],
      "env": {
        "ABLETON_MCP_DISABLE_TELEMETRY": "true"
      }
    }
  }
}
```

For Codex CLI TOML configs, the same server normally looks like:

```toml
[mcp_servers.AbletonMCP]
command = "uvx"
args = ["ableton-mcp"]

[mcp_servers.AbletonMCP.env]
ABLETON_MCP_DISABLE_TELEMETRY = "true"
```

Restart the Codex session after changing MCP config so the `mcp__AbletonMCP.*` tools are discovered.

Only run one AbletonMCP server instance at a time. Multiple clients or duplicate server processes can fight for the same Ableton socket.

Install the Ableton remote script:

1. Get `AbletonMCP_Remote_Script/__init__.py` from `https://github.com/ahujasid/ableton-mcp`.
2. Create an `AbletonMCP` folder in Ableton's MIDI Remote Scripts directory.
3. Put `__init__.py` inside that folder.
4. Restart Ableton Live.
5. Open `Settings` or `Preferences` -> `Link, Tempo & MIDI`.
6. Set `Control Surface` to `AbletonMCP`.
7. Set `Input` to `None`.
8. Set `Output` to `None`.

Likely macOS remote-script locations:

```text
/Applications/Ableton Live 11 Suite.app/Contents/App-Resources/MIDI Remote Scripts/AbletonMCP/__init__.py
/Users/$USER/Library/Preferences/Ableton/Live 11/User Remote Scripts/AbletonMCP/__init__.py
/Users/$USER/Library/Preferences/Ableton/Live 12/User Remote Scripts/AbletonMCP/__init__.py
```

After setup, discover or expose the tools in Codex if needed:

```text
tool_search query: AbletonMCP start_playback stop_playback get_session_info
```

Expected tools include:

- `mcp__AbletonMCP.get_session_info`
- `mcp__AbletonMCP.start_playback`
- `mcp__AbletonMCP.stop_playback`

Some AbletonMCP installs also expose an arrangement-time setter. If no time setter is exposed, use the raw socket fallback in the bar-65 section. Do not profile from the current playhead just because transport works.

## Verify AbletonMCP connectivity

Socket check:

```bash
python3 - <<'PY'
import socket
s = socket.socket()
s.settimeout(0.5)
try:
    s.connect(("127.0.0.1", 9877))
    print("open")
except Exception as e:
    print(type(e).__name__ + ": " + str(e))
finally:
    s.close()
PY
```

Session check through the MCP tool should report the target validation set shape:

```text
tempo: about 128 BPM
track_count: 9
return_track_count: 2
song_length: 860.0
```

The socket check only proves the Ableton remote script is listening. The MCP tool check proves Codex can talk to the MCP server. Require both before relying on automated transport.

If it reports a different set, open the validation set before profiling:

```text
/Users/parkerrex/Desktop/testing-synth Project/demp.als
```

## Build optimized Synthia for Ableton

Use the repo release script. It avoids stale CMake-cache mistakes and builds universal macOS binaries.

Profiling build and local install:

```bash
scripts/build-release-bundles.sh --config RelWithDebInfo --install-local
```

Distribution-candidate package build:

```bash
scripts/build-release-bundles.sh --config Release --package
```

Do not reuse a build directory if its `CMakeCache.txt` belongs to another checkout. The release script fails fast when it detects that condition.

Reference doc:

```text
docs/BUILD_RELEASE.md
```

If a build directory fails because its `CMakeCache.txt` belongs to another checkout, keep the old directory intact and pass a fresh explicit build directory:

```bash
scripts/build-release-bundles.sh --config RelWithDebInfo --build-dir build-perf-release --install-local
```

Record the installed VST3 inode after every install:

```bash
stat -f '%i %Sm %m %z %N' "$HOME/Library/Audio/Plug-Ins/VST3/Synthia.vst3/Contents/MacOS/Synthia"
shasum -a 256 "$HOME/Library/Audio/Plug-Ins/VST3/Synthia.vst3/Contents/MacOS/Synthia"
dwarfdump --uuid "$HOME/Library/Audio/Plug-Ins/VST3/Synthia.vst3/Contents/MacOS/Synthia"
```

## Prove Ableton loaded the current build

```bash
LIVE_PID=$(ps -axo pid=,args= | awk '/Contents\/MacOS\/Live/ && !/awk/ {print $1; exit}')
ps -p "$LIVE_PID" -o pid=,pcpu=,pmem=,rss=,args=
lsof -p "$LIVE_PID" 2>/dev/null | rg -i 'Synthia|sylenth'
stat -f '%i %Sm %m %z %N' "$HOME/Library/Audio/Plug-Ins/VST3/Synthia.vst3/Contents/MacOS/Synthia"
```

If the `lsof` inode differs from the installed inode, stop. Restart Ableton, rescan plugins, or reload the set before profiling.

A valid profile must record:

```text
installed VST3 inode == Ableton mapped VST3 inode
build config == RelWithDebInfo or Release
target set == 9 tracks, about 128 BPM, song length 860
start beat == 256.0
```

## Bar-65 playback rule

Before profiling, position Ableton playback at bar 65. In 4/4 this is beat `256.0` because 64 full bars have elapsed before bar 65.

Use the exposed MCP tool if the current AbletonMCP server provides it:

```text
mcp__AbletonMCP.set_arrangement_time({"time": 256.0})
mcp__AbletonMCP.get_session_info({})
```

The setter response can be stale. The valid confirmation is `current_song_time: 256.0` from the follow-up session read.

Raw socket fallback when the MCP time setter is missing:

```bash
python3 - <<'PY'
import json
import socket

commands = [
    {"type": "set_current_song_time", "params": {"time": 256.0}},
    {"type": "get_session_info", "params": {}},
]

for command in commands:
    with socket.create_connection(("127.0.0.1", 9877), timeout=2.0) as sock:
        sock.sendall(json.dumps(command).encode("utf-8"))
        print(sock.recv(16384).decode("utf-8"))
PY
```

If the MCP and raw socket both fail, have the user set the playhead to bar 65 manually before starting playback.

Do not profile from the beginning of the arrangement for this validation set.

## Profile loop

1. Confirm current installed inode equals Ableton mapped inode.
2. Confirm target validation set shape.
3. Confirm playhead starts at bar 65, beat `256.0`.
4. Start playback through AbletonMCP.
5. Capture 8-10 seconds of process CPU plus a 2-second `sample`.
6. Stop playback through AbletonMCP.
7. Extract Synthia plugin frames from the full sample file.
8. Optimize only the measured hot path.
9. Rebuild/install, then require Ableton reload before reprofile.

Treat profiles as invalid when any of these are true:

- Ableton maps an older `Synthia.vst3` inode.
- The build was `Debug`.
- Playback did not start at beat `256.0`.
- The set shape does not match the 9-track validation set.
- The optimization changes voice count, stack count, preset quality, or audible capability to win CPU.

CPU/sample command:

```bash
REPORT_DIR="build/reports/ableton/perf"
mkdir -p "$REPORT_DIR"
STAMP=$(date +%Y%m%d-%H%M%S)
REPORT="$REPORT_DIR/live-bar65-$STAMP.txt"
SAMPLE_COPY="$REPORT_DIR/live-bar65-$STAMP.sample.txt"
LIVE_PID=$(ps -axo pid=,args= | awk '/Contents\/MacOS\/Live/ && !/awk/ {print $1; exit}')
{
  printf 'start_verified_beats=256.0\n'
  date
  ps -p "$LIVE_PID" -o pid=,pcpu=,pmem=,rss=,args=
  lsof -p "$LIVE_PID" 2>/dev/null | rg -i 'Synthia|sylenth' || true
  stat -f '%i %Sm %m %z %N' "$HOME/Library/Audio/Plug-Ins/VST3/Synthia.vst3/Contents/MacOS/Synthia"
  for i in 1 2 3 4 5 6 7 8 9 10; do
    ps -p "$LIVE_PID" -o pid=,pcpu=,rss=,time=,command=
    sleep 1
  done
  sample "$LIVE_PID" 2 2>&1 | sed -n '1,260p'
} | tee "$REPORT"
LATEST_SAMPLE=$(ls -t /tmp/Live_*.sample.txt 2>/dev/null | head -n 1)
if [ -n "$LATEST_SAMPLE" ]; then
  cp "$LATEST_SAMPLE" "$SAMPLE_COPY"
  printf 'sample_copy=%s\n' "$SAMPLE_COPY" | tee -a "$REPORT"
fi
printf 'report=%s\n' "$REPORT"
```

Extract relevant frames:

```bash
SAMPLE_FILE=$(ls -t build/reports/ableton/perf/live-bar65-*.sample.txt /tmp/Live_*.sample.txt 2>/dev/null | head -n 1)
rg -n 'SynthAudioProcessor|SynthEngine::process|VoiceAllocator::renderSample|Voice::renderSample|Voice::renderLayerOscillators|OscillatorStack::renderSample|Filter::process|Filter::processCore|evaluateTransMod|fingerprintCurrentPresetState|comparePresetDirtyState|refreshPresetWorkflow|timerCallback|drawRotarySlider|LcdDisplay::paint|Panel::paint|AudioCalc|com.apple.audio.IOThread.client' "$SAMPLE_FILE" | sed -n '1,520p'
```

## Current known baseline

Last known valid optimized host profile:

```text
build config: RelWithDebInfo
validation set: /Users/parkerrex/Desktop/testing-synth Project/demp.als
start: bar 65, beat 256.0
installed and mapped VST3 inode: 605331271
report: build/reports/ableton/perf/live-bar65-20260608-212610.txt
process CPU window: about 66.5% to 75.7%
```

The old Debug/stale-cache profiles around `90-100%` are useful history but not valid optimized baselines.

## Interpreting evidence

- UI frames such as `drawRotarySlider`, `LcdDisplay::paint`, `Panel::paint`, and preset dirty-state fingerprinting are valid CPU targets when the editor is open.
- AudioCalc frames under `SynthAudioProcessor::processBlock`, `SynthEngine::process`, `VoiceAllocator::renderSample`, `Voice::renderSample`, `OscillatorStack`, and `Filter` are DSP targets.
- For the current multi-instance supersaw validation set, expect DSP to dominate.
- Prioritize measured AudioCalc hotspots in this order unless the newest sample proves otherwise: `Voice::renderSample`, `Voice::renderLayerOscillators`, `OscillatorStack::renderSample`, `Filter::process` / `Filter::processCore`, then `Voice::evaluateTransMod`.
- Treat stale plugin-image evidence as invalid. Ableton can keep mapping an old VST3/AU binary until the set/plugin is reloaded or Ableton restarts.
- Treat UI optimizations as secondary once `fingerprintCurrentPresetState`, `comparePresetDirtyState`, and paint frames disappear from the sample.
- If CPU improves but stays above target, keep iterating from the newest valid sample.
- Goal completion requires current Ableton set, bar-65 playback, current plugin inode, and sustained process CPU under 50% without quality/capability reductions.

## Optimization boundaries

- Prefer structural DSP savings over feature cuts: active voice iteration, cached per-block parameter preparation, precomputed oscillator/filter state, and fast paths that are mathematically equivalent for the active parameter state.
- Keep validation honest: do not lower default quality, shorten releases, reduce unison, disable effects, narrow presets, or change musical behavior to hit the CPU target.
- Add regression coverage when a DSP optimization changes control flow, interpolation, clamping, denormal handling, or preset/state loading.
