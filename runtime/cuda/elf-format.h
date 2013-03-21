/*
 *  Libstruct
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

#ifndef LIB_UTIL_ELF_FORMAT_H
#define LIB_UTIL_ELF_FORMAT_H

#include <elf.h>
#include <stdio.h>


/* ELF buffer */
struct elf_buffer_t
{
	void *ptr;  /* Contents of the buffer */
	int size;  /* Size of the buffer */
	int pos;  /* Current position in buffer [0..size-1] */
};


/* ELF section */
struct elf_section_t
{
	/* Name of the section */
	char *name;

	/* Pointer to the location in 'elf_file->buffer' where the
	 * section header is located. */
	Elf32_Shdr *header;

	/* Sub-buffer (subset of 'elf_file->buffer') where the contents of
	 * the section can be found. */
	struct elf_buffer_t buffer;
};


/* ELF program header */
struct elf_program_header_t
{
	/* Pointer to the location in 'elf_file->buffer' where the
	 * program header is located. */
	Elf32_Phdr *header;
};


/* ELF Symbol */
struct elf_symbol_t
{
	char *name;
	unsigned int value;
	unsigned int size;
	int section;
	unsigned char info;
	unsigned char visibility;
};


/* ELF file */
struct elf_file_t
{
	/* File name, or NULL if loaded from buffer */
	char *path;

	/* ELF buffer */
	struct elf_buffer_t buffer;

	/* ELF header - pointer to a position within 'buffer' */
	Elf32_Ehdr *header;

	/* ELF sections */
	struct list_t *section_list;  /* Elements of type 'struct elf_section_t' */
	struct elf_section_t *string_table;  /* String table section */

	/* ELF program headers */
	struct list_t *program_header_list;  /* Elements of type 'struct elf_program_header_t' */

	/* Symbol table */
	struct list_t *symbol_table;  /* Elements of type 'struct elf_symbol_t' */
};


#define elf_debug(...) debug(elf_debug_category, __VA_ARGS__)
extern int elf_debug_category;

void elf_buffer_seek(struct elf_buffer_t *buffer, int pos);
void *elf_buffer_tell(struct elf_buffer_t *buffer);
int elf_buffer_read(struct elf_buffer_t *buffer, void *ptr, int size);
int elf_buffer_read_line(struct elf_buffer_t *buffer, char *str, int size);
void elf_buffer_dump(struct elf_buffer_t *buffer, FILE *f);

struct elf_symbol_t *elf_symbol_get_by_address(struct elf_file_t *elf_file,
	unsigned int addr, unsigned int *offset_ptr);
struct elf_symbol_t *elf_symbol_get_by_name(struct elf_file_t *elf_file, char *name);

/* Read the content in a section pointed to by a symbol value and size. If the
 * symbol points to an invalid section or its value/size point to an invalid
 * part of the section, the function returns FALSE. Otherwise, TRUE. */
int elf_symbol_read_content(struct elf_file_t *elf_file, struct elf_symbol_t *symbol,
		struct elf_buffer_t *elf_buffer);

struct elf_file_t *elf_file_create_from_buffer(void *ptr, int size, char *name);
struct elf_file_t *elf_file_create_from_path(char *path);
void elf_file_free(struct elf_file_t *elf_file);

void elf_file_read_header(char *path, Elf32_Ehdr *ehdr);


#endif

