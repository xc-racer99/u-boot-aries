/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 */

#ifndef _S5PC1XX_CPU_H
#define _S5PC1XX_CPU_H

#define S5P_CPU_NAME		"S5P"
#define S5PC1XX_ADDR_BASE	0xE0000000

/* S5PC100 */
#define S5PC100_PRO_ID		0xE0000000
#define S5PC100_CLOCK_BASE	0xE0100000
#define S5PC100_GPIO_BASE	0xE0300000
#define S5PC100_VIC0_BASE	0xE4000000
#define S5PC100_VIC1_BASE	0xE4100000
#define S5PC100_VIC2_BASE	0xE4200000
#define S5PC100_DMC_BASE	0xE6000000
#define S5PC100_SROMC_BASE	0xE7000000
#define S5PC100_ONENAND_BASE	0xE7100000
#define S5PC100_PWMTIMER_BASE	0xEA000000
#define S5PC100_WATCHDOG_BASE	0xEA200000
#define S5PC100_UART_BASE	0xEC000000
#define S5PC100_MMC_BASE	0xED800000

#ifndef __ASSEMBLY__
#include <asm/io.h>
/* CPU detection macros */
extern unsigned int s5p_cpu_id;
extern unsigned int s5p_cpu_rev;

static inline int s5p_get_cpu_rev(void)
{
	return s5p_cpu_rev;
}

static inline void s5p_set_cpu_id(void)
{
	s5p_cpu_id = readl(S5PC100_PRO_ID);
	s5p_cpu_rev = s5p_cpu_id & 0x000000FF;
	s5p_cpu_id = 0xC000 | ((s5p_cpu_id & 0x00FFF000) >> 12);
}

static inline char *s5p_get_cpu_name(void)
{
	return S5P_CPU_NAME;
}

#define IS_SAMSUNG_TYPE(type, id)			\
static inline int cpu_is_##type(void)			\
{							\
	return s5p_cpu_id == id ? 1 : 0;		\
}

IS_SAMSUNG_TYPE(s5pc100, 0xc100)

#define SAMSUNG_BASE(device, base)				\
static inline unsigned int samsung_get_base_##device(void)	\
{								\
	if (cpu_is_s5pc100())					\
		return S5PC100_##base;				\
	else if (cpu_is_s5pc110())				\
		return S5PC110_##base;				\
	else							\
		return 0;					\
}

SAMSUNG_BASE(clock, CLOCK_BASE)
SAMSUNG_BASE(gpio, GPIO_BASE)
SAMSUNG_BASE(pro_id, PRO_ID)
SAMSUNG_BASE(mmc, MMC_BASE)
SAMSUNG_BASE(sromc, SROMC_BASE)
SAMSUNG_BASE(timer, PWMTIMER_BASE)
SAMSUNG_BASE(uart, UART_BASE)
SAMSUNG_BASE(watchdog, WATCHDOG_BASE)
#endif

#endif	/* _S5PC1XX_CPU_H */
