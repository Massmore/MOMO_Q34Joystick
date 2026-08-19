#!/usr/bin/env bash
# Run the MomoJoy core unit tests with a plain g++ (no PlatformIO needed).
# Equivalent to `pio test -e native`, useful in CI or offline environments.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="${ROOT}/.build"
mkdir -p "${OUT}"

g++ -std=gnu++17 -O1 -Wall -Wextra -Wno-unused-parameter \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"${ROOT}/Arduino/libraries/MomoJoy/src" \
    -I"${ROOT}/Arduino/libraries/MomoJoy/src/core" \
    -I"${ROOT}/tools/unity_shim" \
    "${ROOT}/PlatformIO/test/test_core/test_core.cpp" \
    -o "${OUT}/test_core"

"${OUT}/test_core"
