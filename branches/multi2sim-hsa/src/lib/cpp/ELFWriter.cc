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

#include <cstring>
#include <iomanip>
#include <fstream>

#include "ELFWriter.h"
#include "Misc.h"
#include "String.h"


namespace ELFWriter
{



//
// Class 'Buffer'
//

size_t Buffer::getSize()
{
	unsigned int pos;
	unsigned int size;

	pos = stream.tellp();
	stream.seekp(0, std::ios_base::end);
	size = stream.tellp();
	stream.seekp(pos, std::ios_base::beg);
	return size;
}


void Buffer::Dump(std::ostream &os) const
{
	// Empty buffer
	std::string s = stream.str();
	if (!s.length())
	{
		os << "\nBuffer is empty\n\n";
		return;
	}

	// Print buffer
	for (unsigned i = 0; i < s.length(); i += 16)
	{
		// Print offset
		os << misc::fmt("%08x ", i);

		// Print hex bytes
		for (int j = 0; j < 16; j++)
		{
			// Space every 8 bytes
			if (!(j % 8))
				os << ' ';

			// Print hex values
			if (i + j < s.length())
			{
				unsigned char c = s[i + j];
				os << misc::fmt("%02x ", (int) c);
			}
			else
			{
				os << "   ";
			}
		}

		// Print character bytes
		os << " |";
		for (int j = 0; j < 16; j++)
		{
			// End of buffer reached
			if (i + j >= s.length())
				break;

			// Print character
			unsigned char c = s[i + j];
			if (c < 32 || c > 127)
				c = '.';
			os << c;
		}
		os << "|\n";
	}
	
	os << "\n\n";
}




//
// Class 'Section'
//

Section::Section(File *file, std::string name, Buffer *first_buffer,
		Buffer *last_buffer, int index)
{
	this->file = file;
	this->name = name;
	this->first_buffer = first_buffer;
	this->last_buffer = last_buffer;
	this->index = index;
	info = { 0 };

	// Check ordered indices
	if (last_buffer->getIndex() < first_buffer->getIndex())
		misc::fatal("%s: invalid buffer order", __FUNCTION__);
}


void Section::Dump(std::ostream &os) const
{
	// Dump section
	int first = first_buffer->getIndex();
	int last = last_buffer->getIndex();
	os << "Dumping section '" << name << "' (buffers " << first <<
			last << ")\n";

	// Dump buffers
	for (int i = first; i <= last; i++)
	{
		Buffer *buffer = file->getBuffer(i);
		os << "  Dumping buffer " << i << ":\n";
		buffer->Dump(os);
	}
}




//
// Class 'Segment'
//

Segment::Segment(File *file, std::string name, Buffer *first_buffer,
		Buffer *last_buffer, int index)
{
	this->file = file;
	this->name = name;
	this->first_buffer = first_buffer;
	this->last_buffer = last_buffer;
	this->index = index;
	info = { 0 };
}


void Segment::Dump(std::ostream &os) const
{
	// Dump segment
	int first = first_buffer->getIndex();
	int last = last_buffer->getIndex();
	os << "Dumping segment '" << name << "' (buffers " << first <<
			last << ")\n";

	// Buffers
	for (int i = first; i <= last; i++)
	{
		Buffer *buffer = file->getBuffer(i);
		os << "  Dumping buffer " << i << ":\n";
		buffer->Dump(os);
	}
}



//
// Class 'SymbolTable'
//

SymbolTable::SymbolTable(File *file, const std::string &symtab,
		const std::string &strtab)
{
	// Initialize
	this->file = file;

	// Create associated buffers and sections
	symtab_buffer = file->newBuffer();
	strtab_buffer = file->newBuffer();
	symtab_section = file->newSection(symtab, symtab_buffer, symtab_buffer);
	strtab_section = file->newSection(strtab, strtab_buffer, strtab_buffer);
	
	// Set symtab and strtab properties
	symtab_section->setType(SHT_SYMTAB);
	strtab_section->setType(SHT_STRTAB);
	symtab_section->setEntSize(sizeof(Elf32_Sym));
	symtab_section->setLink(strtab_section->getIndex());

	// Add a null symbol
	newSymbol("");
}


Symbol *SymbolTable::newSymbol(const std::string &name)
{
	
	symbols.push_back(std::unique_ptr<Symbol>(new Symbol(name)));
	return symbols.back().get();
}


void SymbolTable::Generate()
{
	// Populate symtab and strtab buffers
	symtab_buffer->Clear();
	strtab_buffer->Clear();
	for (auto &symbol : symbols)
	{
		// Update offset for symbol name, pointing to the current end of
		// the strtab buffer. */
		symbol->info.st_name = strtab_buffer->getSize();

		// Write symbol into symtab buffer and symbol name into the
		// strtab buffer */
		symtab_buffer->Write((char *) &symbol->info, sizeof(Elf32_Sym));
		strtab_buffer->Write(symbol->name.c_str(),
				symbol->name.length() + 1);
	}
}



//
// Class 'File'
//


File::File()
{
	// Initialize
	memset(&info, 0, sizeof info);

	// Create null section
	Buffer *null_buffer = newBuffer();
	Section *null_section = newSection("", null_buffer, null_buffer);

	// Create section string table
	Buffer *shstrtab_buffer = newBuffer();
	Section *shstrtab_section = newSection(".shstrtab", shstrtab_buffer,
			shstrtab_buffer);
	shstrtab_section->info.sh_name = 1;
	shstrtab_section->info.sh_type = SHT_STRTAB;
	
	// Write null section and shstrtab names into buffer
	shstrtab_buffer->Write(null_section->name.c_str(),
			null_section->name.length() + 1);
	shstrtab_buffer->Write(shstrtab_section->name.c_str(),
			shstrtab_section->name.length() + 1);
}


Buffer *ELFWriter::File::newBuffer()
{
	buffers.push_back(std::unique_ptr<Buffer>(new Buffer(this,
			buffers.size())));
	return buffers.back().get();
}


Segment *File::newSegment(const std::string &name, Buffer *first, Buffer *last)
{
	segments.push_back(std::unique_ptr<Segment>(new Segment(this, name,
			first, last, segments.size())));
	return segments.back().get();
}


Section *File::newSection(const std::string &name, Buffer *first, Buffer *last)
{
	// Add to list and set index
	sections.push_back(std::unique_ptr<Section>(new Section(this, name,
			first, last, sections.size())));
	Section *section = sections.back().get();

	// Store section name
	if (sections.size() > 2)
	{
		Buffer *shstrtab_buffer = buffers[1].get();
		section->info.sh_name = shstrtab_buffer->getSize();
		shstrtab_buffer->Write(name.c_str(), name.length() + 1);
	}

	// Return section
	return section;
}


SymbolTable *File::newSymbolTable(const std::string &symtab,
		const std::string &strtab)
{
	symbol_tables.push_back(std::unique_ptr<SymbolTable>(new
			SymbolTable(this, symtab, strtab)));
	return symbol_tables.back().get();
}


void File::Generate(std::ostream& os)
{
	// ELF Magic Characters
	info.e_ident[0] = ELFMAG0;
	info.e_ident[1] = ELFMAG1;
	info.e_ident[2] = ELFMAG2;
	info.e_ident[3] = ELFMAG3;

	// Set remaining e_ident properties - e_ident[7-15] is padding
	info.e_ident[4] = ELFCLASS32;
	info.e_ident[5] = ELFDATA2LSB;
	info.e_ident[6] = EV_CURRENT;
	info.e_ident[16] = EI_NIDENT;
	info.e_type = ET_EXEC;
	
	// Set ELF Header Properties for sections. Use elf.h structs for
	// shentsize and ehsize
	info.e_shentsize = sizeof(Elf32_Shdr);
	info.e_shnum = sections.size();
	info.e_shstrndx = 1;
	info.e_ehsize = sizeof(Elf32_Ehdr);

	// Set ELF Header Properties for segments - If there are no segments,
	// even the phentsize should be zero
	if (segments.size())
	{	
		info.e_phoff = sizeof(Elf32_Ehdr);
		info.e_phnum = segments.size();
		info.e_phentsize = sizeof(Elf32_Phdr);
	}
	
	// Calculate Program Header and Section Header table size - Actual
	// header tables are created at the end
	unsigned phtab_size = sizeof(Elf32_Phdr) * segments.size();
	unsigned shtab_size = sizeof(Elf32_Shdr) * sections.size();

	//Write symbols into symbol table buffers
	//for (auto &symtab : symbol_tables)
	//	symtab->Generate();


	// Find segment offsets
	for (auto &segment : segments)
	{
		unsigned buf_offset = 0;
		for (int j = 0; j < segment->first_buffer->index; j++)
			buf_offset += buffers[j]->getSize();
		
		/* Add up all offsets to find segment offset */
		segment->info.p_offset = sizeof(Elf32_Ehdr) +
				phtab_size + buf_offset;
	}
		
	// Find actual segment sizes
	for (auto &segment : segments)
	{
		for (int j = segment->first_buffer->index;
				j <= segment->last_buffer->index; j++)
		{
			segment->info.p_filesz += buffers[j]->getSize();
			if (segment->info.p_type == PT_LOAD)
				segment->info.p_memsz += buffers[j]->getSize();
		}
	}
	
	// Find section offsets
	for (auto &section : sections)
	{
		unsigned buf_offset = 0;
		for (int j = 0; j < section->first_buffer->index; j++)
			buf_offset += buffers[j]->getSize();
		section->info.sh_offset = sizeof(Elf32_Ehdr) +
				phtab_size + buf_offset;
	}
	
	// Null section should not have an offset
	sections[0]->info.sh_offset = 0;
		
	// Find actual section sizes
	for (auto &section : sections)
	{
		for (int j = section->first_buffer->index;
				j <= section->last_buffer->index; j++)
			section->info.sh_size += buffers[j]->getSize();
	}

	// Create Section Header Table
	Elf32_Shdr shtab[sections.size()];
	for (unsigned i = 0; i < sections.size(); i++)
		shtab[i] = sections[i]->info;

	// Create program header table
	Elf32_Phdr phtab[segments.size()];
	for (unsigned i = 0; i < segments.size(); i++)
		phtab[i] = segments[i]->info;
	
	// Calculate total size of buffers. Needed to calculate offset
	// for section header table. Section header table is typically
	// at the end (though not required).
	unsigned buf_offset = 0;
	for (unsigned i = 0; i < buffers.size(); i++)
		buf_offset += buffers[i]->getSize();
	info.e_shoff = sizeof(Elf32_Ehdr) + phtab_size + buf_offset;
	
	// Write headers, sections, etc. into buffer
	os.write((char *) &info, sizeof(Elf32_Ehdr));
	os.write((char *) phtab, phtab_size);
	for (unsigned i = 0; i < buffers.size(); i++)
		os << buffers[i]->getStream().str();
	os.write((char *) shtab, shtab_size);
}


void File::Generate(const std::string &path)
{
	std::ofstream of(path);
	if (!of)
		misc::fatal("%s: cannot write to file", path.c_str());
	Generate(of);
	of.close();
}


}  // namespace ELFWriter

