/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Kerem Kilic (Ellic Studio) */
#ifndef MELODEE_MIDI_KNOBS_H
#define MELODEE_MIDI_KNOBS_H
enum { MK_OFF, MK_ABSOLUTE, MK_RELATIVE, MK_MODES };
/* Device preference, independent of projects. Opt in to avoid stealing synth CCs. */
static volatile uint8_t settings_midi_knobs;
/* A MiniLab is attached to USB HOST (controller.c): its USB input uses Arturia's CCs. */
static volatile uint8_t midi_knob_minilab;
#endif
