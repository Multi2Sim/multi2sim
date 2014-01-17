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


//#include <arch/fermi/asm/arg.h>
//#include <arch/southern-islands/asm/bin-file.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/elf-encode.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

//#include "metadata.h"
#include "outer-bin.h"

#define NUM_PROG_INFO_ELEM 114

struct str_map_t frm2bin_outer_bin_device_map = 
{
	4,
	{
			{"invalid", frm2bin_outer_bin_invalid},
			{"cape_verde", frm2bin_outer_bin_cape_verde},
			{"pitcairn", frm2bin_outer_bin_pitcairn},
			{"tahiti", frm2bin_outer_bin_tahiti},
		}

};

struct frm2bin_outer_bin_t *frm2bin_outer_bin_create(void)
{
	struct frm2bin_outer_bin_t *outer_bin;

	/* Initialize */
	outer_bin = xcalloc(1, sizeof(struct frm2bin_outer_bin_t));

	outer_bin->file = elf_enc_file_create();

	outer_bin->inner_bin_list = list_create();

	/* outer_bin->metadata_list = list_create(); */

	/* Set default device type to tahiti */
	outer_bin->device = frm2bin_outer_bin_tahiti;

	return outer_bin;

}

void frm2bin_outer_bin_free(struct frm2bin_outer_bin_t *outer_bin)
{
	int i;

	elf_enc_file_free(outer_bin->file);

	LIST_FOR_EACH(outer_bin->inner_bin_list, i)
	{
		frm2bin_inner_bin_free(list_get(outer_bin->inner_bin_list, i));
	}
	list_free(outer_bin->inner_bin_list);

	//LIST_FOR_EACH(outer_bin->metadata_list, i)
	//{
	//	frm2bin_metadata_free(list_get(outer_bin->metadata_list, i));
	//}
	//list_free(outer_bin->metadata_list);
	
	free(outer_bin);
}

void frm2bin_outer_bin_add(struct frm2bin_outer_bin_t *outer_bin,
	struct frm2bin_inner_bin_t *inner_bin)
{
	list_add(outer_bin->inner_bin_list, inner_bin);
	/* list_add(outer_bin->metadata_list, metadata); */
}

void frm2bin_outer_bin_generate(struct frm2bin_outer_bin_t *outer_bin,
	struct elf_enc_buffer_t *buffer)
{
	/* do nothing temporarily */
	int i;

	struct frm2bin_inner_bin_t *inner_bin;

	struct elf_enc_section_t *text_section;

	struct elf_enc_buffer_t *text_buffer;
	struct elf_enc_buffer_t *kernel_buffer;

	/* Create Text Section */
	text_buffer = elf_enc_buffer_create();
	text_section =
		elf_enc_section_create(".text", text_buffer, text_buffer);

	/* loop over all inner kernels */
	LIST_FOR_EACH(outer_bin->inner_bin_list, i)
	{
		/* a lot processing here */

		kernel_buffer = elf_enc_buffer_create();

		inner_bin = list_get(outer_bin->inner_bin_list, i);

		/* Generate Inner Bin File */
		frm2bin_inner_bin_generate(inner_bin, kernel_buffer);

		/* write (inner) kernel_buffer to (outer) text_buffer */
		elf_enc_buffer_write(text_buffer, kernel_buffer->ptr,
			kernel_buffer->offset);

		/* free kernel_buffer */
		elf_enc_buffer_free(kernel_buffer);
	}

	/* Add Text Section and Text Buffer to Outer ELF */
	elf_enc_file_add_buffer(outer_bin->file, text_buffer);
	elf_enc_file_add_section(outer_bin->file, text_section);
}

