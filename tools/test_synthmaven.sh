#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/gen build/host
for g in font:felucca_font icons:felucca_icons samples:felucca_samples drumkits:felucca_drumkits logo:sloop_logo tables:felucca_tables fm6_patches:felucca_fm6 wavetables:felucca_wavetables; do
 python3 "tools/gen_${g%%:*}.py" "build/gen/${g#*:}.h" >/dev/null
done
CC="${CC:-cc} -O1 -w -DSYNTHMAVEN_SYNTH=1 -Ibuild/gen -Ifirmware/src -Ifirmware/hal"
for t in synthmaven_test synthmaven_storage_test synthmaven_ui_test synthmaven_stock_test hybrid_editor_test; do
 $CC -o "build/host/$t" "tests/$t.c" -lm
 if [ "$t" = hybrid_editor_test ]; then "build/host/$t" build/host/hy_factory.json; else "build/host/$t"; fi
done
$CC -o build/host/midi_uart_test tests/midi_uart_test.c -lm
build/host/midi_uart_test
if [ "${SM_SKIP_FULL_WEB:-0}" != 1 ]; then node web/test_web.mjs; fi
node web/test_synthmaven.mjs

$CC -o build/host/wt_rom_test tests/wt_rom_test.c firmware/vendor/tinf/tinflate.c
build/host/wt_rom_test
node web/test_korg.mjs

$CC -o build/host/synthmaven_ble_test tests/synthmaven_ble_test.c
build/host/synthmaven_ble_test

for packed in 0 1; do
 $CC -DSM_PACKED_WT=$packed -o build/host/wt_render_$packed tests/wt_render_test.c firmware/vendor/tinf/tinflate.c -lm
 build/host/wt_render_$packed > build/host/wt_render_$packed.txt
done
cmp build/host/wt_render_0.txt build/host/wt_render_1.txt
echo 'WAVE render: every table/mip buffer matches unpacked ROM PASS'
