// SPDX-License-Identifier: GPL-2.0+
/*
 * s6e63m0 AMOLED LCD panel driver.
 *
 * Copyright (C) 2019 Jonathan Bakker <xc-racer2@live.ca>
 *
 * Based on kernel driver that is
 * Copyright (C) 2019 Paweł Chmiel <pawel.mikolaj.chmiel@gmail.com>
 */

#include <common.h>
#include <spi.h>
#include <dm.h>
#include <panel.h>
#include <mipi_display.h>
#include <asm/gpio.h>

/* Manufacturer Command Set */
#define MCS_ELVSS_ON                0xb1
#define MCS_MIECTL1                 0xc0
#define MCS_BCMODE                  0xc1
#define MCS_DISCTL                  0xf2
#define MCS_SRCCTL                  0xf6
#define MCS_IFCTL                   0xf7
#define MCS_PANELCTL                0xf8
#define MCS_PGAMMACTL               0xfa

#define DATA_MASK                   0x100

struct s6e63m0_panel_priv {
	struct gpio_desc reset;
};

static int s6e63m0_spi_write_word(struct spi_slave *slave, u16 data)
{
	return spi_xfer(slave, 9, &data, NULL, SPI_XFER_BEGIN);
}

static void s6e63m0_dcs_write(struct spi_slave *slave, const u8 *data, size_t len)
{
	int ret = 0;

	if (len == 0)
		return;

	pr_debug("%s: writing dcs seq: %*ph\n", __func__, (int)len, data);
	ret = s6e63m0_spi_write_word(slave, *data);

	while (!ret && --len) {
		++data;
		ret = s6e63m0_spi_write_word(slave, *data | DATA_MASK);
	}

	if (ret)
		pr_err("%s: error %d writing dcs seq: %*ph\n", __func__, ret,
			      (int)len, data);

	udelay(300);
}

#define s6e63m0_dcs_write_seq_static(slave, seq ...) \
	({ \
		static const u8 d[] = { seq }; \
		s6e63m0_dcs_write(slave, d, ARRAY_SIZE(d)); \
	})

static int s6e63m0_panel_enable_backlight(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);

	if (spi_claim_bus(slave)) {
		pr_err("%s: Cannot claim SPI bus\n", __func__);
		return -1;
	}

 	s6e63m0_dcs_write_seq_static(slave, MIPI_DCS_SET_DISPLAY_ON);

	spi_release_bus(slave);

	return 0;
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

	if (spi_claim_bus(slave)) {
		pr_err("%s: Cannot claim SPI bus\n", __func__);
		return -1;
	}

	dm_gpio_set_value(&priv->reset, 0);

	udelay(120 * 1000);

	s6e63m0_dcs_write_seq_static(slave, MCS_PANELCTL,
				     0x01, 0x27, 0x27, 0x07, 0x07, 0x54, 0x9f,
				     0x63, 0x86, 0x1a, 0x33, 0x0d, 0x00, 0x00);

	s6e63m0_dcs_write_seq_static(slave, MCS_DISCTL,
				     0x02, 0x03, 0x1c, 0x10, 0x10);
	s6e63m0_dcs_write_seq_static(slave, MCS_IFCTL,
				     0x03, 0x00, 0x00);

	s6e63m0_dcs_write_seq_static(slave, MCS_PGAMMACTL,
				     0x00, 0x18, 0x08, 0x24, 0x64, 0x56, 0x33,
				     0xb6, 0xba, 0xa8, 0xac, 0xb1, 0x9d, 0xc1,
				     0xc1, 0xb7, 0x00, 0x9c, 0x00, 0x9f, 0x00,
				     0xd6);
	s6e63m0_dcs_write_seq_static(slave, MCS_PGAMMACTL,
				     0x01);

	s6e63m0_dcs_write_seq_static(slave, MCS_SRCCTL,
				     0x00, 0x8c, 0x07);
	s6e63m0_dcs_write_seq_static(slave, 0xb3,
				     0xc);

	s6e63m0_dcs_write_seq_static(slave, 0xb5,
				     0x2c, 0x12, 0x0c, 0x0a, 0x10, 0x0e, 0x17,
				     0x13, 0x1f, 0x1a, 0x2a, 0x24, 0x1f, 0x1b,
				     0x1a, 0x17, 0x2b, 0x26, 0x22, 0x20, 0x3a,
				     0x34, 0x30, 0x2c, 0x29, 0x26, 0x25, 0x23,
				     0x21, 0x20, 0x1e, 0x1e);

	s6e63m0_dcs_write_seq_static(slave, 0xb6,
				     0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x44,
				     0x44, 0x55, 0x55, 0x66, 0x66, 0x66, 0x66,
				     0x66, 0x66);

	s6e63m0_dcs_write_seq_static(slave, 0xb7,
				     0x2c, 0x12, 0x0c, 0x0a, 0x10, 0x0e, 0x17,
				     0x13, 0x1f, 0x1a, 0x2a, 0x24, 0x1f, 0x1b,
				     0x1a, 0x17, 0x2b, 0x26, 0x22, 0x20, 0x3a,
				     0x34, 0x30, 0x2c, 0x29, 0x26, 0x25, 0x23,
				     0x21, 0x20, 0x1e, 0x1e, 0x00, 0x00, 0x11,
				     0x22, 0x33, 0x44, 0x44, 0x44, 0x55, 0x55,
				     0x66, 0x66, 0x66, 0x66, 0x66, 0x66);

	s6e63m0_dcs_write_seq_static(slave, 0xb9,
				     0x2c, 0x12, 0x0c, 0x0a, 0x10, 0x0e, 0x17,
				     0x13, 0x1f, 0x1a, 0x2a, 0x24, 0x1f, 0x1b,
				     0x1a, 0x17, 0x2b, 0x26, 0x22, 0x20, 0x3a,
				     0x34, 0x30, 0x2c, 0x29, 0x26, 0x25, 0x23,
				     0x21, 0x20, 0x1e, 0x1e);

	s6e63m0_dcs_write_seq_static(slave, 0xba,
				     0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x44,
				     0x44, 0x55, 0x55, 0x66, 0x66, 0x66, 0x66,
				     0x66, 0x66);

	s6e63m0_dcs_write_seq_static(slave, MCS_BCMODE,
				     0x4d, 0x96, 0x1d, 0x00, 0x00, 0x01, 0xdf,
				     0x00, 0x00, 0x03, 0x1f, 0x00, 0x00, 0x00,
				     0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x06,
				     0x09, 0x0d, 0x0f, 0x12, 0x15, 0x18);

	s6e63m0_dcs_write_seq_static(slave, 0xb2,
				     0x10, 0x10, 0x0b, 0x05);

	s6e63m0_dcs_write_seq_static(slave, MCS_MIECTL1,
				     0x01);

	s6e63m0_dcs_write_seq_static(slave, MCS_ELVSS_ON,
				     0x0b);

	s6e63m0_dcs_write_seq_static(slave, MIPI_DCS_EXIT_SLEEP_MODE);

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
