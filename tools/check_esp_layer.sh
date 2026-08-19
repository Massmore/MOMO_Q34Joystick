#!/usr/bin/env bash
# Syntax-check the ESP32/NimBLE layer of MomoJoy on a PC using the stub headers
# in tools/stub. This does NOT replace a real `pio run -e readall` build - it
# only proves the library's own code is internally consistent (no typos, all
# members exist, every example compiles).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="${ROOT}/.build"
mkdir -p "${OUT}"

COMMON=(-std=gnu++17 -fsyntax-only -Wall -Wextra -Wno-unused-parameter
        -I"${ROOT}/tools/stub"
        -I"${ROOT}/Arduino/libraries/MomoJoy/src"
        -I"${ROOT}/Arduino/libraries/MomoJoy/src/core")

for f in "${ROOT}"/Arduino/libraries/MomoJoy/src/*.cpp "${ROOT}"/PlatformIO/src/app_*.cpp; do
  echo "  checking $(basename "$f")"
  g++ "${COMMON[@]}" "$f"
done

echo "ESP32 layer: syntax OK"
