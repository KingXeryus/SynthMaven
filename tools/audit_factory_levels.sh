#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/host
python3 tools/render_factory_levels.py
python3 tools/match_factory_levels.py build/host/factory-stereo.raw build/host/factory-levels.txt --out build/host/factory-level-audit.json --verify
