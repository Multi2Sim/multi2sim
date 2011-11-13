/*
 *  Libstruct
 *  Copyright (C) 2007  Rafael Ubal Tena (ubal@gap.upv.es)
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

#ifndef ELF_FORMAT_H
#define ELF_FORMAT_H

#include <elf.h>


/* ELF buffer */
struct elf2_buffer_t
{
	void *ptr;  /* Contents of the buffer */
	int size;  /* Size of the buffer */
	int pos;  /* Current position in buffer [0..size-1] */
};


/* ELF section */
struct elf2_section_t
{
	/* Name of the section */
	char *name;

	/* Pointer to the location in 'elf_file->buffer' where the
	 * section header is located. */
	Elf32_Shdr *header;

	/* Sub-buffer (subset of 'elf_file->buffer') where the contents of
	 * the section can be found. */
	struct elf2_buffer_t buffer;
};


/* ELF program header */
struct elf2_program_header_t
{
	/* Pointer to the location in 'elf_file->buffer' where the
	 * program header is located. */
	Elf32_Phdr *header;
};


/* ELF Symbol */
struct elf2_symbol_t
{
	char *name;
	uint32_t value;
	uint32_t size;
	int section;
};


/* ELF file */
struct elf2_file_t
{
	/* File name, or NULL if loaded from buffer */
	char *path;

	/* ELF buffer */
	struct elf2_buffer_t buffer;

	/* ELF header - pointer to a position within 'buffer' */
	Elf32_Ehdr *header;

	/* ELF sections */
	struct list_t *section_list;  /* Elements of type 'struct elf2_section_t' */
	struct elf2_section_t *string_table;  /* String table section */

	/* ELF program headers */
	struct list_t *program_header_list;  /* Elements of type 'struct elf2_program_header_t' */
	uint32_t program_header_table_base;  /* Virtual address to load program header table (x86 ELF) */

	/* Symbol table */
	struct list_t *symbol_table;  /* Elements of type 'struct elf2_symbol_t' */
};


#define elf2_debug(...) debug(elf2_debug_category, __VA_ARGS__)
extern int elf2_debug_category;

void elf2_buffer_seek(struct elf2_buffer_t *buffer, int pos);
void *elf2_buffer_tell(struct elf2_buffer_t *buffer);
int elf2_buffer_read(struct elf2_buffer_t *buffer, void *ptr, int size);
int elf2_buffer_read_line(struct elf2_buffer_t *buffer, char *str, int size);

struct elf2_symbol_t *elf2_symbol_get_by_address(struct elf2_file_t *elf_file, uint32_t addr, uint32_t *offset_ptr);
struct elf2_symbol_t *elf2_symbol_get_by_name(struct elf2_file_t *elf_file, char *name);

struct elf2_file_t *elf2_file_create_from_buffer(void *ptr, int size, char *name);
struct elf2_file_t *elf2_file_create_from_path(char *path);
void elf2_file_free(struct elf2_file_t *elf_file);


#endif

