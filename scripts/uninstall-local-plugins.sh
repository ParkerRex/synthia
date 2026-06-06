#!/usr/bin/env bash
set -euo pipefail

dry_run=0
if [[ "${1:-}" == "--dry-run" ]]; then
  dry_run=1
fi

remove_bundle() {
  local label="$1"
  local path="$2"

  if [[ ! -e "$path" ]]; then
    printf '%s not installed: %s\n' "$label" "$path"
    return
  fi

  if [[ "$dry_run" == "1" ]]; then
    printf 'would remove %s: %s\n' "$label" "$path"
    return
  fi

  rm -rf "$path"
  printf 'removed %s: %s\n' "$label" "$path"
}

remove_bundle "AU" "$HOME/Library/Audio/Plug-Ins/Components/sylenth-ai.component"
remove_bundle "VST3" "$HOME/Library/Audio/Plug-Ins/VST3/sylenth-ai.vst3"
remove_bundle "legacy AU" "$HOME/Library/Audio/Plug-Ins/Components/Synth.component"
remove_bundle "legacy VST3" "$HOME/Library/Audio/Plug-Ins/VST3/Synth.vst3"

if [[ "$dry_run" == "1" ]]; then
  printf 'dry run only; no files removed.\n'
else
  printf 'rescan plug-ins in Ableton after uninstalling.\n'
fi
