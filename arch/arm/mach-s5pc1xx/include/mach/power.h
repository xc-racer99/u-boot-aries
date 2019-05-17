/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_POWER_H_
#define __ASM_ARM_ARCH_POWER_H_

/*
 * Power control
 */
#define S5PC100_OTHERS			0xE0108200
#define S5PC100_RST_STAT		0xE0108300
#define S5PC100_SLEEP_WAKEUP		(1 << 3)
#define S5PC100_WAKEUP_STAT		0xE0108304
#define S5PC100_INFORM0			0xE0108400

#endif
