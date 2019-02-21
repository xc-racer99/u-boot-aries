/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/power.h>

/*
 * Register usages:
 *
 * r5 has zero always
 * r7 has S5PC100 GPIO base, 0xE0300000
 * r8 has real GPIO base, 0xE0300000, 0xE0200000 at S5PC100, S5PC110 repectively
 * r9 has Mobile DDR size, 1 means 1GiB, 2 means 2GiB and so on
 */

	.globl do_ibl
do_ibl:
	mov	r10, lr

	bl	jump_to_sgs_ibl

	mov	pc, r10

/*
 * jump_to_sgs_ibl: Helper to jump to platform config from IBL
 */
jump_to_sgs_ibl:
	stmfd	sp!, {lr}
	.align	4
	.incbin	"/home/jon/android/bootloaders/hummingbird-hibl/HummingBirdInterceptorBootloader/init_by_rebell.bin",0,0x8c0
