// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/tzpc.h>

/* Setting TZPC[TrustZone Protection Controller] */
void tzpc_init(void)
{
	struct exynos_tzpc *tzpc[4];
	int i;

	tzpc[0] = (struct exynos_tzpc *) samsung_get_base_tzpc0();
	tzpc[1] = (struct exynos_tzpc *) samsung_get_base_tzpc1();
	tzpc[2] = (struct exynos_tzpc *) samsung_get_base_tzpc2();
	tzpc[3] = (struct exynos_tzpc *) samsung_get_base_tzpc3();

	/* No secure RAM */
	writel(0x0, &tzpc[0]->r0size);

	/* All peripherals unsecure */
	for (i = 0; i < ARRAY_SIZE(tzpc); i++) {
		writel(DECPROTXSET, &tzpc[i]->decprot0set);
		writel(DECPROTXSET, &tzpc[i]->decprot1set);
		writel(DECPROTXSET, &tzpc[i]->decprot2set);
		writel(DECPROTXSET, &tzpc[i]->decprot3set);
	}
}
