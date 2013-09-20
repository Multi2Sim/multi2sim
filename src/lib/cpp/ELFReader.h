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

	/* Return the first symbol at a given address/name, or the closest
	 * one with a higher address. If argument 'offset' is passed, the
	 * offset of the symbol relative to the address is returned. */
	Symbol *GetSymbol(std::string name);
	Symbol *GetSymbol(unsigned int address);
	Symbol *GetSymbol(unsigned int address, unsigned int &offset);

	/* Read the content pointed to by a symbol, eather returning a pair
	 * buffer/size, or a string stream pointing to the interal ELF buffer */
	void GetSymbolContent(Symbol *symbol, char *&buffer, long &size);
	void GetSymbolContent(Symbol *symbol, std::stringstream &ss);
};



}  /* namespace File */

#endif

