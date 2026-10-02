#!/bin/bash
# Usage: ./build.sh [-d]   (-d = Debug build, default is Release)
#
# Everything runs inside ( ... ), a subshell. So even if the script is
# sourced (`source build.sh`), an error or `exit` only stops the build,
# never closes your terminal, and the `cd` doesn't change your shell's folder.

(
  # Script location: BASH_SOURCE works when sourced from bash, $0 otherwise (zsh).
  cd "$(dirname "${BASH_SOURCE[0]:-$0}")" || exit 1

  BUILD_TYPE=Release
  OPTIND=1 # reset, in case getopts was already used in this shell
  while getopts "d" opt; do
    case $opt in
      d) BUILD_TYPE=Debug ;;
      *) echo "Usage: build.sh [-d]" >&2; exit 1 ;;
    esac
  done

  echo "==> Configuring ($BUILD_TYPE)"
  # Configure (creates build/ if missing). Re-running also re-copies scenarios/.
  if ! cmake -S . -B build -DCMAKE_BUILD_TYPE="$BUILD_TYPE"; then
    echo -e "\n\033[31m==> CMake configure FAILED (see errors above)\033[0m" >&2
    exit 1
  fi

  echo "==> Building"
  if ! cmake --build build --target sim -j"$(nproc)"; then
    echo -e "\n\033[31m==> Build FAILED (see errors above)\033[0m" >&2
    exit 1
  fi

  echo -e "\033[32m==> Build OK ($BUILD_TYPE): build/sim\033[0m"
)
