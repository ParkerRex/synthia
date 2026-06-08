#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"
EXCLUDES=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --exclude)
            EXCLUDES+=("${2#./}")
            shift 2
            ;;
        *)
            echo "Unknown argument: $1" >&2
            exit 2
            ;;
    esac
done

find_clang_tidy() {
    if [[ -n "${CLANG_TIDY:-}" && -x "${CLANG_TIDY}" ]]; then
        printf '%s\n' "${CLANG_TIDY}"
        return 0
    fi

    if command -v clang-tidy >/dev/null 2>&1; then
        command -v clang-tidy
        return 0
    fi

    if [[ -x /opt/homebrew/opt/llvm/bin/clang-tidy ]]; then
        printf '%s\n' /opt/homebrew/opt/llvm/bin/clang-tidy
        return 0
    fi

    return 1
}

CLANG_TIDY_BIN="$(find_clang_tidy)" || {
    echo "clang-tidy not found. Set CLANG_TIDY=/path/to/clang-tidy or install Homebrew llvm." >&2
    exit 1
}

if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
    echo "Missing ${BUILD_DIR}/compile_commands.json." >&2
    echo "Run: cmake -S . -B ${BUILD_DIR#${ROOT_DIR}/} -DSYNTHIA_ENABLE_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON" >&2
    exit 1
fi

TIDY_COMPILE_DB_DIR="${BUILD_DIR}/clang-tidy-compile-db"
TIDY_ARCH="${SYNTHIA_TIDY_ARCH:-$(uname -m)}"
TIDY_SDK_ROOT="${SDKROOT:-$(xcrun --show-sdk-path 2>/dev/null || true)}"
TIDY_LIBCXX_INCLUDE=""
if [[ -n "${TIDY_SDK_ROOT}" && -d "${TIDY_SDK_ROOT}/usr/include/c++/v1" ]]; then
    TIDY_LIBCXX_INCLUDE="${TIDY_SDK_ROOT}/usr/include/c++/v1"
fi
mkdir -p "${TIDY_COMPILE_DB_DIR}"
python3 - \
    "${BUILD_DIR}/compile_commands.json" \
    "${TIDY_COMPILE_DB_DIR}/compile_commands.json" \
    "${TIDY_ARCH}" \
    "${TIDY_SDK_ROOT}" \
    "${TIDY_LIBCXX_INCLUDE}" <<'PY'
import json
import shlex
import sys

input_path, output_path, tidy_arch, sdk_root, libcxx_include = sys.argv[1:6]

with open(input_path, "r", encoding="utf-8") as input_file:
    commands = json.load(input_file)


def sanitize_arguments(arguments):
    sanitized = []
    saw_arch = False
    kept_arch = False
    index = 0

    while index < len(arguments):
        argument = arguments[index]
        if argument == "-arch" and index + 1 < len(arguments):
            saw_arch = True
            if arguments[index + 1] == tidy_arch and not kept_arch:
                sanitized.extend(["-arch", tidy_arch])
                kept_arch = True
            index += 2
            continue

        sanitized.append(argument)
        index += 1

    if saw_arch and not kept_arch:
        sanitized[1:1] = ["-arch", tidy_arch]

    inserted_arguments = []
    if sdk_root and "-isysroot" not in sanitized:
        inserted_arguments.extend(["-isysroot", sdk_root])
    if libcxx_include and libcxx_include not in sanitized:
        inserted_arguments.extend(["-isystem", libcxx_include])
    if inserted_arguments:
        sanitized[1:1] = inserted_arguments

    return sanitized


deduped_commands = []
seen_files = set()

for command in commands:
    source_file = command.get("file")
    if source_file in seen_files:
        continue
    seen_files.add(source_file)

    command = dict(command)
    if "arguments" in command:
        command["arguments"] = sanitize_arguments(command["arguments"])
    elif "command" in command:
        command["command"] = shlex.join(sanitize_arguments(shlex.split(command["command"])))

    deduped_commands.append(command)

with open(output_path, "w", encoding="utf-8") as output_file:
    json.dump(deduped_commands, output_file, indent=2)
    output_file.write("\n")
PY

CPP_FILES=()
while IFS= read -r file; do
    CPP_FILES+=("${file}")
done < <(git -C "${ROOT_DIR}" ls-files 'src/**/*.cpp' 'tests/**/*.cpp')

if [[ ${#EXCLUDES[@]} -gt 0 && ${#CPP_FILES[@]} -gt 0 ]]; then
    ORIGINAL_CPP_FILES=("${CPP_FILES[@]}")
    CPP_FILES=()
    for file in "${ORIGINAL_CPP_FILES[@]}"; do
        excluded=0
        for exclude in "${EXCLUDES[@]}"; do
            if [[ "${file}" == "${exclude}" ]]; then
                excluded=1
                break
            fi
        done
        if [[ "${excluded}" -eq 0 ]]; then
            CPP_FILES+=("${file}")
        fi
    done
fi

if [[ ${#CPP_FILES[@]} -eq 0 ]]; then
    echo "No tracked C++ translation units found."
    exit 0
fi

ABS_CPP_FILES=()
for file in "${CPP_FILES[@]}"; do
    ABS_CPP_FILES+=("${ROOT_DIR}/${file}")
done

"${CLANG_TIDY_BIN}" \
    -p "${TIDY_COMPILE_DB_DIR}" \
    --quiet \
    "${ABS_CPP_FILES[@]}"

echo "clang-tidy passed for ${#CPP_FILES[@]} translation units."
