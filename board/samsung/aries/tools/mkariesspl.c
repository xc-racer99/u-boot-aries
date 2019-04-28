/*
 *	1. Compute checksum for S5PC110 iROM  code
 *	2. Pad 0xFFFFFFFF
 *
 *	djpark (2009.08.10)
 */
#include <stdio.h>
#include "BL1_stage1_bin.h"

#define BL1_PAD_LENGTH	0x2000

int make_image(char* input_file, char* output_file)
{
	FILE *fp_read = NULL;
	FILE *fp_write = NULL;
	int ret = 0;
	unsigned int data = 0;
	unsigned int length = 0;

	fp_read = fopen(input_file, "rb");
	if (fp_read == NULL)
	{
		printf("File open error! - %s\n", input_file);
		goto err;
	}

	fp_write = fopen(output_file, "wb");
	if (fp_write == NULL)
	{
		printf("File open error! - %s\n", output_file);
		goto err;
	}

	/* Signed shim */
	fwrite(BL1_stage1_bin, sizeof(unsigned char), BL1_stage1_bin_len, fp_write);

	while ((ret = fread(&data, sizeof(unsigned int), 1, fp_read)))
	{
		length += 4;
		fwrite(&data, sizeof(unsigned int), 1, fp_write);
	}

	data = 0;
	for (; length < BL1_PAD_LENGTH; length += 4)
	{
		fwrite(&data, sizeof(unsigned int), 1, fp_write);
	}

err:
	if (fp_read != NULL)	fclose(fp_read);
	if (fp_write != NULL)	fclose(fp_write);

	return 0;
}

int main(int argc, char* argv[])
{
	if (argc == 3)
	{
		return make_image(argv[1], argv[2]);
	}
	else
	{
		printf("Error: Unsupported input parameter!\n");
		printf("usage: %s [<input_file>] [<output_file>]\n", argv[0]);
		return 1;
	}
}
