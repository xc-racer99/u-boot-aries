#include <common.h>
#include <asm/arch/dmc.h>

#include <linux/mtd/onenand_regs.h>

/**
 * Determines the memory configuration
 * of the board - Aries has two possiblities
 * which depend on how much OneNAND there is
 */
struct dmc_memconfigs s5pc110_get_dmc_memconfigs(void)
{
	struct dmc_memconfigs cfgs;
	unsigned int dev_id, density;

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
	 * There are two variants
	 * one which is always paired with
	 * 8Gb datasize OneNAND, one
	 * with 4Gb datasize OneNAND
	 */
	dev_id = readw(CONFIG_SYS_ONENAND_BASE
			+ ONENAND_REG_DEVICE_ID);
	density = dev_id >> ONENAND_DEVICE_DENSITY_SHIFT;
	density &= ONENAND_DEVICE_DENSITY_MASK;

	if (density == ONENAND_DEVICE_DENSITY_8Gb) {
		/*
		 * DMC1: CS1 : S5PC110
		 * 0x50 -> 0x40000000
		 * 0xf8 -> 0x47FFFFFF (1Gib)
		 * [15:12] 1: Interleaved
		 * [11:8 ] 2: 9 bits  - Col (1Gib)
		 * [ 7:4 ] 2: 14 bits - Row
		 * [ 3:0 ] 2: 4 banks
		 */
		cfgs.dmc1memconfig1 = 0x50f81222;
	} else {
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
	}

	return cfgs;
}
