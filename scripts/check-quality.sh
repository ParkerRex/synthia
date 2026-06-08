#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"
RUN_CLANG_TIDY=0
RUN_FORMAT=0
FORMAT_SCOPE="changed"
EXCLUDES=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --with-format)
            RUN_FORMAT=1
            FORMAT_SCOPE="changed"
            shift
            ;;
        --all-format)
            RUN_FORMAT=1
            FORMAT_SCOPE="all"
            shift
            ;;
        --with-tidy)
            RUN_CLANG_TIDY=1
            shift
            ;;
        --exclude)
            EXCLUDES+=("${2#./}")
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        *)
            echo "Unknown argument: $1" >&2
            exit 2
            ;;
    esac
done

cd "${ROOT_DIR}"

git diff --check
git diff --cached --check
scripts/check-cpp-fitness.py

if [[ "${RUN_FORMAT}" -eq 1 ]]; then
    FORMAT_ARGS=("--${FORMAT_SCOPE}")
    set +u
    for exclude in "${EXCLUDES[@]}"; do
        FORMAT_ARGS+=("--exclude" "${exclude}")
    done
    set -u
    scripts/check-cpp-format.sh "${FORMAT_ARGS[@]}"
fi

cmake -S . -B "${BUILD_DIR}" \
    -DSYNTHIA_ENABLE_TESTS=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_BUILD_TYPE=Debug
cmake --build "${BUILD_DIR}" --config Debug
ctest --test-dir "${BUILD_DIR}" -C Debug --output-on-failure

RENDER_EXE="${BUILD_DIR}/SynthiaRender"
if [[ ! -x "${RENDER_EXE}" && -x "${BUILD_DIR}/Debug/SynthiaRender" ]]; then
    RENDER_EXE="${BUILD_DIR}/Debug/SynthiaRender"
fi
"${RENDER_EXE}" --suite core --output-dir "${BUILD_DIR}/reports/core"

if [[ "${RUN_CLANG_TIDY}" -eq 1 ]]; then
    TIDY_ARGS=()
    set +u
    for exclude in "${EXCLUDES[@]}"; do
        TIDY_ARGS+=("--exclude" "${exclude}")
    done
    BUILD_DIR="${BUILD_DIR}" scripts/check-cpp-tidy.sh "${TIDY_ARGS[@]}"
    set -u
fi

echo "Quality gate passed."
