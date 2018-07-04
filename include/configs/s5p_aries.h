/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Kyungmin Park <kyungmin.park@samsung.com>
 *
 * Configuation settings for the Samsung Aries board (first gen Galaxy S).
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_SAMSUNG		1	/* in a SAMSUNG core */
#define CONFIG_S5P		1	/* which is in a S5P Family */
#define CONFIG_S5PC110		1	/* which is in a S5PC110 */

#include <linux/sizes.h>
#include <asm/arch/cpu.h>		/* get chip and board defs */

#define CONFIG_ARCH_CPU_INIT

/* Disable all caches - enabling they causes slowdowns of ~50% in Linux */
#define CONFIG_SYS_ICACHE_OFF
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_L2CACHE_OFF

/* input clock of PLL: has 24MHz input clock at S5PC110 */
#define CONFIG_SYS_CLK_FREQ_C110	24000000

/* DRAM Base */
#define CONFIG_SYS_SDRAM_BASE		0x30000000

/*
 * Machine type - as defined in 3.0 Android kernel
 * Stock (Froyo/Gingerbread - 2.6) used 2193 which
 * is the machine number for SMDKC110
 *
 * Note the SAMSUNG_ARIES as opposed to just ARIES
 * because ARIES is already definied
 */
#define MACH_TYPE_SAMSUNG_ARIES		9999

/* Atags */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_INITRD_TAG

/* Size of malloc() pool before and after relocation */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (80 << 20))

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL2			1	/* use SERIAL2 */

/* MMC */
#define SDHCI_MAX_HOSTS			4

/* PWM */
#define CONFIG_PWM			1

/* USB Composite download gadget - g_dnl */
#define CONFIG_SYS_DFU_DATA_BUF_SIZE SZ_32M
#define DFU_DEFAULT_POLL_TIMEOUT 300

/* USB Samsung's IDs */

#define CONFIG_G_DNL_THOR_VENDOR_NUM 0x04E8
#define CONFIG_G_DNL_THOR_PRODUCT_NUM 0x685D
#define CONFIG_G_DNL_UMS_VENDOR_NUM 0x0525
#define CONFIG_G_DNL_UMS_PRODUCT_NUM 0xA4A5

#define CONFIG_BOOTCOMMAND	"run android_3_0_boot;"

#define CONFIG_DEFAULT_CONSOLE	"ttySAC2,115200n8"

#define CONFIG_COMMON_BOOT	"${console} ${meminfo} ${mtdparts} androidboot.serialno=${serial#} androidboot.mode=${boot_mode}"

/* 3.0 Kernel sets serialno, mtdparts automatically/as needed */
#define BOOTARGS_3_0		"${console} androidboot.mode=${boot_mode}"

#define VOLUME_UP_KEY		"gph31"

#define CONFIG_MISC_COMMON
#define CONFIG_MISC_INIT_R

#define CONFIG_ENV_OVERWRITE
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"boottrace=setenv opts initcall_debug; run bootcmd\0" \
	"bootchart=set opts init=/sbin/bootchartd; run bootcmd\0" \
	"console=" CONFIG_DEFAULT_CONSOLE "\0"\
	"meminfo=mem=80M mem=256M@0x40000000 mem=128M@0x50000000\0" \
	"normal_boot=onenand read 0x32000000 0x1980000 0xA00000; bootm 0x32000000\0" \
	"recovery_boot=onenand read 0x32000000 0x2380000 0xA00000; bootm 0x32000000\0" \
	"mmc_boot=fatload mmc 0 0x32000000 uImage-dtb; fatload mmc 0 0x33000000 initrd.img; bootm 0x32000000 0x33000000\0" \
	"android_3_0_boot=set bootargs " BOOTARGS_3_0 "; run common_boot\0" \
	"upstream_boot=set bootargs " CONFIG_COMMON_BOOT "; run common_boot\0" \
	"common_boot=if gpio input " VOLUME_UP_KEY " || test ${boot_mode} = recovery; then run recovery_boot; else run normal_boot; fi\0"


#define CONFIG_SYS_PBSIZE	384	/* Print Buffer Size */
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x5000000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x4000000)

/* Aries has 3 banks of DRAM, but swap the bank */
#define CONFIG_NR_DRAM_BANKS	3
#define PHYS_SDRAM_1		CONFIG_SYS_SDRAM_BASE	/* OneDRAM Bank #0 */
#define PHYS_SDRAM_1_SIZE	(80 << 20)		/* 80 MB in Bank #0 */
#define PHYS_SDRAM_2		0x40000000		/* mDDR DMC1 Bank #1 */
#define PHYS_SDRAM_2_SIZE	(256 << 20)		/* 256 MB in Bank #1 */
#define PHYS_SDRAM_3		0x50000000		/* mDDR DMC2 Bank #2 */
#define PHYS_SDRAM_3_SIZE	(128 << 20)		/* 128 MB in Bank #2 */

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* 256 KiB */

/* FLASH and environment organization */
#define CONFIG_MMC_DEFAULT_DEV		0
#define CONFIG_SYS_MMC_ENV_DEV		CONFIG_MMC_DEFAULT_DEV
#define CONFIG_ENV_SIZE			4096
#define CONFIG_ENV_OFFSET		((32 - 4) << 10) /* 32KiB - 4KiB */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_USE_ONENAND_BOARD_INIT
#define CONFIG_SAMSUNG_ONENAND		1
#define CONFIG_SYS_ONENAND_BASE		0xB0000000
#define CONFIG_SYS_NAND_MAX_ECCPOS	128
#define CONFIG_SYS_NAND_MAX_OOBFREE	8

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_LOAD_ADDR - 0x1000000)

#define CONFIG_USB_GADGET_DWC2_OTG_PHY

#endif	/* __CONFIG_H */
