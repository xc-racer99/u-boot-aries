#include <linux/string.h>

#include "aries.h"

enum board cur_board = BOARD_MAX;

static const char *board_fit_name[BOARD_MAX] = {
	[BOARD_UNKNOWN] = "s5pc1xx-aries",
	[BOARD_CAPTIVATE] = "s5pc1xx-aries",
	[BOARD_FASCINATE] = "s5pc1xx-aries",
	[BOARD_FASCINATE4G] = "s5pc1xx-fascinate4g",
	[BOARD_GALAXYS] = "s5pc1xx-galaxys",
	[BOARD_GALAXYS4G] = "s5pc1xx-aries",
	[BOARD_VIBRANT] = "s5pc1xx-aries",
	[BOARD_UNKNOWN] = "s5pc1xx-aries",
};

enum board guess_board(void)
{
	// TODO - Determine method of detecting each variant
	return BOARD_FASCINATE4G;
}

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	if (cur_board == BOARD_MAX) {
		cur_board = guess_board();
	}

	if (!strcmp(name, board_fit_name[cur_board]))
		return 0;
	return -1;
}
#endif
