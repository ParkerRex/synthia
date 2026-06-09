#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="build-release-current"
config="RelWithDebInfo"
jobs="$(sysctl -n hw.ncpu 2>/dev/null || printf '4')"
install_local=0
package_bundles=0
skip_check=0

fail() {
  printf 'error: %s\n' "$*" >&2
  exit 1
}

usage() {
  cat <<'USAGE'
usage: scripts/build-release-bundles.sh [options]

Build optimized macOS AU/VST3/Standalone bundles without reusing stale CMake caches.

Options:
  --config <name>       CMake build config: RelWithDebInfo, Release, Debug, MinSizeRel.
                        Default: RelWithDebInfo.
  --build-dir <path>    Build directory relative to repo root or absolute.
                        Default: build-release-current.
  --jobs <n>            Parallel build jobs. Default: host CPU count.
  --install-local       Install AU/VST3 into the current user's plugin folders.
  --package             Zip Standalone, AU, and VST3 bundles into build/release-artifacts/.
  --skip-check          Skip scripts/check-plugin-bundles.sh after build.
  -h, --help            Show this help.

Recommended local host-validation build:
  scripts/build-release-bundles.sh --config RelWithDebInfo --install-local

Recommended release-candidate package build:
  scripts/build-release-bundles.sh --config Release --package
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --config)
      [[ $# -ge 2 ]] || fail "--config needs a value"
      config="$2"
      shift 2
      ;;
    --build-dir)
      [[ $# -ge 2 ]] || fail "--build-dir needs a value"
      build_dir="$2"
      shift 2
      ;;
    --jobs)
      [[ $# -ge 2 ]] || fail "--jobs needs a value"
      jobs="$2"
      shift 2
      ;;
    --install-local)
      install_local=1
      shift
      ;;
    --package)
      package_bundles=1
      shift
      ;;
    --skip-check)
      skip_check=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      fail "unknown option: $1"
      ;;
  esac
done

case "$config" in
  Debug|Release|RelWithDebInfo|MinSizeRel) ;;
  *) fail "unsupported config: $config" ;;
esac

[[ "$jobs" =~ ^[0-9]+$ ]] || fail "--jobs must be a positive integer"
[[ "$jobs" -gt 0 ]] || fail "--jobs must be greater than zero"

case "$build_dir" in
  /*) build_path="$build_dir" ;;
  *) build_path="$root_dir/$build_dir" ;;
esac

cache_file="$build_path/CMakeCache.txt"
if [[ -f "$cache_file" ]]; then
  cached_source="$(sed -n 's/^CMAKE_HOME_DIRECTORY:INTERNAL=//p' "$cache_file" | head -n 1)"
  if [[ -n "$cached_source" && "$cached_source" != "$root_dir" ]]; then
    fail "CMake cache in $build_path belongs to $cached_source; choose a fresh --build-dir"
  fi
fi

printf 'configure: %s (%s)\n' "$build_path" "$config"
cmake -S "$root_dir" -B "$build_path" \
  -DSYNTHIA_ENABLE_TESTS=ON \
  -DCMAKE_BUILD_TYPE="$config" \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"

printf 'build: %s (%s, jobs=%s)\n' "$build_path" "$config" "$jobs"
cmake --build "$build_path" --config "$config" -j "$jobs"

if [[ "$skip_check" != "1" ]]; then
  "$root_dir/scripts/check-plugin-bundles.sh" "$build_path" "$config"
fi

if [[ "$install_local" == "1" ]]; then
  "$root_dir/scripts/install-local-plugins.sh" "$build_path" "$config"
fi

artifact_dir="$build_path/SynthiaPlugin_artefacts/$config"
standalone="$artifact_dir/Standalone/Synthia.app"
au="$artifact_dir/AU/Synthia.component"
vst3="$artifact_dir/VST3/Synthia.vst3"

if [[ "$package_bundles" == "1" ]]; then
  version="$(awk '/^[[:space:]]*VERSION / { print $2; exit }' "$root_dir/CMakeLists.txt")"
  [[ -n "$version" ]] || version="0.0.0"
  stamp="$(date +%Y%m%d-%H%M%S)"
  package_dir="$root_dir/build/release-artifacts/synthia-$version-$config-$stamp"
  mkdir -p "$package_dir"

  [[ -d "$standalone" ]] || fail "Standalone bundle missing: $standalone"
  [[ -d "$au" ]] || fail "AU bundle missing: $au"
  [[ -d "$vst3" ]] || fail "VST3 bundle missing: $vst3"

  ditto -c -k --sequesterRsrc --keepParent "$standalone" "$package_dir/Synthia-Standalone-$version-$config.zip"
  ditto -c -k --sequesterRsrc --keepParent "$au" "$package_dir/Synthia-AU-$version-$config.zip"
  ditto -c -k --sequesterRsrc --keepParent "$vst3" "$package_dir/Synthia-VST3-$version-$config.zip"
  shasum -a 256 "$package_dir"/*.zip > "$package_dir/SHA256SUMS.txt"

  printf 'package_dir: %s\n' "$package_dir"
  printf 'release upload example: gh release create v%s %s/*.zip %s/SHA256SUMS.txt --draft --title "Synthia %s"\n' \
    "$version" "$package_dir" "$package_dir" "$version"
fi

printf 'artifact_dir: %s\n' "$artifact_dir"
printf 'VST3 executable inode: '
stat -f '%i %Sm %m %z %N' "$vst3/Contents/MacOS/Synthia"
