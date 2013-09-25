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

#ifndef LIB_CPP_ELF_WRITER_H
#define LIB_CPP_ELF_WRITER_H

#include <elf.h>
#include <iostream>
#include <sstream>
#include <vector>


namespace ELFWriter
{

/* Forward declarations */
class File;


class Buffer
{
	friend class File;

	File *file;

	/* Index that this buffer occupies in vector File::buffers */
	unsigned int index;

	/* Stream with content */
	std::ostringstream stream;

	/* Constructor */
	Buffer(File *file, unsigned int index);

public:

	/* Return reference to internal stream */
	std::ostringstream& GetStream() { return stream; }

	/* Short-hand operantions on stream */
	void Write(const char *buffer, unsigned int size)
			{ stream.write(buffer, size); }
	void Seek(unsigned int pos) { stream.seekp(pos); }
	unsigned int Tell() { return stream.tellp(); }
	void Clear() { stream.str(""); }

	/* Hex dump */
	void Dump(std::ostream& os);

	/* Getters */
	unsigned int GetIndex() { return index; }
};


class Section
{
	friend class File;

	File *file;
	std::string name;
	Elf32_Shdr info;

	/* Index that the section has within File::sections */
	unsigned int index;
	
	/* Set in constructor. First and last buffers that build up the
	 * section. */
	Buffer *first_buffer;
	Buffer *last_buffer;

	/* Constructor */
	Section(File *file, std::string name, Buffer *first_buffer,
			Buffer *last_buffer, unsigned int index);

public:

	void Dump(std::ostream& os);

	/* Writable fields of section info */
	void SetType(Elf32_Word type) { info.sh_type = type; }
	void SetFlags(Elf32_Word flags) { info.sh_flags = flags; }
	void SetAddr(Elf32_Addr addr) { info.sh_addr = addr; }
	void SetLink(Elf32_Word link) { info.sh_link = link; }
	void SetInfo(Elf32_Word info) { this->info.sh_info = info; }
	void SetAddralign(Elf32_Word addralign) { info.sh_addralign = addralign; }
	void SetEntsize(Elf32_Word entsize) { info.sh_entsize = entsize; }

	/* Getters */
	unsigned int GetIndex() { return index; }
};
	

class Segment
{
	friend class File;

	File *file;
	std::string name;
	Elf32_Phdr info;
	
	/* Index within vector File::segments */
	unsigned int index;

	/* First and last buffer forming the segment */
	Buffer *first_buffer;
	Buffer *last_buffer;

	/* Constructor */
	Segment(File *file, std::string name, Buffer *first_buffer,
			Buffer *last_buffer, unsigned int index);
public:

	void Dump(std::ostream& os);

	/* Writable fields of segment info */
	void SetType(Elf32_Word type) { info.p_type = type; }
	void SetVaddr(Elf32_Addr vaddr) { info.p_vaddr = vaddr; }
	void SetPaddr(Elf32_Addr paddr) { info.p_paddr = paddr; }
	void SetFlags(Elf32_Word flags) { info.p_flags = flags; }
	void SetAlign(Elf32_Word align) { info.p_align = align; }

	/* Getters */
	unsigned int GetIndex() { return index; }
};


class Symbol
{
	friend class SymbolTable;

	std::string name;
	Elf32_Sym info;

	Symbol(std::string name);

public:

	/* Writable fields of symbol info */
	void SetValue(Elf32_Addr value) { info.st_value = value; }
	void SetSize(Elf32_Word size) { info.st_size = size; }
	void SetInfo(unsigned char info) { this->info.st_info = info; }
	void SetOther(unsigned char other) { info.st_other = other; }
	void SetShndx(Elf32_Section shndx) { info.st_shndx = shndx; }
	void SetSection(Section *section) { info.st_shndx = section->GetIndex(); }
};


class SymbolTable
{
	friend class File;

	File *file;
	std::vector<Symbol *> symbols;

	/* Buffer with the content of a section of type SHT_SYMTAB, containing a
	 * list of 'Elf32_Sym' entries. This buffer is internally created when
	 * the symbol table is created, and its contents are automatically
	 * generated. Same for string table. */
	Buffer *symtab_buffer;
	Buffer *strtab_buffer;

	/* Sections containing the symbol table and string table buffers. */
	Section *symtab_section;
	Section *strtab_section;

	/* Constructor */
	SymbolTable(File *file, std::string symtab, std::string strtab);
	~SymbolTable();

	/* Populate symtab and strtab buffers */
	void Generate();

public:

	Symbol *NewSymbol(std::string name);
};


class File
{
	std::string path;
	Elf32_Ehdr info;

	std::vector<Section *> sections;
	std::vector<Segment *> segments;
	std::vector<SymbolTable *> symbol_tables;
	std::vector<Buffer *> buffers;

public:

	File(std::string path);
	~File();

	/* Writable fields */
	void SetIdent(int index, unsigned char value) { info.e_ident[index] = value; }
	void SetType(Elf32_Half type) { info.e_type = type; }
	void SetMachine(Elf32_Half machine) { info.e_machine = machine; }
	void SetVersion(Elf32_Word version) { info.e_version = version; }
	void SetEntry(Elf32_Addr entry) { info.e_entry = entry; }

	/* Add components */
	Buffer *NewBuffer();
	Section *NewSection(std::string name, Buffer *first, Buffer *last);
	Segment *NewSegment(std::string name, Buffer *first, Buffer *last);
	SymbolTable *NewSymbolTable(std::string symtab, std::string strtab);

	/* Getters */
	Buffer *GetBuffer(unsigned int index) { return index < buffers.size() ?
			buffers[index] : NULL; }
	Section *GetSection(unsigned int index) { return index < sections.size() ?
			sections[index] : NULL; }
	Segment *GetSegment(unsigned int index) { return index < segments.size() ?
			segments[index] : NULL; }
	SymbolTable *GetSymbolTable(unsigned int index) { return index <
			symbol_tables.size() ? symbol_tables[index] : NULL; }

	/* Produce binary */
	void Generate(std::ostream& os);
	void Generate();
};


}  /* namespace ELFWriter */

#endif

