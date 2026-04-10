#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
TARGET="arena_demo"

cmake --build "$BUILD_DIR" --target "$TARGET"
"./$BUILD_DIR/$TARGET"