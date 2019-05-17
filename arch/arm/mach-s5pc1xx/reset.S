/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2009 Samsung Electronics.
 * Minkyu Kang <mk7.kang@samsung.com>
 */

#include <asm/arch/cpu.h>
#include <linux/linkage.h>

#define S5PC100_SWRESET			0xE0200000

ENTRY(reset_cpu)
	/* S5PC100 */
	ldr	r1, =S5PC100_SWRESET
	ldr	r2, =0xC100
	b	200f
	str	r2, [r1]
_loop_forever:
	b	_loop_forever
ENDPROC(reset_cpu)
