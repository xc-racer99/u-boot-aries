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

	/* Enable power domain */
	if(!(readl(S5PC110_BLK_PWR_STAT) & 0x1 << 3)) {
		setbits_le32(S5PC110_NORMAL_CFG, 0x1 << 3);

		/* Wait for update */
		while(!(readl(S5PC110_BLK_PWR_STAT) & 0x1 << 3))
			mdelay(1);
	}

	/**
	 * Set GPF0 as LVD_HSYNC,VSYNC,VCLK,VDEN,VD[3:0]
	 * Set GPF1 as VD[11:4]
	 * Set GPF2 as VD[19:12]
	 * Set GPF3 as VD[23:20]
	 * Disable pulls, drive strength to max
	 */
	for (i = S5PC110_GPIO_F00; i < S5PC110_GPIO_F34; i++) {
		gpio_cfg_pin(i, S5P_GPIO_FUNC(2));
		gpio_set_pull(i, S5P_GPIO_PULL_NONE);
		gpio_set_drv(i, S5P_GPIO_DRV_4X);
	}

	/* Set everything to go to FIMD */
	writel(0x2, S5PC110_MDNIE_SEL);
}

void set_system_display_ctrl(void)
{
	if (cpu_is_s5pc110())
		s5pc110_set_system_display();
}
