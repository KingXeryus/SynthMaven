# Filter capability map

These are the existing source-engine controls, not new DSP. Cutoff uses each engine’s native descriptor (Hz where the native scale is logarithmic cutoff); resonance and drive use percent, and envelope amount uses its native signed/unsigned depth. HYBRID exposes filter 1 base controls here; its cutoff/resonance offsets and filter 2/routing remain in the editor.

| Engine | Architecture | Cutoff | Resonance | Envelope | Drive |
|---|---|---|---|---|---|
| ANALOG | multi | P_E4 | P_E5 | P_ED_FLT | P_E6 |
| DIGITAL | none | -1 | -1 | -1 | -1 |
| PHASE | none | -1 | -1 | -1 | -1 |
| LOFI | ONE POLE | P_E7 | -1 | P_ED_FLT | -1 |
| SAMPLE | ONE POLE | P_E4 | -1 | P_ED_FLT | -1 |
| VOICE | none | -1 | -1 | -1 | -1 |
| TRIO | TRIO | P_E5 | P_E6 | P_ED_FLT | -1 |
| WHEEL | none | -1 | -1 | -1 | -1 |
| GRAIN | ONE POLE | P_E7 | -1 | P_ED_FLT | -1 |
| FM6 | none | -1 | -1 | -1 | -1 |
| PHYS | none | -1 | -1 | -1 | -1 |
| NOISE | SVF | P_E2 | P_E3 | P_ED_FLT | -1 |
| HYBRID | HYBRID | P_E0 | P_E1 | P_E2 | -1 |
| WAVE | multi | P_E4 | P_E5 | P_ED_FLT | P_E6 |
| ACID | LADDER | P_E1 | P_E2 | P_E3 | P_E6 |
| STRINGS | SVF | P_E4 | P_E7 | P_ED_FLT | -1 |
| MACRO | LPG | P_E4 | -1 | P_ED_FLT | -1 |
| VPS | SVF | P_E4 | P_E5 | P_ED_FLT | -1 |
| TERRAIN | SVF | P_E6 | P_E7 | P_ED_FLT | -1 |
| FMX | none | -1 | -1 | -1 | -1 |
| WSEQ | SVF | P_E6 | P_E7 | P_ED_FLT | -1 |
| JUNO | LADDER | P_E5 | P_E6 | P_ED_FLT | -1 |
| VECTOR | multi | P_E7 | -1 | P_ED_FLT | -1 |

`-1` means unavailable. An engine’s brightness modulation may change an FM index, vowel, physical-model excitation or other timbre control instead of filter cutoff; it is not relabeled as a filter. All engines still retain the separate part/filter and effects controls where supported. The optional COMP EST control is an estimated resonance/drive attenuation (up to 6 dB each), off by default. It is not measured loudness normalization or a guarantee of equal level; audition the result and use sound trim as needed.
