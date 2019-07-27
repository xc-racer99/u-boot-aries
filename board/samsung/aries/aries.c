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
#include <power/battery.h>
#include <power/charger.h>
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

static const char *board_linux_fdt_name[BOARD_MAX] = {
	[BOARD_UNKNOWN] = "s5pv210-aries.dtb",
	[BOARD_CAPTIVATE] = "s5pv210-aries.dtb",
	[BOARD_FASCINATE] = "s5pv210-aries.dtb",
	[BOARD_FASCINATE4G] = "s5pv210-fascinate4g.dtb",
	[BOARD_GALAXYS] = "s5pv210-galaxys.dtb",
	[BOARD_GALAXYS4G] = "s5pv210-fascinate4g.dtb",
	[BOARD_GALAXYSB] = "s5pv210-aries.dtb",
	[BOARD_VIBRANT] = "s5pv210-aries.dtb",
};

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

/*
 * This is only needed when not using the SPL
 * but as there is the option to use the stock
 * SBL it should remain here regardless
 */
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

int power_init_board(void)
{
	int val, ret, reg;
	struct udevice *dev;

	ret = pmic_get("max8998-pmic", &dev);
	if (ret) {
		pr_err("Failed to get max8998-pmic");
		return ret;
	}

	/* LDO7 1.8V */
	val = 0x02; /* (1800 - 1600) / 100; */
	ret |= pmic_reg_write(dev, MAX8998_REG_LDO7, val);

	/* LDO17 3.0V */
	val = 0xe; /* (3000 - 1600) / 100; */
	ret |= pmic_reg_write(dev, MAX8998_REG_LDO17, val);

	reg = pmic_reg_read(dev, MAX8998_REG_ONOFF3);
	reg |= MAX8998_LDO17;
	ret = pmic_reg_write(dev, MAX8998_REG_ONOFF3, reg);

	reg = pmic_reg_read(dev, MAX8998_REG_ONOFF2);
	reg |= MAX8998_LDO7;
	ret = pmic_reg_write(dev, MAX8998_REG_ONOFF2, reg);

	if (ret) {
		pr_err("MAX8998 LDO setting error!\n");
		return -EINVAL;
	}

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

/* Keymask for different bootmodes */
int bootmenu_keymask;
int recovery_keymask;

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	set_board_info();
	env_set("fdtfile", board_linux_fdt_name[cur_board]);

	/* Storage setup - larger OneNAND and no MMC */
	switch (cur_board) {
		case BOARD_FASCINATE4G:
		case BOARD_GALAXYS4G:
			env_set("mtdparts", "mtdparts=b0600000.onenand:256k@25856k(uboot-env),10240k(boot),10240k(recovery),980480k(ubi)");
			break;
		default:
			env_set("mtdparts", "mtdparts=b0600000.onenand:256k@25856k(uboot-env),10240k(boot),10240k(recovery),466432k(ubi)");
			break;
	}

	/* Keymasks for boot modes as some variants have home buttons */
	switch (cur_board) {
		case BOARD_GALAXYS:
		case BOARD_GALAXYSB:
			bootmenu_keymask = KEY_HOME;
			recovery_keymask = KEY_VOLUME_UP;
			break;
		default:
			bootmenu_keymask = KEY_VOLUME_UP;
			recovery_keymask = KEY_VOLUME_DOWN;
			break;
	}
#endif
	return 0;
}
#endif

int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}

void setup_android_options(int pressed)
{
	unsigned int inform5;

	inform5 = readl(S5PC110_INFORM5);

	/* Check for magical normal reboot value */
	if (inform5 == 0x12345678) {
		writel(0x0, S5PC110_INFORM5);
		inform5 = 0;
	}

	if (inform5 != 0) {
		env_set("boot_mode", "charger");
		env_set("onenand_load_offset", "0x1980000");
	} else if (pressed & recovery_keymask || readl(S5PC110_INFORM6) == 2) {
		env_set("boot_mode", "recovery");
		env_set("onenand_load_offset", "0x2380000");
	} else {
		env_set("boot_mode", "normal");
		env_set("onenand_load_offset", "0x1980000");
	}
}

static void power_off(void)
{
	unsigned int reg = readl(S5PC110_PS_HOLD_CTRL);
	reg &= 0xFFFFFEFF;
	writel(reg, S5PC110_PS_HOLD_CTRL);
}

static void battery_check(void)
{
	struct udevice *bat, *charger;
	int ret, state;

	ret = uclass_get_device(UCLASS_BATTERY, 0, &bat);
	if (ret) {
		printf("%s: failed to get battery device: %d\n", __func__, ret);
		return;
	}

	state = battery_get_status(bat);
	if (state != BAT_STATE_NEED_CHARGING)
		return;

	ret = uclass_get_device(UCLASS_CHARGER, 0, &charger);
	if (ret) {
		printf("%s: failed to get charger device: %d\n", __func__, ret);
		return;
	}

	if (charger_get_status(charger) == CHARGE_STATE_DISCHARGING) {
		/* Low power, not charging, so power off */
		printf("low_battery, not charging, so powering off\n");
		power_off();
		return;
	}

	/* wait until we have enough power */
	while (battery_get_status(bat) == BAT_STATE_NEED_CHARGING) {
		/* Make sure charger is still plugged in */
		if (charger_get_status(charger) == CHARGE_STATE_DISCHARGING)
			power_off();
		mdelay(500);
	}
}

int setup_bootcmd(void)
{
	int vol_up, vol_down;
	int power = S5PC110_GPIO_H26;
	int home = -1;
	int pressed = 0;
	int num_mmc = 2;
	unsigned int i = 0;
	char bootmenu_entry[14];

	switch (cur_board) {
		case BOARD_FASCINATE4G:
		case BOARD_GALAXYS4G:
			num_mmc = 1;
			/* fall through */
		case BOARD_VIBRANT:
			vol_up = S5PC110_GPIO_H31;
			vol_down = S5PC110_GPIO_H32;
			break;
		case BOARD_FASCINATE:
			vol_up = S5PC110_GPIO_H33;
			vol_down = S5PC110_GPIO_H31;
			break;
		default:
			vol_up = S5PC110_GPIO_H32;
			vol_down = S5PC110_GPIO_H31;
			home = S5PC110_GPIO_H35;
			break;
	}

	/* this is necessary on non-SGS4G devices for working volume keys */
	if (cur_board != BOARD_FASCINATE4G && cur_board != BOARD_GALAXYS4G) {
		gpio_request(S5PC110_GPIO_H21, "kbc1");
		gpio_cfg_pin(S5PC110_GPIO_H21, S5P_GPIO_OUTPUT);
		gpio_set_pull(S5PC110_GPIO_H21, S5P_GPIO_PULL_NONE);
		gpio_set_value(S5PC110_GPIO_H21, 0);
	}

	gpio_request(power, "power");
	gpio_cfg_pin(power, S5P_GPIO_INPUT);
	gpio_set_pull(power, S5P_GPIO_PULL_NONE);

	gpio_request(vol_up, "volume_up");
	gpio_cfg_pin(vol_up, S5P_GPIO_INPUT);
	gpio_set_pull(vol_up, S5P_GPIO_PULL_NONE);

	gpio_request(vol_down, "volume_down");
	gpio_cfg_pin(vol_down, S5P_GPIO_INPUT);
	gpio_set_pull(vol_down, S5P_GPIO_PULL_NONE);

	if (home > 0) {
		gpio_request(home, "home");
		gpio_cfg_pin(home, S5P_GPIO_INPUT);
		gpio_set_pull(home, S5P_GPIO_PULL_NONE);
	}

	if (gpio_get_value(vol_up) == 0)
		pressed |= KEY_VOLUME_UP;

	if (gpio_get_value(vol_down) == 0)
		pressed |= KEY_VOLUME_DOWN;

	if (home > 0 && gpio_get_value(home) == 0)
		pressed |= KEY_HOME;

	/* set variables that Android might need */
	setup_android_options(pressed);

	/* if we don't want a menu, use the last bootcmd */
	if (!(pressed & bootmenu_keymask) && env_get("prev_cmd")) {
		env_set("bootcmd", env_get("prev_cmd"));
		return 0;
	}

	env_set("bootcmd", "sleep 1; bootmenu 20;");

	/* add standard bootmenu entries */
	sprintf(bootmenu_entry, "bootmenu_%d", i++);
	env_set(bootmenu_entry,
			"Fastboot=ubi part ubi; fastboot usb 0; bootd;");

	sprintf(bootmenu_entry, "bootmenu_%d", i++);
	env_set(bootmenu_entry, env_get("onenand_android_boot"));

	if (num_mmc == 2) {
		sprintf(bootmenu_entry, "bootmenu_%d", i++);
		env_set(bootmenu_entry, "MMC Boot=if test \"${prev_cmd}\" !="
			" \"run bootcmd_mmc1\";"
			" then setenv prev_cmd run bootcmd_mmc1;"
			" saveenv; fi; run bootcmd_mmc1;");
	}

	sprintf(bootmenu_entry, "bootmenu_%d", i++);
	env_set(bootmenu_entry, "SD Card Boot=if test \"${prev_cmd}\" !="
		" \"run bootcmd_mmc0\"; "
		" then setenv prev_cmd run bootcmd_mmc0;"
		" saveenv; fi; run bootcmd_mmc0;");

	sprintf(bootmenu_entry, "bootmenu_%d", i++);
	env_set(bootmenu_entry, "UBI Boot=if test \"${prev_cmd}\" != "
			" \"run bootcmd_ubifs0\"; "
			" then setenv prev_cmd run bootcmd_ubifs0;"
			" saveenv; fi; run bootcmd_ubifs0;");

	if (num_mmc == 2) {
		sprintf(bootmenu_entry, "bootmenu_%d", i++);
		env_set(bootmenu_entry, "MMC - Mass Storage=ums 0 mmc 1;");
	}

	sprintf(bootmenu_entry, "bootmenu_%d", i++);
	env_set(bootmenu_entry, "SD Card - Mass Storage=ums 0 mmc 0;");

	sprintf(bootmenu_entry, "bootmenu_%d", i++);
	env_set(bootmenu_entry,
		"Update u-boot.bin from SD=run uboot_update; sleep 5; bootd;");

	sprintf(bootmenu_entry, "bootmenu_%d", i++);
	env_set(bootmenu_entry,
		"Update SPL from SD=run spl_update; sleep 5; bootd;");

	/* terminate bootmenu entries, in case of residual configs */
	sprintf(bootmenu_entry, "bootmenu_%d", i++);
	env_set(bootmenu_entry, NULL);

	return 0;
}

int board_late_init(void)
{
	uint64_t board_serial = 0;
	char board_serial_str[17];

	/* Base the serial number on values in PRO_ID + 0x14/0x18 */
	if (!env_get("serial#")) {
		board_serial = (uint64_t) readl(samsung_get_base_clock() + 0x18) << 32
				| readl(samsung_get_base_clock() + 0x14);

		sprintf(board_serial_str, "%016llx", board_serial);
		env_set("serial#", board_serial_str);
	}

	battery_check();

	return setup_bootcmd();
}
