// SPDX-License-Identifier: GPL-2.0+
/*
 * s6e63m0 AMOLED LCD panel driver.
 *
 * Copyright (C) 2019 Jonathan Bakker <xc-racer2@live.ca>
 */

#include <common.h>
#include <spi.h>
#include <dm.h>
#include <panel.h>
#include <mipi_display.h>
#include <asm/gpio.h>

#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK			0xFF00
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

struct s6e63m0_panel_priv {
	struct gpio_desc reset;
};

/* gamma value: 1.9 */
static const unsigned int s6e63m0_19_300[] = {
	0x18, 0x08, 0x24, 0x61, 0x5F, 0x39, 0xBA,
	0xBD, 0xAD, 0xB1, 0xB6, 0xA5, 0xC4, 0xC5,
	0xBC, 0x00, 0xA0, 0x00, 0xA4, 0x00, 0xDB
};

static const unsigned short SEQ_PANEL_CONDITION_SET[] = {
	0xF8, 0x01,
	DATA_ONLY, 0x27,
	DATA_ONLY, 0x27,
	DATA_ONLY, 0x07,
	DATA_ONLY, 0x07,
	DATA_ONLY, 0x54,
	DATA_ONLY, 0x9f,
	DATA_ONLY, 0x63,
	DATA_ONLY, 0x86,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x0d,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_DISPLAY_CONDITION_SET[] = {
	0xf2, 0x02,
	DATA_ONLY, 0x03,
	DATA_ONLY, 0x1c,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x10,

	0xf7, 0x03,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_GAMMA_SETTING[] = {
	0xfa, 0x00,
	DATA_ONLY, 0x18,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x64,
	DATA_ONLY, 0x56,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0xb6,
	DATA_ONLY, 0xba,
	DATA_ONLY, 0xa8,
	DATA_ONLY, 0xac,
	DATA_ONLY, 0xb1,
	DATA_ONLY, 0x9d,
	DATA_ONLY, 0xc1,
	DATA_ONLY, 0xc1,
	DATA_ONLY, 0xb7,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x9c,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x9f,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xd6,

	0xfa, 0x01,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_ETC_CONDITION_SET[] = {
	0xf6, 0x00,
	DATA_ONLY, 0x8c,
	DATA_ONLY, 0x07,

	0xb3, 0xc,

	0xb5, 0x2c,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x0c,
	DATA_ONLY, 0x0a,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x0e,
	DATA_ONLY, 0x17,
	DATA_ONLY, 0x13,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x2a,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1b,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x17,

	DATA_ONLY, 0x2b,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x3a,
	DATA_ONLY, 0x34,
	DATA_ONLY, 0x30,
	DATA_ONLY, 0x2c,
	DATA_ONLY, 0x29,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x25,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x21,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x1e,
	DATA_ONLY, 0x1e,

	0xb6, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,

	DATA_ONLY, 0x55,
	DATA_ONLY, 0x55,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,

	0xb7, 0x2c,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x0c,
	DATA_ONLY, 0x0a,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x0e,
	DATA_ONLY, 0x17,
	DATA_ONLY, 0x13,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x2a,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1b,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x17,

	DATA_ONLY, 0x2b,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x3a,
	DATA_ONLY, 0x34,
	DATA_ONLY, 0x30,
	DATA_ONLY, 0x2c,
	DATA_ONLY, 0x29,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x25,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x21,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x1e,
	DATA_ONLY, 0x1e,

	0xb8, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,

	DATA_ONLY, 0x55,
	DATA_ONLY, 0x55,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,

	0xb9, 0x2c,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x0c,
	DATA_ONLY, 0x0a,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x0e,
	DATA_ONLY, 0x17,
	DATA_ONLY, 0x13,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x2a,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1b,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x17,

	DATA_ONLY, 0x2b,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x3a,
	DATA_ONLY, 0x34,
	DATA_ONLY, 0x30,
	DATA_ONLY, 0x2c,
	DATA_ONLY, 0x29,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x25,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x21,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x1e,
	DATA_ONLY, 0x1e,

	0xba, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,

	DATA_ONLY, 0x55,
	DATA_ONLY, 0x55,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,

	0xc1, 0x4d,
	DATA_ONLY, 0x96,
	DATA_ONLY, 0x1d,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0xdf,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
	DATA_ONLY, 0x06,
	DATA_ONLY, 0x09,
	DATA_ONLY, 0x0d,
	DATA_ONLY, 0x0f,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x15,
	DATA_ONLY, 0x18,

	0xb2, 0x10,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x0b,
	DATA_ONLY, 0x05,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_ACL_ON[] = {
	/* ACL on */
	0xc0, 0x01,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_ELVSS_ON[] = {
	/* ELVSS on */
	0xb1, 0x0b,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_STAND_BY_OFF[] = {
	0x11, COMMAND_ONLY,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_DISPLAY_ON[] = {
	0x29, COMMAND_ONLY,

	ENDDEF, 0x0000
};

static void s6e63m0_spi_write_byte(struct spi_slave *slave,
		int addr, int data)
{
	u16 tmp = (addr << 8) | data;
	u16 data_out;

	/*
	 * Data are transmitted in 9-bit words:
	 * the first bit is command/parameter, the other are the value.
	 * The value's LSB is shifted to MSB position, to be sent as 9th bit
	 */
	data_out = tmp >> 1;
	if (tmp & 0x01)
		data_out += 0x8000;
	spi_xfer(slave, 9, &data_out, NULL, SPI_XFER_BEGIN);
}

static void s6e63m0_spi_write(struct spi_slave *slave, unsigned char address,
	unsigned char command)
{
	if (address != DATA_ONLY)
		s6e63m0_spi_write_byte(slave, 0x0, address);
	if (command != COMMAND_ONLY)
		s6e63m0_spi_write_byte(slave, 0x1, command);
}

static void s6e63m0_panel_send_sequence(struct spi_slave *slave,
	const unsigned short *wbuf)
{
	int i = 0;

	while ((wbuf[i] & DEFMASK) != ENDDEF) {
		if ((wbuf[i] & DEFMASK) != SLEEPMSEC) {
			s6e63m0_spi_write(slave, wbuf[i], wbuf[i+1]);
		} else
			udelay(wbuf[i+1]*1000);
		i += 2;
	}
}

static int s6e63m0_panel_ofdata_to_platdata(struct udevice *dev)
{
	struct s6e63m0_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_request_by_name(dev, "reset-gpios", 0, &priv->reset,
				   GPIOD_IS_OUT);
	if (ret) {
		pr_err("%s: cannot get reset GPIO: ret=%d\n",
		      __func__, ret);
		return ret;
	}

	return 0;
}

static int s6e63m0_panel_probe(struct udevice *dev)
{
	struct s6e63m0_panel_priv *priv = dev_get_priv(dev);
	struct spi_slave *slave = dev_get_parent_priv(dev);
	int i;

	const unsigned short *init_seq[] = {
		SEQ_PANEL_CONDITION_SET,
		SEQ_DISPLAY_CONDITION_SET,
		SEQ_GAMMA_SETTING,
		SEQ_ETC_CONDITION_SET,
		SEQ_ACL_ON,
		SEQ_ELVSS_ON,
	};

	if (spi_claim_bus(slave)) {
		pr_err("%s: Cannot claim SPI bus\n", __func__);
		return -1;
	}

	dm_gpio_set_value(&priv->reset, 0);

	udelay(10 * 1000);

	dm_gpio_set_value(&priv->reset, 1);

	udelay(10 * 1000);

	dm_gpio_set_value(&priv->reset, 0);

	udelay(10 * 1000);

	for (i = 0; i < ARRAY_SIZE(init_seq); i++)
		s6e63m0_panel_send_sequence(slave, init_seq[i]);

	spi_release_bus(slave);

	return 0;
}

static int s6e63m0_panel_enable_backlight(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	int i;

	const unsigned short *enable_seq[] = {
		SEQ_STAND_BY_OFF,
		SEQ_DISPLAY_ON,
	};

	if (spi_claim_bus(slave)) {
		pr_err("%s: Cannot claim SPI bus\n", __func__);
		return -1;
	}

	for (i = 0; i < ARRAY_SIZE(enable_seq); i++)
		s6e63m0_panel_send_sequence(slave, enable_seq[i]);

	/* disable gamma table updating. */
	s6e63m0_spi_write(slave, 0xfa, 0x00);

	for (i = 0 ; i < ARRAY_SIZE(s6e63m0_19_300); i++)
		s6e63m0_spi_write(slave, DATA_ONLY, s6e63m0_19_300[i]);

	/* update gamma table. */
	s6e63m0_spi_write(slave, 0xfa, 0x01);

	spi_release_bus(slave);

	return 0;
}

static const struct panel_ops s6e63m0_panel_ops = {
	.enable_backlight	= s6e63m0_panel_enable_backlight,
};

static const struct udevice_id s6e63m0_panel_ids[] = {
	{ .compatible = "samsung,s6e63m0" },
	{ }
};

U_BOOT_DRIVER(s6e63m0_panel) = {
	.name	= "s6e63m0_panel",
	.id	= UCLASS_PANEL,
	.of_match = s6e63m0_panel_ids,
	.ops	= &s6e63m0_panel_ops,
	.ofdata_to_platdata	= s6e63m0_panel_ofdata_to_platdata,
	.probe		= s6e63m0_panel_probe,
	.priv_auto_alloc_size	= sizeof(struct s6e63m0_panel_priv),
};
