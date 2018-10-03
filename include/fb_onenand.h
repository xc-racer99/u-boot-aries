/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Broadcom Corporation.
 * Copyright 2015 Free Electrons.
 */

void fb_onenand_flash_write(const char *cmd, void *download_buffer,
			 unsigned int download_bytes);
void fb_onenand_erase(const char *cmd);
