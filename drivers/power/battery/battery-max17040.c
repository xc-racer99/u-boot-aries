// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Jonathan Bakker <xc-racer2@live.ca>
 *
 * Based on a kernel driver
 *  Copyright (C) 2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 */
#include <common.h>
#include <dm.h>
#include <power/battery.h>
#include <i2c.h>

#define MAX17040_VCELL_MSB	0x02
#define MAX17040_VCELL_LSB	0x03
#define MAX17040_SOC_MSB	0x04
#define MAX17040_SOC_LSB	0x05
#define MAX17040_MODE_MSB	0x06
#define MAX17040_MODE_LSB	0x07
#define MAX17040_VER_MSB	0x08
#define MAX17040_VER_LSB	0x09
#define MAX17040_RCOMP_MSB	0x0C
#define MAX17040_RCOMP_LSB	0x0D
#define MAX17040_CMD_MSB	0xFE
#define MAX17040_CMD_LSB	0xFF

static int max17040_read_reg(struct udevice *dev, int reg)
{
	int ret;

	ret = dm_i2c_reg_read(dev, reg);
	if (ret < 0)
		dev_err(dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static int max17040_write_reg(struct udevice *dev, int reg, u8 val)
{
	int ret;

	ret = dm_i2c_reg_write(dev, reg, val);

	if (ret < 0)
		dev_err(dev, "%s: err %d\n", __func__, ret);

	return ret;

}

static void max17040_reset(struct udevice *dev)
{
	max17040_write_reg(dev, MAX17040_CMD_MSB, 0x54);
	max17040_write_reg(dev, MAX17040_CMD_LSB, 0x00);
}

static void max17040_get_version(struct udevice *dev)
{
	u8 msb;
	u8 lsb;

	msb = max17040_read_reg(dev, MAX17040_VER_MSB);
	lsb = max17040_read_reg(dev, MAX17040_VER_LSB);

	dev_info(dev, "MAX17040 Fuel-Gauge Ver %d%d\n", msb, lsb);
}


static int max17040_get_vcell(struct udevice *dev, unsigned int *uV)
{
	u8 msb;
	u8 lsb;

	msb = max17040_read_reg(dev, MAX17040_VCELL_MSB);
	lsb = max17040_read_reg(dev, MAX17040_VCELL_LSB);

	if (msb < 0 || lsb < 0)
		return -ENXIO;

	*uV = ((msb << 4) + (lsb >> 4)) * 1250;

	return 0;
}

static int max17040_get_soc(struct udevice *dev)
{
	u8 msb;
	u8 lsb;
	u32 soc = 0;
	u32 temp = 0;
	u32 temp_soc = 0;

	msb = max17040_read_reg(dev, MAX17040_SOC_MSB);
	lsb = max17040_read_reg(dev, MAX17040_SOC_LSB);

	temp = msb * 100 + ((lsb * 100) / 256);

	if (temp >= 100)
		temp_soc = temp;
	else {
		if (temp >= 70)
			temp_soc = 100;
		else
			temp_soc = 0;
	}

	/* rounding off and changing to percentage */
	soc = temp_soc / 100;

	if (temp_soc % 100 >= 50)
		soc += 1;

	if (soc >= 26)
		soc += 4;
	else
		soc = (30 * temp_soc) / 26 / 100;

	if (soc >= 100)
		soc = 100;

	return soc;
}

static int max17040_get_status(struct udevice *dev)
{
	unsigned int uV = 0;
	u16 soc;
	int ret;

	ret = max17040_get_vcell(dev, &uV);
	if (ret || uV == 0)
		return BAT_STATE_NOT_PRESENT;

	soc = max17040_get_soc(dev);
	if (soc < 5 || uV < 3600000)
		return BAT_STATE_NEED_CHARGING;

	return BAT_STATE_NORMAL;
}

static struct dm_battery_ops max17040_battery_ops = {
	.get_voltage = max17040_get_vcell,
	.get_status = max17040_get_status,
	.get_soc = max17040_get_soc,
};

static int max17040_probe(struct udevice *dev)
{
	u32 rcomp;
	int ret;

	max17040_reset(dev);

	mdelay(250);

	max17040_get_version(dev);

	ret = ofnode_read_u32(dev->node , "maxim,rcomp-value", &rcomp);

	/* set rcomp value for bat chemistry */
	if (ret == 0) {
		max17040_write_reg(dev, MAX17040_RCOMP_MSB, (u8) rcomp & 0xFF);
		max17040_write_reg(dev, MAX17040_RCOMP_LSB, (u8) rcomp >> 8);
	}

	return 0;
}

static const struct udevice_id max17040_ids[] = {
	{ .compatible = "maxim,max17040" },
	{ .compatible = "maxim,max77836-battery" },
	{ }
};

U_BOOT_DRIVER(battery_max17040) = {
	.name = "max17040_battery",
	.id = UCLASS_BATTERY,
	.of_match = max17040_ids,
	.ops = &max17040_battery_ops,
	.probe = max17040_probe,
};
