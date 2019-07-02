// SPDX-License-Identifier: GPL-2.0+
#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <power/charger.h>
#include <power/max8998_pmic.h>
#include <power/pmic.h>

DECLARE_GLOBAL_DATA_PTR;

static int max8998_get_status(struct udevice *dev)
{
	u8 reg;
	int ret;

	ret = pmic_read(dev->parent, MAX8998_REG_STATUS2, &reg, 1);
	if (ret)
		return ret;

	/* No battery present */
	if (reg & (1 << 4))
		return CHARGE_STATE_UNKNOWN;

	/* No vbus detected */
	if (!(reg & (1 << 5)))
		return CHARGE_STATE_DISCHARGING;

	/* Charging done */
	if (reg & (1 << 6))
		return CHARGE_STATE_FULL;

	/* Charging on */
	if (reg & (1 << 3))
		return CHARGE_STATE_CHARGING;

	return CHARGE_STATE_NOT_CHARGING;
}

static struct dm_charger_ops max8998_chg_ops = {
	/* no get/set current yet */
	.get_status = max8998_get_status,
};

U_BOOT_DRIVER(charger_max8998) = {
	.name = MAX8998_CHARGER_DRIVER,
	.id = UCLASS_CHARGER,
	.ops = &max8998_chg_ops,
//	.probe = max8998_probe,
};
