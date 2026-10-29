/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
static void autosave_tick(void)                /* main loop */
{
#if FELUCCA_FLASH
    uint32_t h, now = fm1_ms;
    if (!flash_ok || song.playing || transport_req || rec_wait || ft_on || ui.menu ||
        now - ui_input_ms < AUTOSAVE_IDLE || now - autosave_ms < AUTOSAVE_GAP || now - autosave_checked < 1000u)
        return;
    autosave_checked = now;
    /* Even taking the snapshot masks interrupts. Defer it until notes are quiet. */
    if (!audio_quiet())
        return;
#if SYNTHMAVEN_SYNTH
    program_capture(&autosave_work.program);h=autosave_work.program.sum;
#else
    proj_capture(&autosave_buf);h=autosave_buf.sum;
#endif
    if (h == autosave_hash)
        return;
    if (st_save(OBJ_AUTOSAVE, &autosave_buf, AUTOSAVE_SIZE) == 0)
        autosave_hash = h;
    autosave_ms = fm1_ms;
#endif
}

