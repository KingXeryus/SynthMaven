#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/gen build/host
for g in font:felucca_font icons:felucca_icons samples:felucca_samples drumkits:felucca_drumkits logo:sloop_logo tables:felucca_tables fm6_patches:felucca_fm6 wavetables:felucca_wavetables; do
 python3 "tools/gen_${g%%:*}.py" "build/gen/${g#*:}.h" >/dev/null
done
CC="${CC:-cc} -O1 -w -DSYNTHMAVEN_SYNTH=1 -Ibuild/gen -Ifirmware/src -Ifirmware/hal"
for t in synthmaven_test synthmaven_storage_test synthmaven_ui_test synthmaven_stock_test; do
 $CC -o "build/host/$t" "tests/$t.c" -lm
 "build/host/$t"
done
$CC -o build/host/midi_uart_test tests/midi_uart_test.c -lm
build/host/midi_uart_test
node web/test_web.mjs
node web/test_synthmaven.mjs
