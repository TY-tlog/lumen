#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
./scripts/dev_build.sh
cd build && ctest --output-on-failure
