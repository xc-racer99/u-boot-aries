// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation.
 * Copyright 2015 Free Electrons.
 */

#include <config.h>
#include <common.h>

#include <fastboot.h>
#include <image-sparse.h>

#include <linux/mtd/mtd.h>
#include <jffs2/jffs2.h>
#include <onenand_uboot.h>

struct fb_onenand_sparse {
	struct mtd_info		*mtd;
	struct part_info	*part;
};

static int fb_onenand_lookup(const char *partname,
			     struct mtd_info **mtd,
			     struct part_info **part,
			     char *response)
{
	struct mtd_device *dev;
	int ret;
	u8 pnum;

	ret = mtdparts_init();
	if (ret) {
		pr_err("Cannot initialize MTD partitions\n");
		fastboot_fail("cannot init mtdparts", response);
		return ret;
	}

	ret = find_dev_and_part(partname, &dev, &pnum, part);
	if (ret) {
		pr_err("cannot find partition: '%s'", partname);
		fastboot_fail("cannot find partition", response);
		return ret;
	}

	if (dev->id->type != MTD_DEV_TYPE_ONENAND) {
		pr_err("partition '%s' is not stored on a OneNAND device",
		      partname);
		fastboot_fail("not a OneNAND device", response);
		return -EINVAL;
	}

	/* Like the onenand cmd, only one OneNAND device is supported */
	*mtd = &onenand_mtd;

	return 0;
}

static lbaint_t fb_onenand_sparse_write(struct sparse_storage *info,
		lbaint_t blk, lbaint_t blkcnt, const void *buffer)
{
	struct fb_onenand_sparse *sparse = info->priv;
	size_t written;
	int ret;

	/* OneNAND needs erasing prior to writing */
	ret = onenand_block_erase(sparse->mtd, blk * info->blksz, blkcnt * info->blksz, 0);
	if (ret < 0) {
		printf("Failed to erase prior to writing spare chunk\n");
		return ret;
	}

	ret = onenand_block_write(sparse->mtd, blk * info->blksz, blkcnt * info->blksz, &written, (void *)buffer, 0);
	if (ret < 0) {
		printf("Failed to write sparse chunk\n");
		return ret;
	}

	/* the return value must be 'blkcnt' ("good-blocks") written */
	return written == blkcnt * info->blksz ? 0 : -1;
}

static lbaint_t fb_onenand_sparse_reserve(struct sparse_storage *info,
		lbaint_t blk, lbaint_t blkcnt)
{
	struct fb_onenand_sparse *sparse = info->priv;
	int bad_blocks;

	bad_blocks = onenand_checkbad(sparse->mtd, blk * info->blksz, blkcnt * info->blksz);

	/*
	 * the return value must be 'blkcnt' ("good-blocks") plus the
	 * number of "bad-blocks" encountered within this space.
	 */
	return blkcnt + bad_blocks;
}

/**
 * fastboot_onenand_get_part_info() - Lookup OneNAND partion by name
 *
 * @part_name: Named device to lookup
 * @part_info: Pointer to returned part_info pointer
 * @response: Pointer to fastboot response buffer
 */
int fastboot_onenand_get_part_info(char *part_name, struct part_info **part_info,
				char *response)
{
	struct mtd_info *mtd = NULL;

	return fb_onenand_lookup(part_name, &mtd, part_info, response);
}

/**
 * fastboot_onenand_flash_write() - Write image to OneNAND for fastboot
 *
 * @cmd: Named device to write image to
 * @download_buffer: Pointer to image data
 * @download_bytes: Size of image data
 * @response: Pointer to fastboot response buffer
 */
void fastboot_onenand_flash_write(const char *cmd, void *download_buffer,
			       u32 download_bytes, char *response)
{
	struct part_info *part;
	struct mtd_info *mtd = NULL;
	size_t retlen;
	int ret;

	ret = fb_onenand_lookup(cmd, &mtd, &part, response);
	if (ret) {
		pr_err("invalid OneNAND device");
		fastboot_fail("invalid OneNAND device", response);
		return;
	}

	if (is_sparse_image(download_buffer)) {
		struct fb_onenand_sparse sparse_priv;
		struct sparse_storage sparse;

		sparse_priv.mtd = mtd;
		sparse_priv.part = part;

		sparse.blksz = mtd->writesize;
		sparse.start = part->offset / sparse.blksz;
		sparse.size = part->size / sparse.blksz;
		sparse.write = fb_onenand_sparse_write;
		sparse.reserve = fb_onenand_sparse_reserve;
		sparse.mssg = fastboot_fail;

		printf("Flashing sparse image at offset " LBAFU "\n",
		       sparse.start);

		sparse.priv = &sparse_priv;
		ret = write_sparse_image(&sparse, cmd, download_buffer,
					 response);
		if (!ret)
			fastboot_okay(NULL, response);
	} else {
		printf("Flashing raw image at offset 0x%llx\n",
		       part->offset);

		/* OneNAND needs erasing prior to writing */
		ret = onenand_block_erase(mtd, part->offset, part->size, 0);
		if (ret) {
			fastboot_fail("error erasing prior to writing image", response);
			return;
		}

		ret = onenand_block_write(mtd, part->offset, download_bytes, &retlen, download_buffer, 0);

		printf("........ wrote %u bytes to '%s'\n",
		       retlen, part->name);
	}

	if (ret) {
		fastboot_fail("error writing the image", response);
		return;
	}

	fastboot_okay(NULL, response);
}

/**
 * fastboot_onenand_flash_erase() - Erase OneNAND for fastboot
 *
 * @cmd: Named device to erase
 * @response: Pointer to fastboot response buffer
 */
void fastboot_onenand_erase(const char *cmd, char *response)
{
	struct part_info *part;
	struct mtd_info *mtd = NULL;
	int ret;

	ret = fb_onenand_lookup(cmd, &mtd, &part, response);
	if (ret) {
		pr_err("invalid OneNAND device");
		fastboot_fail("invalid OneNAND device", response);
		return;
	}

	ret = onenand_block_erase(mtd, part->offset, part->size, 0);
	if (ret) {
		pr_err("failed erasing from device %s", mtd->name);
		fastboot_fail("failed erasing from device", response);
		return;
	}

	fastboot_okay(NULL, response);
}
