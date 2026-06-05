#!/usr/bin/env bash
set -euo pipefail

build_dir="${1:-build}"
root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
artifact_dir="$root_dir/$build_dir/SynthPlugin_artefacts"

au_src="$artifact_dir/AU/Synth.component"
vst3_src="$artifact_dir/VST3/Synth.vst3"
au_dest="$HOME/Library/Audio/Plug-Ins/Components"
vst3_dest="$HOME/Library/Audio/Plug-Ins/VST3"

[[ -d "$au_src" ]] || { printf 'error: AU bundle missing: %s\n' "$au_src" >&2; exit 1; }
[[ -d "$vst3_src" ]] || { printf 'error: VST3 bundle missing: %s\n' "$vst3_src" >&2; exit 1; }

mkdir -p "$au_dest" "$vst3_dest"

rsync -a --delete "$au_src" "$au_dest/"
rsync -a --delete "$vst3_src" "$vst3_dest/"

printf 'installed AU: %s/Synth.component\n' "$au_dest"
printf 'installed VST3: %s/Synth.vst3\n' "$vst3_dest"
printf 'rescan plug-ins in Ableton before validation.\n'
