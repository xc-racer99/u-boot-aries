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
#define CONFIG_SKIP_LOWLEVEL_INIT

/* Disable L2 cache - enabling causes slowdowns of ~50% in Linux */
#define CONFIG_SYS_L2CACHE_OFF

/* input clock of PLL: has 24MHz input clock at S5PC110 */
#define CONFIG_SYS_CLK_FREQ_C110	24000000

/* DRAM Base */
#define CONFIG_SYS_SDRAM_BASE		0x30000000

#define CONFIG_SPL_TEXT_BASE		0xD0021000
#define CONFIG_SPL_MAX_FOOTPRINT	0x2000
#define CONFIG_SPL_STACK		0xD0036000

/* Max u-boot.bin size - 3 256K OneNAND pages */
#define CONFIG_BOARD_SIZE_LIMIT		0xC0000

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

#define CONFIG_DEFAULT_CONSOLE	"ttySAC2,115200n8"

#define CONFIG_MISC_COMMON

#define CONFIG_ENV_OVERWRITE
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"boottrace=setenv opts initcall_debug; run bootcmd\0" \
	"bootchart=set opts init=/sbin/bootchartd; run bootcmd\0" \
	"console=" CONFIG_DEFAULT_CONSOLE "\0"\
	"kernel_load_addr=0x32000000\0" \
	"fdt_load_addr=0x33000000\0" \
	"uboot_load_addr=0x33000000\0" \
	"uboot_onenand_off="__stringify(CONFIG_SYS_ONENAND_U_BOOT_OFFS)"\0" \
	"uboot_onenand_size="__stringify(CONFIG_BOARD_SIZE_LIMIT)"\0" \
	"meminfo=mem=80M mem=256M@0x40000000 mem=128M@0x50000000\0" \
	"stdin=serial,gpio-keys,fsa9480\0" \
	"stdout=serial,vidconsole\0" \
	"stderr=serial,vidconsole\0" \
	"opts=ignore_loglevel earlyprintk\0" \
	"check_dtb=" \
		"if run loaddtb; then " \
			"setenv fdt_addr ${fdt_load_addr};" \
		"else " \
			"setenv fdt_addr;" \
		"fi;\0" \
	"loadkernel=" \
		"load mmc ${mmcdev}:${mmcpart} ${kernel_load_addr} uImage "\
		"|| load mmc ${mmcdev}:${mmcpart} ${kernel_load_addr} /boot/uImage;\0" \
	"loaddtb=" \
		"load mmc ${mmcdev}:${mmcpart} ${fdt_load_addr} ${fdtfile} "\
		"|| load mmc ${mmcdev}:${mmcpart} ${fdt_load_addr} /boot/${fdtfile};\0" \
	"setup_kernel_args=" \
		"setenv bootargs root=/dev/mmcblk${rootdev}p${mmcpart}" \
		" rootwait rw ${console} ${meminfo} ${opts} ${mtdparts} ubi.mtd=ubi no_console_suspend;\0" \
	"sddev=1\0" \
	"rootdev=1\0" \
	"uboot_update=if test -e mmc 0:1 u-boot.bin; then "\
		"load mmc 0:1 ${uboot_load_addr} u-boot.bin; "\
		"if onenand checkbad ${uboot_onenand_off} ${uboot_onenand_size}; "\
			"then onenand erase ${uboot_onenand_off} ${uboot_onenand_size}; "\
			"onenand write ${uboot_load_addr} ${uboot_onenand_off} ${uboot_onenand_size}; echo Success!; " \
			"else echo OneNAND contains bad blocks, must use Odin/Heimdall; " \
		"fi;" \
		"else echo u-boot.bin not present, not updating; fi;\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0" \
	"mmcboot="\
		"run check_dtb;" \
		"run loadkernel;" \
		"run setup_kernel_args;" \
		"bootm ${kernel_load_addr} - ${fdt_addr}\0"\
	"onenand_boot=" \
		"if test ${default_boot_mode} != onenand ; then " \
			"setenv default_boot_mode onenand;" \
			"saveenv;" \
		"fi;" \
		"setenv bootargs ${mtdparts} ubi.mtd=ubi androidboot.mode=${boot_mode} androidboot.serialno=${serial#};" \
		"onenand read ${kernel_load_addr} ${onenand_load_offset} 0xA00000;" \
		"bootm ${kernel_load_addr};\0" \
	"bootmenu_0=Erase OneNAND IBL+PBL=onenand erase 0x0 0x40000; sleep 5; bootd;\0" \
	"bootmenu_1=Update U-Boot from SD Card Partition 1=run uboot_update; sleep 5; bootd;\0" \
	"bootmenu_2=OneNAND Main Boot=setenv boot_mode normal; setenv onenand_load_offset 0x1980000; run onenand_boot;\0" \
	"bootmenu_3=OneNAND Recovery Boot=setenv boot_mode recovery; setenv onenand_load_offset 0x2380000; run onenand_boot;\0" \
	"bootmenu_4=SD Card Partition 1 Boot=setenv mmcdev 0; setenv mmcpart 1; setenv rootdev ${sddev};" \
			"setenv default_boot_mode mmc; saveenv; run mmcboot;\0" \
	"bootmenu_5=SD Card Partition 2 Boot=setenv mmcdev 0; setenv mmcpart 2; setenv rootdev ${sddev};" \
			"setenv default_boot_mode mmc; saveenv; run mmcboot;\0" \
	"bootmenu_6=SD Card - Mass Storage=ums 0 mmc 0;\0" \
	"bootmenu_7=MMC Partition 1 Boot=setenv mmcdev 1; setenv mmcpart 1; setenv rootdev 0;" \
			"setenv default_boot_mode mmc; saveenv; run mmcboot;\0" \
	"bootmenu_8=MMC Partition 2 Boot=setenv mmcdev 1; setenv mmcpart 2; setenv rootdev 0;" \
			"setenv default_boot_mode mmc; saveenv; run mmcboot;\0" \
	"bootmenu_9=MMC - Mass Storage=ums 0 mmc 1;\0"

#define MMC_BOOTMENU1	"bootmenu_7"
#define MMC_BOOTMENU2	"bootmenu_8"
#define MMC_BOOTMENU3	"bootmenu_9"

#define CONFIG_SYS_PBSIZE	384	/* Print Buffer Size */
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x5000000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x4000000)

/* Aries has 3 banks of DRAM, but swap the bank */
#define PHYS_SDRAM_1		CONFIG_SYS_SDRAM_BASE	/* OneDRAM Bank #0 */
#define PHYS_SDRAM_1_SIZE	(80 << 20)		/* 80 MB in Bank #0 */
#define PHYS_SDRAM_2		0x40000000		/* mDDR DMC1 Bank #1 */
#define PHYS_SDRAM_2_SIZE	(256 << 20)		/* 256 MB in Bank #1 */
#define PHYS_SDRAM_3		0x50000000		/* mDDR DMC2 Bank #2 */
#define PHYS_SDRAM_3_SIZE	(128 << 20)		/* 128 MB in Bank #2 */

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* 256 KiB */

/* Environment organization */
#define CONFIG_ENV_SIZE			(256 << 10) /* 256k */
#define CONFIG_ENV_ADDR			(25856 << 10) /* 25856k */

#define CONFIG_USE_ONENAND_BOARD_INIT
#define CONFIG_SYS_ONENAND_BASE		0xB0000000
#define CONFIG_SYS_NAND_MAX_ECCPOS	128
#define CONFIG_SYS_NAND_MAX_OOBFREE	8

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_LOAD_ADDR - 0x1000000)

#define CONFIG_USB_GADGET_DWC2_OTG_PHY

#define CONFIG_EXYNOS_FB

#endif	/* __CONFIG_H */
