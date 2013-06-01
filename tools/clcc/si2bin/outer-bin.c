/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/elf-encode.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

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
	list_free(outer_bin->inner_bin_list);

	LIST_FOR_EACH(outer_bin->metadata_list, i)
	{
		si2bin_metadata_free(list_get(outer_bin->metadata_list, i));
	}
	list_free(outer_bin->metadata_list);
	
	free(outer_bin);
}

void si2bin_outer_bin_add(struct si2bin_outer_bin_t *outer_bin,
		struct si2bin_inner_bin_t *inner_bin, struct si2bin_metadata_t *metadata)
{
	list_add(outer_bin->inner_bin_list, inner_bin);
	list_add(outer_bin->metadata_list, metadata);
}

void si2bin_outer_bin_generate(struct si2bin_outer_bin_t *outer_bin,
		struct elf_enc_buffer_t *buffer)
{
	struct si2bin_inner_bin_t *inner_bin;
	char line[MAX_LONG_STRING_SIZE];

	inner_bin = list_get(outer_bin->inner_bin_list, 0);

	/* Print kernel name */
	snprintf(line, sizeof line, ";ARGSTART:__OpenCL_%s_kernel\n", inner_bin->name);
	elf_enc_buffer_write(buffer, line, strlen(line) + 1);

	/* Version */
	snprintf(line, sizeof line, "version:3:1:104\n");
	elf_enc_buffer_write(buffer, line, strlen(line) + 1);
	
	/* Device */
	snprintf(line, sizeof line, "device:tahiti\n");
	elf_enc_buffer_write(buffer, line, strlen(line) + 1);
	
	//;uniqueid:";

	//snprintf(line, 10, "%d", inner_bin->uniqueid);


}

