// SPDX-License-Identifier: GPL-2.0+
/*
 * Polled GPIO keys keyboard driver
 *
 * (c) 2018 Jonathan Bakker <xc-racer2@live.ca>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <input.h>
#include <keyboard.h>
#include <stdio_dev.h>
#include <asm/gpio.h>
#include <dm/ofnode.h>

#define MAX_KEYS		8
#define REPEAT_RATE_MS		350
#define REPEAT_DELAY_MS		350

struct gpio_keys_button {
	unsigned int code;
	struct gpio_desc gpio;
};

struct gpio_keys_polled_priv {
	struct input_config *input; /* The input layer */
	struct gpio_keys_button buttons[MAX_KEYS]; /* Array of gpio_keys_button struct */
	int nbuttons; /* Number of buttons */
};

/**
 * Check the keyboard, and send any keys that are pressed.
 *
 * This is called by input_tstc() and input_getc() when they need more
 * characters
 *
 * @param input		Input configuration
 * @return 1 to indicate that we have something to look at, 0 otherwise
 */
int gpio_keys_check(struct input_config *input)
{
	struct udevice *dev = input->dev;
	struct gpio_keys_polled_priv *priv = dev_get_priv(dev);
	int keycodes[MAX_KEYS];
	int num_keycodes = 0;
	int i;

	for (i = 0; i < priv->nbuttons; i++) {
		if (dm_gpio_get_value(&priv->buttons[i].gpio)) {
			keycodes[num_keycodes] = priv->buttons[i].code;
			num_keycodes += 1;
		}	
	}

	if (num_keycodes > 0) {
		input_send_keycodes(input, keycodes, num_keycodes);

		return 1;
	}

	/* No keycodes */
	return 0;
}

/**
 * Decode gpio key button details from the device tree
 *
 * @param parent	Ofnode of this node
 * @param config	Private data read from fdt
 * @return 0 if ok, -1 on error
 */
static int gpio_keys_polled_decode_fdt(ofnode parent,
				   struct gpio_keys_polled_priv *config)
{
	ofnode button;
	int num = 0;
	int ret;

	/* Decode each subnode */
	ofnode_for_each_subnode(button, parent) {
		ret = gpio_request_by_name_nodev(button, "gpios", 0,
				&config->buttons[num].gpio, GPIOD_IS_IN);
		if (ret) {
			pr_err("%s: Cannot get gpio (ret=%d)\n", __func__, ret);
			return -EINVAL;
		}

		ret = ofnode_read_u32(button, "linux,code", &config->buttons[num].code);
		if (ret) {
			pr_err("%s: Key without a code (ret=%d)\n", __func__, ret);
			return -EINVAL;
		}

		num++;
	}

	config->nbuttons = num;

	return 0;
}

static int gpio_keys_polled_probe(struct udevice *dev)
{
	struct gpio_keys_polled_priv *priv = dev_get_priv(dev);
	struct keyboard_priv *uc_priv = dev_get_uclass_priv(dev);
	struct stdio_dev *sdev = &uc_priv->sdev;
	struct input_config *input = &uc_priv->input;
	int ret;

	ret = gpio_keys_polled_decode_fdt(dev->node, priv);
	if (ret) {
		pr_err("%s: Cannot decode node (ret=%d)\n", __func__, ret);
		return -EINVAL;
	}
	input_set_delays(input, REPEAT_DELAY_MS, REPEAT_RATE_MS);

	priv->input = input;
	input->dev = dev;
	input_add_tables(input, false);
	input->read_keys = gpio_keys_check;
	strcpy(sdev->name, "gpio-keys");

	/* Register the device */
	return input_stdio_register(sdev);
}

static const struct keyboard_ops gpio_keys_polled_ops = {
};

static const struct udevice_id gpio_keys_polled_ids[] = {
	{ .compatible = "gpio-keys" },
	{ .compatible = "gpio-keys-polled" },
	{ }
};

U_BOOT_DRIVER(gpio_keys_polled) = {
	.name	= "gpio_keys",
	.id	= UCLASS_KEYBOARD,
	.of_match = gpio_keys_polled_ids,
	.probe = gpio_keys_polled_probe,
	.ops	= &gpio_keys_polled_ops,
	.priv_auto_alloc_size = sizeof(struct gpio_keys_polled_priv),
};
