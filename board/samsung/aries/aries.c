// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2008-2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/arch/mmc.h>
#include <dm.h>
#include <power/pmic.h>
#include <usb/dwc2_udc.h>
#include <asm/arch/cpu.h>
#include <power/max8998_pmic.h>
#include <samsung/misc.h>
#include <mmc.h>
#include <usb.h>
#include <usb_mass_storage.h>
#include <asm/mach-types.h>

#include <mach/power.h>

#include "aries.h"

DECLARE_GLOBAL_DATA_PTR;

u32 get_board_rev(void)
{
	int i;
	u32 hwrev = 0;

	int hwrev_gpios[4] = {
		S5PC110_GPIO_J02,
		S5PC110_GPIO_J03,
		S5PC110_GPIO_J04,
		S5PC110_GPIO_J07,
	};

	for (i = 0; i < 4; i++) {
		gpio_request(hwrev_gpios[i], "hw_rev");
		gpio_cfg_pin(hwrev_gpios[i], S5P_GPIO_INPUT);
		gpio_set_pull(hwrev_gpios[i], S5P_GPIO_PULL_NONE);
		hwrev |= gpio_get_value(hwrev_gpios[i]) << i;
		gpio_free(hwrev_gpios[i]);
	}

	return hwrev;
}

int board_early_init_f(void)
{
	gpio_set_pull(S5PC110_GPIO_MP057, S5P_GPIO_PULL_UP);
	gpio_cfg_pin(S5PC110_GPIO_MP057, S5P_GPIO_INPUT);
	gpio_set_value(S5PC110_GPIO_MP057, 1);

	return 0;
}

int board_init(void)
{
	/* Set Initial global variables */
	gd->bd->bi_arch_number = MACH_TYPE_SAMSUNG_ARIES;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

#ifdef CONFIG_SYS_I2C_INIT_BOARD
void i2c_init_board(void)
{
	gpio_request(S5PC110_GPIO_J43, "i2c_clk");
	gpio_request(S5PC110_GPIO_J40, "i2c_data");
	gpio_direction_output(S5PC110_GPIO_J43, 1);
	gpio_direction_output(S5PC110_GPIO_J40, 1);
}
#endif

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE + PHYS_SDRAM_2_SIZE +
			PHYS_SDRAM_3_SIZE;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board:\tAries\n");
	return 0;
}
#endif

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bis)
{
	struct udevice *dev;
	int i, reg, ret, ret_sd = 0;

	/* Enable vmmc LDO */
	ret = pmic_get("max8998-pmic", &dev);
	if (ret) {
		pr_err("Failed to get max8998-pmic");
		return ret;
	}

	reg = pmic_reg_read(dev, MAX8998_REG_ONOFF1);
	reg |= MAX8998_LDO5;
	ret = pmic_reg_write(dev, MAX8998_REG_ONOFF1, reg);
	if (ret) {
		pr_err("MAX8998 LDO setting error!\n");
		return -EINVAL;
	}

	/*
	 * Register SD card first so it is device 0 on all variants
	 *
	 * SD card (T_FLASH) detect and init
	 * T_FLASH_DETECT: EINT28: GPH3[4] input mode
	 */
	gpio_request(S5PC110_GPIO_H34, "t_flash_detect");
	gpio_cfg_pin(S5PC110_GPIO_H34, S5P_GPIO_INPUT);
	gpio_set_pull(S5PC110_GPIO_H34, S5P_GPIO_PULL_UP);

	if (!gpio_get_value(S5PC110_GPIO_H34)) {
		for (i = S5PC110_GPIO_G20; i < S5PC110_GPIO_G27; i++) {
			if (i == S5PC110_GPIO_G22)
				continue;

			/* GPG2[0:6] special function 2 */
			gpio_cfg_pin(i, 0x2);
			/* GPG2[0:6] pull disable */
			gpio_set_pull(i, S5P_GPIO_PULL_NONE);
			/* GPG2[0:6] drv 4x */
			gpio_set_drv(i, S5P_GPIO_DRV_4X);
		}

		ret_sd = s5p_mmc_init(2, 4);
		if (ret_sd)
			pr_err("MMC: Failed to init SD card\n");
	}

	/* Now register emmc for devices with it */
	if (cur_board != BOARD_FASCINATE4G && cur_board != BOARD_GALAXYS4G) {
		/* MASSMEMORY_EN: XMSMDATA7: GPJ2[7] output high */
		gpio_request(S5PC110_GPIO_J27, "massmemory_en");
		gpio_direction_output(S5PC110_GPIO_J27, 1);

		/*
		 * MMC0 GPIO
		 * GPG0[0]	SD_0_CLK
		 * GPG0[1]	SD_0_CMD
		 * GPG0[2]	SD_0_CDn	-> Not used
		 * GPG0[3:6]	SD_0_DATA[0:3]
		 */
		for (i = S5PC110_GPIO_G00; i < S5PC110_GPIO_G07; i++) {
			if (i == S5PC110_GPIO_G02)
				continue;
			/* GPG0[0:6] special function 2 */
			gpio_cfg_pin(i, 0x2);
			/* GPG0[0:6] pull disable */
			gpio_set_pull(i, S5P_GPIO_PULL_NONE);
			/* GPG0[0:6] drv 4x */
			gpio_set_drv(i, S5P_GPIO_DRV_4X);
		}

		ret = s5p_mmc_init(0, 4);
		if (ret)
			pr_err("MMC: Failed to init emmc\n");
	}

	return ret & ret_sd;
}
#endif

#ifdef CONFIG_USB_GADGET
static int s5pc1xx_phy_control(int on)
{
	struct udevice *dev;
	static int status;
	int reg, ret;

	ret = pmic_get("max8998-pmic", &dev);
	if (ret)
		return ret;

	if (on && !status) {
		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF1);
		reg |= MAX8998_LDO3;
		ret = pmic_reg_write(dev, MAX8998_REG_ONOFF1, reg);
		if (ret) {
			puts("MAX8998 LDO setting error!\n");
			return -EINVAL;
		}

		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF2);
		reg |= MAX8998_LDO8;
		ret = pmic_reg_write(dev, MAX8998_REG_ONOFF2, reg);
		if (ret) {
			puts("MAX8998 LDO setting error!\n");
			return -EINVAL;
		}
		status = 1;
	} else if (!on && status) {
		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF1);
		reg &= ~MAX8998_LDO3;
		ret = pmic_reg_write(dev, MAX8998_REG_ONOFF1, reg);
		if (ret) {
			puts("MAX8998 LDO setting error!\n");
			return -EINVAL;
		}

		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF2);
		reg &= ~MAX8998_LDO8;
		ret = pmic_reg_write(dev, MAX8998_REG_ONOFF2, reg);
		if (ret) {
			puts("MAX8998 LDO setting error!\n");
			return -EINVAL;
		}
		status = 0;
	}
	udelay(10000);
	return 0;
}

struct dwc2_plat_otg_data s5pc110_otg_data = {
	.phy_control = s5pc1xx_phy_control,
	.regs_phy = S5PC110_PHY_BASE,
	.regs_otg = S5PC110_OTG_BASE,
	.usb_phy_ctrl = S5PC110_USB_PHY_CONTROL,
};

int board_usb_init(int index, enum usb_init_type init)
{
	debug("USB_udc_probe\n");
	return dwc2_udc_probe(&s5pc110_otg_data);
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	set_board_info();
#endif
	return 0;
}
#endif

int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}

int board_late_init(void)
{
	int val;
	uint64_t board_serial = 0;
	char board_serial_str[17];

	/* Base the serial number on the SD card since we don't have an emmc */
	if (!env_get("serial#")) {
		struct mmc *mmc = find_mmc_device(0);
		if (!mmc)
			pr_err("%s: couldn't get serial number - no MMC device found!\n", __func__);
		else if (mmc_init(mmc))
			pr_err("%s: MMC init failed!\n", __func__);
		else
			board_serial = ((uint64_t)mmc->cid[2] << 32) | mmc->cid[3];

		sprintf(board_serial_str, "%016llx", board_serial);
		env_set("serial#", board_serial_str);
	}

	val = readl(S5PC110_INFORM5);

	if (val) {
		env_set("boot_mode", "charger");
		return 0;
	}

	val = readl(S5PC110_INFORM6);

	if (val == 6) {
		env_set("boot_mode", "recovery");
		return 0;
	}

	env_set("boot_mode", "normal");
	return 0;
}
