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

#ifndef LIB_CPP_ELF_READER_H
#define LIB_CPP_ELF_READER_H

#include <deque>
#include <list>
#include <elf.h>
#include <iostream>
#include <sstream>


namespace ELFReader
{

/* Forward declarations */
class File;


class Section
{
	/* File that it belongs to */
	File *file;

public:
	/* Name of the section */
	std::string name;

	/* Buffer and its size */
	char *buffer;
	long size;

	/* Section information */
	Elf32_Shdr *info;

	/* Content of section. This string stream is pointing to an internal
	 * subset of the ELF file's buffer. */
	std::stringstream ss;

	/* Constructor and destructor */
	Section(File *file, long pos);
};


class ProgramHeader
{
	/* File that it belongs to */
	File *file;

public:
	/* Program header information */
	Elf32_Phdr *info;

	/* Constructor and destructor */
	ProgramHeader(File *file, long pos);
};


class Symbol
{
	/* File and section that it belongs to */
	File *file;
	Section *section;

public:
	/* Symbol name */
	std::string name;

	/* Symbol information, pointing to an internal position of the ELF
	 * file's buffer. */
	Elf32_Sym *info;

	/* Constructor and destructor */
	Symbol(File *file, Section *section, long pos);

	/* Comparison between symbols */
	static bool Compare(Symbol *a, Symbol *b);
};


class File
{
	void ReadHeader();
	void ReadSections();
	void ReadProgramHeaders();
	void ReadSymbols();

public:
	/* Path if loaded from a file */
	std::string path;

	/* Full content of file and its size */
	char *buffer;
	long size;

	/* String stream pointing to buffer */
	std::stringstream ss;

	/* ELF header */
	Elf32_Ehdr *info;

	/* List of sections */
	std::deque<Section *> section_list;

	/* String table section */
	Section *string_table;

	/* List of program headers */
	std::list<ProgramHeader *> program_header_list;

	/* List of symbols */
	std::deque<Symbol *> symbol_list;


	/* Constructor and destructor */
	File(std::string path);
	~File(void);

	/* Dump file information into output stream */
	friend std::ostream &operator<<(std::ostream &os, const File &file);
};

/* Default constructor */
void ELFReaderCreate(File *self, const char *path);

/* Create ELF file from the content of a buffer given in 'ptr' with a size of
 * 'size' bytes. */
void ELFReaderCreateFromBuffer(File *self, void *ptr, int size);

/* Destructor */
void ELFReaderDestroy(File *self);

/* Return the first symbol with an address lower or equal to 'address'. The
 * distance between the returned symbol and 'address' is returned in
 * 'offset_ptr', if not NULL. If there is no symbol with an address lower or
 * equal to 'address', return NULL. */
Symbol *ELFReaderGetSymbolByAddress(File *self, unsigned int address,
		unsigned int *offset_ptr);

/* Return the first symbol with name 'name'. If there is no such symbol, return
 * NULL. */
Symbol *ELFReaderGetSymbolByName(File *self, char *name);

/* Read the content in a section pointed to by a symbol value and size. If the
 * symbol points to an invalid section or its value/size point to an invalid
 * part of the section, return NULL. The caller is responsible for freeing the
 * buffer object returned. */
//Buffer *ELFReaderReadSymbolContent(File *self, Symbol *symbol);




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


}  /* namespace File */

#endif

