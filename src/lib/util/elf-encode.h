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

#ifndef LIB_UTIL_ELF_ENCODE_H
#define LIB_UTIL_ELF_ENCODE_H

#include <elf.h>
#include <stdio.h>


/* Forward type declarations */

struct elf_enc_section_t;
struct elf_enc_segment_t;
struct elf_enc_file_t;
struct list_t;



/*
 * ELF_enc_Buffer
 */

/* Sections and segments will consist of portions of enc_buffer list */

struct elf_enc_buffer_t
{
	/* Contents of buffer. Will grow dynamically */
	void *ptr; 

	/* Current position, automatically updated with 'elf_enc_buffer_read' and
	 * 'elf_enc_buffer_write' functions.
	 * Read-only. */
	unsigned int offset;

	/* Size occupied with buffer bytes. Updated automatically by calls to
	 * 'elf_enc_buffer_write'.
	 * Read-only. */
	unsigned int size;

	/* Size currently allocated for the buffer. The size actually occupied
	 * by the buffer content is not necessary the same. Automatically updated
	 * in 'elf_enc_buffer_write' is current size is exceeded.
	 * Private field. */
	unsigned int alloc_size;

	/* 
	 * Private field. */
	int index;
};

struct elf_enc_buffer_t *elf_enc_buffer_create(void);
void elf_enc_buffer_free(struct elf_enc_buffer_t *buffer);
void elf_enc_buffer_dump(struct elf_enc_buffer_t *buffer, FILE *f);

/* Writes 'size' bytes from the buffer pointed to by 'ptr' into the current position
 * in the section. If the written contents exceed the size of the section, this size
 * will increase automatically. The current position in the section moves forward
 * 'size' bytes. */
void elf_enc_buffer_write(struct elf_enc_buffer_t *buffer,
	void *ptr, unsigned int size);

/* Tries to read 'size' bytes from the section at the current position, and stores
 * them into 'ptr'. The function returns the number of bytes actually read. The
 * current position in the buffer moves forward as many bytes as were read. It
 * is the user's responsibility to make sure ptr has enough space for its contents */
unsigned int elf_enc_buffer_read(struct elf_enc_buffer_t *buffer,
	void *ptr, unsigned int size);

/* Set the current position in the buffer. The effective position is clamped
 * between 0 and the size of the buffer. The function returns the effective
 * new position. */
unsigned int elf_enc_buffer_seek(struct elf_enc_buffer_t *buffer,
	unsigned int offset);

/* Write the entire content of an ELF buffer at the current position of a file
 * stream given in 'f'. The function returns the number of bytes written. */
int elf_enc_buffer_write_to_file(struct elf_enc_buffer_t *buffer, FILE *f);

/* Read the content of the entire file 'f' into the current position of an
 * ELF buffer. The function returns the number of bytes read. */
int elf_enc_buffer_read_from_file(struct elf_enc_buffer_t *buffer, FILE *f);



/*
 * ELF Encoded Section
 */

/* TODO - fix section interface with same ideas as segments */
struct elf_enc_section_t
{
	/* Read-only, set in constructor */
	char *name;

	/* These fields can be set by the user:
	 * 	- sh_type
	 *	- sh_flags
	 *	- sh_addr
	 *	- sh_link
	 *	- sh_info
	 *	- sh_addralign
	 *	- sh_entsize
	 * These fields are populated in call 'elf_enc_generate':
	 *	- sh_name
	 *	- sh_offset
	 *	- sh_size (by 'elf_enc_section_read' and 'elf_enc_section_write')
	 */
	Elf32_Shdr header;


	/* Index that the section has within section list in ELF binary. This
	 * field is populated in call 'elf_enc_add_section'. */
	/* Private Field */
	int index;
	
	/* Set in constructor. These will indicate the first and last buffers in 
	 * the elf_enc_file's buffer_list field that make up the section.*/
	/* Read Only */
	struct elf_enc_buffer_t *first_buffer;
	struct elf_enc_buffer_t *last_buffer;
};

struct elf_enc_section_t *elf_enc_section_create(char *name,
		struct elf_enc_buffer_t *first_buffer,
		struct elf_enc_buffer_t *last_buffer);
void elf_enc_section_free(struct elf_enc_section_t *section);
void elf_enc_section_dump(struct elf_enc_section_t *section,
		struct elf_enc_file_t *file, FILE *f);




/*
 * ELF Encoded Segment
 */


struct elf_enc_segment_t
{
	/* Read-only, set in constructor */
	char *name;

	Elf32_Phdr header;
		/* These fields are populated in call 'elf_enc_generate':
		 *		-p_offset;	file offset
		 *		-p_filesz;	file size
		 *		-p_memsz;	memory size
	 	 * These fields can be set by the user:
		 *		-p_vaddr;	virtual address
		 *		-p_paddr;	physical address
 		 *		-p_flags;	entry flags
		 *		-p_align;	memory/file alignment
		 */
	
	/* Index that the segment has within segment list in ELF binary. This
	 * field is populated in call 'elf_enc_add_segment'.
	 * Private field. */
	int index;

	/* Set in constructor. These will indicate the first and last buffers in 
	 * the elf_enc_file's buffer_list field that make up the segment.*/
	/* Read Only */
	struct elf_enc_buffer_t *first_buffer;
	struct elf_enc_buffer_t *last_buffer;
};


struct elf_enc_segment_t *elf_enc_segment_create(char *name,
		struct elf_enc_buffer_t *first_buffer,
		struct elf_enc_buffer_t *last_buffer);
void elf_enc_segment_free(struct elf_enc_segment_t *segment);
void elf_enc_segment_dump(struct elf_enc_segment_t *segment,
		struct elf_enc_file_t *file, FILE *f);



/*
 * ELF Encoded Symbol
 */

struct elf_enc_symbol_t
{
	char *name;
	Elf32_Sym symbol;
		/* These fields are populated in call 'elf_enc_generate':
		 *		-st_name;
		 * These fields can be set by the user:
		 *		-st_value;
	 	 *		-st_size;
	 	 *		-st_info;
	 	 *		-st_other; 
		 *		-st_shndx;  -> 	st_shndx will refer to the section list
		 */
};	

struct elf_enc_symbol_t *elf_enc_symbol_create(char *name);
void elf_enc_symbol_free(struct elf_enc_symbol_t *symbol);




/*
 * ELF Symbol Table
 */

struct elf_enc_symbol_table_t
{
	/* List of symbols. Each element is of type 'struct elf_enc_symbol_t' */
	struct list_t *symbol_list;

	/* Buffer with the content of a section of type SHT_SYMTAB, containing a
	 * list of 'Elf32_Sym' entries. This buffer is internally created when
	 * the symbol table is created, and its contents are automatically
	 * generated. Same for string table.
	 * Private fields. */
	struct elf_enc_buffer_t *symbol_table_buffer;
	struct elf_enc_buffer_t *string_table_buffer;

	/* Sections containing the symbol table and string table buffers.
	 * Private fields. */
	struct elf_enc_section_t *symbol_table_section;
	struct elf_enc_section_t *string_table_section;
};
/* FIXME - replace 'symbol_table' and 'string_table' fields with 'symtab' and
 * 'strtab'. */


/* Create a symbol table. The two names given in the arguments correspond to the
 * names of the symbol table and string table sections, automatically created. */
struct elf_enc_symbol_table_t *elf_enc_symbol_table_create(char *symtab_name,
		char *strtab_name);

void elf_enc_symbol_table_free(struct elf_enc_symbol_table_t *table);

void elf_enc_symbol_table_add(struct elf_enc_symbol_table_t *table,
		struct elf_enc_symbol_t *symbol);





/*
 * ELF File Structure
 */


struct elf_enc_file_t
{
	char *path;

	Elf32_Ehdr header; 
		/* These fields can be set by the user:
		 *		-e_ident[EI_NIDENT];	ident bytes
		 *		-e_type;		file type
		 *		-e_machine;		target machine
		 *		-e_version;		file version
		 *		-e_entry;		start address
		 * These fields are populated in call 'elf_enc_generate':
		 *		-e_ident[EI_NIDENT];	ident bytes
		 *		-e_phoff;		phdr file offset
		 *		-e_shoff;		shdr file offset
		 *		-e_flags;		file flags
		 *		-e_ehsize;		sizeof ehdr
		 *		-e_phentsize;		sizeof phdr
		 *		-e_phnum;		number phdrs
		 *		-e_shentsize;		sizeof shdr
		 *		-e_shnum;		number shdrs
		 *		-e_shstrndx;		shdr string index 
		 */

	struct list_t *section_list; /* Elements of type elf_enc_section_t */

	struct list_t *segment_list; /* Elements of type elf_enc_segment_t */
	
	/* List of symbols tables. Each element is of type
	 * 'struct elf_enc_symbol_table_t'. */	
	struct list_t *symbol_table_list;
	
	/* Elements of type enc_buffer_t. Sections and segments consist of 
	portions of this list */
	struct list_t *buffer_list;
};

/* Will create an ELF binary based on the information in 'elf_file' */
void elf_enc_file_generate(struct elf_enc_file_t *elf_file,
		struct elf_enc_buffer_t *buffer);

/* Constructor and destructor for elf_enc_file_t */
struct elf_enc_file_t *elf_enc_file_create(void);
void elf_enc_file_free(struct elf_enc_file_t *file);

/* Add buffer to the buffer list of the ELF file. The buffer must not have
 * been inserted before. When the ELF file is freed, it will care care of
 * freeing this inserted buffer as well. */
void elf_enc_file_add_buffer(struct elf_enc_file_t *file,
	struct elf_enc_buffer_t *buffer);

void elf_enc_file_add_segment(struct elf_enc_file_t *file,
	struct elf_enc_segment_t *segment);

void elf_enc_file_add_section(struct elf_enc_file_t *file,
	struct elf_enc_section_t *section);

void elf_enc_file_add_symbol_table(struct elf_enc_file_t *file,
	struct elf_enc_symbol_table_t *table);

#endif
