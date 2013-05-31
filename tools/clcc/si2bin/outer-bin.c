
#include <lib/mhandle/mhandle.h>
#include <lib/util/elf-encode.h>
#include <lib/util/list.h>

#include "outer-bin.h"

struct si2bin_outer_bin_t *si2bin_outer_bin_create(void)
{
	struct si2bin_outer_bin_t *outer_bin;

	/* Initialize */
	outer_bin = xcalloc(1, sizeof(struct si2bin_outer_bin_t));

	outer_bin->file = elf_enc_file_create();

	outer_bin->inner_bin_list = list_create();

	return outer_bin;

}

void si2bin_outer_bin_free(struct si2bin_outer_bin_t *outer_bin)
{
	int i;

	elf_enc_file_free(outer_bin->file);
	
	LIST_FOR_EACH(outer_bin->inner_bin_list, i)
	{
		si2bin_inner_bin_free(list_get(outer_bin->inner_bin_list, i));
	}

	free(outer_bin);
}

void si2bin_outer_bin_add(struct si2bin_outer_bin_t *outer_bin,
		struct si2bin_inner_bin_t *inner_bin)
{
	list_add(outer_bin->inner_bin_list, inner_bin);
}

void si2bin_outer_bin_generate(struct si2bin_outer_bin_t *outer_bin,
		struct elf_enc_buffer_t *buffer)
{
	
}

