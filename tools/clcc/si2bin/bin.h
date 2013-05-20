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

#ifndef TOOLS_CLCC_SI2BIN_BIN_H
#define TOOLS_CLCC_SI2BIN_BIN_H

#include <elf.h>


/* Forward declaration */
struct elf_enc_buffer_t;
struct list_t;


/*
 * Note in the PT_NOTE segment
 */

struct si2bin_bin_note_t
{
	unsigned int type;
	unsigned int size;
	void *payload;
};


struct si2bin_bin_note_t *si2bin_bin_note_create(unsigned int type, unsigned int size,
		void *payload);
void si2bin_bin_note_free(struct si2bin_bin_note_t *note);

void si2bin_bin_note_dump(struct elf_enc_buffer_t *buffer, FILE *fp);



/*
 * Encoding Dictionary Entry
 */

/* Encoding dictionary entry header (as encoded in ELF file) */
struct si2bin_bin_entry_header_t
{
	Elf32_Word d_machine;
	Elf32_Word d_type;
	Elf32_Off d_offset;
	Elf32_Word d_size;
	Elf32_Word d_flags;
};

struct si2bin_bin_entry_t
{
	/* Public fields:
	 *	- d_machine
	 * Private fields:
	 *	- d_type
	 *	- d_offset - offset for encoding data (PT_NOTE + PT_LOAD segments)
	 *	- d_size - size of encoding data (PT_NOTE + PT_LOAD segments)
	 *	- d_flags */
	struct si2bin_bin_entry_header_t header;

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

	/* List of notes. Each element is of type 'si2bin_bin_note_t'.
	 * Private field. */
	struct list_t *note_list;
	struct elf_enc_buffer_t *note_buffer;
};

struct si2bin_bin_entry_t *si2bin_bin_entry_create(void);
void si2bin_bin_entry_free(struct si2bin_bin_entry_t *entry);

void si2bin_bin_entry_add_note(struct si2bin_bin_entry_t *entry,
		struct si2bin_bin_note_t *note);



/*
 * AMD Binary File (Internal ELF)
 */

/* si_elf_enc_create kernel will create an enc_elf_file that has the following sections
 * and segments:
	
	Sections
	------------------
	[ 0] Null section
	[ 1] .shstrtab
	[ 2] .text
	[ 3] .data
	[ 4] .symtab
	[ 5] .strtab
	[ 6] .text
	[ 7] .data
	[ 8] .symtab
	[ 9] .strtab

	Segments
	------------------
	00 Encoding Dictionary     - LOPROC+2
	01 Note for Segment 02     - PT_NOTE
	02 Sections 2 - 5          - PT_LOAD
	03 Note for Segment 04     - PT_NOTE
	04 Sections 6 - 9          - PT_LOAD
 
 */

struct si2bin_bin_t
{
	/* ELF file created internally.
	 * Private field. */
	struct elf_enc_file_t *file;

	/* Each element of type 'si2bin_bin_entry_t'.
	 * Private field. */
	struct list_t *entry_list;
};

struct si2bin_bin_t *si2bin_bin_create(void);
void si2bin_bin_free(struct si2bin_bin_t *bin);

void si2bin_bin_add_entry(struct si2bin_bin_t *bin, struct si2bin_bin_entry_t *entry);
void si2bin_bin_generate(struct si2bin_bin_t *bin, struct elf_enc_buffer_t *bin_buffer);


#endif

