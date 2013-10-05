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

#include <vector>
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
	friend class File;

	/* File that it belongs to */
	File *file;

	/* Name of the section */
	std::string name;

	/* File content */
	char *buffer;
	unsigned int size;

	/* Section information */
	int index;
	Elf32_Shdr *info;

	/* Constructor */
	Section(File *file, int index, unsigned int pos);
public:
	/* Section header */
	int GetIndex() { return index; }
	const std::string& GetName() { return name; }
	Elf32_Word GetType() { return info->sh_type; }
	Elf32_Word GetFlags() { return info->sh_flags; }
	Elf32_Addr GetAddr() { return info->sh_addr; }
	Elf32_Off GetOffset() { return info->sh_offset; }
	Elf32_Word GetSize() { return info->sh_size; }
	Elf32_Word GetLink() { return info->sh_link; }
	Elf32_Word GetInfo() { return info->sh_info; }
	Elf32_Word GetAddralign() { return info->sh_addralign; }
	Elf32_Word GetEntsize() { return info->sh_entsize; }

	/* Section content */
	char *GetBuffer() { return buffer; }
	void GetStream(std::istringstream& stream) { GetStream(stream, 0, size); }
	void GetStream(std::istringstream& stream, unsigned int offset,
			unsigned int size);
};


class ProgramHeader
{
	friend class File;

	/* File that it belongs to */
	File *file;

	/* Program header information */
	int index;
	Elf32_Phdr *info;

	/* File content */
	char *buffer;
	unsigned int size;

	/* Constructor */
	ProgramHeader(File *file, int index, unsigned int pos);
public:

	/* Program header information */
	int GetIndex() { return index; }
	Elf32_Word GetType() { return info->p_type; }
	Elf32_Off GetOffset() { return info->p_offset; }
	Elf32_Addr GetVaddr() { return info->p_vaddr; }
	Elf32_Addr GetPaddr() { return info->p_paddr; }
	Elf32_Word GetFilesz() { return info->p_filesz; }
	Elf32_Word GetMemsz() { return info->p_memsz; }
	Elf32_Word GetFlags() { return info->p_flags; }
	Elf32_Word GetAlign() { return info->p_align; }

	/* File content pointed to by program header (segment) */
	unsigned int GetSize() { return size; }
	char *GetBuffer() { return buffer; }
	void GetStream(std::istringstream& stream) { GetStream(stream, 0, size); }
	void GetStream(std::istringstream& stream, unsigned int offset,
			unsigned int size);
};


class Symbol
{
	friend class File;

	/* File that it belongs to */
	File *file;

	/* Section that the symbol points to. This section is not the section
	 * passed as an argument to the constructor (i.e., the symbol table
	 * section). It is the section pointed to by its 'st_shndx' field, or
	 * null if this fields points to an invalid section. */
	Section *section;

	/* Symbol name */
	std::string name;

	/* Content pointed to by symbol in the section, or null if it points
	 * to an invalid region of the section, or doesn't point to any section. */
	char *buffer;

	/* Symbol information, pointing to an internal position of the ELF
	 * file's buffer. */
	Elf32_Sym *info;

	/* Constructor and destructor */
	Symbol(File *file, Section *section, unsigned int pos);

	/* Comparison between symbols */
	static bool Compare(Symbol *a, Symbol *b);

public:

	Section *GetSection() { return section; }
	const std::string& GetName() { return name; }

	/* Symbol information */
	Elf32_Addr GetValue() { return info->st_value; }
	Elf32_Word GetSize() { return info->st_size; }
	unsigned char GetInfo() { return info->st_info; }
	unsigned char GetOther() { return info->st_other; }
	Elf32_Section GetShndx() { return info->st_shndx; }

	/* Section content pointed to by symbol, if valid */
	char *GetBuffer() { return buffer; }
	void GetStream(std::istringstream& stream) { GetStream(stream, 0,
			info->st_size); }
	void GetStream(std::istringstream& stream, unsigned int offset,
			unsigned int size);
};


class File
{
	void ReadHeader();
	void ReadSections();
	void ReadProgramHeaders();
	void ReadSymbols();

	/* Path if loaded from a file */
	std::string path;

	/* File content */
	char *buffer;
	unsigned int size;
	std::istringstream stream;

	/* ELF header */
	Elf32_Ehdr *info;

	/* String table section */
	Section *string_table;

	/* Sections, program headers, and symbols */
	std::vector<Section *> sections;
	std::vector<ProgramHeader *> program_headers;
	std::vector<Symbol *> symbols;

public:

	File(std::string path);
	File(const char *buffer, unsigned int size);
	~File(void);

	/* Dump file information into output stream */
	friend std::ostream &operator<<(std::ostream &os, const File &file);

	const std::string& GetPath() { return path; }

	/* Sections */
	int GetNumSections() { return sections.size(); }
	Section *GetSection(unsigned int index) { return index <
			sections.size() ? sections[index] : NULL; }

	/* Program headers */
	int GetNumProgramHeaders() { return program_headers.size(); }
	ProgramHeader *GetProgramHeader(unsigned int index) { return index <
			program_headers.size() ? program_headers[index] : NULL; }

	/* Symbols */
	int GetNumSymbols() { return symbols.size(); }
	Symbol *GetSymbol(unsigned int index) { return index < symbols.size() ?
			symbols[index] : NULL; }
	Symbol *GetSymbol(std::string name);

	/* Return the section corresponding to the string table, or NULL if
	 * the ELF file doesn't contain one. */
	Section *GetStringTable() { return string_table; }

	/* File content */
	unsigned int GetSize() { return size; }
	char *GetBuffer() { return buffer; }
	void GetStream(std::istringstream& stream) { GetStream(stream, 0, size); }
	void GetStream(std::istringstream& stream, unsigned int offset,
			unsigned int size);

	/* Return the first symbol at a given address/name, or the closest
	 * one with a higher address. If argument 'offset' is passed, the
	 * offset of the symbol relative to the address is returned. */
	Symbol *GetSymbolByAddress(unsigned int address);
	Symbol *GetSymbolByAddress(unsigned int address, unsigned int &offset);

	/* ELF header */
	unsigned char *GetIdent() { return info->e_ident; }
	Elf32_Half GetType() { return info->e_type; }
	Elf32_Half GetMachine() { return info->e_machine; }
	Elf32_Word GetVersion() { return info->e_version; }
	Elf32_Addr GetEntry() { return info->e_entry; }
	Elf32_Off GetPhoff() { return info->e_phoff; }
	Elf32_Off GetShoff() { return info->e_shoff; }
	Elf32_Word GetFlags() { return info->e_flags; }
	Elf32_Half GetEhsize() { return info->e_ehsize; }
	Elf32_Half GetPhentsize() { return info->e_phentsize; }
	Elf32_Half GetPhnum() { return info->e_phnum; }
	Elf32_Half GetShentsize() { return info->e_shentsize; }
	Elf32_Half GetShnum() { return info->e_shnum; }
	Elf32_Half GetShstrndx() { return info->e_shstrndx; }
};



}  /* namespace File */

#endif

