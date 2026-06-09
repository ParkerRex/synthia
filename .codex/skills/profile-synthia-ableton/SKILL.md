---
name: profile-synthia-ableton
description: Profile and optimize the Synthia JUCE plugin in Ableton Live using AbletonMCP, current plugin-image verification, bar-65 playback start, CPU sampling, and iterative C++ performance fixes. Use when working in the Synthia repo on Ableton CPU/load profiling, multi-instance supersaw validation, stale VST3/AU cache checks, MCP transport control, or sub-50-percent process CPU optimization without reducing synth capability.
---

# Profile Synthia in Ableton

Use this skill for the Synthia Ableton profiling loop. The goal is real host evidence, not synthetic comfort checks.

## Non-negotiables

- Do not count a CPU profile unless Ableton maps the same Synthia binary inode as the currently installed plugin.
- Do not reduce synth capability to win CPU: no hard voice caps, no stack-count caps, no lower default quality, no weakening presets.
- Start playback at bar 65 for the validation loop. Earlier bars are not representative for this set.
- Verify the playhead after positioning it. The raw `set_current_song_time` response can be stale; trust a follow-up session read, not the setter response.
- Use AbletonMCP for transport when available.
- Preserve the user’s current Ableton set; do not rebuild the test arrangement unless explicitly asked.

## Build and install

From repo root:

```bash
cmake --build build-ableton --config Release --target SynthiaPlugin_AU SynthiaPlugin_VST3
scripts/install-local-plugins.sh build-ableton
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

If the `lsof` inode differs from the installed inode, stop. Ask for Ableton restart/rescan/reload before profiling.

## AbletonMCP requirements

Expected MCP server:

```bash
codex mcp get AbletonMCP
```

Expected Ableton-side state:

- Control Surface: `AbletonMCP`
- Input: `None`
- Output: `None`
- Socket: `localhost:9877` open

Socket check:

```bash
python3 - <<'PY'
import socket
s=socket.socket(); s.settimeout(0.5)
try:
    s.connect(('127.0.0.1', 9877)); print('open')
except Exception as e:
    print(type(e).__name__ + ': ' + str(e))
finally:
    s.close()
PY
```

## Bar-65 playback rule

Before profiling, position Ableton playback at bar 65. In 4/4 this is beat `256.0` because 64 full bars have elapsed before bar 65.

The installed AbletonMCP remote script supports `set_current_song_time` over its raw socket even when Codex only exposes start/stop tools. Use this command before starting playback:

```bash
python3 - <<'PY'
import json
import socket

command = {"type": "set_current_song_time", "params": {"time": 256.0}}
with socket.create_connection(("127.0.0.1", 9877), timeout=2.0) as sock:
    sock.sendall(json.dumps(command).encode("utf-8"))
    print(sock.recv(8192).decode("utf-8"))
PY
```

Then start playback with AbletonMCP. If the raw socket command fails, have the user set the playhead to bar 65 manually before starting playback.

Verify the playhead before starting playback:

```bash
python3 - <<'PY'
import json
import socket

command = {"type": "get_session_info", "params": {}}
with socket.create_connection(("127.0.0.1", 9877), timeout=2.0) as sock:
    sock.sendall(json.dumps(command).encode("utf-8"))
    print(sock.recv(8192).decode("utf-8"))
PY
```

The valid start time is `256.0` beats. If playback has already started, record the verified start time separately; a later `current_song_time` only proves that the transport advanced.

Do not profile from the beginning of the arrangement for this validation set.

## Profile loop

1. Confirm current installed inode equals Ableton mapped inode.
2. Confirm playhead starts at bar 65.
3. Start playback through AbletonMCP.
4. Capture 8-10 seconds of process CPU plus a 2-second `sample`.
5. Stop playback through AbletonMCP.
6. Extract Synthia plugin frames from the full sample file.
7. Optimize only the measured hot path.
8. Rebuild/install, then require Ableton reload before reprofile.

CPU/sample command:

```bash
REPORT_DIR="build/reports/ableton/perf"
mkdir -p "$REPORT_DIR"
STAMP=$(date +%Y%m%d-%H%M%S)
REPORT="$REPORT_DIR/live-bar65-$STAMP.txt"
SAMPLE_COPY="$REPORT_DIR/live-bar65-$STAMP.sample.txt"
LIVE_PID=$(ps -axo pid=,args= | awk '/Contents\/MacOS\/Live/ && !/awk/ {print $1; exit}')
{
  date
  ps -p "$LIVE_PID" -o pid=,pcpu=,pmem=,rss=,args=
  lsof -p "$LIVE_PID" 2>/dev/null | rg -i 'Synthia|sylenth' || true
  stat -f '%i %Sm %m %z %N' "$HOME/Library/Audio/Plug-Ins/VST3/Synthia.vst3/Contents/MacOS/Synthia"
  for i in 1 2 3 4 5 6 7 8; do
    ps -p "$LIVE_PID" -o pid=,pcpu=,rss=,time=,command=
    sleep 1
  done
  sample "$LIVE_PID" 2 2>&1 | sed -n '1,220p'
} | tee "$REPORT"
LATEST_SAMPLE=$(ls -t /tmp/Live_*.sample.txt | head -n 1)
if [ -n "$LATEST_SAMPLE" ]; then
  cp "$LATEST_SAMPLE" "$SAMPLE_COPY"
  printf 'sample_copy=%s\n' "$SAMPLE_COPY" | tee -a "$REPORT"
fi
```

Extract relevant frames:

```bash
SAMPLE_FILE=$(ls -t build/reports/ableton/perf/live-bar65-*.sample.txt /tmp/Live_*.sample.txt 2>/dev/null | head -n 1)
rg -n 'SynthAudioProcessor|SynthEngine::process|VoiceAllocator::renderSample|Voice::renderSample|Voice::renderLayerOscillators|OscillatorStack::renderSample|Filter::process|Filter::processCore|evaluateTransMod|fingerprintCurrentPresetState|comparePresetDirtyState|refreshPresetWorkflow|timerCallback|drawRotarySlider|LcdDisplay::paint|Panel::paint|AudioCalc|com.apple.audio.IOThread.client' "$SAMPLE_FILE" | sed -n '1,320p'
```

## Interpreting evidence

- UI frames such as `drawRotarySlider`, `LcdDisplay::paint`, `Panel::paint`, and preset dirty-state fingerprinting are valid CPU targets when the editor is open.
- AudioCalc frames under `SynthAudioProcessor::processBlock`, `SynthEngine::process`, `VoiceAllocator::renderSample`, `Voice::renderSample`, `OscillatorStack`, and `Filter` are DSP targets.
- For the current multi-instance supersaw validation set, expect DSP to dominate. Prioritize measured AudioCalc hotspots in this order unless the newest sample proves otherwise: `Voice::renderSample`, `Voice::renderLayerOscillators`, `OscillatorStack::renderSample`, `Filter::process` / `Filter::processCore`, then `Voice::evaluateTransMod`.
- Treat stale plugin-image evidence as invalid. Ableton can keep mapping an old VST3/AU binary until the set/plugin is reloaded or Ableton restarts.
- Treat UI optimizations as secondary once `fingerprintCurrentPresetState`, `comparePresetDirtyState`, and paint frames disappear from the sample.
- If CPU improves but stays above target, keep iterating from the newest valid sample.
- Goal completion requires current Ableton set, bar-65 playback, current plugin inode, and sustained process CPU under 50% without quality/capability reductions.

## Optimization boundaries

- Prefer structural DSP savings over feature cuts: active voice iteration, cached per-block parameter preparation, precomputed oscillator/filter state, and fast paths that are mathematically equivalent for the active parameter state.
- Keep validation honest: do not lower default quality, shorten releases, reduce unison, disable effects, narrow presets, or change musical behavior to hit the CPU target.
- Add regression coverage when a DSP optimization changes control flow, interpolation, clamping, denormal handling, or preset/state loading.
