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

static int fb_onenand_lookup(const char *partname,
			  struct mtd_info **mtd,
			  struct part_info **part)
{
	struct mtd_device *dev;
	int ret;
	u8 pnum;

	ret = mtdparts_init();
	if (ret) {
		pr_err("Cannot initialize MTD partitions\n");
		fastboot_fail("cannot init mtdparts");
		return ret;
	}

	ret = find_dev_and_part(partname, &dev, &pnum, part);
	if (ret) {
		pr_err("cannot find partition: '%s'", partname);
		fastboot_fail("cannot find partition");
		return ret;
	}

	if (dev->id->type != MTD_DEV_TYPE_ONENAND) {
		pr_err("partition '%s' is not stored on a NAND device",
		      partname);
		fastboot_fail("not a NAND device");
		return -EINVAL;
	}

	*mtd = &onenand_mtd;

	return 0;
}

static lbaint_t fb_onenand_sparse_write(struct sparse_storage *info,
		lbaint_t blk, lbaint_t blkcnt, const void *buffer)
{
	size_t written;
	int ret;

	/* OneNAND needs erasing prior to writing */
	ret = onenand_block_erase(blk * info->blksz, blkcnt * info->blksz, 0);
	if (ret < 0) {
		printf("Failed to erase prior to writing chunk\n");
		return ret;
	}

	ret = onenand_block_write(blk * info->blksz, blkcnt * info->blksz, &written, buffer, 0);
	if (ret < 0) {
		printf("Failed to write sparse chunk\n");
		return ret;
	}

/* TODO - verify that the value "written" includes the "bad-blocks" ... */

	/*
	 * the return value must be 'blkcnt' ("good-blocks") plus the
	 * number of "bad-blocks" encountered within this space...
	 */
	return written / info->blksz;
}

static lbaint_t fb_onenand_sparse_reserve(struct sparse_storage *info,
		lbaint_t blk, lbaint_t blkcnt)
{
	int bad_blocks = 0;

/*
 * TODO - implement a function to determine the total number
 * of blocks which must be used in order to reserve the specified
 * number ("blkcnt") of "good-blocks", starting at "blk"...
 * ( possibly something like the "check_skip_len()" function )
 */

	/*
	 * the return value must be 'blkcnt' ("good-blocks") plus the
	 * number of "bad-blocks" encountered within this space...
	 */
	return blkcnt + bad_blocks;
}

void fb_onenand_flash_write(const char *cmd, void *download_buffer,
			 unsigned int download_bytes)
{
	struct part_info *part;
	struct mtd_info *mtd = NULL;
	size_t retlen;
	int ret;

	ret = fb_onenand_lookup(cmd, &mtd, &part);
	if (ret) {
		pr_err("invalid NAND device");
		fastboot_fail("invalid NAND device");
		return;
	}

	if (is_sparse_image(download_buffer)) {
		struct sparse_storage sparse;

		sparse.blksz = mtd->writesize;
		sparse.start = part->offset / sparse.blksz;
		sparse.size = part->size / sparse.blksz;
		sparse.write = fb_onenand_sparse_write;
		sparse.reserve = fb_onenand_sparse_reserve;

		printf("Flashing sparse image at offset " LBAFU "\n",
		       sparse.start);

		write_sparse_image(&sparse, cmd, download_buffer,
				   download_bytes);
	} else {
		printf("Flashing raw image at offset 0x%llx\n",
		       part->offset);

		/* OneNAND needs erasing prior to writing */
		ret = onenand_block_erase(part->offset, download_bytes, 0);

		if (ret) {
			printf("error erasing prior to writing");
			fastboot_fail("error erasing prior to writing");
			return;
		}

		ret = onenand_block_write(part->offset, download_bytes, &retlen, download_buffer, 0);

		printf("........ wrote %u bytes to '%s'\n",
		       retlen, part->name);
	}

	if (ret) {
		fastboot_fail("error writing the image");
		return;
	}

	fastboot_okay("");
}

void fb_onenand_erase(const char *cmd)
{
	struct part_info *part;
	struct mtd_info *mtd = NULL;
	int ret;

	ret = fb_onenand_lookup(cmd, &mtd, &part);
	if (ret) {
		pr_err("invalid NAND device");
		fastboot_fail("invalid NAND device");
		return;
	}

	ret = onenand_block_erase(part->offset, part->size, 0);
	if (ret) {
		pr_err("failed erasing from device %s", mtd->name);
		fastboot_fail("failed erasing from device");
		return;
	}

	fastboot_okay("");
}
