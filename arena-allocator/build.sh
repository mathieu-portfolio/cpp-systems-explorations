#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
BUILD_TYPE="${1:-Debug}"
TARGET="arena_test"

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_DIR" --target "$TARGET"

"./$BUILD_DIR/$TARGET"