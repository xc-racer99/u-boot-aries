#include <common.h>
#include <asm-generic/gpio.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dmc.h>
#include <asm/arch/gpio.h>

#include <linux/mtd/onenand_regs.h>

#include "common_setup.h"

/**
 * Determines the memory configuration
 * of the board
 * Defaults to MCP D (2Gib + 2 Gib)
 * if not overriden
 */
__weak struct dmc_memconfigs s5pc110_get_dmc_memconfigs(void)
{
	struct dmc_memconfigs cfgs;

	/*
	 * DMC0: CS0 : S5PC110
	 * 0x30 -> 0x30000000
	 * 0xf8 -> 0x37FFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits
	 * [ 7:4 ] 2: 14 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	cfgs.dmc0memconfig0 = 0x30F82222;

	/* Dummy */
	cfgs.dmc0memconfig1 = 0x40F02222;

	/*
	 * DMC1: CS0 : S5PC110
	 * 0x40 -> 0x40000000
	 * 0xf0 -> 0x4FFFFFFF (2Gib)
	 * [15:12] 0: Linear
	 * [11:8 ] 3: 10 bits - Col (2Gib)
	 * [ 7:4 ] 2: 14 bits - Row
	 * [ 3:0 ] 2: 4 banks
	 */
	cfgs.dmc1memconfig0 = 0x40f01322;

	/*
	 * DMC1: CS1 : S5PC110
	 * 0x50 -> 0x40000000
	 * 0xf8 -> 0x47FFFFFF (1Gib)
	 * [15:12] 1: Interleaved
	 * [11:8 ] 3: 10 bits  - Col (1Gib)
	 * [ 7:4 ] 1: 13 bits - Row
	 * [ 3:0 ] 2: 4 banks
	 */
	cfgs.dmc1memconfig1 = 0x50f81312;

	return cfgs;
}

static void s5pc110_mem_ctrl_init(int reset)
{
	struct s5pc110_dmc *dmcs[2];
	struct dmc_memconfigs dmc_configs;
	unsigned int i;

	dmcs[0] = (struct s5pc110_dmc *) samsung_get_base_dmc_ctrl();
	dmcs[1] = (struct s5pc110_dmc *)(samsung_get_base_dmc_ctrl()
		+ S5PC110_DMC_OFFSET);

	for (i = 0; i < ARRAY_SIZE(dmcs); i++) {
		/* DLL parameter setting */
		writel(0x50101000, &dmcs[i]->phycontrol0);
		writel(0x000000f4, &dmcs[i]->phycontrol1);

		/* DLL on */
		writel(0x50101002, &dmcs[i]->phycontrol0);

		/* DLL start */
		writel(0x50101003, &dmcs[i]->phycontrol0);

		sdelay(0x4000);

		/* Force value locking for DLL off */
		writel(0x50101003, &dmcs[i]->phycontrol0);

		/* DLL off */
		writel(0x50101009, &dmcs[i]->phycontrol0);

		/* Auto refresh off */
		writel(0xff001010 | (1 << 7), &dmcs[i]->concontrol);

		/*
		 * Burst Length 4, 2 chips, 32-bit, LPDDR
		 * OFF: dynamic self refresh, force precharge, dynamic power down off
		 */
		writel(0x00212100, &dmcs[i]->memcontrol);
	}

	dmc_configs = s5pc110_get_dmc_memconfigs();

	/* DMC0 memconfig */
	writel(dmc_configs.dmc0memconfig0, &dmcs[0]->memconfig0);
	writel(dmc_configs.dmc0memconfig1, &dmcs[0]->memconfig1);

	/* DMC1 memconfig */
	writel(dmc_configs.dmc1memconfig0, &dmcs[1]->memconfig0);
	writel(dmc_configs.dmc1memconfig1, &dmcs[1]->memconfig1);

	writel(0x20000000, &dmcs[0]->prechconfig);
	writel(0x20000000, &dmcs[1]->prechconfig);

	/*
	 * DMC0: CS0: 166MHz
	 * DMC1: CS0: 200MHz
	 *
	 * 7.8us * 200MHz %LE %LONG1560(0x618)
	 * 7.8us * 166MHz %LE %LONG1294(0x50E)
	 * 7.8us * 133MHz %LE %LONG1038(0x40E),
	 * 7.8us * 100MHz %LE %LONG780(0x30C),
	 */
	writel(0x0000050E, &dmcs[0]->timingaref);
	writel(0x00000618, &dmcs[1]->timingaref);

	writel(0x14233287, &dmcs[0]->timingrow);
	writel(0x182332c8, &dmcs[1]->timingrow);

	writel(0x12130005, &dmcs[0]->timingdata);
	writel(0x13130005, &dmcs[1]->timingdata);

	writel(0x0E140222, &dmcs[0]->timingpower);
	writel(0x0E180222, &dmcs[1]->timingpower);

	for (i = 0; i < ARRAY_SIZE(dmcs); i++) {
		/* chip0 Deselect */
		writel(0x07000000, &dmcs[i]->directcmd);

		/* chip0 PALL */
		writel(0x01000000, &dmcs[i]->directcmd);

		/* chip0 REFA */
		writel(0x05000000, &dmcs[i]->directcmd);
		writel(0x05000000, &dmcs[i]->directcmd);

		/* chip0 MRS */
		writel(0x00000032, &dmcs[i]->directcmd);

		/* chip0 EMRS */
		writel(0x00020020, &dmcs[i]->directcmd);

		/* chip1 Deselect */
		writel(0x07100000, &dmcs[i]->directcmd);

		/* chip1 PALL */
		writel(0x01100000, &dmcs[i]->directcmd);

		/* chip1 REFA */
		writel(0x05100000, &dmcs[i]->directcmd);
		writel(0x05100000, &dmcs[i]->directcmd);

		/* chip1 MRS */
		writel(0x00100032, &dmcs[i]->directcmd);

		/* chip1 EMRS */
		writel(0x00120020, &dmcs[i]->directcmd);

		/* auto refresh on */
		writel(0xFF002030 | (1 << 7), &dmcs[i]->concontrol);

		/* PwrdnConfig */
		writel(0x00100002, &dmcs[i]->pwrdnconfig);

		writel(0x00212113, &dmcs[i]->memcontrol);
	}
}

void mem_ctrl_init(int reset)
{
	if (cpu_is_s5pc110())
		s5pc110_mem_ctrl_init(reset);
}
