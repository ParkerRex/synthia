#!/usr/bin/env bash
set -euo pipefail

build_dir="${1:-build}"
root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
artifact_root="$root_dir/$build_dir/SynthPlugin_artefacts"

fail() {
  printf 'error: %s\n' "$*" >&2
  exit 1
}

resolve_artifact_dir() {
  local config_dir
  local config_dirs=(Release Debug RelWithDebInfo MinSizeRel "")

  if [[ -n "${2:-}" ]]; then
    config_dirs=("$2" Release Debug RelWithDebInfo MinSizeRel "")
  fi

  for config_dir in "${config_dirs[@]}"; do
    [[ -n "$config_dir" ]] || {
      printf '%s\n' "$1"
      return
    }

    [[ -d "$1/$config_dir" ]] || continue
    [[ -d "$1/$config_dir/Standalone/Synth.app" ]] || continue
    [[ -d "$1/$config_dir/AU/Synth.component" ]] || continue
    [[ -d "$1/$config_dir/VST3/Synth.vst3" ]] || continue

    printf '%s\n' "$1/$config_dir"
    return
  done
}

artifact_dir="$(resolve_artifact_dir "$artifact_root" "${2:-}")"

check_bundle() {
  local label="$1"
  local bundle="$2"
  local executable="$3"

  [[ -d "$bundle" ]] || fail "$label bundle missing: $bundle"
  [[ -f "$executable" ]] || fail "$label executable missing: $executable"

  printf '%s: %s\n' "$label" "$bundle"
  file "$executable"

  if command -v lipo >/dev/null 2>&1; then
    lipo -archs "$executable" || true
  fi

  if command -v codesign >/dev/null 2>&1; then
    codesign --verify --deep --strict "$bundle" >/dev/null 2>&1 \
      && printf '%s codesign: valid\n' "$label" \
      || printf '%s codesign: not valid for distribution\n' "$label"
  fi

  local plist="$bundle/Contents/Info.plist"
  if [[ -f "$plist" ]] && command -v plutil >/dev/null 2>&1; then
    plutil -p "$plist" | grep -E 'CFBundleIdentifier|CFBundleName|CFBundleShortVersionString|CFBundleVersion' || true
  fi

  local preset_dir="$bundle/Contents/Resources/factory"
  [[ -f "$preset_dir/init.json" ]] || fail "$label factory preset missing: $preset_dir/init.json"
  [[ -f "$preset_dir/pluck-core-01.json" ]] || fail "$label factory preset missing: $preset_dir/pluck-core-01.json"
}

check_bundle "Standalone" \
  "$artifact_dir/Standalone/Synth.app" \
  "$artifact_dir/Standalone/Synth.app/Contents/MacOS/Synth"

check_bundle "AU" \
  "$artifact_dir/AU/Synth.component" \
  "$artifact_dir/AU/Synth.component/Contents/MacOS/Synth"

check_bundle "VST3" \
  "$artifact_dir/VST3/Synth.vst3" \
  "$artifact_dir/VST3/Synth.vst3/Contents/MacOS/Synth"

printf 'bundle checks completed for %s\n' "$build_dir"
