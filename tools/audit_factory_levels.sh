#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/host
cc -O2 -w -DSYNTHMAVEN_SYNTH=1 -Ibuild/gen -Ifirmware/src -Ifirmware/hal tests/factory_level.c -o build/host/factory_level -lm
build/host/factory_level /dev/null build/host/factory-levels.txt build/host/factory-stereo.raw
python3 tools/match_factory_levels.py build/host/factory-stereo.raw build/host/factory-levels.txt --out build/host/factory-level-audit.json --verify
