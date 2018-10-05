#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/gpio.h>
#include <linux/mtd/onenand_regs.h>
#include <linux/string.h>

#include "aries.h"

#define GPD1_BASE 0x0C0
#define GPJ0_BASE 0x240
#define MP04_BASE 0x340

enum board cur_board = BOARD_MAX;

static const char *board_fit_name[BOARD_MAX] = {
	[BOARD_UNKNOWN] = "s5pc1xx-aries",
	[BOARD_CAPTIVATE] = "s5pc1xx-aries",
	[BOARD_FASCINATE] = "s5pc1xx-aries",
	[BOARD_FASCINATE4G] = "s5pc1xx-fascinate4g",
	[BOARD_GALAXYS] = "s5pc1xx-galaxys",
	[BOARD_GALAXYS4G] = "s5pc1xx-fascinate4g",
	[BOARD_GALAXYSB] = "s5pc1xx-aries",
	[BOARD_VIBRANT] = "s5pc1xx-aries",
};

/**
 * Low-level implementation of gpio_get_value which we can't use at this point
 * as it's not available yet.  gpio_cfg_pin and gpio_set_pull work as they are
 * implemented directly by drivers/gpio/s5p_gpio.c
 */
static unsigned int get_value(unsigned int base, unsigned int off)
{
	unsigned int value;

	value = readl(base + 0x04);
	return !!(value & (1 << off));
}

static unsigned int get_hwrev(void)
{
	unsigned int board_rev = 0;
	int i;

	/* GPIOs to configure */
	int hwrev_gpios[4] = {
		S5PC110_GPIO_J02,
		S5PC110_GPIO_J03,
		S5PC110_GPIO_J04,
		S5PC110_GPIO_J07,
	};

	/* Offsets from GPJ0_BASE corresponding to HWREV GPIOs */
	int hwrev_offs[4] = {
		2,
		3,
		4,
		7,
	};

	for (i = 0; i < 4; i++) {
		gpio_cfg_pin(hwrev_gpios[i], S5P_GPIO_INPUT);
		gpio_set_pull(hwrev_gpios[i], S5P_GPIO_PULL_NONE);
		board_rev |= get_value(S5PC110_GPIO_BASE + GPJ0_BASE, hwrev_offs[i]) << i;
	}

	return board_rev;
}

static bool is_galaxys(void)
{
	/**
	 * On i9000/i9000B:
	 * GPD1(2) and GPD1(3) - emulated I2C bus with external pull-up.
	 * When internally pulled down, value should stay high.
	 * Everything else:
	 * No external pull-up.  Goes low when interally pulled down.
	 */
	gpio_cfg_pin(S5PC110_GPIO_D12, S5P_GPIO_INPUT);
	gpio_set_pull(S5PC110_GPIO_D12, S5P_GPIO_PULL_DOWN);

	return get_value(S5PC110_GPIO_BASE + GPD1_BASE, 2);
}

static bool is_galaxys4g(void)
{
	/**
	 * On SGH-T959P/SGH-T959V/SGH-T959W:
	 * Onenand datasize is 8Gb
	 * Everything else:
	 * Onenand datasize is 4Gb
	 */
	unsigned int dev_id, density;

	dev_id = readw(CONFIG_SYS_ONENAND_BASE + ONENAND_REG_DEVICE_ID);
	density = dev_id >> ONENAND_DEVICE_DENSITY_SHIFT;
	density &= ONENAND_DEVICE_DENSITY_MASK;

	return density == ONENAND_DEVICE_DENSITY_8Gb;
}

static bool is_captivate(void)
{
	/**
	 * On i897:
	 * MP04(4) and MP04(5) - emulated I2C bus with external pull-up.
	 * When internally pulled down, value should stay high.
	 * Everything else:
	 * No external pull-up.  Goes low when interally pulled down.
	 */
	gpio_cfg_pin(S5PC110_GPIO_MP044, S5P_GPIO_INPUT);
	gpio_set_pull(S5PC110_GPIO_MP044, S5P_GPIO_PULL_DOWN);

	return get_value(S5PC110_GPIO_BASE + MP04_BASE, 4);
}

enum board guess_board(void)
{
	if (is_galaxys()) {
		if (get_hwrev() == 0xf)
			return BOARD_GALAXYSB;
		else
			return BOARD_GALAXYS;
	}

	if (is_galaxys4g()) {
		if (get_hwrev() == 0xf)
			return BOARD_FASCINATE4G;
		else
			return BOARD_GALAXYS4G;
	}

	if (is_captivate())
		return BOARD_CAPTIVATE;
	else if (get_hwrev() == 0x7)
		return BOARD_FASCINATE;
	else
		return BOARD_VIBRANT;
}

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	if (cur_board == BOARD_MAX) {
		cur_board = guess_board();
	}

	if (!strcmp(name, board_fit_name[cur_board]))
		return 0;
	return -1;
}
#endif
