// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/power.h>
#include <asm/arch/system.h>

static void s5pc110_set_system_display(void)
{
	int i;

	for (i = S5PC110_GPIO_F00; i < S5PC110_GPIO_F34; i++) {
		gpio_cfg_pin(i, S5P_GPIO_FUNC(2));
		gpio_set_pull(i, S5P_GPIO_PULL_NONE);
		gpio_set_drv(i, S5P_GPIO_DRV_4X);
	}

	writel(0x2, S5PC110_MDNIE_SEL);
}

void set_system_display_ctrl(void)
{
	if (cpu_is_s5pc110())
		s5pc110_set_system_display();
}
