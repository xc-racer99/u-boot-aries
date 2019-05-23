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

#define S5P_CHECK_SLEEP			(1 << 16)
#define S5P_CHECK_DIDLE			(1 << 19)
#define S5P_CHECK_DSTOP			(1 << 18)

#define S5PC110_RST_STAT		0xE010A000
#define S5PC110_MDNIE_SEL		0xE0107008
#define S5PC110_SLEEP_WAKEUP		(1 << 3)
#define S5PC110_NORMAL_CFG		0xE010C010
#define S5PC110_OSC_FREQ		0xE010C100
#define S5PC110_MTC_STABLE		0xE010C110
#define S5PC110_CLAMP_STABLE		0xE010C114
#define S5PC110_WAKEUP_STAT		0xE010C200
#define S5PC110_BLK_PWR_STAT		0xE010C204
#define S5PC110_OTHERS			0xE010E000
#define S5PC110_USB_PHY_CON		0xE010E80C
#define S5PC110_PS_HOLD_CTRL		0xE010E81C
#define S5PC110_INFORM0			0xE010F000
#define S5PC110_INFORM1			0xE010F004
#define S5PC110_INFORM2			0xE010F008
#define S5PC110_INFORM3			0xE010F00C
#define S5PC110_INFORM4			0xE010F010
#define S5PC110_INFORM5			0xE010F014
#define S5PC110_INFORM6			0xE010F018
#define S5PC110_INFORM7			0xE010F00C

/*
 * Set ps_hold data driving value high
 * This enables the machine to stay powered on
 * after the initial power-on condition goes away
 * (e.g. power button).
 */
void set_ps_hold_ctrl(void);

/*
 * Misc power init that is SoC specific
 * eg async registers
 */
void misc_power_init(void);

/*
 * SoC specific function to enable UART
 * parameters
 */
void uart_init(void);

/*
 *  Read SoC specific value to determine if resuming
 *  @return: the value can be either S5P_CHECK_SLEEP or
 *  S5P_CHECK_DIDLE or S5P_CHECK_LPA
 *  if none of these then its normal booting.
 */
uint32_t get_reset_status(void);


/* Read the resume function and call it */
void power_exit_wakeup(void);

#endif
