/*
 *  U-Boot utils for OneNAND support
 *
 *  Copyright (C) 2005-2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <malloc.h>

#include <linux/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#include <asm/io.h>

int onenand_block_read(struct mtd_info *mtd, loff_t from, size_t len,
		       size_t *retlen, u_char *buf, int oob)
{
	struct onenand_chip *this = mtd->priv;
	int blocks = (int) len >> this->erase_shift;
	int blocksize = (1 << this->erase_shift);
	loff_t ofs = from;
	struct mtd_oob_ops ops = {
		.retlen		= 0,
	};
	int ret;

	if (oob)
		ops.ooblen = blocksize;
	else
		ops.len = blocksize;

	while (blocks) {
		ret = mtd_block_isbad(mtd, ofs);
		if (ret) {
			printk("Bad blocks %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			ofs += blocksize;
			continue;
		}

		if (oob)
			ops.oobbuf = buf;
		else
			ops.datbuf = buf;

		ops.retlen = 0;
		ret = mtd_read_oob(mtd, ofs, &ops);
		if (ret) {
			printk("Read failed 0x%x, %d\n", (u32)ofs, ret);
			ofs += blocksize;
			continue;
		}
		ofs += blocksize;
		buf += blocksize;
		blocks--;
		*retlen += ops.retlen;
	}

	return 0;
}

int onenand_write_oneblock_withoob(struct mtd_info *mtd, loff_t to, const u_char * buf,
				   size_t *retlen)
{
	struct mtd_oob_ops ops = {
		.len = mtd->writesize,
		.ooblen = mtd->oobsize,
		.mode = MTD_OPS_AUTO_OOB,
	};
	int page, ret = 0;
	for (page = 0; page < (mtd->erasesize / mtd->writesize); page ++) {
		ops.datbuf = (u_char *)buf;
		buf += mtd->writesize;
		ops.oobbuf = (u_char *)buf;
		buf += mtd->oobsize;
		ret = mtd_write_oob(mtd, to, &ops);
		if (ret)
			break;
		to += mtd->writesize;
	}

	*retlen = (ret) ? 0 : mtd->erasesize;
	return ret;
}

int onenand_block_write(struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char * buf, int withoob)
{
	struct onenand_chip *this = mtd->priv;
	int blocks = len >> this->erase_shift;
	int blocksize = (1 << this->erase_shift);
	loff_t ofs;
	size_t _retlen = 0;
	int ret;

	if ((to & (mtd->writesize - 1)) != 0) {
		printf("Attempt to write non block-aligned data\n");
		*retlen = 0;
		return 1;
	}

	ofs = to;

	while (blocks) {
		ret = mtd_block_isbad(mtd, ofs);
		if (ret) {
			printk("Bad blocks %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			goto next;
		}

		if (!withoob)
			ret = mtd_write(mtd, ofs, blocksize, &_retlen, buf);
		else
			ret = onenand_write_oneblock_withoob(mtd, ofs, buf, &_retlen);
		if (ret) {
			printk("Write failed 0x%x, %d", (u32)ofs, ret);
			goto next;
		}

		buf += blocksize;
		blocks--;
		*retlen += _retlen;
next:
		ofs += blocksize;
	}

	return 0;
}

int onenand_block_erase(struct mtd_info *mtd, u32 start, u32 size, int force)
{
	struct onenand_chip *this = mtd->priv;
	struct erase_info instr = {
		.callback	= NULL,
	};
	loff_t ofs;
	int ret;
	int blocksize = 1 << this->erase_shift;

	for (ofs = start; ofs < (start + size); ofs += blocksize) {
		ret = mtd_block_isbad(mtd, ofs);
		if (ret && !force) {
			printf("Skip erase bad block %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			continue;
		}

		instr.addr = ofs;
		instr.len = blocksize;
		instr.priv = force;
		instr.mtd = mtd;
		ret = mtd_erase(mtd, &instr);
		if (ret) {
			printf("erase failed block %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			continue;
		}
	}

	return 0;
}

int onenand_block_test(struct mtd_info *mtd, u32 start, u32 size)
{
	struct onenand_chip *this = mtd->priv;
	struct erase_info instr = {
		.callback	= NULL,
		.priv		= 0,
	};

	int blocks;
	loff_t ofs;
	int blocksize = 1 << this->erase_shift;
	int start_block, end_block;
	size_t retlen;
	u_char *buf;
	u_char *verify_buf;
	int ret;

	buf = malloc(blocksize);
	if (!buf) {
		printf("Not enough malloc space available!\n");
		return -1;
	}

	verify_buf = malloc(blocksize);
	if (!verify_buf) {
		printf("Not enough malloc space available!\n");
		return -1;
	}

	start_block = start >> this->erase_shift;
	end_block = (start + size) >> this->erase_shift;

	/* Protect boot-loader from badblock testing */
	if (start_block < 2)
		start_block = 2;

	if (end_block > (mtd->size >> this->erase_shift))
		end_block = mtd->size >> this->erase_shift;

	blocks = start_block;
	ofs = start;
	while (blocks < end_block) {
		printf("\rTesting block %d at 0x%x", (u32)(ofs >> this->erase_shift), (u32)ofs);

		ret = mtd_block_isbad(mtd, ofs);
		if (ret) {
			printf("Skip erase bad block %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			goto next;
		}

		instr.addr = ofs;
		instr.len = blocksize;
		ret = mtd_erase(mtd, &instr);
		if (ret) {
			printk("Erase failed 0x%x, %d\n", (u32)ofs, ret);
			goto next;
		}

		ret = mtd_write(mtd, ofs, blocksize, &retlen, buf);
		if (ret) {
			printk("Write failed 0x%x, %d\n", (u32)ofs, ret);
			goto next;
		}

		ret = mtd_read(mtd, ofs, blocksize, &retlen, verify_buf);
		if (ret) {
			printk("Read failed 0x%x, %d\n", (u32)ofs, ret);
			goto next;
		}

		if (memcmp(buf, verify_buf, blocksize))
			printk("\nRead/Write test failed at 0x%x\n", (u32)ofs);

next:
		ofs += blocksize;
		blocks++;
	}
	printf("...Done\n");

	free(buf);
	free(verify_buf);

	return 0;
}

int onenand_checkbad(struct mtd_info *mtd, u32 start, u32 size)
{
	struct onenand_chip *this = mtd->priv;

	int blocks;
	loff_t ofs;
	int blocksize = 1 << this->erase_shift;
	int start_block, end_block;
	int ret;

	start_block = start >> this->erase_shift;
	end_block = (start + size) >> this->erase_shift;

	/* Protect boot-loader from badblock testing */
	if (start_block < 2)
		start_block = 2;

	if (end_block > (mtd->size >> this->erase_shift))
		end_block = mtd->size >> this->erase_shift;

	blocks = start_block;
	ofs = start;
	while (blocks < end_block) {
		printf("Testing block %d at 0x%x\n", (u32)(ofs >> this->erase_shift), (u32)ofs);

		ret = mtd_block_isbad(mtd, ofs);
		if (ret) {
			printf("Bad block %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			return 1;
		}

		ofs += blocksize;
		blocks++;
	}

	printf("No bad blocks\n");

	return 0;
}

int onenand_dump(struct mtd_info *mtd, ulong off, int only_oob)
{
	int i;
	u_char *datbuf, *oobbuf, *p;
	struct mtd_oob_ops ops;
	loff_t addr;

	datbuf = malloc(mtd->writesize + mtd->oobsize);
	oobbuf = malloc(mtd->oobsize);
	if (!datbuf || !oobbuf) {
		puts("No memory for page buffer\n");
		return 1;
	}
	off &= ~(mtd->writesize - 1);
	addr = (loff_t) off;
	memset(&ops, 0, sizeof(ops));
	ops.datbuf = datbuf;
	ops.oobbuf = oobbuf;
	ops.len = mtd->writesize;
	ops.ooblen = mtd->oobsize;
	ops.retlen = 0;
	i = mtd_read_oob(mtd, addr, &ops);
	if (i < 0) {
		printf("Error (%d) reading page %08lx\n", i, off);
		free(datbuf);
		free(oobbuf);
		return 1;
	}
	printf("Page %08lx dump:\n", off);
	i = mtd->writesize >> 4;
	p = datbuf;

	while (i--) {
		if (!only_oob)
			printf("\t%02x %02x %02x %02x %02x %02x %02x %02x"
			       "  %02x %02x %02x %02x %02x %02x %02x %02x\n",
			       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			       p[8], p[9], p[10], p[11], p[12], p[13], p[14],
			       p[15]);
		p += 16;
	}
	puts("OOB:\n");
	i = mtd->oobsize >> 3;
	p = oobbuf;

	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
		       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}
	free(datbuf);
	free(oobbuf);

	return 0;
}
