// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/armv7.h>
#include <asm/gpio.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/power.h>

#define INTENCLEAR_OFFSET 0x14

static uint32_t s5pc110_get_reset_status(void)
{
	return readl(S5PC110_RST_STAT);
}

uint32_t get_reset_status(void)
{
	if (cpu_is_s5pc110())
		return s5pc110_get_reset_status();
	else
		return 0;
}

static void s5pc110_set_ps_hold_ctrl(void)
{
	unsigned int reg = readl(S5PC110_PS_HOLD_CTRL);
	reg |= 0x301;
	writel(reg, S5PC110_PS_HOLD_CTRL);
}

/*
 * Set ps_hold data driving value high
 * This enables the machine to stay powered on
 * after the initial power-on condition goes away
 * (e.g. power button).
 */
void set_ps_hold_ctrl(void)
{
	if (cpu_is_s5pc110())
		s5pc110_set_ps_hold_ctrl();
}

static void s5pc110_uart_init(void)
{
	int i;

	for (i = S5PC110_GPIO_A00; i <= S5PC110_GPIO_A13; i++)
		gpio_cfg_pin(i, S5P_GPIO_FUNC(2));

	/* UART_SEL */
	gpio_cfg_pin(S5PC110_GPIO_MP057, S5P_GPIO_OUTPUT);
	gpio_set_pull(S5PC110_GPIO_MP057, S5P_GPIO_PULL_NONE);
	gpio_set_value(S5PC110_GPIO_MP057, 1);
}

void uart_init(void)
{
	if (cpu_is_s5pc110())
		s5pc110_uart_init();
}

static void s5pc110_misc_power_init(void)
{
	/*
	 * Initialize Async Register Setting for EVT1
	 * Because we are setting EVT1 as the default value of EVT0,
	 * setting EVT0 as well does not make things worse.
	 * Thus, for the simplicity, we set for EVT0, too
	 *
	 * The "Async Registers" are:
	 *	0xE0F0_0000
	 *	0xE1F0_0000
	 *	0xF180_0000
	 *	0xF190_0000
	 *	0xF1A0_0000
	 *	0xF1B0_0000
	 *	0xF1C0_0000
	 *	0xF1D0_0000
	 *	0xF1E0_0000
	 *	0xF1F0_0000
	 *	0xFAF0_0000
	 */

	clrsetbits_le32(0xe0f00000, 0xfffffffe, 0);
	clrsetbits_le32(0xe1f00000, 0xfffffffe, 0);
	clrsetbits_le32(0xf1800000, 0xfffffffe, 0);
	clrsetbits_le32(0xf1900000, 0xfffffffe, 0);
	clrsetbits_le32(0xf1a00000, 0xfffffffe, 0);
	clrsetbits_le32(0xf1b00000, 0xfffffffe, 0);
	clrsetbits_le32(0xf1c00000, 0xfffffffe, 0);
	clrsetbits_le32(0xf1d00000, 0xfffffffe, 0);
	clrsetbits_le32(0xf1e00000, 0xfffffffe, 0);
	clrsetbits_le32(0xf1f00000, 0xfffffffe, 0);
	clrsetbits_le32(0xfaf00000, 0xfffffffe, 0);

	/*
	 * Diable ABB block to reduce sleep current at low temperature
	 * Note that it's hidden register setup don't modify it
	 */
	writel(0x00800000, 0xE010C300);

	/* Disable Watchdog */
	writel(0, S5PC110_WATCHDOG_BASE);

	/* setting SRAM */
	writel(0x9, S5PC110_SROMC_BASE);

	/* Disable all interrupts (VIC0, VIC1, VIC2 and VIC3) */
	writel(0, S5PC110_VIC0_BASE + INTENCLEAR_OFFSET);
	writel(0, S5PC110_VIC1_BASE + INTENCLEAR_OFFSET);
	writel(0, S5PC110_VIC2_BASE + INTENCLEAR_OFFSET);
	writel(0, S5PC110_VIC3_BASE + INTENCLEAR_OFFSET);
}

void misc_power_init(void)
{
	if (cpu_is_s5pc110())
		s5pc110_misc_power_init();
}

static void s5pc110_power_exit_wakeup(void)
{
	uint32_t addr;

	/* turn off L2 cache */
	v7_outer_cache_disable();

	/* invalidate L2 cache also */
	invalidate_dcache_all();

	/* turn on L2 cache */
	v7_outer_cache_enable();

	addr = readl(S5PC110_INFORM0);

	asm("mov pc, %[addr]" : : [addr] "r" (addr));
}

void power_exit_wakeup(void)
{
	if (cpu_is_s5pc110())
		s5pc110_power_exit_wakeup();
}
