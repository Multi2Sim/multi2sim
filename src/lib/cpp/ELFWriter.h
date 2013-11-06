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
#include <fstream>
#include <memory>
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
	int index;

	/* Stream with content */
	std::ostringstream stream;

	/* Constructor */
	Buffer(File *file, int index) :
		file(file), index(index) { }

public:

	/* Return reference to internal stream */
	std::ostringstream &getStream() { return stream; }

	/* Short-hand operations on stream */
	void Write(const char *buffer, size_t size)
			{ stream.write(buffer, size); }
	void Seek(size_t pos) { stream.seekp(pos); }
	size_t Tell() { return stream.tellp(); }
	size_t getSize();
	void Clear() { stream.str(""); }

	/* Hex dump */
	void Dump(std::ostream& os) const;

	/* Getters */
	int getIndex() const { return index; }
};


class Section
{
	friend class File;

	File *file;
	std::string name;
	Elf32_Shdr info;

	/* Index that the section has within File::sections */
	int index;
	
	/* Set in constructor. First and last buffers that build up the
	 * section. */
	Buffer *first_buffer;
	Buffer *last_buffer;

	/* Constructor */
	Section(File *file, std::string name, Buffer *first_buffer,
			Buffer *last_buffer, int index);

public:

	void Dump(std::ostream &os) const;

	/* Writable fields of section info */
	void setType(Elf32_Word type) { info.sh_type = type; }
	void setFlags(Elf32_Word flags) { info.sh_flags = flags; }
	void setAddr(Elf32_Addr addr) { info.sh_addr = addr; }
	void setLink(Elf32_Word link) { info.sh_link = link; }
	void setInfo(Elf32_Word info) { this->info.sh_info = info; }
	void setAddrAlign(Elf32_Word addralign) { info.sh_addralign = addralign; }
	void setEntSize(Elf32_Word entsize) { info.sh_entsize = entsize; }

	/* Getters */
	int getIndex() const { return index; }
};
	

class Segment
{
	friend class File;

	File *file;
	std::string name;
	Elf32_Phdr info;
	
	/* Index within vector File::segments */
	int index;

	/* First and last buffer forming the segment */
	Buffer *first_buffer;
	Buffer *last_buffer;

	/* Constructor */
	Segment(File *file, std::string name, Buffer *first_buffer,
			Buffer *last_buffer, int index);
public:

	void Dump(std::ostream& os) const;

	/* Writable fields of segment info */
	void setType(Elf32_Word type) { info.p_type = type; }
	void setVaddr(Elf32_Addr vaddr) { info.p_vaddr = vaddr; }
	void setPaddr(Elf32_Addr paddr) { info.p_paddr = paddr; }
	void setFlags(Elf32_Word flags) { info.p_flags = flags; }
	void setAlign(Elf32_Word align) { info.p_align = align; }

	/* Getters */
	int getIndex() const { return index; }
};


class Symbol
{
	friend class SymbolTable;

	std::string name;
	Elf32_Sym info;

	Symbol(const std::string &name) :
		name(name), info({0}) { }

public:

	/* Writable fields of symbol info */
	void setValue(Elf32_Addr value) { info.st_value = value; }
	void setSize(Elf32_Word size) { info.st_size = size; }
	void setInfo(unsigned char info) { this->info.st_info = info; }
	void setOther(unsigned char other) { info.st_other = other; }
	void setShndx(Elf32_Section shndx) { info.st_shndx = shndx; }
	void setSection(Section *section) { info.st_shndx = section->getIndex(); }
};


class SymbolTable
{
	friend class File;

	File *file;
	std::vector<std::unique_ptr<Symbol>> symbols;

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
	SymbolTable(File *file, const std::string &symtab,
			const std::string &strtab);

	/* Populate symtab and strtab buffers */
	void Generate();

public:

	Symbol *NewSymbol(const std::string &name);

	Buffer *getSymbolTableBuffer() const { return symtab_buffer; }
	Buffer *getStringTableBuffer() const { return strtab_buffer; }
};


/// Class representing an output ELF file. The class supports
/// operations to add sections, segments, and symbols into an ELF file, as well
/// as to modify its headers.
class File
{
	// File header
	Elf32_Ehdr info;

	// Vectors of elements. Each element contains a 'unique_ptr', which will
	// automatically free the pointed objects when the current class is
	// destructed.
	std::vector<std::unique_ptr<Section>> sections;
	std::vector<std::unique_ptr<Segment>> segments;
	std::vector<std::unique_ptr<SymbolTable>> symbol_tables;
	std::vector<std::unique_ptr<Buffer>> buffers;

public:

	/// Constructor
	File();

	/// Set field \a e_ident of the ELF header
	void setIdent(int index, unsigned char value) { info.e_ident[index] = value; }

	/// Set field \a e_type of the ELF header
	void setType(Elf32_Half type) { info.e_type = type; }

	/// Set field \a e_machine of the ELF header
	void setMachine(Elf32_Half machine) { info.e_machine = machine; }

	/// Set field \a e_version of the ELF header
	void setVersion(Elf32_Word version) { info.e_version = version; }

	/// Set field \a e_entry of the ELF header
	void setEntry(Elf32_Addr entry) { info.e_entry = entry; }

	/// Add a new buffer object to an internal list of buffers.
	/// Buffers of type Buffer can only be instantiated through this function.
	Buffer *NewBuffer();

	/// Create a new ELF section and add it to the internal list of
	/// sections. The section is created by specifying a set of consecutive
	/// buffers that will form it. These buffers must have been created and
	/// inserted in order with calls to NewBuffer().
	///
	/// \param name Name of the new section.
	/// \param first First buffer contained in the section.
	/// \param last Last buffer contained in the section.
	/// \return The function returns the newly created section. The section
	///		properties can then be modified using calls to member
	///		functions of class Section.
	///
	Section *NewSection(const std::string &name, Buffer *first, Buffer *last);

	/// Create a new segment, pointed to by an ELF program header. A
	/// segment is created by specifying a set of consecutive buffers that
	/// will form it. These buffers must have been created and inserted in
	/// order with calls to NewBuffer().
	///
	/// \param name Name of the new segment.
	/// \param first First buffer contained in the segment.
	/// \param last Last buffer contained in the segment.
	/// \return The function returns the newly created segment. The segment
	///		properties can then be modified using calls to member
	///		functions of class Segment.
	///
	Segment *NewSegment(const std::string &name, Buffer *first, Buffer *last);

	/// Create a new symbol table in the ELF file. A symbol table is
	/// composed of two sections: the symbol section (\a symtab) containing
	/// the very ELF symbol data structures, and the string table
	/// (\a strtab) containing the symbol names. These two sections are
	/// created automatically.
	///
	/// \param symtab Name of the symbol section (e.g., ".symtab")
	/// \param strtab Name of the string table with symbol names
	///		(e.g., ".strtab").
	/// \return The function returns the newly created symbol table. New
	///		symbols can be added to this symbol table using member
	///		functions of class SymbolTable.
	///
	SymbolTable *NewSymbolTable(const std::string &symtab,
			const std::string &strtab);

	/// Return buffer at position \a index from the buffer list
	Buffer *getBuffer(int index) const {
		return index >= 0 && index < (int) buffers.size() ?
				buffers[index].get() : nullptr;
	}

	/// Return section at position \a index in the section list
	Section *getSection(int index) const {
		return index >= 0 && index < (int) sections.size() ?
				sections[index].get() : nullptr;
	}

	/// Return segment at position \a index in the segment list
	Segment *getSegment(int index) const {
		return index >= 0 && index < (int) segments.size() ?
				segments[index].get() : nullptr;
	}

	/// Return symbol table at position \a index in the list of
	/// symbol tables.
	SymbolTable *getSymbolTable(int index) const {
		return index >= 0 && index < (int) symbol_tables.size() ?
				symbol_tables[index].get() : nullptr;
	}

	/// Return the number of buffers created with calls to NewBuffer()
	int getBufferCount() const { return buffers.size(); }

	/// Return the number of sections created with calls to NewSection()
	int getSectionCount() const { return sections.size(); }

	/// Return the number of segments created with calls to NewSegment()
	int getSegmentCount() const { return segments.size(); }

	/// Return the number of symbol tables created with calls to
	/// NewSymbolTable()
	int getSymbolTableCount() const { return symbol_tables.size(); }

	/// Produce binary into the output stream given in \a os
	void Generate(std::ostream &os);

	/// Produce binary into the output file given in \a path
	void Generate(const std::string &path);
};


}  // namespace ELFWriter

#endif

