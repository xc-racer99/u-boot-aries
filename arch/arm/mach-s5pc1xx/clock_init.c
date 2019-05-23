/* SPDX-License-Identifier: GPL-2.0+ */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/power.h>

#include "common_setup.h"

void s5pc110_system_clock_init(void)
{
	struct s5pc110_clock *clk =
		(struct s5pc110_clock *)samsung_get_base_clock();
	int reg;

	/* Set OSC_FREQ value */
	writel(0xf, S5PC110_OSC_FREQ);

	/* Set MTC_STABLE value */
	writel(0xffffffff, S5PC110_MTC_STABLE);

	/* Set CLAMP_STABLE value */
	writel(0x3ff03ff, S5PC110_CLAMP_STABLE);

	/* Set Clock divider
	 * 1:1:4:4, 1:4:5
	 */
	writel(0x14131330, &clk->div0);

	/* UART[3210]: MMC[3210] */
	writel(0x11110111, &clk->div4);

	/* Set Lock Time
	 * 30us for APLL
	 * 3600us for MPLL, EPLL, VPLL
	 */
	writel(0x2cf, &clk->apll_lock);
	writel(0xe10, &clk->mpll_lock);
	writel(0xe10, &clk->epll_lock);
	writel(0xe10, &clk->vpll_lock);

	/* S5PC110_APLL_CON - 800MHz */
	writel(0x80C80601, &clk->apll_con);

	/* S5PC110_MPLL_CON - 667MHz */
	writel(0x829B0C01, &clk->mpll_con);

	/* S5PC110_EPLL_CON - 96MHz /*/
	writel(0x88500303, &clk->epll_con);

	/* S5PC110_VPLL_CON - 54MHz */
	writel(0x806C0603, &clk->vpll_con);

	/* Set Source Clock
	 * SRC0: A, M, E, VPLL Muxing
	 * SRC4: UART/MMC
	 */
	writel(0x10001111, &clk->src0);
	writel(0x66667777, &clk->src4);

	/* OneDRAM(DMC0) clock setting
	 * ONEDRAM_SEL[25:24] 1 SCLKMPLL
	 * ONEDRAM_RATIO[31:28] 3 + 1
	 */
	writel(0x01000000, &clk->src6);
	writel(0x30000000, &clk->div6);

	/* XCLKOUT = XUSBXTI 24MHz */
	reg = readl(S5PC110_OTHERS);
	reg |= (0x3 << 8);
	writel(reg, S5PC110_OTHERS);

	/* CLK_IP0
	 * DMC[1:0] PDMA0[3] IMEM[5]
	 */
	writel(0x8fefeeb, &clk->gate_ip0);

	/* CLK_IP1
	 * FIMD[0] USBOTG[16]
	 * NANDXL[24]
	 */
	writel(0xe9fdf0fb, &clk->gate_ip1);

	/* CLK_IP2
	 * CORESIGHT[8] MODEM[9]
	 * HOSTIF[10] HSMMC0[16]
	 * HSMMC2[18] VIC[27:24]
	 */
	writel(0xf75f7fc, &clk->gate_ip2);

	/* CLK_IP3
	 * I2C[8:6]
	 * SYSTIMER[16] UART0[17]
	 * UART1[18] UART2[19]
	 * UART3[20] WDT[22]
	 * PWM[23] GPIO[26] SYSCON[27]
	 */
	writel(0x8eff038c, &clk->gate_ip3);

	/* CLK_IP4
	 * CHIP_ID[0] TZPC[8:5]
	 */
	writel(0xfffffff1, &clk->gate_ip4);

	/* wait at least 200us to stablize all clock */
	sdelay(0x10000);
}

void system_clock_init(void)
{
	if (cpu_is_s5pc110())
		s5pc110_system_clock_init();
}
