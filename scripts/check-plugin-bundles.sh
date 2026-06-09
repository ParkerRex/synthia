#!/usr/bin/env bash
set -euo pipefail

build_dir="${1:-build}"
root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
product_bundle="Synthia"

case "$build_dir" in
  /*) artifact_root="$build_dir/SynthiaPlugin_artefacts" ;;
  *) artifact_root="$root_dir/$build_dir/SynthiaPlugin_artefacts" ;;
esac

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
    [[ -d "$1/$config_dir/Standalone/$product_bundle.app" ]] || continue
    [[ -d "$1/$config_dir/AU/$product_bundle.component" ]] || continue
    [[ -d "$1/$config_dir/VST3/$product_bundle.vst3" ]] || continue

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
  [[ -f "$preset_dir/Init/Init.SynthiaPreset" ]] || fail "$label factory preset missing: $preset_dir/Init/Init.SynthiaPreset"
  [[ -f "$preset_dir/Pluck/PL - Pluck Core 01.SynthiaPreset" ]] || fail "$label factory preset missing: $preset_dir/Pluck/PL - Pluck Core 01.SynthiaPreset"
}

check_bundle "Standalone" \
  "$artifact_dir/Standalone/$product_bundle.app" \
  "$artifact_dir/Standalone/$product_bundle.app/Contents/MacOS/$product_bundle"

check_bundle "AU" \
  "$artifact_dir/AU/$product_bundle.component" \
  "$artifact_dir/AU/$product_bundle.component/Contents/MacOS/$product_bundle"

check_bundle "VST3" \
  "$artifact_dir/VST3/$product_bundle.vst3" \
  "$artifact_dir/VST3/$product_bundle.vst3/Contents/MacOS/$product_bundle"

printf 'bundle checks completed for %s\n' "$build_dir"
