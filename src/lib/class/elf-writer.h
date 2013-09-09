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

#ifndef LIB_CLASS_ELF_WRITER_H
#define LIB_CLASS_ELF_WRITER_H

#include <elf.h>

#include "class.h"


/*
 * Class 'ELFWriterBuffer'
 */

CLASS_BEGIN(ELFWriterBuffer, Object)
	
	/* ELF writer object that the buffer belongs to. This field is set
	 * during call 'ELFWriterAddBuffer'. */
	ELFWriter *writer;

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

CLASS_END(ELFWriterBuffer);

void ELFWriterBufferCreate(ELFWriterBuffer *self);
void ELFWriterBufferDestroy(ELFWriterBuffer *self);

void ELFWriterBufferDump(Object *self, FILE *f);

/* Writes 'size' bytes from the buffer pointed to by 'ptr' into the current position
 * in the section. If the written contents exceed the size of the section, this size
 * will increase automatically. The current position in the section moves forward
 * 'size' bytes. */
void ELFWriterBufferWrite(ELFWriterBuffer *self, void *ptr, unsigned int size);

/* Tries to read 'size' bytes from the section at the current position, and stores
 * them into 'ptr'. The function returns the number of bytes actually read. The
 * current position in the buffer moves forward as many bytes as were read. It
 * is the user's responsibility to make sure ptr has enough space for its contents */
unsigned int ELFWriterBufferRead(ELFWriterBuffer *self, void *ptr, unsigned int size);

/* Set the current position in the buffer. The effective position is clamped
 * between 0 and the size of the buffer. The function returns the effective
 * new position. */
unsigned int ELFWriterBufferSeek(ELFWriterBuffer *self, unsigned int offset);

/* Write the entire content of an ELF buffer at the current position of a file
 * stream given in 'f'. The function returns the number of bytes written. */
int ELFWriterBufferWriteToFile(ELFWriterBuffer *self, FILE *f);

/* Read the content of the entire file 'f' into the current position of an
 * ELF buffer. The function returns the number of bytes read. */
int ELFWriterBufferReadFromFile(ELFWriterBuffer *self, FILE *f);



/*
 * Class 'ELFWriterSection'
 */

CLASS_BEGIN(ELFWriterSection, Object)

	/* ELF writer object that the section belongs to. This field is set
	 * during call 'ELFWriterAddSection'. */
	ELFWriter *writer;

	/* Read-only, set in constructor */
	String *name;

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
	ELFWriterBuffer *first_buffer;
	ELFWriterBuffer *last_buffer;
	
CLASS_END(ELFWriterSection)

void ELFWriterSectionCreate(ELFWriterSection *self, char *name,
		ELFWriterBuffer *first_buffer,
		ELFWriterBuffer *last_buffer);
		
void ELFWriterSectionDestroy(ELFWriterSection *self);

void ELFWriterSectionDump(Object *self, FILE *f);




/*
 * Class 'ELFWriterSegment'
 */

CLASS_BEGIN(ELFWriterSegment, Object)
	
	/* ELF writer object where it belongs */
	ELFWriter *writer;
	
	/* Read-only, set in constructor */
	String *name;

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
	ELFWriterBuffer *first_buffer;
	ELFWriterBuffer *last_buffer;

CLASS_END(ELFWriterSegment)


void ELFWriterSegmentCreate(ELFWriterSegment *self, char *name,
		ELFWriterBuffer *first_buffer, ELFWriterBuffer *last_buffer);
void ELFWriterSegmentDestroy(ELFWriterSegment *self);

void ELFWriterSegmentDump(Object *self, FILE *f);




/*
 * Class 'ELFWriterSymbol'
 */

CLASS_BEGIN(ELFWriterSymbol, Object)
	
	String *name;
	
	/* These fields are populated in call 'elf_enc_generate':
	 *		-st_name;
	 * These fields can be set by the user:
	 *		-st_value;
 	 *		-st_size;
 	 *		-st_info;
 	 *		-st_other; 
	 *		-st_shndx;  -> 	st_shndx will refer to the section list
	 */
	Elf32_Sym symbol;

CLASS_END(ELFWriterSymbol)

void ELFWriterSymbolCreate(ELFWriterSymbol *self, char *name);
void ELFWriterSymbolDestroy(ELFWriterSymbol *self);





/*
 * Class 'ELFWriterSymbolTable'
 */

CLASS_BEGIN(ELFWriterSymbolTable, Object)
	
	/* List of symbols. Each element is of type 'ELFWriterSymbol' */
	List *symbol_list;

	/* Buffer with the content of a section of type SHT_SYMTAB, containing a
	 * list of 'Elf32_Sym' entries. This buffer is internally created when
	 * the symbol table is created, and its contents are automatically
	 * generated. Same for string table.
	 * Private fields. */
	ELFWriterBuffer *symbol_table_buffer;
	ELFWriterBuffer *string_table_buffer;

	/* Sections containing the symbol table and string table buffers.
	 * Private fields. */
	ELFWriterSection *symbol_table_section;
	ELFWriterSection *string_table_section;

CLASS_END(ELFWriterSymbolTable)


/* Create a symbol table. The two names given in the arguments correspond to the
 * names of the symbol table and string table sections, automatically created. */
void ELFWriterSymbolTableCreate(ELFWriterSymbolTable *self, char *symtab_name,
		char *strtab_name);

void ELFWriterSymbolTableDestroy(ELFWriterSymbolTable *self);

void ELFWriterSymbolTableAdd(ELFWriterSymbolTable *self,
		ELFWriterSymbol *symbol);




/*
 * Class 'ELFWriter'
 */

CLASS_BEGIN(ELFWriter, Object)
	
	String *path;

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
	Elf32_Ehdr header; 

	/* Elements of type 'ELFWriterSection' */
	Array *section_array;

	/* Elements of type 'ELFWriterSegment' */
	Array *segment_array;
	
	/* Elements of type 'ELFWriterSymbolTable */
	List *symbol_table_list;
	
	/* Elements of type 'ELFWriterBuffer' */
	Array *buffer_array;

CLASS_END(ELFWriter)


void ELFWriterCreate(ELFWriter *self);
void ELFWriterDestroy(ELFWriter *self);

/* Add buffer to the buffer list of the ELF file. The buffer must not have
 * been inserted before. When the ELF file is freed, it will take care of
 * freeing this inserted buffer as well. */
void ELFWriterAddBuffer(ELFWriter *self, ELFWriterBuffer *buffer);

void ELFWriterAddSegment(ELFWriter *self, ELFWriterSegment *segment);

void ELFWriterAddSection(ELFWriter *self, ELFWriterSection *section);

void ELFWriterAddSymbolTable(ELFWriter *self,
		ELFWriterSymbolTable *symbol_table);

/* Encodes the ELF file into a new buffer object. If this object is later dumped
 * into a file, this file contains a complete ELF binary. */
void ELFWriterGenerate(ELFWriter *self, ELFWriterBuffer *buffer);




#endif

