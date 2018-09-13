// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2008-2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <linux/dma-direction.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>
#include <linux/mtd/samsung_onenand.h>
#include <onenand_uboot.h>

#define S5PC110_DMA_SRC_ADDR		0x400
#define S5PC110_DMA_SRC_CFG		0x404
#define S5PC110_DMA_DST_ADDR		0x408
#define S5PC110_DMA_DST_CFG		0x40C
#define S5PC110_DMA_TRANS_SIZE		0x414
#define S5PC110_DMA_TRANS_CMD		0x418
#define S5PC110_DMA_TRANS_STATUS	0x41C
#define S5PC110_DMA_TRANS_DIR		0x420
#define S5PC110_INTC_DMA_CLR		0x1004
#define S5PC110_INTC_ONENAND_CLR	0x1008
#define S5PC110_INTC_DMA_MASK		0x1024
#define S5PC110_INTC_ONENAND_MASK	0x1028
#define S5PC110_INTC_DMA_PEND		0x1044
#define S5PC110_INTC_ONENAND_PEND	0x1048
#define S5PC110_INTC_DMA_STATUS		0x1064
#define S5PC110_INTC_ONENAND_STATUS	0x1068

#define S5PC110_INTC_DMA_TD		(1 << 24)
#define S5PC110_INTC_DMA_TE		(1 << 16)

#define S5PC110_DMA_CFG_SINGLE		(0x0 << 16)
#define S5PC110_DMA_CFG_4BURST		(0x2 << 16)
#define S5PC110_DMA_CFG_8BURST		(0x3 << 16)
#define S5PC110_DMA_CFG_16BURST		(0x4 << 16)

#define S5PC110_DMA_CFG_INC		(0x0 << 8)
#define S5PC110_DMA_CFG_CNT		(0x1 << 8)

#define S5PC110_DMA_CFG_8BIT		(0x0 << 0)
#define S5PC110_DMA_CFG_16BIT		(0x1 << 0)
#define S5PC110_DMA_CFG_32BIT		(0x2 << 0)

#define S5PC110_DMA_SRC_CFG_READ	(S5PC110_DMA_CFG_16BURST | \
					S5PC110_DMA_CFG_INC | \
					S5PC110_DMA_CFG_16BIT)
#define S5PC110_DMA_DST_CFG_READ	(S5PC110_DMA_CFG_16BURST | \
					S5PC110_DMA_CFG_INC | \
					S5PC110_DMA_CFG_32BIT)
#define S5PC110_DMA_SRC_CFG_WRITE	(S5PC110_DMA_CFG_16BURST | \
					S5PC110_DMA_CFG_INC | \
					S5PC110_DMA_CFG_32BIT)
#define S5PC110_DMA_DST_CFG_WRITE	(S5PC110_DMA_CFG_16BURST | \
					S5PC110_DMA_CFG_INC | \
					S5PC110_DMA_CFG_16BIT)

#define S5PC110_DMA_TRANS_CMD_TDC	(0x1 << 18)
#define S5PC110_DMA_TRANS_CMD_TEC	(0x1 << 16)
#define S5PC110_DMA_TRANS_CMD_TR	(0x1 << 0)

#define S5PC110_DMA_TRANS_STATUS_TD	(0x1 << 18)
#define S5PC110_DMA_TRANS_STATUS_TB	(0x1 << 17)
#define S5PC110_DMA_TRANS_STATUS_TE	(0x1 << 16)

#define S5PC110_DMA_DIR_READ		0x0
#define S5PC110_DMA_DIR_WRITE		0x1

static dma_addr_t dma_map_single(void *ptr, size_t size,
				 enum dma_data_direction dir)
{
	unsigned long addr = (unsigned long)ptr;

	if (dir == DMA_FROM_DEVICE)
		invalidate_dcache_range(addr, addr + size);
	else
		flush_dcache_range(addr, addr + size);

	return addr;
}

static void dma_unmap_single(dma_addr_t addr, size_t size,
			     enum dma_data_direction dir)
{
	if (dir != DMA_TO_DEVICE)
		invalidate_dcache_range(addr, addr + size);
}

static int dma_mapping_error(dma_addr_t addr)
{
	return 0;
}

static void __iomem *dma_addr = (void *) 0xB0600000;

static int s5pc110_dma_poll(void *dst, void *src, size_t count, int direction)
{
	void __iomem *base = dma_addr;
	int status;
	unsigned long timeout, time_start;

	writel((unsigned long) src, base + S5PC110_DMA_SRC_ADDR);
	writel((unsigned long) dst, base + S5PC110_DMA_DST_ADDR);

	if (direction == S5PC110_DMA_DIR_READ) {
		writel(S5PC110_DMA_SRC_CFG_READ, base + S5PC110_DMA_SRC_CFG);
		writel(S5PC110_DMA_DST_CFG_READ, base + S5PC110_DMA_DST_CFG);
	} else {
		writel(S5PC110_DMA_SRC_CFG_WRITE, base + S5PC110_DMA_SRC_CFG);
		writel(S5PC110_DMA_DST_CFG_WRITE, base + S5PC110_DMA_DST_CFG);
	}

	writel(count, base + S5PC110_DMA_TRANS_SIZE);
	writel(direction, base + S5PC110_DMA_TRANS_DIR);

	writel(S5PC110_DMA_TRANS_CMD_TR, base + S5PC110_DMA_TRANS_CMD);

	/*
	 * There's no exact timeout values at Spec.
	 * In real case it takes under 1 msec.
	 * So 20 msecs are enough.
	 */
#if 0
	timeout = jiffies + msecs_to_jiffies(20);

	do {
		status = readl(base + S5PC110_DMA_TRANS_STATUS);
		if (status & S5PC110_DMA_TRANS_STATUS_TE) {
			writel(S5PC110_DMA_TRANS_CMD_TEC,
					base + S5PC110_DMA_TRANS_CMD);
			return -EIO;
		}
	} while (!(status & S5PC110_DMA_TRANS_STATUS_TD) &&
		time_before(jiffies, timeout));
#else
	timeout = (CONFIG_SYS_HZ * 20) / 1000;
	time_start = get_timer(0);
	do {
		status = readl(base + S5PC110_DMA_TRANS_STATUS);
		if (status & S5PC110_DMA_TRANS_STATUS_TE) {
			writel(S5PC110_DMA_TRANS_CMD_TEC,
					base + S5PC110_DMA_TRANS_CMD);
			return -EIO;
		}
	} while (!(status & S5PC110_DMA_TRANS_STATUS_TD) &&
		get_timer(time_start) <= timeout);
#endif

	writel(S5PC110_DMA_TRANS_CMD_TDC, base + S5PC110_DMA_TRANS_CMD);

	return 0;
}


static int s5pc110_read_bufferram(struct mtd_info *mtd, loff_t addr, int area,
				  unsigned char *buffer, int offset,
				  size_t count)
{
	struct onenand_chip *this = mtd->priv;
	void __iomem *p;
	void *buf = (void *) buffer;
	dma_addr_t dma_src, dma_dst;
	int err;

	p = this->base + area;
	if (ONENAND_CURRENT_BUFFERRAM(this)) {
		if (area == ONENAND_DATARAM)
			p += this->writesize;
		else
			p += mtd->oobsize;
	}

	if (offset & 3 || (size_t) buf & 3 || count != mtd->writesize)
		goto normal;

	/* DMA routine */
	dma_src = (dma_addr_t) p;
	dma_dst = dma_map_single(buf, count, DMA_FROM_DEVICE);

	if (dma_mapping_error(dma_dst)) {
		pr_err("Couldn't map a %d byte buffer for DMA\n", count);
		goto normal;
	}
	err = s5pc110_dma_poll((void *) dma_dst, (void *) dma_src,
			count, S5PC110_DMA_DIR_READ);

	dma_unmap_single(dma_dst, count, DMA_FROM_DEVICE);

	if (!err)
		return 0;

normal:
	if (count != mtd->writesize) {
		/* Copy the bufferram to memory to prevent unaligned access */
		memcpy(this->page_buf, p, mtd->writesize);
		p = this->page_buf + offset;
	}

	memcpy(buffer, p, count);

	return 0;
}

int aries_onenand_chip_probe(struct mtd_info *mtd)
{
	return 0;
}

int onenand_board_init(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;

	this->base = (void *)CONFIG_SYS_ONENAND_BASE;
#if 0
	this->options |= ONENAND_SKIP_UNLOCK_CHECK;
#else
	this->options |= ONENAND_RUNTIME_BADBLOCK_CHECK;
#endif
	this->read_bufferram = s5pc110_read_bufferram;
	this->chip_probe = aries_onenand_chip_probe;

	return 0;
}
