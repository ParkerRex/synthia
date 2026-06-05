#!/usr/bin/env bash
set -euo pipefail

build_dir="${1:-build}"
root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
artifact_dir="$root_dir/$build_dir/SynthPlugin_artefacts"

fail() {
  printf 'error: %s\n' "$*" >&2
  exit 1
}

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
