/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Charles Vestal: from X0X (github.com/charlesvestal/fm1-x0x, commit 00bff73),
 * after Melodee's dual-core audio by Kerem Kilic (github.com/keremimo/melodee). Brought into SLOOP
 * unchanged but for this notice (the SLOOP dualcore branch, github.com/andrerezemini/sloop-lab). */
/* The AC79's second core (docs/plans/2026-10-04-dual-core-and-clock.md), with what Melodee's
 * dual-core audio (github.com/keremimo/melodee, GPL-3.0) found on the device.
 *
 * Start (the AC79 SDK's EnableOtherCpu): core 1's interrupt bank off (0x1EEF300..), its entry
 * address into the word at 0x01C7FFF8, bit 3 of 0x10008 held while it starts, C1_CON (0x1EEE004)
 * bit 3 set (enabled) and bit 1 cleared (out of hold). It boots through the chip's ROM, which the
 * PC limits refuse, so they are open until it has reported in from fm1_c1_main. Hold is bit 1
 * set, then bit 3 cleared.
 *
 * Work: one job at a time, a function and an argument; core 0 hands it over (c1_job) and waits
 * for it (c1_done), single writer each way, csync around each (the cores share the caches).
 *
 * Flash: the waiting loop is in RAM (.c1_text), so an idle core 1 never fetches from the flash;
 * it works only for the audio interrupt, which waits for it before it returns, and core 0 turns
 * the flash off only with interrupts off. So the flash is never off under it. */
#pragma once
#include <stdint.h>

#define FM1_C1_CON (*(volatile uint32_t *)0x1EEE004u)
#define FM1_C1_CLK (*(volatile uint32_t *)0x00010008u)
#define FM1_C1_T4 (*(volatile uint32_t *)0x10804u)       /* TIMER4, 24 MHz */
#define FM1_C1_SYNC() __asm__ volatile("csync" ::: "memory")

extern void fm1_c1_entry(void);
extern uint32_t _c1_ustack[];
#define FM1_C1_STACK_WORDS (4096u / 4u)             /* (SynthMaven 0.7: fm1_cpu1.S, 4 KB) */
#define FM1_C1_MARK 0x43314D4Bu                  /* "C1MK": how deep core 1's stack has gone */
/* Keep each writer on its own cache line. Validate the entire published job
 * before CPU1 follows a shared function pointer (fm1-base/Melodee worker pattern). */
static struct {
    volatile uint32_t job, arg, check;
    void (*volatile fn)(uint32_t);
    volatile uint32_t alive __attribute__((aligned(32)));
    volatile uint32_t done, rejected;
} fm1_c1_mailbox __attribute__((aligned(32)));
#define fm1_c1_alive fm1_c1_mailbox.alive
#define fm1_c1_job fm1_c1_mailbox.job
#define fm1_c1_done fm1_c1_mailbox.done
#define fm1_c1_arg fm1_c1_mailbox.arg
#define fm1_c1_fn fm1_c1_mailbox.fn
#define FM1_C1_CHECK(fn,arg,job) ((uint32_t)(uintptr_t)(fn) ^ (arg) ^ (job)*0x9E3779B1u)
volatile uint32_t fm1_c1_trace;                 /* breadcrumbs (fm1_cpu1.S, fm1_c1_main) */

static uint8_t fm1_c1_on;                       /* started and answering: core 0 may hand it work */

void fm1_c1_main(void);
void __attribute__((section(".c1_text"), noreturn, used)) fm1_c1_main(void)
{
    uint32_t done = fm1_c1_done;
    /* CPU1 has a separate EMU bank; do not inherit the ROM's stack/FPU flags.
     * Keep the existing stack size and trap before an overflow reaches RAM code. */
    uint32_t lo=(uint32_t)(uintptr_t)_c1_ustack;
    uint32_t hi=lo+FM1_C1_STACK_WORDS*4u+256u;
    *(volatile uint32_t *)0x1EEF2D8u=hi;
    *(volatile uint32_t *)0x1EEF2DCu=lo;
    *(volatile uint32_t *)0x1EEF2E0u=hi;
    *(volatile uint32_t *)0x1EEF2E4u=lo;
    *(volatile uint32_t *)0x1EEF2D0u=(*(volatile uint32_t *)0x1EEF2D0u & ~((1u<<2)|(0x1Fu<<16)))|(1u<<3);
    fm1_c1_trace = 0xC1000002;
    fm1_c1_alive = 1;
    FM1_C1_SYNC();
    for (;;) {
        uint32_t j = fm1_c1_job;
        FM1_C1_SYNC();
        if (j == done) {
            /* Bound idle SRAM polling traffic while the radio DMA shares the bus.
             * The 64 NOPs run only between jobs; active rendering is unchanged. */
            __asm__ volatile(".rept 64\n\tnop\n\t.endr" ::: "memory");
            continue;
        }
        if (j != done) {
            void (*fn)(uint32_t) = fm1_c1_fn;
            uint32_t arg = fm1_c1_arg;
            if (j != done + 1u || fm1_c1_mailbox.check != FM1_C1_CHECK(fn,arg,j)) {
                fm1_c1_mailbox.rejected++;
                FM1_C1_SYNC();
                continue;
            }
            fn(arg);              /* the work itself runs from the flash */
            FM1_C1_SYNC();
            done = j;
            fm1_c1_done = j;
            FM1_C1_SYNC();
        }
    }
}

static void fm1_cpu1_hold(void)
{
    fm1_cpu1_halt();                            /* (2.7.1, fm1_sys.h) the bus-invalid traps off around the hold */
    FM1_C1_SYNC();
    fm1_c1_on = 0;
}

/* 0 = core 1 runs and answered; else it is held again. The top of RAM must be writable
 * (fm1_guard_unlock_top) and the PC limits open (fm1_guard_pc_open): the caller's */
static int fm1_cpu1_start(void)
{
    uint32_t saved, t0, i;
    if (fm1_c1_on)
        return 0;
    fm1_cpu1_hold();
    for (i = 0; i < FM1_C1_STACK_WORDS; i++)
        _c1_ustack[i] = FM1_C1_MARK;
    fm1_c1_alive = 0;
    fm1_c1_trace = 0;
    fm1_c1_done = fm1_c1_job;
    for (i = 0; i < 32u; i++)                   /* its interrupt bank: all off (core 0's is at 0x1EEF100) */
        *(volatile uint32_t *)(0x1EEF300u + 4u * i) = 0;
    *(volatile uint32_t *)0x01C7FFF8u = (uint32_t)(uintptr_t)&fm1_c1_entry;
    saved = FM1_C1_CLK;
    FM1_C1_CLK = saved | 0x8u;
    FM1_C1_SYNC();
    FM1_C1_CON |= 0x8u;
    FM1_C1_CON &= ~0x2u;
    t0 = FM1_C1_T4;
    while (!fm1_c1_alive && FM1_C1_T4 - t0 < 24000u * 20u)   /* 20 ms */
        FM1_C1_SYNC();
    FM1_C1_CLK = saved;
    if (!fm1_c1_alive) {
        fm1_cpu1_hold();
        return -1;
    }
    fm1_c1_on = 1;
    return 0;
}

/* hand core 1 a job: 1 = it took it (fm1_cpu1_wait before touching what it works on) */
static inline int fm1_cpu1_run(void (*fn)(uint32_t), uint32_t arg)
{
    if (!fm1_c1_on)
        return 0;
    fm1_c1_fn = fn;
    fm1_c1_arg = arg;
    fm1_c1_mailbox.check = FM1_C1_CHECK(fn,arg,fm1_c1_job+1u);
    FM1_C1_SYNC();
    fm1_c1_job = fm1_c1_job + 1u;
    FM1_C1_SYNC();
    return 1;
}

/* 0 = done; -1 = it did not finish in time: core 1 is held from now on (the caller does the work) */
static inline int fm1_cpu1_wait(void)
{
    uint32_t t0 = FM1_C1_T4;
    for (;;) {
        FM1_C1_SYNC();
        if (fm1_c1_done == fm1_c1_job)
            return 0;
        if (FM1_C1_T4 - t0 > 24000u * 8u) {     /* 8 ms: far past any stretch */
            fm1_cpu1_hold();
            return -1;
        }
    }
}
