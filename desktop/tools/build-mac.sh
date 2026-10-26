#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail
cd "$(dirname "$0")/../.."
export DEVELOPER_DIR="${DEVELOPER_DIR:-/Library/Developer/CommandLineTools}"
PYTHON="${PYTHON:-python3}"
CMAKE="${CMAKE:-cmake}"
"$PYTHON" desktop/tools/prepare.py
"$CMAKE" -S desktop -B build/desktop -G Ninja \
 -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' \
 -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
"$CMAKE" --build build/desktop --parallel 4
"$CMAKE" --build build/desktop --target test
