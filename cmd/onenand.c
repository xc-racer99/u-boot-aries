/*
 *  U-Boot command for OneNAND support
 *
 *  Copyright (C) 2005-2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>

#include <linux/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#include <asm/io.h>

static struct mtd_info *mtd;

static int arg_off_size_onenand(int argc, char * const argv[], ulong *off,
				size_t *size)
{
	if (argc >= 1) {
		if (!(str2long(argv[0], off))) {
			printf("'%s' is not a number\n", argv[0]);
			return -1;
		}
	} else {
		*off = 0;
	}

	if (argc >= 2) {
		if (!(str2long(argv[1], (ulong *)size))) {
			printf("'%s' is not a number\n", argv[1]);
			return -1;
		}
	} else {
		*size = mtd->size - *off;
	}

	if ((*off + *size) > mtd->size) {
		printf("total chip size (0x%llx) exceeded!\n", mtd->size);
		return -1;
	}

	if (*size == mtd->size)
		puts("whole chip\n");
	else
		printf("offset 0x%lx, size 0x%x\n", *off, *size);

	return 0;
}

static int do_onenand_info(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	printf("%s\n", mtd->name);
	return 0;
}

static int do_onenand_bad(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong ofs;

	/* Currently only one OneNAND device is supported */
	printf("\nDevice %d bad blocks:\n", 0);
	for (ofs = 0; ofs < mtd->size; ofs += mtd->erasesize) {
		if (mtd_block_isbad(mtd, ofs))
			printf("  %08x\n", (u32)ofs);
	}

	return 0;
}

static int do_onenand_checkbad(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong ofs;
	size_t len;

	/*
	 * Syntax is:
	 *   0       1         2   3
	 *   onenand checkbad  off size
	 */

	printf("\nOneNAND checkbad: ");

	if (arg_off_size_onenand(argc - 1, argv + 1, &ofs, &len) != 0)
		return CMD_RET_USAGE;

	return onenand_checkbad(mtd, ofs, len);
}

static int do_onenand_read(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	char *s;
	int oob = 0;
	ulong addr, ofs;
	size_t len;
	int ret = 0;
	size_t retlen = 0;

	if (argc < 3)
		return CMD_RET_USAGE;

	s = strchr(argv[0], '.');
	if ((s != NULL) && (!strcmp(s, ".oob")))
		oob = 1;

	addr = (ulong)simple_strtoul(argv[1], NULL, 16);

	printf("\nOneNAND read: ");
	if (arg_off_size_onenand(argc - 2, argv + 2, &ofs, &len) != 0)
		return 1;

	ret = onenand_block_read(mtd, ofs, len, &retlen, (u8 *)addr, oob);

	printf(" %d bytes read: %s\n", retlen, ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}

static int do_onenand_write(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr, ofs;
	size_t len;
	int ret = 0, withoob = 0;
	size_t retlen = 0;

	if (argc < 3)
		return CMD_RET_USAGE;

	if (strncmp(argv[0] + 6, "yaffs", 5) == 0)
		withoob = 1;

	addr = (ulong)simple_strtoul(argv[1], NULL, 16);

	printf("\nOneNAND write: ");
	if (arg_off_size_onenand(argc - 2, argv + 2, &ofs, &len) != 0)
		return 1;

	ret = onenand_block_write(mtd, ofs, len, &retlen, (u8 *)addr, withoob);

	printf(" %d bytes written: %s\n", retlen, ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}

static int do_onenand_erase(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong ofs;
	int ret = 0;
	size_t len;
	int force;

	/*
	 * Syntax is:
	 *   0       1     2       3    4
	 *   onenand erase [force] [off size]
	 */
	argc--;
	argv++;
	if (argc)
	{
		if (!strcmp("force", argv[0]))
		{
			force = 1;
			argc--;
			argv++;
		}
	}
	printf("\nOneNAND erase: ");

	/* skip first two or three arguments, look for offset and size */
	if (arg_off_size_onenand(argc, argv, &ofs, &len) != 0)
		return 1;

	ret = onenand_block_erase(mtd, ofs, len, force);

	printf("%s\n", ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}

static int do_onenand_test(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong ofs;
	int ret = 0;
	size_t len;

	/*
	 * Syntax is:
	 *   0       1     2       3    4
	 *   onenand test [force] [off size]
	 */

	printf("\nOneNAND test: ");

	/* skip first two or three arguments, look for offset and size */
	if (arg_off_size_onenand(argc - 1, argv + 1, &ofs, &len) != 0)
		return 1;

	ret = onenand_block_test(mtd, ofs, len);

	printf("%s\n", ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}

static int do_onenand_dump(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong ofs;
	int ret = 0;
	char *s;

	if (argc < 2)
		return CMD_RET_USAGE;

	s = strchr(argv[0], '.');
	ofs = (int)simple_strtoul(argv[1], NULL, 16);

	if (s != NULL && strcmp(s, ".oob") == 0)
		ret = onenand_dump(mtd, ofs, 1);
	else
		ret = onenand_dump(mtd, ofs, 0);

	return ret == 0 ? 1 : 0;
}

static int do_onenand_markbad(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	ulong addr;

	argc -= 2;
	argv += 2;

	if (argc <= 0)
		return CMD_RET_USAGE;

	while (argc > 0) {
		addr = simple_strtoul(*argv, NULL, 16);

		if (mtd_block_markbad(mtd, addr)) {
			printf("block 0x%08lx NOT marked "
				"as bad! ERROR %d\n",
				addr, ret);
			ret = 1;
		} else {
			printf("block 0x%08lx successfully "
				"marked as bad\n",
				addr);
		}
		--argc;
		++argv;
	}
	return ret;
}

static cmd_tbl_t cmd_onenand_sub[] = {
	U_BOOT_CMD_MKENT(info, 1, 0, do_onenand_info, "", ""),
	U_BOOT_CMD_MKENT(bad, 1, 0, do_onenand_bad, "", ""),
	U_BOOT_CMD_MKENT(checkbad, 3, 0, do_onenand_checkbad, "", ""),
	U_BOOT_CMD_MKENT(read, 4, 0, do_onenand_read, "", ""),
	U_BOOT_CMD_MKENT(write, 4, 0, do_onenand_write, "", ""),
	U_BOOT_CMD_MKENT(write.yaffs, 4, 0, do_onenand_write, "", ""),
	U_BOOT_CMD_MKENT(erase, 3, 0, do_onenand_erase, "", ""),
	U_BOOT_CMD_MKENT(test, 3, 0, do_onenand_test, "", ""),
	U_BOOT_CMD_MKENT(dump, 2, 0, do_onenand_dump, "", ""),
	U_BOOT_CMD_MKENT(markbad, CONFIG_SYS_MAXARGS, 0, do_onenand_markbad, "", ""),
};

#ifdef CONFIG_NEEDS_MANUAL_RELOC
void onenand_reloc(void) {
	fixup_cmdtable(cmd_onenand_sub, ARRAY_SIZE(cmd_onenand_sub));
}
#endif

static int do_onenand(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	mtd = &onenand_mtd;

	/* Strip off leading 'onenand' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_onenand_sub[0], ARRAY_SIZE(cmd_onenand_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	onenand,	CONFIG_SYS_MAXARGS,	1,	do_onenand,
	"OneNAND sub-system",
	"info - show available OneNAND devices\n"
	"onenand bad - show bad blocks\n"
	"onenand checkbad off size - test 'size' bytes from offset off\n"
	"onenand read[.oob] addr off size\n"
	"onenand write[.yaffs] addr off size\n"
	"    read/write 'size' bytes starting at offset 'off'\n"
	"    to/from memory address 'addr', skipping bad blocks.\n"
	"onenand erase [force] [off size] - erase 'size' bytes from\n"
	"onenand test [off size] - test 'size' bytes from\n"
	"    offset 'off' (entire device if not specified)\n"
	"onenand dump[.oob] off - dump page\n"
	"onenand markbad off [...] - mark bad block(s) at offset (UNSAFE)"
);
