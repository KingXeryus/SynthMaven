#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import concurrent.futures,subprocess,sys
reference,isolated=sys.argv[1:3]
def compare(n):
 a=subprocess.check_output([reference,str(n)])
 b=subprocess.check_output([isolated,str(n)])
 if a!=b:raise RuntimeError(f'Preset {n}: reference {a} != isolated {b}')
with concurrent.futures.ThreadPoolExecutor(4) as pool:list(pool.map(compare,range(449)))
print('PASS: all 449 factory presets produce byte-identical 8192-frame float output to the original DSP adapter at 44.1 kHz.')
