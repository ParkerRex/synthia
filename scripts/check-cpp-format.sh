#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODE="check"
SCOPE="changed"
EXCLUDES=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --fix)
            MODE="fix"
            shift
            ;;
        --all)
            SCOPE="all"
            shift
            ;;
        --changed)
            SCOPE="changed"
            shift
            ;;
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

find_clang_format() {
    if [[ -n "${CLANG_FORMAT:-}" && -x "${CLANG_FORMAT}" ]]; then
        printf '%s\n' "${CLANG_FORMAT}"
        return 0
    fi

    if command -v clang-format >/dev/null 2>&1; then
        command -v clang-format
        return 0
    fi

    if [[ -x /opt/homebrew/opt/llvm/bin/clang-format ]]; then
        printf '%s\n' /opt/homebrew/opt/llvm/bin/clang-format
        return 0
    fi

    if command -v xcrun >/dev/null 2>&1; then
        xcrun --find clang-format 2>/dev/null && return 0
    fi

    return 1
}

find_git_clang_format() {
    if [[ -n "${GIT_CLANG_FORMAT:-}" && -x "${GIT_CLANG_FORMAT}" ]]; then
        printf '%s\n' "${GIT_CLANG_FORMAT}"
        return 0
    fi

    if command -v git-clang-format >/dev/null 2>&1; then
        command -v git-clang-format
        return 0
    fi

    if [[ -x /opt/homebrew/opt/llvm/bin/git-clang-format ]]; then
        printf '%s\n' /opt/homebrew/opt/llvm/bin/git-clang-format
        return 0
    fi

    return 1
}

CLANG_FORMAT_BIN="$(find_clang_format)" || {
    echo "clang-format not found. Set CLANG_FORMAT=/path/to/clang-format or install Homebrew llvm." >&2
    exit 1
}

CPP_FILES=()
if [[ "${SCOPE}" == "all" ]]; then
    while IFS= read -r file; do
        CPP_FILES+=("${file}")
    done < <(git -C "${ROOT_DIR}" ls-files '*.cpp' '*.h' '*.hpp' '*.mm')
else
    while IFS= read -r file; do
        if git -C "${ROOT_DIR}" ls-files --error-unmatch "${file}" >/dev/null 2>&1; then
            CPP_FILES+=("${file}")
        fi
    done < <(
        {
            git -C "${ROOT_DIR}" diff --name-only --diff-filter=ACMR -- '*.cpp' '*.h' '*.hpp' '*.mm'
            git -C "${ROOT_DIR}" diff --cached --name-only --diff-filter=ACMR -- '*.cpp' '*.h' '*.hpp' '*.mm'
        } | sort -u
    )
fi

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
    echo "No ${SCOPE} C++ files found."
    exit 0
fi

if [[ "${SCOPE}" == "changed" ]]; then
    GIT_CLANG_FORMAT_BIN="$(find_git_clang_format)" || {
        echo "git-clang-format not found. Set GIT_CLANG_FORMAT=/path/to/git-clang-format or install Homebrew llvm." >&2
        exit 1
    }

    if [[ "${MODE}" == "fix" ]]; then
        set +e
        FORMAT_OUTPUT="$("${GIT_CLANG_FORMAT_BIN}" \
            --binary "${CLANG_FORMAT_BIN}" \
            --extensions cpp,h,hpp,mm \
            --force \
            HEAD \
            -- "${CPP_FILES[@]}")"
        FORMAT_STATUS=$?
        set -e
        if [[ -n "${FORMAT_OUTPUT}" ]]; then
            printf '%s\n' "${FORMAT_OUTPUT}"
        fi
        if [[ "${FORMAT_STATUS}" -ne 0 && "${FORMAT_OUTPUT}" != changed\ files:* ]]; then
            exit "${FORMAT_STATUS}"
        fi
        echo "Formatted changed hunks in ${#CPP_FILES[@]} files."
    else
        set +e
        FORMAT_DIFF="$("${GIT_CLANG_FORMAT_BIN}" \
            --binary "${CLANG_FORMAT_BIN}" \
            --extensions cpp,h,hpp,mm \
            --diff \
            HEAD \
            -- "${CPP_FILES[@]}")"
        FORMAT_STATUS=$?
        set -e
        if [[ "${FORMAT_STATUS}" -ne 0 && -z "${FORMAT_DIFF}" ]]; then
            echo "git-clang-format failed without output." >&2
            exit "${FORMAT_STATUS}"
        fi
        if [[ "${FORMAT_DIFF}" != "no modified files to format" && "${FORMAT_DIFF}" != "clang-format did not modify any files" ]]; then
            printf '%s\n' "${FORMAT_DIFF}" >&2
            exit 1
        fi
        echo "C++ format gate passed for changed hunks in ${#CPP_FILES[@]} files."
    fi
    exit 0
fi

ABS_CPP_FILES=()
for file in "${CPP_FILES[@]}"; do
    ABS_CPP_FILES+=("${ROOT_DIR}/${file}")
done

if [[ "${MODE}" == "fix" ]]; then
    "${CLANG_FORMAT_BIN}" -i "${ABS_CPP_FILES[@]}"
    echo "Formatted ${#CPP_FILES[@]} ${SCOPE} C++ files."
else
    "${CLANG_FORMAT_BIN}" --dry-run -Werror "${ABS_CPP_FILES[@]}"
    echo "C++ format gate passed for ${#CPP_FILES[@]} ${SCOPE} files."
fi
