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

#include "Error.h"


namespace ELFReader
{

class File;


/// Exception class thrown by the class functions in the ELFReader name space.
class Error : public misc::Error
{
public:

	/// Constructor
	///
	/// \param path
	///	ELF file path causing the exception
	///
	/// \param error
	///	Error string
	Error(const std::string &path, const std::string &message) :
			misc::Error(message)
	{
		// Add module prefix
		AppendPrefix("ELF Reader");
		AppendPrefix(path);
	}
};


/// This class represents an ELF section. Instances of this class are created
/// automatically by class File when an ELF file is loaded. The user cannot
/// instantiate objects of class Section, since its constructor is made private.
class Section
{
	// File that it belongs to
	File *file;

	// Name of the section
	std::string name;

	// Section content
	const char *buffer = nullptr;

	// Size of the section in bytes
	unsigned size;

	// Section information
	int index;

	// Raw section header
	Elf32_Shdr *info;

public:

	/// Constructor. An ELF section is only created internally by the ELF
	/// reader, so this function should not be invoked.
	///
	/// \param file
	///	ELF file that the section belongs to.
	///
	/// \param index
	///	Section index within the ELF file
	///
	/// \param info_offset
	///	Offset within the ELF file where the section header can be
	///	found.
	Section(File *file, int index, unsigned info_offset);

	/// Set the section name. This function is used internally by the ELF
	/// reader, and should not be called.
	void setName(const std::string &name) { this->name = name; }

	/// Return the index that the section occupies in the section
	/// list of the File object where it belongs.
	int getIndex() const { return index; }

	/// Return a pointer to the Elf32_Shdr structure representing the
	/// section header. Each field of this structure can be queried with
	/// dedicated getters instead.
	Elf32_Shdr *getRawInfo() const { return info; }

	/// Return the section name
	const std::string &getName() const { return name; }

	/// Return the value of field \a sh_name of the section header
	Elf32_Word getNameOffset() const { return info->sh_name; }

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

	/// Return a stream pointing to part of the content of the section,
	/// without involving any copy operations.
	///
	/// \param stream
	///	Input stream whose internal pointers will be modified to point
	///	to the content of the section.
	///
	/// \param offset
	///	Offset in the section that the stream points to.
	///
	/// \param size
	///	Number of bytes to include in the stream.
	void getStream(std::istringstream &stream,
			unsigned offset,
			unsigned size)
			const;
	
	/// Alternative syntax of getStream() to obtain a stream containing the
	/// entire section content.
	void getStream(std::istringstream &stream) const
	{
		getStream(stream, 0, size);
	}

};


/// This class represents an ELF program header, used to represent segments.
/// Object of this class are created automatically in class File when a new ELF
/// file is loaded. A programmer shouldn't instantiate objects of type
/// ProgramHeader (in fact, it has a private constructor).
class ProgramHeader
{
	// File that it belongs to
	File *file;

	// Index of the program header inf the ELF file
	int index;

	// Program header information
	Elf32_Phdr *info;

	// Content pointed to by the program header
	const char *buffer = nullptr;

	// Size of the content pointed to by program header
	unsigned size;

public:

	/// Constructor. A program header is created internally by the ELF
	/// file, so this function should not be invoked directly.
	///
	/// \param file
	///	ELF file that the program header belongs to.
	///
	/// \param index
	///	Index in the list of program headers of the ELF file.
	///
	/// \param info_offset
	///	Offset in ELF file content where the program header is found.
	ProgramHeader(File *file, int index, unsigned info_offset);

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

	/// Modify an input stream to hold the content pointed to by the program
	/// header, without involving any copy operations.
	///
	/// \param stream
	///	Input stream whose internal pointers will be modified.
	///
	/// \param offset
	///	Offset in the program header content that the stream points to.
	///
	/// \param size
	///	Number of bytes to include in the stream.
	void getStream(std::istringstream &stream,
			unsigned offset,
			unsigned size)
			const;
	
	/// Alternative syntax for getStream() to obtain the entire content
	/// pointed to by the program header.
	void getStream(std::istringstream &stream) const
	{
		getStream(stream, 0, size);
	}

};


/// This class represents an ELF symbol. Objects of this class are created
/// automatically by an instance of File when the ELF file is loaded. The
/// programmer shouldn't create new instances of Symbol (in fact, its
/// constructor is private).
class Symbol
{
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

public:

	/// Comparison function between symbols, used for sorting purposes.
	/// This function is used internally by the ELF file and should not be
	/// called.
	static bool Compare(const std::unique_ptr<Symbol> &a,
			const std::unique_ptr<Symbol> &b);

	/// Constructor. A symbol is only created internally by the ELF reader,
	/// so this function should not be invoked.
	///
	/// \param file
	///	ELF file that the symbol belongs to.
	///
	/// \param section
	///	Section pointed to by the symbol.
	///
	/// \param info_offset
	///	Offset in ELF file content where the symbol is found.
	Symbol(File *file, Section *section, unsigned info_offset);

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

	/// Modify an input stream to hold the content pointed to by the symbol,
	/// without involving any copy operations.
	///
	/// \param stream
	///	Input stream whose internal pointers will be modified.
	///
	/// \param offset
	///	Offset in the symbol content that the stream will point to.
	///
	/// \param size
	///	Number of bytes to include in the stream.
	void getStream(std::istringstream &stream,
			unsigned offset,
			unsigned size) const;
	
	
	/// Alternative syntax for getStream() to obtain the entire content
	/// pointed to by the symbol.
	void getStream(std::istringstream &stream) const
	{
		getStream(stream, 0, info->st_size);
	}
};


/// Class representing an input ELF file. The class contains constructors to
/// load an ELF file from a file system or from a buffer in memory. It also
/// contains functions to traverse its sections, segments, or symbols.
class File
{
	// Read the ELF header
	void ReadHeader();

	// Populate the section list
	void ReadSections();

	// Populate the program header list
	void ReadProgramHeaders();

	// Populate the symbol list
	void ReadSymbols();

	// Path if loaded from a file
	std::string path;

	// Content of the ELF file
	std::unique_ptr<char[]> buffer;

	// Total size of the ELF file
	unsigned size;

	// Stream pointing to the file content
	std::istringstream stream;

	// ELF header
	Elf32_Ehdr *info;

	// String table section
	Section *string_table;

	// List of sections
	std::vector<std::unique_ptr<Section>> sections;

	// List of program headers
	std::vector<std::unique_ptr<ProgramHeader>> program_headers;

	// List of symbols
	std::vector<std::unique_ptr<Symbol>> symbols;

public:

	/// Load an ELF file from the file system.
	///
	/// \param path
	///	Path to load the ELF file from.
	///
	/// \param read_content
	///	If true (or omitted), interpret the entire content of the ELF
	///	file. If false, read only the ELF reader. The ELF reader will
	///	return no program header, section, or symbol for the file.
	File(const std::string &path, bool read_content = true);

	/// Load an ELF file from a buffer in memory.
	///
	/// \param buffer
	///	Buffer to read the ELF file from. The content of the buffer
	///	will be replicated internally, so any external changes in it
	///	will not affect the ELF reader.
	///
	/// \param size
	///	Size of the buffer in bytes.
	///
	/// \param read_content
	///	If true (or omitted), interpret the entire content of the ELF
	///	file. If false, read only the ELF reader. The ELF reader will
	///	return no program header, section, or symbol for the file.
	File(const char *buffer, unsigned size, bool read_content = true);

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
	Section *getSection(int index) const
	{
		return index >= 0 && index < (int) sections.size() ?
				sections[index].get() :
				nullptr;
	}

	/// Return a section by its name, or \a null if not found.
	Section *getSection(const std::string &name) const;

	/// Return a read-only reference to the vector of sections. This
	/// function is useful for convenient iteration through sections using
	/// the following code:
	///
	/// \code
	///	for (auto &section : file.getSections())
	/// \endcode
	const std::vector<std::unique_ptr<Section>> &getSections() const
	{
		return sections;
	}

	/// Return the number of program headers
	int getNumProgramHeaders() const { return program_headers.size(); }

	/// Return the program header at position \a index, or \a null if
	/// the value given in \a index is out of range.
	ProgramHeader *getProgramHeader(int index) const
	{
		return index >= 0 && index < (int) program_headers.size() ?
				program_headers[index].get() :
				nullptr;
	}

	/// Return a constant reference to the list of program headers for
	/// convenient iteration.
	const std::vector<std::unique_ptr<ProgramHeader>>
			&getProgramHeaders() const
	{
		return program_headers;
	}

	/// Return the number of symbols
	int getNumSymbols() const { return symbols.size(); }

	/// Return the symbol at position \a index, or \a null if the
	/// value given in \a index is out of range.
	Symbol *getSymbol(int index) const
	{
		return index >= 0 && index < (int) symbols.size() ?
				symbols[index].get() :
				nullptr;
	}

	/// Return a symbol given its \a name. Symbols are stored
	/// internally in a sorted data structure, with an access cost of
	/// log(n).
	Symbol *getSymbol(const std::string &name) const;

	/// Return a constant reference to the list of symbols for convenient
	/// iteration.
	const std::vector<std::unique_ptr<Symbol>> &getSymbols() const
	{
		return symbols;
	}

	/// Return the section corresponding to the string table, or \a null if
	/// the ELF file doesn't contain one.
	Section *getStringTable() const { return string_table; }

	/// Return the total size of the file
	unsigned int getSize() const { return size; }

	/// Return a buffer to the content of the file
	const char *getBuffer() const { return buffer.get(); }

	/// Obtain a subset (\a size bytes starting at position \a
	/// offset) of the ELF file into the input string stream given in \a
	/// stream.
	void getStream(std::istringstream& stream,
			unsigned offset,
			unsigned size) const;

	/// Alternative syntax to getStream() to return a stream to the entire
	/// content of the ELF file.
	void getStream(std::istringstream &stream) const
	{
		getStream(stream, 0, size);
	}

	/// Return the first symbol at a given address/name, or the
	/// closest one with a higher address.
	Symbol *getSymbolByAddress(unsigned int address) const;

	/// Alternative syntax to getSymbolByAddress(), where the offset of the
	/// returned symbol (if any) is also returned in argument \a offset.
	Symbol *getSymbolByAddress(unsigned int address,
			unsigned int &offset) const;

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

