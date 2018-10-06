// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation.
 * Copyright 2015 Free Electrons.
 */

#include <config.h>
#include <common.h>

#include <fastboot.h>
#include <image-sparse.h>

#include <ubi_uboot.h>

struct fb_ubi_sparse {
	const char *name;
	int full_size;
	bool started;
};

static lbaint_t fb_ubi_sparse_write(struct sparse_storage *info,
		lbaint_t blk, lbaint_t blkcnt, const void *buffer)
{
	struct fb_ubi_sparse *sparse = info->priv;
	int ret;

	if (!sparse->started) {
		/* Start flashing */
		ret = ubi_volume_begin_write(sparse->name, buffer, blkcnt * info->blksz, sparse->full_size);
		sparse->started = true;
	} else {
		ret = ubi_volume_continue_write(sparse->name, buffer, blkcnt * info->blksz);
	}

	if (ret < 0) {
		printf("Failed to write sparse chunk\n");
		return ret;
	}

	return blkcnt;
}

static lbaint_t fb_ubi_sparse_reserve(struct sparse_storage *info,
		lbaint_t blk, lbaint_t blkcnt)
{
	struct fb_ubi_sparse *sparse = info->priv;
	int size = blkcnt * info->blksz;
	void *buf;
	int ret;

	/* Write garbage to seek to the correct offset */
	buf = malloc(size);
	if (!buf) {
		return -ENOMEM;
	}

	if (!sparse->started) {
		/* Start flashing */
		ret = ubi_volume_begin_write(sparse->name, buf, size, sparse->full_size);
		sparse->started = true;
	} else {
		ret = ubi_volume_continue_write(sparse->name, buf, size);
	}

	free(buf);

	if (ret) {
		printf("Failed to write junk from sparse chunk\n");
		return ret;
	}

	return size;
}

/**
 * fastboot_ubi_flash_write() - Write image to ubi for fastboot
 *
 * @cmd: Named device to write image to
 * @download_buffer: Pointer to image data
 * @download_bytes: Size of image data
 * @response: Pointer to fastboot response buffer
 */
void fastboot_ubi_flash_write(const char *cmd, void *download_buffer,
			       u32 download_bytes, char *response)
{
	int ret;

	if (!is_ubi_initialized()) {
		fastboot_fail("UBI not initialized", response);
		return;
	}

	if (is_sparse_image(download_buffer)) {
		struct fb_ubi_sparse sparse_priv;
		struct sparse_storage sparse;
		sparse_header_t *s_header = (sparse_header_t *)download_buffer;
		struct ubi_device *ubi;
		struct ubi_volume *volume;

		sparse_priv.name = cmd;
		sparse_priv.started = false;
		sparse_priv.full_size = s_header->total_blks * s_header->blk_sz;

		ubi = get_ubi_device();
		volume = get_ubi_volume(cmd);

		sparse.blksz = ubi->min_io_size;
		sparse.start = 0;
		sparse.size = volume->reserved_pebs * ubi->peb_size / sparse.blksz;
		sparse.write = fb_ubi_sparse_write;
		sparse.reserve = fb_ubi_sparse_reserve;
		sparse.mssg = fastboot_fail;

		printf("Flashing sparse image at offset " LBAFU "\n",
		       sparse.start);

		sparse.priv = &sparse_priv;
		ret = write_sparse_image(&sparse, cmd, download_buffer,
					 response);
		if (!ret)
			fastboot_okay(NULL, response);
	} else {
		printf("Flashing raw image to %s", cmd);

		ret = ubi_volume_write(cmd, download_buffer, download_bytes);

		printf("........ wrote %u bytes to '%s'\n",
		       download_bytes, cmd);
	}

	if (ret) {
		fastboot_fail("error writing the image", response);
		return;
	}

	fastboot_okay(NULL, response);
}

/**
 * fastboot_ubi_flash_erase() - Erase ubi for fastboot
 *
 * @cmd: Named device to erase
 * @response: Pointer to fastboot response buffer
 */
void fastboot_ubi_erase(const char *cmd, char *response)
{
	int ret;

	if (!is_ubi_initialized()) {
		fastboot_fail("UBI not initialized", response);
		return;
	}

	ret = ubi_volume_write(cmd, NULL, 0);

	if (ret) {
		fastboot_fail("Failed to truncate UBI volume", response);
		return;
	}

	fastboot_okay(NULL, response);
}

bool fastboot_ubi_support_part(const char *part_name)
{
	int ret;

	if (!is_ubi_initialized())
		return false;

	ret = ubi_check(part_name);

	return ret == 0 ? true : false;
}

void fastboot_ubi_getvar_partition_size(const char *part_name, char *response)
{
	fastboot_fail("partition size not supported for UBI volumes", response);
}

U_BOOT_FASTBOOT_FLASH(ubi) = {
	.name = "ubi",
	.backend = FASTBOOT_FLASH_UBI,
	.support_part = fastboot_ubi_support_part,
	.erase = fastboot_ubi_erase,
	.flash = fastboot_ubi_flash_write,
	.getvar_partition_size = fastboot_ubi_getvar_partition_size,
};
