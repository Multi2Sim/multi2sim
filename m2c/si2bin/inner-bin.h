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

#ifndef M2C_SI2BIN_INNER_BIN_H
#define M2C_SI2BIN_INNER_BIN_H

#include <elf.h>
#include <stdio.h>

/* Forward declaration */
struct elf_enc_buffer_t;
struct list_t;
struct si_bin_compute_pgm_rsrc2_t;
struct si_bin_enc_user_element_t;

/*
 * Note in the PT_NOTE segment
 */


struct si2bin_inner_bin_note_t
{
	unsigned int type;
	unsigned int size;
	void *payload;
};


struct si2bin_inner_bin_note_t *si2bin_inner_bin_note_create(unsigned int type, unsigned int size,
		void *payload);
void si2bin_inner_bin_note_free(struct si2bin_inner_bin_note_t *note);

void si2bin_inner_bin_note_dump(struct elf_enc_buffer_t *buffer, FILE *fp);



/*
 * Encoding Dictionary Entry
 */


/* Encoding dictionary entry header (as encoded in ELF file) */
struct si2bin_inner_bin_entry_header_t
{
	Elf32_Word d_machine;
	Elf32_Word d_type;
	Elf32_Off d_offset;
	Elf32_Word d_size;
	Elf32_Word d_flags;
};

struct si2bin_inner_bin_entry_t
{
	/* Public fields:
	 *	- d_machine
	 * Private fields:
	 *	- d_type
	 *	- d_offset - offset for encoding data (PT_NOTE + PT_LOAD segments)
	 *	- d_size - size of encoding data (PT_NOTE + PT_LOAD segments)
	 *	- d_flags */
	struct si2bin_inner_bin_entry_header_t header;

	/* This will form the .text section containing the final ISA. The user
	 * is responsible for dumping instructions in this buffer. The buffer is
	 * created and freed internally, however. */
	struct elf_enc_buffer_t *text_section_buffer;  /* Public field */
	struct elf_enc_section_t *text_section;  /* Private field */

	/* Section .data associated with this encoding dictionary entry. The
	 * user can fill it out by adding data into the buffer. */
	struct elf_enc_buffer_t *data_section_buffer;  /* Public field */
	struct elf_enc_section_t *data_section;  /* Private field */

	/* Symbol table associated with the encoding dictionary entry. It is
	 * initialized internally, and the user can just add new symbols
	 * using function 'elf_enc_symbol_table_add'. */
	struct elf_enc_symbol_table_t *symbol_table;  /* Public field */

	/* List of notes. Each element is of type 'si2bin_inner_bin_note_t'.
	 * Private field. */
	struct list_t *note_list;
	struct elf_enc_buffer_t *note_buffer;
};

struct si2bin_inner_bin_entry_t *si2bin_inner_bin_entry_create(void);
void si2bin_inner_bin_entry_free(struct si2bin_inner_bin_entry_t *entry);

void si2bin_inner_bin_entry_add_note(struct si2bin_inner_bin_entry_t *entry,
		struct si2bin_inner_bin_note_t *note);



/*
 * AMD Internal ELF
 */


struct si2bin_inner_bin_t
{
	char *name;

	/* Program Resource */
	struct si_bin_compute_pgm_rsrc2_t *pgm_rsrc2;

	/* RAT_OP */
	int rat_op;

	/* Number of SGPRS and VGPRS */
	int num_sgprs;
	int num_vgprs;

	/* Elements of type si_bin_enc_user_element_t */
	struct list_t *user_element_list;

	/* FloatMode */
	int FloatMode;

	/*IeeeMode */
	int IeeeMode;

	/* ELF file created internally.
	 * Private field. */
	struct elf_enc_file_t *file;

	/* Each element of type 'si2bin_inner_bin_entry_t'.
	 * Private field. */
	struct list_t *entry_list;
};

struct si2bin_inner_bin_t *si2bin_inner_bin_create(char *name);
void si2bin_inner_bin_free(struct si2bin_inner_bin_t *bin);
void si2bin_inner_bin_add_user_element(struct si2bin_inner_bin_t *bin,
		struct si_bin_enc_user_element_t *user_elem, int index);

void si2bin_inner_bin_add_entry(struct si2bin_inner_bin_t *bin, struct si2bin_inner_bin_entry_t *entry);
void si2bin_inner_bin_generate(struct si2bin_inner_bin_t *bin, struct elf_enc_buffer_t *bin_buffer);

#endif

