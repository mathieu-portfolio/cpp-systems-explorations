#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
BUILD_TYPE="${1:-Debug}"

# Build everything (including tests)
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

# Run all tests via CTest
ctest --test-dir "$BUILD_DIR" -C "$BUILD_TYPE" --output-on-failure -V