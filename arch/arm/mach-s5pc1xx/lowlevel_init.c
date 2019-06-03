
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dmc.h>
#include <asm/arch/power.h>
#include <asm/arch/tzpc.h>

/* These are the things we can do during low-level init */
enum {
	DO_WAKEUP	= 1 << 0,
	DO_CLOCKS	= 1 << 1,
	DO_MEM_RESET	= 1 << 2,
	DO_UART		= 1 << 3,
	DO_POWER	= 1 << 4,
};

int do_lowlevel_init(void)
{
	uint32_t reset_status;
	int actions = 0;

	arch_cpu_init();

	reset_status = get_reset_status();

	switch (reset_status) {
	case S5P_CHECK_SLEEP:
		actions = DO_CLOCKS | DO_WAKEUP;
		break;
	case S5P_CHECK_DIDLE:
	case S5P_CHECK_DSTOP:
		actions = DO_WAKEUP;
		break;
	default:
		/* This is a normal boot (not a wake from sleep) */
		actions = DO_CLOCKS | DO_MEM_RESET | DO_POWER | DO_UART;
	}

	if (actions & DO_POWER)
		set_ps_hold_ctrl();

	misc_power_init();

#if 1
	uart_init();
	debug_uart_init();

	for (reset_status = 0; reset_status < 10; reset_status++)
		debug_uart_putc('Y');
#else
	if (actions & DO_UART)
		uart_init();
#endif

	if (actions & DO_CLOCKS) {
		system_clock_init();
#if defined(CONFIG_DEBUG_UART) && defined(CONFIG_SPL_SERIAL_SUPPORT)
		debug_uart_init();
#endif
		mem_ctrl_init(actions & DO_MEM_RESET);
		tzpc_init();
	}

	return actions & DO_WAKEUP;
}
