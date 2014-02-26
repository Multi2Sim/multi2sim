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


/// This class represents an ELF section. Instances of this class are created
/// automatically by class File when an ELF file is loaded. The user cannot
/// instantiate objects of class Section, since its constructor is made private.
class Section
{
	friend class File;

	// File that it belongs to
	File *file;

	// Name of the section
	std::string name;

	// File content
	const char *buffer;
	unsigned int size;

	// Section information
	int index;
	Elf32_Shdr *info;

	// Constructor
	Section(File *file, int index, unsigned int pos);

public:

	/// Return the index that the section occupies in the section
	/// list of the File object where it belongs.
	int getIndex() const { return index; }

	/// Return a pointer to the Elf32_Shdr structure representing the
	/// section header. Each field of this structure can be queried with
	/// dedicated getters instead.
	Elf32_Shdr *getRawInfo() const { return info; }

	/// Return the section name
	const std::string &getName() const { return name; }

	/// Return the value of field \a sh_type of the section header
	Elf32_Word getType() const { return info->sh_type; }

	/// Return the value of field \a sh_flags of the section header
	Elf32_Word getFlags() const { return info->sh_flags; }
	
	/// Return the value of field \a sh_addr of the section header
	Elf32_Addr getAddr() const { return info->sh_addr; }
	
	/// Return the value of field \a sh_offset of the section header
	Elf32_Off getOffset() const { return info->sh_offset; }
	
	/// Return the value of field \a sh_size of the section header
	Elf32_Word getSize() const { return info->sh_size; }
	
	/// Return the value of field \a sh_link of the section header
	Elf32_Word getLink() const { return info->sh_link; }
	
	/// Return the value of field \a sh_info of the section header
	Elf32_Word getInfo() const { return info->sh_info; }
	
	/// Return the value of field \a sh_addralign of the section
	/// header.
	Elf32_Word getAddrAlign() const { return info->sh_addralign; }
	
	/// Return the value of field \a sh_entsize of the section header
	Elf32_Word getEntSize() const { return info->sh_entsize; }

	/// Return a pointer to the section content. The section content
	/// is not a dedicated buffer. Instead, the returned pointer points to
	/// the internal buffer representing the entire ELF file.
	const char *getBuffer() const { return buffer; }

	/// Modify variable \a stream to point to the content of the
	/// section. The section can then be read using I/O operations on the
	/// stream. This operation involves no data copy. Instead, it just makes
	/// \a stream point to the section buffer.
	void getStream(std::istringstream &stream) const {
		getStream(stream, 0, size);
	}

	/// Return a stream pointing to a subset of section, starting at
	/// \a offset and containing \a size bytes.
	void getStream(std::istringstream &stream, unsigned int offset,
			unsigned int size) const;
};


/// This class represents an ELF program header, used to represent segments.
/// Object of this class are created automatically in class File when a new ELF
/// file is loaded. A programmer shouldn't instantiate objects of type
/// ProgramHeader (in fact, it has a private constructor).
class ProgramHeader
{
	friend class File;

	// File that it belongs to
	File *file;

	// Program header information
	int index;
	Elf32_Phdr *info;

	// File content
	const char *buffer;
	unsigned int size;

	// Constructor
	ProgramHeader(File *file, int index, unsigned int pos);

public:

	/// Return the index of the program header in the program header
	/// list of the File object where it belongs.
	int getIndex() const { return index; }

	/// Return a pointer to the Elf32_Phdr structure representing the
	/// program header. Each field of this structure can be queried with
	/// dedicated getters instead.
	Elf32_Phdr *getRawInfo() const { return info; }

	/// Return the \a p_type field of the ELF program header
	Elf32_Word getType() const { return info->p_type; }

	/// Return the \a p_offset field
	Elf32_Off getOffset() const { return info->p_offset; }
	
	/// Return the \a p_vaddr field
	Elf32_Addr getVaddr() const { return info->p_vaddr; }
	
	/// Return the \a p_paddr field
	Elf32_Addr getPaddr() const { return info->p_paddr; }
	
	/// Return the \a p_filesz field
	Elf32_Word getFilesz() const { return info->p_filesz; }
	
	/// Return the \a p_memsz field
	Elf32_Word getMemsz() const { return info->p_memsz; }
	
	/// Return the \a p_flags field
	Elf32_Word getFlags() const { return info->p_flags; }
	
	/// Return the \a p_align field
	Elf32_Word getAlign() const { return info->p_align; }

	/// Return the size of the segment pointed to by the program
	/// header
	unsigned int getSize() const { return size; }

	/// Return a pointer to the segment content
	const char *getBuffer() const { return buffer; }

	/// Modify the input string stream passed in \a stream to point
	/// to the content of the segment. This operation does not replicate any
	/// content
	void getStream(std::istringstream &stream) const {
		getStream(stream, 0, size);
	}

	/// Modify the input string stream passed in \a stream to point
	/// to a subset of the segment, containing \a size bytes starting at
	/// position \a offset.
	void getStream(std::istringstream &stream, unsigned int offset,
			unsigned int size) const;
};


/// This class represents an ELF symbol. Objects of this class are created
/// automatically by an instance of File when the ELF file is loaded. The
/// programmer shouldn't create new instances of Symbol (in fact, its
/// constructor is private).
class Symbol
{
	friend class File;

	// File that it belongs to
	File *file;

	// Section that the symbol points to. This section is not the section
	// passed as an argument to the constructor (i.e., the symbol table
	// section). It is the section pointed to by its 'st_shndx' field, or
	// null if this fields points to an invalid section.
	Section *section;

	// Symbol name
	std::string name;

	// Content pointed to by symbol in the section, or null if it points
	// to an invalid region of the section, or doesn't point to any section.
	const char *buffer;

	// Symbol information, pointing to an internal position of the ELF
	// file's buffer.
	Elf32_Sym *info;

	// Constructor
	Symbol(File *file, Section *section, unsigned int pos);

	// Comparison between symbols
	static bool Compare(const std::unique_ptr<Symbol> &a,
			const std::unique_ptr<Symbol> &b);

public:

	/// Return a pointer to the Elf32_Sym structure representing the
	/// symbol. Each field of this structure can be queried with dedicated
	/// getters instead.
	Elf32_Sym *getRawInfo() const { return info; }

	/// Return the section associated with the symbol
	Section *getSection() const { return section; }

	/// Return the symbol name
	const std::string &getName() const { return name; }

	/// Return field \a st_value
	Elf32_Addr getValue() const { return info->st_value; }

	/// Return field \a st_size
	Elf32_Word getSize() const { return info->st_size; }

	/// Return field \a st_info
	unsigned char getInfo() const { return info->st_info; }

	/// Return field \a st_other
	unsigned char getOther() const { return info->st_other; }

	/// Return field \a st_shndx
	Elf32_Section getShndx() const { return info->st_shndx; }

	/// Return a pointer to the section content pointed to by the
	/// symbol, if any.
	const char *getBuffer() const { return buffer; }

	/// If the symbol points to valid content, modify the input
	/// string stream passed in \a stream to point to it.
	void getStream(std::istringstream &stream) const {
		getStream(stream, 0, info->st_size);
	}

	/// Modify the input string stream passed in \a stream to contain
	/// a subset of the content pointed to by the symbol, taking \a size
	/// bytes starting at position \a offset.
	void getStream(std::istringstream &stream, unsigned int offset,
			unsigned int size) const;
};


/// Class representing an input ELF file. The class contains constructors to
/// load an ELF file from a file system or from a buffer in memory. It also
/// contains functions to traverse its sections, segments, or symbols.
class File
{
	// Functions to load file content
	void ReadHeader();
	void ReadSections();
	void ReadProgramHeaders();
	void ReadSymbols();

	// Path if loaded from a file
	std::string path;

	// File content
	char *buffer;
	unsigned int size;
	std::istringstream stream;

	// ELF header
	Elf32_Ehdr *info;

	// String table section
	Section *string_table;

	// Sections, program headers, and symbols. Each vector has exclusive
	// ownership of the object points. When the vector is destroyed, all
	// dynamically allocated objects are automatically freed as well.
	std::vector<std::unique_ptr<Section>> sections;
	std::vector<std::unique_ptr<ProgramHeader>> program_headers;
	std::vector<std::unique_ptr<Symbol>> symbols;

public:

	/// Load an ELF file from the file system
	File(const std::string &path);

	/// Load an ELF file from a \a buffer in memory of \a size bytes
	File(const char *buffer, unsigned int size);

	/// Destructor
	~File(void);

	/// Dump file information into output stream
	friend std::ostream &operator<<(std::ostream &os, const File &file);

	/// Return the path where the file was loaded from, in case is
	/// was loaded from the file system. If the ELF file was loaded from a
	/// buffer in memory, it returns an empty string.
	const std::string &getPath() const { return path; }

	/// Return the number of sections
	int getNumSections() const { return sections.size(); }

	/// Return the section at position \a index, or \a null if the
	/// value given in \a index is out of range.
	Section *getSection(int index) const {
		return index >= 0 && index < (int) sections.size() ?
				sections[index].get() : nullptr;
	}

	/// Return a read-only reference to the vector of sections. This
	/// function is useful for convenient iteration through sections using
	/// the following code:
	///
	/// \code
	///	for (auto &section : file.getSections())
	/// \endcode
	const std::vector<std::unique_ptr<Section>> &getSections() const {
			return sections; }

	/// Return the number of program headers
	int getNumProgramHeaders() const { return program_headers.size(); }

	/// Return the program header at position \a index, or \a null if
	/// the value given in \a index is out of range.
	ProgramHeader *getProgramHeader(int index) const {
		return index >= 0 && index < (int) program_headers.size() ?
				program_headers[index].get() : nullptr;
	}

	/// Return a constant reference to the list of program headers for
	/// convenient iteration.
	const std::vector<std::unique_ptr<ProgramHeader>> &getProgramHeaders()
			const { return program_headers; }

	/// Return the number of symbols
	int getNumSymbols() const { return symbols.size(); }

	/// Return the symbol at position \a index, or \a null if the
	/// value given in \a index is out of range.
	Symbol *getSymbol(int index) const {
		return index >= 0 && index < (int) symbols.size() ?
				symbols[index].get() : nullptr;
	}

	/// Return a symbol given its \a name. Symbols are stored
	/// internally in a sorted data structure, with an access cost of
	/// log(n).
	Symbol *getSymbol(const std::string &name) const;

	/// Return a constant reference to the list of symbols for convenient
	/// iteration.
	const std::vector<std::unique_ptr<Symbol>> &getSymbols() const {
			return symbols; }

	/// Return the section corresponding to the string table, or \a null if
	/// the ELF file doesn't contain one.
	Section *getStringTable() const { return string_table; }

	/// Return the total size of the file
	unsigned int getSize() const { return size; }

	/// Return a buffer to the content of the file
	const char *getBuffer() const { return buffer; }

	/// Modify the input string stream given in \a stream to point to
	/// the entire content of the ELF file.
	void getStream(std::istringstream &stream) const {
		getStream(stream, 0, size);
	}

	/// Obtain a subset (\a size bytes starting at position \a
	/// offset) of the ELF file into the input string stream given in \a
	/// stream.
	void getStream(std::istringstream& stream, unsigned int offset,
			unsigned int size) const;

	/// @{
	/// Return the first symbol at a given address/name, or the
	/// closest one with a higher address. If argument \a offset is passed,
	/// the offset of the symbol relative to the address is returned.
	Symbol *getSymbolByAddress(unsigned int address) const;
	Symbol *getSymbolByAddress(unsigned int address,
			unsigned int &offset) const;
	/// @}

	/// Return \a e_ident field of ELF header
	unsigned char *getIdent() const { return info->e_ident; }
	
	/// Return \a e_type field of ELF header
	Elf32_Half getType() const { return info->e_type; }
	
	/// Return \a e_machine field of ELF header
	Elf32_Half getMachine() const { return info->e_machine; }
	
	/// Return \a e_version field of ELF header
	Elf32_Word getVersion() const { return info->e_version; }
	
	/// Return \a e_entry field of ELF header
	Elf32_Addr getEntry() const { return info->e_entry; }
	
	/// Return \a e_phoff field of ELF header
	Elf32_Off getPhoff() const { return info->e_phoff; }
	
	/// Return \a e_shoff field of ELF header
	Elf32_Off getShoff() const { return info->e_shoff; }
	
	/// Return \a e_flags field of ELF header
	Elf32_Word getFlags() const { return info->e_flags; }
	
	/// Return \a e_ehsize field of ELF header
	Elf32_Half getEhsize() const { return info->e_ehsize; }
	
	/// Return \a e_phentsize field of ELF header
	Elf32_Half getPhentsize() const { return info->e_phentsize; }
	
	/// Return \a e_phnum field of ELF header
	Elf32_Half getPhnum() const { return info->e_phnum; }
	
	/// Return \a e_shentsize field of ELF header
	Elf32_Half getShentsize() const { return info->e_shentsize; }
	
	/// Return \a e_shnum field of ELF header
	Elf32_Half getShnum() const { return info->e_shnum; }
	
	/// Return \a e_shstrndx field of ELF header
	Elf32_Half getShstrndx() const { return info->e_shstrndx; }
};



}  // namespace ELFReader

#endif

