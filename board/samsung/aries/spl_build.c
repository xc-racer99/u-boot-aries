#include <common.h>
#include <config.h>
#include <onenand_uboot.h>

#include <mach/power.h>

DECLARE_GLOBAL_DATA_PTR;

void memzero(void *s, size_t n)
{
	char *ptr = s;
	size_t i;

	for (i = 0; i < n; i++)
		*ptr++ = '\0';
}

/**
 * Set up the U-Boot global_data pointer
 *
 * This sets the address of the global data, and sets up basic values.
 *
 * @param gdp   Value to give to gd
 */
static void setup_global_data(gd_t *gdp)
{
	gd = gdp;
	memzero((void *)gd, sizeof(gd_t));
	gd->flags |= GD_FLG_RELOC;
	gd->baudrate = CONFIG_BAUDRATE;
	gd->have_console = 1;
}

extern void do_ibl(void);
extern void do_lowlevel_init(void);

void (*start_usb_boot)(void) = (void *) 0xD0007D78;
void (*init_system)(void) = (void *) 0xD0007CAC;
void (*irom_enable_interrupts)(void) = (void *) 0xD000C694;

void board_init_f(unsigned long bootflag)
{
	unsigned int reg;
	__aligned(8) gd_t local_gd;
	__attribute__((noreturn)) void (*uboot)(void);

	setup_global_data(&local_gd);

	reg = readl(S5PC110_PS_HOLD_CTRL);
	reg |= 0x301;
	writel(reg, S5PC110_PS_HOLD_CTRL);

#if 0
	/* Use blob init_by_rebell to initialize memory and clocks
	 * Results in working display */
	do_ibl();

	/* Clears interrupts and enable them for SBL from USB loading */
	init_system();
#else
	/* Use u-boot's clock and memory init
	 * Display doesn't work in SBL
	 */
	do_lowlevel_init();

	/* Enables interrupts for SBL from USB loading */
	irom_enable_interrupts();
#endif

#if 0
	/* Load stock SBL from OneNAND
	 * Currently not working for some reason
	 */
	onenand_spl_load_image(0xa80000,
			0x140000, (void *) 0x40244000);

	/* Jump to U-Boot image */
	uboot = (void *)0x40244000;
#elif 1
	/* Load SBL from USB, aka HIBL */
	memcpy((void *)0xD0035400, (void *)0xD000C90C, 0x70);

	start_usb_boot();
	uboot = (void *)readl(0xD00354D0);
#else
	/* Load u-boot directly */
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
			CONFIG_BOARD_SIZE_LIMIT, (void *) CONFIG_SYS_TEXT_BASE);

	/* Jump to U-Boot image */
	uboot = (void *)CONFIG_SYS_TEXT_BASE;
#endif

	(*uboot)();
	/* Never returns Here */
}

/* Place Holders */
void board_init_r(gd_t *id, ulong dest_addr)
{
	/* Function attribute is no-return */
	/* This Function never executes */
	while (1)
		;
}
