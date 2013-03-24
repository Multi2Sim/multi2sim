/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_EVERGREEN_EMU_BIN_FILE_H
#define ARCH_EVERGREEN_EMU_BIN_FILE_H

#include <lib/util/elf-format.h>


/* Encoding dictionary entry header (as encoded in ELF file) */
struct evg_bin_enc_dict_entry_header_t
{
	Elf32_Word d_machine;
	Elf32_Word d_type;
	Elf32_Off d_offset;  /* Offset for encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_size;  /* Size of encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_flags;
};


/* Constats embedded in the '.data' section */
struct evg_bin_enc_dict_entry_consts_t
{
	float float_consts[256][4];
	unsigned int int_consts[32][4];
	unsigned int bool_consts[32];
};


/* Encoding dictionary entry */
struct evg_bin_enc_dict_entry_t
{
	/* Header (pointer to ELF buffer contents) */
	struct evg_bin_enc_dict_entry_header_t *header;

	/* Buffers containing PT_LOAD and PT_NOTE segments */
	struct elf_buffer_t pt_load_buffer;
	struct elf_buffer_t pt_note_buffer;

	/* Buffers containing sections */
	struct elf_buffer_t sec_text_buffer;
	struct elf_buffer_t sec_data_buffer;
	struct elf_buffer_t sec_symtab_buffer;
	struct elf_buffer_t sec_strtab_buffer;

	/* Constants extract from '.data' section */
	struct evg_bin_enc_dict_entry_consts_t *consts;

	/* Info read from pt_notes */
	int num_gpr_used;
	int lds_size_used;
	int stack_size_used;
};


/* Binary file */
struct evg_bin_file_t
{
	/* Associated ELF file */
	struct elf_file_t *elf_file;

	/* Encoding dictionary.
	 * Elements are of type 'struct evg_bin_enc_dict_entry_t'
	 * Each element of the dictionary contains the binary for a different architecture
	 * (Evergreen, x86, etc.) */
	struct list_t *enc_dict;

	/* Encoding dictionary entry containing the Evergree kernel.
	 * This is a member of the 'enc_dict' list. */
	struct evg_bin_enc_dict_entry_t *enc_dict_entry_evergreen;
};

struct evg_bin_file_t *evg_bin_file_create(void *ptr, int size, char *name);
void evg_bin_file_free(struct evg_bin_file_t *bin);


#endif

