#!/usr/bin/env bash
# Configure + build with sanitizers.
set -euo pipefail
cd "$(dirname "$0")/.."

cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DLUMEN_ENABLE_ASAN=ON \
    -DLUMEN_ENABLE_UBSAN=ON

cmake --build build --parallel
