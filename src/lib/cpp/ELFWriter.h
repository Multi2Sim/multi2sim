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

class File;

/// Class Buffer represents a raw sequence of bytes inside of an ELF file. The
/// way an ELF file is created is by adding a sequence of buffers in order to
/// the file, and then grouping them into sections and segments. The sections
/// and the segments can then overlap.
///
/// The constructor of this class is private, and can only be accessed
/// internally by friend class File. A new buffer can only be created with
/// a call to File::newBuffer()
///
class Buffer
{
	friend class File;

	File *file;

	// Index that this buffer occupies in vector File::buffers
	int index;

	// Stream with content
	std::stringstream stream;

	// Constructor
	Buffer(File *file, int index) :
		file(file), index(index) { }

public:

	/// Return reference to internal stream.
	/// The stream can then be used to read or write content into the
	/// buffer.
	std::stringstream &getStream() { return stream; }

	/// Reads \a size bytes from the buffer object and copies them into \a
	/// buffer. This is a shortcut operation equivalent to retrievent the
	/// stream with getStream() and reading from it with standard I/O
	/// operators.
	void Read(char *buffer, size_t size) {
		stream.read(buffer, size);
	}

	/// Writes \a size bytes from \a buffer into the buffer. This
	/// is a shortcut operation equivalent to retrieving the stream with
	/// getStream() and writing into it with standard I/O operators.
	void Write(const char *buffer, size_t size) {
		stream.write(buffer, size);
	}

	/// Set the current position in the buffer for write operations.
	/// Equivalent to getStream().seekp()
	void setWritePosition(size_t pos) { stream.seekp(pos); }

	/// Get the current position in the buffer for write operations.
	/// Equivalent to getStream().tellp()
	size_t getWritePosition() { return stream.tellp(); }

	/// Set the current position in the buffer for read operations.
	/// Equivalent to getStream().seekg()
	void setReadPosition(size_t pos) { stream.seekg(pos); }

	/// Get the current position in the buffer for read operations.
	/// Equivalent to getStream().tellg()
	size_t getReadPosition() { return stream.tellg(); }

	/// Return the number of bytes in the buffer
	size_t getSize();

	/// reset the content of the buffer.
	void Clear() { stream.str(""); }


	/// Get the index of this buffer in the list of buffers of the
	/// File object that it belongs to.
	int getIndex() const { return index; }

	/// Dump the content of the buffer in a format identical to
	/// UNIX command <tt> hexdump -C </tt>.
	void Dump(std::ostream& os) const;
};


/// Class Section represents an ELF section, defined as a sequence of buffers
/// in the File objects. The buffers belonging to a section can be a subset of,
/// a superset of, or can overlap with the buffers belonging to a segment.
///
/// An instance of class Section cannot be created directly (its constructor
/// is private). Instead, the caller should use File::newSection() to do so.
///
class Section
{
	friend class File;

	File *file;
	std::string name;
	Elf32_Shdr info;

	// Index in the list of sections
	int index;
	
	/// Set in constructor. First and last buffers of the section
	Buffer *first_buffer;
	Buffer *last_buffer;

	// Priate constructor
	Section(File *file, std::string name, Buffer *first_buffer,
			Buffer *last_buffer, int index);

public:

	/// Dump the content of the section into an output stream. The
	/// section is dumped by calling function Buffer::Dump() for every
	/// buffer that it contains.
	void Dump(std::ostream &os) const;

	/// Set the \a sh_type field of the section header
	void setType(Elf32_Word type) { info.sh_type = type; }

	/// Set the \a sh_flags field of the section header
	void setFlags(Elf32_Word flags) { info.sh_flags = flags; }

	/// Set the \a sh_addr field of the section header
	void setAddr(Elf32_Addr addr) { info.sh_addr = addr; }

	/// Set the \a sh_link field of the section header
	void setLink(Elf32_Word link) { info.sh_link = link; }

	/// Set the \a sh_info field of the section header
	void setInfo(Elf32_Word info) { this->info.sh_info = info; }

	/// Set the \a sh_addralign field of the section header
	void setAddrAlign(Elf32_Word addralign) { info.sh_addralign = addralign; }

	/// Set the \a sh_entsize field of the section header
	void setEntSize(Elf32_Word entsize) { info.sh_entsize = entsize; }

	/// Return the index of the section in the section list of the
	/// File object that it belongs to.
	int getIndex() const { return index; }
};
	

/// Class Segment represents an ELF segment. The information of a segment is
/// encoded as a program header in a program header table in the final
/// binary. A segment points to a certain content of the file, represented
/// as a range of Buffer objects previously added to the File.
///
/// An instance of class segment cannot be created directly (its constructor
/// is defined private). Instead, function File::newSegment() can be invoked
/// for this purpose, which will create the segment and add it to the
/// list of segments of the ELF file.
///
class Segment
{
	friend class File;

	File *file;
	std::string name;
	Elf32_Phdr info;
	
	// Index within the list of segments
	int index;

	// First and last buffer forming the segment
	Buffer *first_buffer;
	Buffer *last_buffer;

	// Private constructor
	Segment(File *file, std::string name, Buffer *first_buffer,
			Buffer *last_buffer, int index);
public:

	/// Dump the content of a segment into an output stream.
	/// The segment is dumped by internally calling function
	/// Buffer::Dump() for every buffer forming the segment.
	void Dump(std::ostream& os) const;

	/// Set the \a p_type field of the associated ELF program header
	void setType(Elf32_Word type) { info.p_type = type; }

	/// Set the \a p_vaddr field of the program header
	void setVaddr(Elf32_Addr vaddr) { info.p_vaddr = vaddr; }

	/// Set the \a p_addr field of the program header
	void setPaddr(Elf32_Addr paddr) { info.p_paddr = paddr; }

	/// Set the \a p_flags field of the program header
	void setFlags(Elf32_Word flags) { info.p_flags = flags; }

	/// Set the \a p_align field of the program header
	void setAlign(Elf32_Word align) { info.p_align = align; }

	/// Return the index of this segment in the list of segments
	/// of the File object where it belongs.
	int getIndex() const { return index; }
};


/// Class representing an ELF symbol. A symbol cannot be created directly
/// (its constructor is private). Instead, they should be created with a call
/// to SymbolTable::newSymbol(). The symbol table that a symbol belongs to
/// handles both the symbol section and the string table section that contains
/// the symbol name. The member functions of this class can be used to
/// modify certain attributes of an ELF symbol.
///
class Symbol
{
	friend class SymbolTable;

	std::string name;
	Elf32_Sym info;

	Symbol(const std::string &name) :
		name(name), info({0}) { }

public:

	/// Set the \a st_value field of the associated ELF symbol
	/// data structure
	void setValue(Elf32_Addr value) { info.st_value = value; }

	/// Set the \a st_size field of the ELF symbol
	void setSize(Elf32_Word size) { info.st_size = size; }

	/// Set the \a st_info field of the ELF symbol
	void setInfo(unsigned char info) { this->info.st_info = info; }

	/// Set the \a st_other field of the ELF symbol
	void setOther(unsigned char other) { info.st_other = other; }

	/// Set the \a st_shndx field of the ELF symbol. Notice that
	/// this value points to a certain section in the ELF file, and should
	/// be a number between 0 and the number of sections minus 1. To
	/// make the symbol point to a Section instance, use setSection()
	/// instead.
	void setShndx(Elf32_Section shndx) { info.st_shndx = shndx; }

	/// Set the \a st_shndx field of the ELF symbol, making it point
	/// to a section previously created.
	void setSection(Section *section) { info.st_shndx =
			section->getIndex(); }
};


class SymbolTable
{
	friend class File;

	File *file;
	std::vector<std::unique_ptr<Symbol>> symbols;

	// Buffer with the content of a section of type SHT_SYMTAB, containing a
	// list of 'Elf32_Sym' entries. This buffer is internally created when
	// the symbol table is created, and its contents are automatically
	// generated. Same for string table.
	Buffer *symtab_buffer;
	Buffer *strtab_buffer;

	// Sections containing the symbol table and string table buffers.
	Section *symtab_section;
	Section *strtab_section;

	// Private constructor
	SymbolTable(File *file, const std::string &symtab,
			const std::string &strtab);

	
public:
	// Populate symtab and strtab buffers
	void Generate();

	/// Create a new symbol named \a name. This is the only way to
	/// create a new instance of class Symbol.
	Symbol *newSymbol(const std::string &name);

	/// Return the Buffer object associated with the symbol table
	/// section automatically created for this symbol table.
	Buffer *getSymbolTableBuffer() const { return symtab_buffer; }

	/// Return the Buffer object associated with the string table
	/// section automatically created for this symbol table.
	Buffer *getStringTableBuffer() const { return strtab_buffer; }

	/// Return the Section object containing the ELF symbol data
	/// structures in the symbol table. This section is automatically
	/// created and managed by the symbol table.
	Section *getSymbolTableSection() const { return symtab_section; }

	/// Return the Section object containing the string table
	/// with the symbol names. This section is created and managed
	/// automatically by the symbol table.
	Section *getStringTableSection() const { return symtab_section; }
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
	void setIdent(int index, unsigned char value) {
		info.e_ident[index] = value;
	}

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
	Buffer *newBuffer();

	/// Create a new ELF section and add it to the internal list of
	/// sections. The section is created by specifying a set of consecutive
	/// buffers that will form it. These buffers must have been created and
	/// inserted in order with calls to newBuffer().
	///
	/// \param name Name of the new section.
	/// \param first First buffer contained in the section.
	/// \param last Last buffer contained in the section.
	/// \return The function returns the newly created section. The section
	///		properties can then be modified using calls to member
	///		functions of class Section.
	///
	Section *newSection(const std::string &name, Buffer *first, Buffer *last);

	/// Create a new segment, pointed to by an ELF program header. A
	/// segment is created by specifying a set of consecutive buffers that
	/// will form it. These buffers must have been created and inserted in
	/// order with calls to newBuffer().
	///
	/// \param name Name of the new segment.
	/// \param first First buffer contained in the segment.
	/// \param last Last buffer contained in the segment.
	/// \return The function returns the newly created segment. The segment
	///		properties can then be modified using calls to member
	///		functions of class Segment.
	///
	Segment *newSegment(const std::string &name, Buffer *first, Buffer *last);

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
	SymbolTable *newSymbolTable(const std::string &symtab,
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

	/// Return the number of buffers created with calls to newBuffer()
	int getBufferCount() const { return buffers.size(); }

	/// Return the number of sections created with calls to newSection()
	int getSectionCount() const { return sections.size(); }

	/// Return the number of segments created with calls to newSegment()
	int getSegmentCount() const { return segments.size(); }

	/// Return the number of symbol tables created with calls to
	/// newSymbolTable()
	int getSymbolTableCount() const { return symbol_tables.size(); }

	/// Produce a complete ELF binary into the output stream given in \a os
	void Generate(std::ostream &os);

	/// Produce a complete ELF binary and dump it into the output
	/// file given in \a path
	void Generate(const std::string &path);
};


}  // namespace ELFWriter

#endif

