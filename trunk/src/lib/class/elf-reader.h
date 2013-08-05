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

#ifndef LIB_CLASS_ELF_READER_H
#define LIB_CLASS_ELF_READER_H

#include <elf.h>

#include "class.h"


/*
 * Class 'ELFBuffer'
 */

CLASS_BEGIN(ELFBuffer, Object)
	
	void *ptr;
	int size;
	int pos;

CLASS_END(ELFBuffer)


void ELFBufferCreate(ELFBuffer *self, void *ptr, int size);
void ELFBufferDestroy(ELFBuffer *self);

/* Dump contents of buffer into file 'f' without any kind of format
 * transformation (the output is binary). The buffer is dumped at the current
 * position of the file. */
void ELFBufferDump(Object *self, FILE *f);

/* Select new position in buffer. The function will fatal if the position given
 * in 'pos' is an invalid value (out of range). */
void ELFBufferSeek(ELFBuffer *self, int pos);

/* Return a pointer to the current position of the buffer. */
void *ELFBufferTell(ELFBuffer *self);

/* Return true if the position in the buffer is past its last byte. */
int ELFBufferIsEnd(ELFBuffer *self);

/* Read 'size' bytes from ELF buffer, and store them in '*ptr'.
 *  -If 'size' is greater than the number of bytes left in the buffer, only the
 *   remaining contents of the buffer will be read.
 *  -The current position in the buffer is moved forward.
 *  -If 'ptr' is NULL, only the position in the buffer will be moved, without
 *   actually reading the contents of the buffer.
 *  -The function returns the number of bytes effectively read.
 */
int ELFBufferRead(ELFBuffer *self, void *ptr, int size);

/* Read a line of text from an ELF buffer at its current position, stopping at
 * the first occurrence of the newline ('\n') character. A new string is
 * allocated and returned containing the line read, and not including the
 * newline character. If no more lines are available in the buffer, return NULL.
 */
String *ELFBufferReadLine(ELFBuffer *self);



/*
 * Class 'ELFSection'
 */


CLASS_BEGIN(ELFSection, Object)

	/* Name of the section */
	String *name;

	/* Pointer to the location in 'elf_file->buffer' where the
	 * section header is located. */
	Elf32_Shdr *header;

	/* Buffer pointing to a subset of ELFReader->buffer */
	ELFBuffer *buffer;

CLASS_END(ELFSection)


/* Create a section from extracted from the ELF reader object given in 'reader',
 * where 'offset' points to the location where the section header can be found.
 */
void ELFSectionCreate(ELFSection *self);

/* Destructor */
void ELFSectionDestroy(ELFSection *self);



/*
 * Class 'ELFProgramHeader'
 */

CLASS_BEGIN(ELFProgramHeader, Object)
	
	/* Pointer to the location in 'ELFReader->buffer' where the program
	 * header is located. */
	Elf32_Phdr *header;

CLASS_END(ELFProgramHeader)


void ELFProgramHeaderCreate(ELFProgramHeader *self);
void ELFProgramHeaderDestroy(ELFProgramHeader *self);



/*
 * Class 'ELFSymbol'
 */

CLASS_BEGIN(ELFSymbol, Object)

	/* Symbol name */
	char *name;

	/* Pointer to the location in 'ELFReader->buffer' where the symbo
	 * information can be found. */
	Elf32_Sym *sym;

CLASS_END(ELFSymbol)

void ELFSymbolCreate(ELFSymbol *self);
void ELFSymbolDestroy(ELFSymbol *self);

int ELFSymbolCompare(Object *self, Object *object);




/*
 * Class 'ELFReader'
 */

CLASS_BEGIN(ELFReader, Object)

	/* Path if loaded from a file */
	String *path;

	/* Buffer containing the entire ELF file */
	ELFBuffer *buffer;
	
	/* ELF header - pointer to a position within 'buffer' */
	Elf32_Ehdr *header;

	/* Array of sections */
	Array *section_array;

	/* String table section */
	ELFSection *string_table;

	/* List of program headers */
	List *program_header_list;

	/* Array of symbols */
	Array *symbol_array;

CLASS_END(ELFReader)

/* Default constructor */
void ELFReaderCreate(ELFReader *self, const char *path);

/* Create ELF file from the content of a buffer given in 'ptr' with a size of
 * 'size' bytes. */
void ELFReaderCreateFromBuffer(ELFReader *self, void *ptr, int size);

/* Destructor */
void ELFReaderDestroy(ELFReader *self);

/* Return the first symbol with an address lower or equal to 'address'. The
 * distance between the returned symbol and 'address' is returned in
 * 'offset_ptr', if not NULL. If there is no symbol with an address lower or
 * equal to 'address', return NULL. */
ELFSymbol *ELFReaderGetSymbolByAddress(ELFReader *self, unsigned int address,
		unsigned int *offset_ptr);

/* Return the first symbol with name 'name'. If there is no such symbol, return
 * NULL. */
ELFSymbol *ELFReaderGetSymbolByName(ELFReader *self, char *name);

/* Read the content in a section pointed to by a symbol value and size. If the
 * symbol points to an invalid section or its value/size point to an invalid
 * part of the section, return NULL. The caller is responsible for freeing the
 * buffer object returned. */
ELFBuffer *ELFReaderReadSymbolContent(ELFReader *self, ELFSymbol *symbol);




/*
 * Public
 */

#define elf_reader_debug(...) debug(elf_reader_debug_category, __VA_ARGS__)
extern int elf_reader_debug_category;

/* Read the ELF header from a file, without creating an entire ELF object, and
 * return it in 'ehdr_ptr'.
 * This function is useful to pre-process an ELF file before deciding a
 * specific action for it that depends on a field of the header. */
void ELFFileReadHeader(char *path, Elf32_Ehdr *ehdr_ptr);

#endif

