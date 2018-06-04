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
#include <usb.h>
#include <usb_mass_storage.h>
#include <asm/mach-types.h>

#include <mach/power.h>

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

int mach_cpu_init(void)
{
	int val;
	int i;

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

	int regs[11] = {
		0xe0f00000,
		0xe1f00000,
		0xf1800000,
		0xf1900000,
		0xf1a00000,
		0xf1b00000,
		0xf1c00000,
		0xf1d00000,
		0xf1e00000,
		0xf1f00000,
		0xfaf00000,
	};

	for (i = 0; i < 11; i++) {
		val = readl(regs[i]);
		val = val & (!0x1);
		writel(val, regs[i]);
	}

	/*
	 * Diable ABB block to reduce sleep current at low temperature
	 * Note that it's hidden register setup don't modify it
	 */
	writel(0x00800000, 0xE010C300);

	/* Disable Watchdog */
	writel(0, S5PC110_WATCHDOG_BASE);

	/* setting SRAM */
	writel(0x9, S5PC110_SROMC_BASE);

	/*
	 * S5PC110 has 3 groups of interrupt sources
	 * Disable all interrupts (VIC0, VIC1 and VIC2)
	 */
	writel(0, S5PC110_VIC0_BASE + 0x14); /* INTENCLEAR */
	writel(0, S5PC110_VIC0_BASE + 0x00100000 + 0x14); /* VIC1 + INTENCLEAR */
	writel(0, S5PC110_VIC0_BASE + 0x00200000 + 0x14); /* VIC2 + INTENCLEAR */

	/* Set all interrupts as IRQ */
	writel(0, S5PC110_VIC0_BASE + 0xc); /* INTSELECT */
	writel(0, S5PC110_VIC0_BASE + 0x00100000 + 0xc); /* VIC1 + INTSELECT */
	writel(0, S5PC110_VIC0_BASE + 0x00200000 + 0xc); /* VIC2 + INTSELECT */

	/* Pending Interrupt Clear */
	writel(0, S5PC110_VIC0_BASE + 0xf00); /* INTADDRESS */
	writel(0, S5PC110_VIC0_BASE + 0x00100000 + 0xf00); /* VIC1 + INTADDRESS */
	writel(0, S5PC110_VIC0_BASE + 0x00200000 + 0xf00); /* VIC2 + INTADDRESS */

	/*
	 * uart_asm_init: Initialize UART's pins
	 *
	 * set GPIO to enable UART0-UART4
	 */
	writel(0x22222222, S5PC110_GPIO_BASE + 0x0); /* GPIO_A0_OFFSET */
	writel(0x00002222, S5PC110_GPIO_BASE + 0x20); /* GPIO_A1_OFFSET */

	/*
	 * Note that the following address
	 * 0xE020'0360 is reserved address at S5PC100
	 */
	/* UART_SEL MP0_5[7] at S5PC110 */
	val = readl(S5PC110_GPIO_BASE + 0x360 + 0x0); /* S5PC110_GPIO_MP0_5_OFFSET + S5PC1XX_GPIO_CON_OFFSET*/
	val &= (!(0xf << 28)); /* 28 = 7 * 4-bit */
	val |= (0x1 << 28); /* Output */
	writel(val, S5PC110_GPIO_BASE + 0x360 + 0x0); /* S5PC110_GPIO_MP0_5_OFFSET + S5PC1XX_GPIO_CON_OFFSET*/

	val = readl(S5PC110_GPIO_BASE + 0x360 + 0x8); /* S5PC1XX_GPIO_PULL_OFFSET */
	val &= (!(0x3 << 14)); /* 14 = 7 * 2-bit */
	val |= (0x2 << 14); /* Pull-up enabled */
	writel(val, S5PC110_GPIO_BASE + 0x360 + 0x8); /* S5PC1XX_GPIO_PULL_OFFSET */

	val = readl(S5PC110_GPIO_BASE + 0x360 + 0x4);  /* S5PC1XX_GPIO_DAT_OFFSET */
	val |= (1 << 7); /* 7 = 7 * 1-bit */
	writel(val, S5PC110_GPIO_BASE + 0x360 + 0x4); /* S5PC1XX_GPIO_DAT_OFFSET */

	/* internal_ram_init */
	writel(0, 0xF1500000);

	/* Clear wakeup status register */
	/* TODO - Why do we read this and then write the results back? */
	val = readl(S5PC110_WAKEUP_STAT);
	writel(val, S5PC110_WAKEUP_STAT);

	/* IO retension release */
	val = readl(S5PC110_OTHERS);
	val |= ((1 << 31) | (1 << 30) | (1 << 29) | (1 << 28));
	writel(val, S5PC110_OTHERS);

	return 0;
}

int board_init(void)
{
	/* Set Initial global variables */
	gd->bd->bi_arch_number = MACH_TYPE_GALAXYS4G;
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
	puts("Board:\tGalaxys4G\n");
	return 0;
}
#endif

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bis)
{
	int i, ret_sd = 0;

	/*
	 * SD card (T_FLASH) detect and init
	 * T_FLASH_DETECT: EINT28: GPH3[4] input mode
	 * TODO: Determine why the card has a voltage select timeout
	 */
	gpio_request(S5PC110_GPIO_H34, "t_flash_detect");
	gpio_cfg_pin(S5PC110_GPIO_H34, S5P_GPIO_INPUT);
	gpio_set_pull(S5PC110_GPIO_H34, S5P_GPIO_PULL_UP);

	if (!gpio_get_value(S5PC110_GPIO_H34)) {
		for (i = S5PC110_GPIO_G20; i < S5PC110_GPIO_G27; i++) {
			if (i == S5PC110_GPIO_G22) {
				gpio_set_drv(i, S5P_GPIO_DRV_2X);
				continue;
			}

			/* GPG2[0:6] special function 2 */
			gpio_cfg_pin(i, 0x2);
			/* GPG2[0:6] pull disable */
			gpio_set_pull(i, S5P_GPIO_PULL_NONE);
			/* GPG2[0:6] drv 2x */
			gpio_set_drv(i, S5P_GPIO_DRV_2X);
		}

		ret_sd = s5p_mmc_init(2, 4);
		if (ret_sd)
			pr_err("MMC: Failed to init SD card (MMC:2).\n");
	}

	return ret_sd;
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
