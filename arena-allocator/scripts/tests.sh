#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
BUILD_TYPE="${1:-Debug}"
TARGET="arena_tests"

cmake --build "$BUILD_DIR" --target "$TARGET" --config "$BUILD_TYPE"
"./$BUILD_DIR/$BUILD_TYPE/$TARGET"