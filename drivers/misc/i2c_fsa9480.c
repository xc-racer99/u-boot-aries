// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 */

#include <common.h>
#include <linux/err.h>
#include <dm.h>
#include <i2c.h>

/* FSA9480 I2C registers */
#define FSA9480_REG_DEVID		0x01
#define FSA9480_REG_CTRL		0x02
#define FSA9480_REG_INT1		0x03
#define FSA9480_REG_INT2		0x04
#define FSA9480_REG_INT1_MASK		0x05
#define FSA9480_REG_INT2_MASK		0x06
#define FSA9480_REG_ADC			0x07
#define FSA9480_REG_TIMING1		0x08
#define FSA9480_REG_TIMING2		0x09
#define FSA9480_REG_DEV_T1		0x0a
#define FSA9480_REG_DEV_T2		0x0b
#define FSA9480_REG_BTN1		0x0c
#define FSA9480_REG_BTN2		0x0d
#define FSA9480_REG_CK			0x0e
#define FSA9480_REG_CK_INT1		0x0f
#define FSA9480_REG_CK_INT2		0x10
#define FSA9480_REG_CK_INTMASK1		0x11
#define FSA9480_REG_CK_INTMASK2		0x12
#define FSA9480_REG_MANSW1		0x13
#define FSA9480_REG_MANSW2		0x14

/* Control */
#define CON_SWITCH_OPEN		(1 << 4)
#define CON_RAW_DATA		(1 << 3)
#define CON_MANUAL_SW		(1 << 2)
#define CON_WAIT		(1 << 1)
#define CON_INT_MASK		(1 << 0)
#define CON_MASK		(CON_SWITCH_OPEN | CON_RAW_DATA | \
				CON_MANUAL_SW | CON_WAIT)

/* Interrupt 1 */
#define INT1_MASK		(0xff << 0)
#define INT_DETACH		(1 << 1)
#define INT_ATTACH		(1 << 0)

/* Interrupt 2 mask */
#define INT2_MASK		(0x1f << 0)

/* Timing Set 1 */
#define TIMING1_ADC_500MS	(0x6 << 0)

static int i2c_fsa9480_std_probe(struct udevice *dev)
{
	int ret;
	uint8_t val;
	u8 regs[2];

	/* ADC Detect Time: 500ms */
	val = TIMING1_ADC_500MS;
	ret = dm_i2c_write(dev, FSA9480_REG_TIMING1, &val, sizeof(val));
	if (ret < 0)
		return ret;

	/* configure automatic switching */
	val = CON_MASK;
	ret = dm_i2c_write(dev, FSA9480_REG_CTRL, &val, sizeof(val));
	if (ret < 0)
		return ret;

	/* mask all interrupts except attach and detach */
	val = INT1_MASK & ~(INT_ATTACH | INT_DETACH);
	ret = dm_i2c_write(dev, FSA9480_REG_INT1_MASK, &val, sizeof(val));
	if (ret < 0)
		return ret;
	val = INT2_MASK;
	ret = dm_i2c_write(dev, FSA9480_REG_INT2_MASK, &val, sizeof(val));
	if (ret < 0)
		return ret;

	/* clear interrupt */
	dm_i2c_read(dev, FSA9480_REG_INT1, regs, sizeof(regs));

	return 0;
}

static const struct udevice_id i2c_fsa9480_std_ids[] = {
	{ .compatible = "fcs,fsa9480" },
	{ }
};

U_BOOT_DRIVER(i2c_fsa9480_std) = {
	.name			= "i2c_fsa9480",
	.id			= UCLASS_I2C_GENERIC,
	.of_match		= i2c_fsa9480_std_ids,
	.probe			= i2c_fsa9480_std_probe,
};
