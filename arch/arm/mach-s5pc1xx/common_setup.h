/*
 * Memory initialization
 *
 * @param reset     Reset PHY during initialization.
 */
void mem_ctrl_init(int reset);

 /* System Clock initialization */
void system_clock_init(void);

/*
 * Init subsystems according to the reset status
 *
 * @return 0 for a normal boot, non-zero for a resume
 */
int do_lowlevel_init(void);

void sdelay(unsigned long);
