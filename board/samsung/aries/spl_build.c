#include <common.h>
#include <config.h>
#include <onenand_uboot.h>

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

extern void do_hibl(void);

void board_init_f(unsigned long bootflag)
{
	__aligned(8) gd_t local_gd;
	__attribute__((noreturn)) void (*uboot)(void);

	setup_global_data(&local_gd);

#if 1
	do_hibl();
#else
#if 0
	if (do_lowlevel_init())
		power_exit_wakeup();

	copy_uboot_to_ram();
#else
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
			CONFIG_BOARD_SIZE_LIMIT, (void *) CONFIG_SYS_TEXT_BASE);
#endif

	/* Jump to U-Boot image */
	uboot = (void *)CONFIG_SYS_TEXT_BASE;
	(*uboot)();
	/* Never returns Here */
#endif
}

/* Place Holders */
void board_init_r(gd_t *id, ulong dest_addr)
{
	/* Function attribute is no-return */
	/* This Function never executes */
	while (1)
		;
}
