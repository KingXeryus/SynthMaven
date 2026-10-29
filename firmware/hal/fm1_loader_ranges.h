/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
/* Loader may update application/storage, preserving factory head and tail. */
#define FM1_LOADER_RANGE_OK(off,n) ((uint32_t)(off)>=0x4000u && (uint32_t)(off)<=0xFF000u && (uint32_t)(n)<=0xFF000u-(uint32_t)(off))
