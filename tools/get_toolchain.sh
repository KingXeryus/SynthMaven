#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
# Download JieLi's Linux toolchain (clang 4.0.1 for pi32v2) and link it as DEST/toolchain.
#   tools/get_toolchain.sh [DEST]      (default: ~/.jieli)
set -e
DEST="${1:-$HOME/.jieli}"
mkdir -p "$DEST"
curl -fL --connect-timeout 30 --max-time 180 --retry 2 https://pkgman.jieliapp.com/s/linux-toolchain -o "$DEST/toolchain.tar.xz"
tar -xJ -f "$DEST/toolchain.tar.xz" -C "$DEST"
rm -f "$DEST/toolchain.tar.xz"
TC="$(ls -d "$DEST"/jieli-linux-toolchains-* | sort | tail -1)"
ln -sfn "$(basename "$TC")" "$DEST/toolchain"
echo "JIELI_TOOLCHAIN=$DEST/toolchain ($(basename "$TC"))"
