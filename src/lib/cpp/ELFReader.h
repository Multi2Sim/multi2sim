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

#include <memory>
#include <vector>
#include <list>
#include <elf.h>
#include <iostream>
#include <sstream>


namespace ELFReader
{

class File;


class Section
{
	friend class File;

	// File that it belongs to
	File *file;

	// Name of the section
	std::string name;

	// File content
	char *buffer;
	unsigned int size;

	// Section information
	int index;
	Elf32_Shdr *info;

	// Constructor
	Section(File *file, int index, unsigned int pos);

public:

	/* Section header */
	int getIndex() const { return index; }
	const std::string &getName() const { return name; }
	Elf32_Word getType() const { return info->sh_type; }
	Elf32_Word getFlags() const { return info->sh_flags; }
	Elf32_Addr getAddr() const { return info->sh_addr; }
	Elf32_Off getOffset() const { return info->sh_offset; }
	Elf32_Word getSize() const { return info->sh_size; }
	Elf32_Word getLink() const { return info->sh_link; }
	Elf32_Word getInfo() const { return info->sh_info; }
	Elf32_Word getAddrAlign() const { return info->sh_addralign; }
	Elf32_Word getEntSize() const { return info->sh_entsize; }

	/* Section content */
	const char *getBuffer() const { return buffer; }
	void getStream(std::istringstream &stream) const { getStream(stream, 0, size); }
	void getStream(std::istringstream &stream, unsigned int offset,
			unsigned int size) const;
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
	int getIndex() const { return index; }
	Elf32_Word getType() const { return info->p_type; }
	Elf32_Off getOffset() const { return info->p_offset; }
	Elf32_Addr getVaddr() const { return info->p_vaddr; }
	Elf32_Addr getPaddr() const { return info->p_paddr; }
	Elf32_Word getFilesz() const { return info->p_filesz; }
	Elf32_Word getMemsz() const { return info->p_memsz; }
	Elf32_Word getFlags() const { return info->p_flags; }
	Elf32_Word getAlign() const { return info->p_align; }

	/* File content pointed to by program header (segment) */
	unsigned int getSize() const { return size; }
	const char *getBuffer() const { return buffer; }
	void getStream(std::istringstream &stream) const {
		getStream(stream, 0, size);
	}
	void getStream(std::istringstream &stream, unsigned int offset,
			unsigned int size) const;
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
	const char *buffer;

	/* Symbol information, pointing to an internal position of the ELF
	 * file's buffer. */
	Elf32_Sym *info;

	/* Constructor and destructor */
	Symbol(File *file, Section *section, unsigned int pos);

	/* Comparison between symbols */
	static bool Compare(const std::unique_ptr<Symbol> &a,
			const std::unique_ptr<Symbol> &b);

public:

	Section *getSection() const { return section; }
	const std::string &getName() const { return name; }

	/* Symbol information */
	Elf32_Addr getValue() const { return info->st_value; }
	Elf32_Word getSize() const { return info->st_size; }
	unsigned char getInfo() const { return info->st_info; }
	unsigned char getOther() const { return info->st_other; }
	Elf32_Section getShndx() const { return info->st_shndx; }

	/* Section content pointed to by symbol, if valid */
	const char *getBuffer() const { return buffer; }
	void getStream(std::istringstream &stream) const {
		getStream(stream, 0, info->st_size);
	}
	void getStream(std::istringstream &stream, unsigned int offset,
			unsigned int size) const;
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

	/* Sections, program headers, and symbols. Each vector has exclusive
	 * ownership of the object points. When the vector is destroyed, all
	 * dynamically allocated objects are automatically freed as well. */
	std::vector<std::unique_ptr<Section>> sections;
	std::vector<std::unique_ptr<ProgramHeader>> program_headers;
	std::vector<std::unique_ptr<Symbol>> symbols;

public:

	File(std::string path);
	File(const char *buffer, unsigned int size);
	~File(void);

	/* Dump file information into output stream */
	friend std::ostream &operator<<(std::ostream &os, const File &file);

	const std::string &getPath() const { return path; }

	/* Sections */
	int getNumSections() const { return sections.size(); }

	Section *getSection(unsigned int index) const {
		return index < sections.size() ? sections[index].get()
				: nullptr;
	}

	const std::vector<std::unique_ptr<Section>> &getSections() const {
		return sections;
	}

	/* Program headers */
	int getNumProgramHeaders() const { return program_headers.size(); }

	ProgramHeader *getProgramHeader(unsigned int index) const {
		return index < program_headers.size() ?
				program_headers[index].get() : nullptr;
	}

	/* Symbols */
	int getNumSymbols() const { return symbols.size(); }

	Symbol *getSymbol(unsigned int index) const {
		return index < symbols.size() ?
				symbols[index].get() : nullptr;
	}

	Symbol *getSymbol(const std::string &name) const;

	/* Return the section corresponding to the string table, or NULL if
	 * the ELF file doesn't contain one. */
	Section *getStringTable() const { return string_table; }

	/* File content */
	unsigned int getSize() const { return size; }
	char *getBuffer() const { return buffer; }
	void getStream(std::istringstream& stream) const {
		getStream(stream, 0, size);
	}
	void getStream(std::istringstream& stream, unsigned int offset,
			unsigned int size) const;

	/* Return the first symbol at a given address/name, or the closest
	 * one with a higher address. If argument 'offset' is passed, the
	 * offset of the symbol relative to the address is returned. */
	Symbol *getSymbolByAddress(unsigned int address) const;
	Symbol *getSymbolByAddress(unsigned int address,
			unsigned int &offset) const;

	/* ELF header */
	unsigned char *getIdent() const { return info->e_ident; }
	Elf32_Half getType() const { return info->e_type; }
	Elf32_Half getMachine() const { return info->e_machine; }
	Elf32_Word getVersion() const { return info->e_version; }
	Elf32_Addr getEntry() const { return info->e_entry; }
	Elf32_Off getPhoff() const { return info->e_phoff; }
	Elf32_Off getShoff() const { return info->e_shoff; }
	Elf32_Word getFlags() const { return info->e_flags; }
	Elf32_Half getEhsize() const { return info->e_ehsize; }
	Elf32_Half getPhentsize() const { return info->e_phentsize; }
	Elf32_Half getPhnum() const { return info->e_phnum; }
	Elf32_Half getShentsize() const { return info->e_shentsize; }
	Elf32_Half getShnum() const { return info->e_shnum; }
	Elf32_Half getShstrndx() const { return info->e_shstrndx; }
};



}  /* namespace File */

#endif

