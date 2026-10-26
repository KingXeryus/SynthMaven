#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-only
# simulator (default) or unsigned device. Set SYNTHMAVEN_TEAM for a signed device build.
set -euo pipefail
cd "$(dirname "$0")/../.."
export DEVELOPER_DIR="${DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}"
CMAKE="${CMAKE:-cmake}"
PYTHON="${PYTHON:-python3}"
"$PYTHON" desktop/tools/prepare.py
kind="${1:-simulator}"
if [ "$kind" = simulator ]; then sdk=iphonesimulator; else sdk=iphoneos; fi
folder="build/ios-$kind"
sign=(-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO)
if [ "$kind" != simulator ] && [ -n "${SYNTHMAVEN_TEAM:-}" ]; then
 sign=(-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM="$SYNTHMAVEN_TEAM" -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE=Automatic)
fi
"$CMAKE" -S desktop -B "$folder" -G Xcode -DCMAKE_SYSTEM_NAME=iOS \
 -DCMAKE_OSX_SYSROOT="$sdk" -DCMAKE_OSX_ARCHITECTURES=arm64 \
 -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 "${sign[@]}"
"$CMAKE" --build "$folder" --config Release --target SynthMaven_Standalone --parallel 4
