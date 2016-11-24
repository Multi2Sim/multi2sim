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

#include <algorithm>
#include <cstring>
#include <fstream>
#include <istream>
#include <iomanip>

#include "ELFReader.h"
#include "Misc.h"
#include "String.h"


namespace ELFReader
{



//
// Class 'Section'
//


Section::Section(File *file, int index, unsigned info_offset) :
		file(file),
		index(index)
{
	// Read section header
	info = (Elf32_Shdr *) (file->getBuffer() + info_offset);
	if (info_offset < 0 || info_offset + sizeof(Elf32_Shdr)
			> file->getSize())
		throw Error(file->getPath(), "Invalid position for "
				"section header");

	// Initialize
	size = info->sh_size;

	// Get section contents, if section type is not SHT_NOBITS
	// (sh_type = 8).
	if (info->sh_type != 8)
	{
		// Check valid range
		if (info->sh_offset + info->sh_size > file->getSize())
			throw Error(file->getPath(), "Section out of range");

		// Set up buffer and stream
		buffer = file->getBuffer() + info->sh_offset;
	}
}



//
// Class 'ProgramHeader'
//


ProgramHeader::ProgramHeader(File *file, int index, unsigned info_offset) :
		file(file),
		index(index)
{
	// Initialize
	this->file = file;
	this->index = index;

	// Read program header
	info = (Elf32_Phdr *) (file->getBuffer() + info_offset);
	if (info_offset < 0 || info_offset + sizeof(Elf32_Phdr) >
			file->getSize())
		throw Error(file->getPath(), "Invalid position for program "
				"header");

	// File content
	size = info->p_filesz;
	buffer = file->getBuffer() + info->p_offset;
}


void ProgramHeader::getStream(std::istringstream &stream, unsigned int offset,
		unsigned int size) const
{
	// Check valid offset/size
	if (offset + size > this->size)
		throw Error(file->getPath(), "Invalid offset/size");

	// Set substream
	std::stringbuf *buf = stream.rdbuf();
	buf->pubsetbuf(const_cast<char *>(buffer) + offset, size);
}




//
// Class 'Symbol'
//

Symbol::Symbol(File *file, Section *section, unsigned int pos)
{
	// Initialize
	this->file = file;

	// Read symbol
	info = (Elf32_Sym *) (section->getBuffer() + pos);
	if (pos < 0 || pos + sizeof(Elf32_Sym) > section->getSize())
		throw Error(file->getPath(), "Invalid position for symbol");

	// Get section with symbol name
	unsigned name_section_index = section->getLink();
	Section *name_section = file->getSection(name_section_index);
	if (!name_section)
		throw Error(file->getPath(), "Invalid index for symbol name "
				"section");

	// Get symbol name
	if (info->st_name >= name_section->getSize())
		throw Error(file->getPath(), "Invalid symbol name offset");
	name = name_section->getBuffer() + info->st_name;

	// Get section in 'st_shndx'
	this->section = file->getSection(info->st_shndx);

	// If symbol points to a valid region of the section, set
	// variable buffer.
	buffer = nullptr;
	if (this->section && info->st_value + info->st_size <=
			this->section->getSize())
		buffer = this->section->getBuffer() + info->st_value;
}


bool Symbol::Compare(const std::unique_ptr<Symbol>& a,
		const std::unique_ptr<Symbol>& b)
{
	int bind_a;
	int bind_b;

	// Sort by value
	if (a->info->st_value < b->info->st_value)
		return true;
	else if (a->info->st_value > b->info->st_value)
		return false;

	// Sort symbol with the same address as per their
	// ST_BIND field in st_info (bits 4 to 8)
	bind_a = ELF32_ST_BIND(a->info->st_info);
	bind_b = ELF32_ST_BIND(b->info->st_info);
	if (bind_a < bind_b)
		return true;
	else if (bind_a > bind_b)
		return false;
	
	// Sort alphabetically
	return a->name < b->name;
}


void Symbol::getStream(std::istringstream &stream, unsigned int offset,
		unsigned int size) const
{
	// Symbol without content
	if (!buffer)
		throw Error(file->getPath(),
				misc::fmt("symbol '%s' does not have any valid content",
				name.c_str()));

	// Check valid offset/size
	if (size > info->st_size)
		throw Error(file->getPath(),
				misc::fmt("symbol '%s': invalid size",
				name.c_str()));

	// Set substream
	std::stringbuf *buf = stream.rdbuf();
	buf->pubsetbuf(const_cast<char *>(buffer) + offset, size);
}



//
// Class 'File'
//

static const char *err_64bit =
	"The ELF file being loaded is a 64-bit file, currently not supported "
	"by Multi2Sim. If you are compiling your own source code on a 64-bit "
	"machine, please use the '-m32' flag in the gcc command-line. If you "
	"get compilation errors related with missing '.h' files, check that "
	"the 32-bit gcc package associated with your Linux distribution is "
	"installed.";


void File::ReadHeader()
{
	// Read ELF header
	info = (Elf32_Ehdr *) buffer.get();
	if (size < sizeof(Elf32_Ehdr))
		throw Error(path, "Invalid ELF file");

	// Check that file is a valid ELF file
	if (strncmp((char *) info->e_ident, ELFMAG, 4))
		throw Error(path, "Invalid ELF file");

	// Check that ELF file is a 32-bit object
	if (info->e_ident[EI_CLASS] == ELFCLASS64)
		throw Error(path, misc::fmt(
				"64-bit ELF files not supported\n\n%s",
				err_64bit));
}


void File::ReadSections()
{
	// Check section size and number
	if (!info->e_shnum || info->e_shentsize != sizeof(Elf32_Shdr))
		throw Error(path, misc::fmt("Number of sections is 0 or "
				"section header size is not %d",
				(int) sizeof(Elf32_Shdr)));

	// Read section headers
	for (int i = 0; i < info->e_shnum; i++)
		sections.emplace_back(misc::new_unique<Section>(
				this,
				i,
				info->e_shoff + i * info->e_shentsize));

	// Check string table index
	if (info->e_shstrndx >= info->e_shnum)
		throw Error(path, "Invalid string table index");
	
	// Read string table
	string_table = sections[info->e_shstrndx].get();
	if (string_table->getType() != 3)
		throw Error(path, "Invalid string table type");

	// Read section names
	for (auto &section : sections)
		section->setName(string_table->getBuffer() +
				section->getNameOffset());
}


void File::ReadProgramHeaders()
{
	// Nothing if there are no program headers. Don't even check if the
	// program header size is the right one, it could be 0 in this case.
	if (!info->e_phnum)
		return;
	
	// Check program header size
	if (info->e_phentsize != sizeof(Elf32_Phdr))
		throw Error(path, misc::fmt(
				"Program header size %d (should be %d)",
				info->e_phentsize, (int) sizeof(Elf32_Phdr)));
	
	// Read program headers
	for (int i = 0; i < info->e_phnum; i++)
		program_headers.emplace_back(misc::new_unique<ProgramHeader>(
				this,
				i,
				info->e_phoff + i * info->e_phentsize));
}


void File::ReadSymbols()
{
	// Load symbols from sections
	for (auto &section : sections)
	{
		// Ignore section that don't represent symbol tables
		if (section->getType() != 2 && section->getType() != 11)
			continue;

		// Read symbol table
		int num_symbols = section->getSize() / sizeof(Elf32_Sym);
		for (int i = 0; i < num_symbols; i++)
		{
			// Create symbol in symbol list
			symbols.emplace_back(misc::new_unique<Symbol>(
					this,
					section.get(),
					i * sizeof(Elf32_Sym)));

			// Discard empty symbol
			Symbol *symbol = symbols.back().get();
			if (symbol->getName().empty())
				symbols.pop_back();
		}
	}

	// Sort
	sort(symbols.begin(), symbols.end(), Symbol::Compare);
}


File::File(const std::string &path, bool read_content) :
		path(path)
{
	// Open file
	std::ifstream f(path);
	if (!f)
		throw Error(path, "Cannot open file");

	// Get file size
	f.seekg(0, std::ios_base::end);
	size = f.tellg();
	f.seekg(0, std::ios_base::beg);

	// Check that size is at least equal to header size
	if (size < sizeof(Elf32_Ehdr))
		throw Error(path, "Invalid ELF file");

	// Load entire file into buffer and close
	buffer = misc::new_unique_array<char>(size);
	f.read(buffer.get(), size);
	f.close();

	// Make string stream point to buffer
	std::stringbuf *buf = stream.rdbuf();
	buf->pubsetbuf(buffer.get(), size);

	// Read ELF header
	ReadHeader();

	// Read content
	if (read_content)
	{
		ReadSections();
		ReadProgramHeaders();
		ReadSymbols();
	}
}


File::File(const char *buffer, unsigned size, bool read_content)
{
	// Initialize
	path = "<anonymous>";

	// Check that size is at least equal to header size
	if (size < sizeof(Elf32_Ehdr))
		throw Error(path, "Invalid ELF file");

	// Copy buffer
	this->size = size;
	this->buffer = misc::new_unique_array<char>(size);
	memcpy(this->buffer.get(), buffer, size);

	// Make string stream point to buffer
	std::stringbuf *buf = stream.rdbuf();
	buf->pubsetbuf(this->buffer.get(), size);

	// Read ELF header
	ReadHeader();

	// Read content
	if (read_content)
	{
		ReadSections();
		ReadProgramHeaders();
		ReadSymbols();
	}
}


std::ostream &operator<<(std::ostream &os, const File &file)
{
	// Header
	os << "ELF header:\n";
	os << "  ehdr.e_ident: EI_CLASS=" << (int) file.info->e_ident[5] <<
			", EI_DATA=" << (int) file.info->e_ident[5] <<
			", EI_VERSION=" << (int) file.info->e_ident[6] << "\n";
	os << "  ehdr.e_type: " << file.info->e_type << "\n";
	os << "  ehdr.e_machine: " << file.info->e_machine << "\n";
	os << misc::fmt("  ehdr.e_entry: 0x%x\n", file.info->e_entry);
	os << "  ehdr.e_phoff: " << file.info->e_phoff << "\n";
	os << "  ehdr.e_shoff: " << file.info->e_shoff << "\n";
	os << "  ehdr.e_phentsize: " << file.info->e_phentsize << "\n";
	os << "  ehdr.e_phnum: " << file.info->e_phnum << "\n";
	os << "  ehdr.e_shentsize: " << file.info->e_shentsize << "\n";
	os << "  ehdr.e_shnum: " << file.info->e_shnum << "\n";
	os << "  ehdr.e_shstrndx: " << file.info->e_shstrndx << "\n";
	os << '\n';

	// Dump section headers
	os << "Section headers:\n";
	os << "  [Nr] type flags addr     offset        size     link name\n";
	os << std::string(80, '-') << '\n';
	for (auto &section : file.sections)
	{
		os << misc::fmt("  [%2d]", section->getIndex());
		os << misc::fmt("%4d ", section->getType());
		os << misc::fmt("%5x ", section->getFlags());
		os << misc::fmt("%08x ", section->getAddr());
		os << misc::fmt("%08x ", section->getOffset());
		os << misc::fmt("%9x ", section->getSize());
		os << misc::fmt("%8d ", section->getLink());
		os << section->getName();
		os << '\n';
	}
	os << '\n';
	
	// Dump program headers
	os << "Program headers:\n";
	os << "idx type       offset   vaddr    paddr     "
			<< "filesz     memsz  flags align\n";
	os << std::string(80, '-') << '\n';
	for (auto &ph : file.program_headers)
	{
		os << misc::fmt("%3d ", ph->getIndex());
		os << misc::fmt("%8x ", ph->getType());
		os << misc::fmt("%8x ", ph->getOffset());
		os << misc::fmt("%8x ", ph->getVaddr());
		os << misc::fmt("%8x ", ph->getPaddr());
		os << misc::fmt("%9u ", ph->getFilesz());
		os << misc::fmt("%9u ", ph->getMemsz());
		os << misc::fmt("%6u ", ph->getFlags());
		os << ph->getAlign() << ' ';
		os << '\n';
	}
	os << '\n';

	// Dump
	os << "Symbol table:\n";
	os << misc::fmt("%-40s %-15s %-12s %-12s %-10s %-10s",
			"name", "section", "value", "size", "info", "other");
	os << std::string(80, '-') << '\n';
	for (auto &symbol : file.symbols)
	{
		// Symbol name
		os << misc::fmt("%-40s ", symbol->getName().c_str());

		// Print section
		Section *section = symbol->getSection();
		if (section)
			os << misc::fmt("%-15s ", section->getName().c_str());
		else
			os << misc::fmt("%-15d ", symbol->getShndx());

		// Rest
		os << misc::fmt("%-10x ", symbol->getValue());
		os << misc::fmt("%-12u ", symbol->getSize());
		os << misc::fmt("%-10u ", symbol->getInfo());
		os << misc::fmt("%-10u ", symbol->getOther());
		os << '\n';
	}
	os << '\n';

	// Done
	return os;
}


Section *File::getSection(const std::string &name) const
{
	// Search
	for (auto &section : sections)
		if (section->getName() == name)
			return section.get();
	
	// Not found
	return nullptr;
}


Symbol *File::getSymbol(const std::string &name) const
{
	// Search
	for (auto &symbol : symbols)
		if (symbol->getName() == name)
			return symbol.get();
	
	// Not found
	return nullptr;
}


void File::getStream(std::istringstream &stream, unsigned int offset,
		unsigned int size) const
{
	// Check valid offset/size
	if (offset + size > this->size)
		throw Error(path, "Invalid offset and/or size");

	// Set substream
	std::stringbuf *buf = stream.rdbuf();
	buf->pubsetbuf(buffer.get() + offset, size);
}


Symbol *File::getSymbolByAddress(unsigned int address) const
{
	unsigned int offset;
	return getSymbolByAddress(address, offset);
}


Symbol *File::getSymbolByAddress(unsigned int address,
		unsigned int &offset) const
{
	// Empty symbol table
	if (!symbols.size())
		return nullptr;

	// All symbols in the table have a higher address
	if (address < symbols[0]->getValue())
		return nullptr;

	// Binary search
	int min = 0;
	int max = symbols.size();
	while (min + 1 < max)
	{
		int mid = (max + min) / 2;
		Symbol *symbol = symbols[mid].get();
		if (symbol->getValue() > address)
		{
			max = mid;
		}
		else if (symbol->getValue() < address)
		{
			min = mid;
		}
		else
		{
			min = mid;
			break;
		}
	}

	// Invalid symbol
	Symbol *symbol = symbols[min].get();
	if (!symbol->getValue())
		return nullptr;

	// Go backwards to find first symbol with that address
	for (;;)
	{
		// One symbol before
		min--;
		if (min < 0)
			break;

		// If address is lower, stop
		Symbol *prev_symbol = symbols[min].get();
		if (prev_symbol->getValue() != symbol->getValue())
			break;

		// Take symbol if it has global/local/weak binding
		if (ELF32_ST_BIND(prev_symbol->getInfo()) < 3)
			symbol = prev_symbol;
	}

	// Return the symbol and its address
	offset = address - symbol->getValue();
	return symbol;
}


} // namespace ELFReader

