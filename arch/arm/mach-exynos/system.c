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

static void exynos5_set_usbhost_mode(unsigned int mode)
{
	struct exynos5_sysreg *sysreg =
		(struct exynos5_sysreg *)samsung_get_base_sysreg();

	/* Setting USB20PHY_CONFIG register to USB 2.0 HOST link */
	if (mode == USB20_PHY_CFG_HOST_LINK_EN) {
		setbits_le32(&sysreg->usb20phy_cfg,
				USB20_PHY_CFG_HOST_LINK_EN);
	} else {
		clrbits_le32(&sysreg->usb20phy_cfg,
				USB20_PHY_CFG_HOST_LINK_EN);
	}
}

void set_usbhost_mode(unsigned int mode)
{
	if (cpu_is_exynos5())
		exynos5_set_usbhost_mode(mode);
}

static void exynos3110_set_system_display(void)
{
	int i;

	for (i = EXYNOS3110_GPIO_F00; i < EXYNOS3110_GPIO_F34; i++) {
		gpio_cfg_pin(i, S5P_GPIO_FUNC(2));
		gpio_set_pull(i, S5P_GPIO_PULL_NONE);
		gpio_set_drv(i, S5P_GPIO_DRV_4X);
	}

	writel(0x2, EXYNOS3110_MDNIE_SEL);
}

static void exynos4_set_system_display(void)
{
	struct exynos4_sysreg *sysreg =
	    (struct exynos4_sysreg *)samsung_get_base_sysreg();
	unsigned int cfg = 0;

	/*
	 * system register path set
	 * 0: MIE/MDNIE
	 * 1: FIMD Bypass
	 */
	cfg = readl(&sysreg->display_ctrl);
	cfg |= (1 << 1);
	writel(cfg, &sysreg->display_ctrl);
}

static void exynos5_set_system_display(void)
{
	struct exynos5_sysreg *sysreg =
	    (struct exynos5_sysreg *)samsung_get_base_sysreg();
	unsigned int cfg = 0;

	/*
	 * system register path set
	 * 0: MIE/MDNIE
	 * 1: FIMD Bypass
	 */
	cfg = readl(&sysreg->disp1blk_cfg);
	cfg |= (1 << 15);
	writel(cfg, &sysreg->disp1blk_cfg);
}

void set_system_display_ctrl(void)
{
	if (cpu_is_exynos3())
		exynos3110_set_system_display();
	else if (cpu_is_exynos4())
		exynos4_set_system_display();
	else
		exynos5_set_system_display();
}
