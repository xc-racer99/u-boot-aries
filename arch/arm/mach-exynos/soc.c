// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2010 Samsung Electronics.
 * Minkyu Kang <mk7.kang@samsung.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>

#ifdef CONFIG_TARGET_ESPRESSO7420
/*
 * Exynos7420 uses CPU0 of Cluster-1 as boot CPU. Due to this, branch_if_master
 * fails to identify as the boot CPU as the master CPU. As temporary workaround,
 * setup the slave CPU boot address as "_main".
 */
extern void _main(void);
void *secondary_boot_addr = (void *)_main;
#endif /* CONFIG_TARGET_ESPRESSO7420 */

void reset_cpu(ulong addr)
{
#ifdef CONFIG_CPU_V7A
	writel(0x1, samsung_get_base_swreset());
#endif
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#ifndef CONFIG_SYS_L2CACHE_OFF
void v7_outer_cache_enable_exynos3110(void)
{
	__asm(
		"push    {r0, r1, r2, lr}\n\t"
		"mrc     15, 0, r3, cr1, cr0, 1\n\t"
		"orr     r3, r3, #2\n\t"
		"mcr     15, 0, r3, cr1, cr0, 1\n\t"
		"pop     {r1, r2, r3, pc}"
	);
}

void v7_outer_cache_disable_exynos3110(void)
{
	__asm(
		"push    {r0, r1, r2, lr}\n\t"
		"mrc     15, 0, r3, cr1, cr0, 1\n\t"
		"bic     r3, r3, #2\n\t"
		"mcr     15, 0, r3, cr1, cr0, 1\n\t"
		"pop     {r1, r2, r3, pc}"
	);
}

void v7_outer_cache_enable_exynos3110(void)
{
	if (proid_is_exynos3110())
		v7_outer_cache_enable_exynos3110();
}

void v7_outer_cache_disable_exynos3110(void)
{
	if (proid_is_exynos3110())
		v7_outer_cache_disable_exynos3110();
}

#endif
