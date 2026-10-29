#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Independent 4x output-peak check for adversarial limiter inputs."""
import numpy as np
from match_factory_levels import measure
x=np.fromfile('build/host/limiter-stress.raw',dtype='<i4').reshape(-1,2)/32768
assert len(x)==192000
for i in range(4):
 lu,pk=measure(x[i*48000:(i+1)*48000]);assert lu is not None and pk<=-1,(i,lu,pk)
assert not np.any(np.abs(x)>=1)
print('Limiter: quarter-rate phase peak, high-frequency, impulses and low bass all below -1 dBTP; no clipped samples PASS')
