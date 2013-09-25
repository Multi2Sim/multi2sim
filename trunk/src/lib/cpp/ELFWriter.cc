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

#include "ELFWriter.h"
#include "Misc.h"


using namespace ELFWriter;
using namespace Misc;
using namespace std;


/*
 * Class 'Buffer'
 */

Buffer::Buffer(File *file, unsigned int index)
{
	this->file = file;
	this->index = index;
}


void Buffer::Dump(ostream& os)
{
	unsigned int i;
	unsigned int j;
	unsigned char c;
	string s;
	
	/* Empty buffer */
	s = stream.str();
	if (!s.length())
	{
		os << "\nBuffer is empty\n\n";
		return;
	}

	/* Print buffer */
	for (i = 0; i < s.length(); i += 16)
	{
		/* Print offset */
		os << setw(8) << setfill('0') << hex << i << ' '
				<< setfill(' ') << dec;

		/* Print hex bytes */
		for (j = 0; j < 16; j++)
		{
			/* Space every 8 bytes */
			if (!(j % 8))
				os << ' ';

			/* Print hex values */
			if (i + j < s.length())
			{
				c = s[i + j];
				os << setw(2) << setfill('0') << hex << (int) c
						<< ' ' << setfill(' ') << dec;
			}
			else
			{
				os << "   ";
			}
		}

		/* Print character bytes */
		os << " |";
		for (j = 0; j < 16; j++)
		{
			/* End of buffer reached */
			if (i + j >= s.length())
				break;

			/* Print character */
			c = s[i + j];
			if (c < 32 || c > 127)
				c = '.';
			os << c;
		}
		os << "|\n";
	}
	
	os << "\n\n";
}




/*
 * Class 'Section'
 */

Section::Section(File *file, std::string name, Buffer *first_buffer,
		Buffer *last_buffer, unsigned int index)
{
	this->file = file;
	this->name = name;
	this->first_buffer = first_buffer;
	this->last_buffer = last_buffer;
	this->index = index;
	info = { 0 };

	/* Check ordered indices */
	if (last_buffer->GetIndex() < first_buffer->GetIndex())
		fatal("%s: invalid buffer order", __FUNCTION__);
}


void Section::Dump(std::ostream& os)
{
	Buffer *buffer;

	int first;
	int last;
	int i;

	/* Dump section */
	first = first_buffer->GetIndex();
	last = last_buffer->GetIndex();
	os << "Dumping section '" << name << "' (buffers " << first <<
			last << ")\n";
	for (i = first; i <= last; i++)
	{
		buffer = file->GetBuffer(i);
		os << "  Dumping buffer " << i << ":\n";
		buffer->Dump(os);
	}
}




/*
 * Class 'Segment'
 */

Segment::Segment(File *file, std::string name, Buffer *first_buffer,
		Buffer *last_buffer, unsigned int index)
{
	this->file = file;
	this->name = name;
	this->first_buffer = first_buffer;
	this->last_buffer = last_buffer;
	this->index = index;
	info = { 0 };
}


void Segment::Dump(std::ostream& os)
{
	Buffer *buffer;

	int first;
	int last;
	int i;

	/* Dump section */
	first = first_buffer->GetIndex();
	last = last_buffer->GetIndex();
	os << "Dumping segment '" << name << "' (buffers " << first <<
			last << ")\n";
	for (i = first; i <= last; i++)
	{
		buffer = file->GetBuffer(i);
		os << "  Dumping buffer " << i << ":\n";
		buffer->Dump(os);
	}
}



/*
 * Class 'Symbol'
 */

Symbol::Symbol(string name)
{
	this->name = name;
	info = { 0 };
}




/*
 * Class 'SymbolTable'
 */

SymbolTable::SymbolTable(File *file, string symtab, string strtab)
{
	this->file = file;

	/* Create associated buffers and sections */
	symtab_buffer = file->NewBuffer();
	strtab_buffer = file->NewBuffer();
	symtab_section = file->NewSection(symtab, symtab_buffer, symtab_buffer);
	strtab_section = file->NewSection(strtab, strtab_buffer, strtab_buffer);
	
	/* Set symtab and strtab properties */
	symtab_section->SetType(SHT_SYMTAB);
	strtab_section->SetType(SHT_STRTAB);
	symtab_section->SetEntsize(sizeof(Elf32_Sym));
	symtab_section->SetLink(strtab_section->GetIndex());

	/* Add a null symbol */
	NewSymbol("");
}


SymbolTable::~SymbolTable()
{
	/* Free all symbols */
	for (auto it = symbols.begin(); it != symbols.end(); ++it)
		delete *it;
}


Symbol *SymbolTable::NewSymbol(std::string name)
{
	Symbol *symbol;

	/* Create symbol */
	symbol = new Symbol(name);
	symbols.push_back(symbol);
	return symbol;
}


void SymbolTable::Generate()
{
	Symbol *symbol;

	/* Populate symtab and strtab buffers */
	symtab_buffer->Clear();
	strtab_buffer->Clear();
	for (auto it = symbols.begin(); it != symbols.end(); ++it)
	{
		symbol = *it;

		/* Update offset for symbol name, pointing to the current end of
		 * the strtab buffer. */
		symbol->info.st_name = strtab_buffer->Tell();

		/* Write symbol into symtab buffer and symbol name into the
		 * strtab buffer */
		symtab_buffer->Write((char *) &symbol->info, sizeof(Elf32_Sym));
		strtab_buffer->Write(symbol->name.c_str(),
				symbol->name.length() + 1);
	}
}



/*
 * Class 'File'
 */


File::File(string path)
{
	Buffer *null_buffer;
	Section *null_section;

	Buffer *shstrtab_buffer;
	Section *shstrtab_section;

	/* Initialize */
	this->path = path;
	memset(&info, 0, sizeof info);

	/* Create null section */
	null_buffer = NewBuffer();
	null_section = NewSection("", null_buffer, null_buffer);

	/* Create section string table */
	shstrtab_buffer = NewBuffer();
	shstrtab_section = NewSection(".shstrtab", shstrtab_buffer,
			shstrtab_buffer);
	shstrtab_section->info.sh_name = 1;
	shstrtab_section->info.sh_type = SHT_STRTAB;
	
	/* Write null section and shstrtab names into buffer */
	shstrtab_buffer->Write(null_section->name.c_str(),
			null_section->name.length() + 1);
	shstrtab_buffer->Write(shstrtab_section->name.c_str(),
			shstrtab_section->name.length() + 1);
}


File::~File()
{
	/* Free buffers */
	for (auto it = buffers.begin(); it != buffers.end(); ++it)
		delete *it;

	/* Free sections */
	for (auto it = sections.begin(); it != sections.end(); ++it)
		delete *it;

	/* Free segments */
	for (auto it = segments.begin(); it != segments.end(); ++it)
		delete *it;

	/* Free symbol tables */
	for (auto it = symbol_tables.begin(); it != symbol_tables.end(); ++it)
		delete *it;
}


Buffer *ELFWriter::File::NewBuffer()
{
	Buffer *buffer = new Buffer(this, buffers.size());
	buffers.push_back(buffer);
	return buffer;
}


Segment *File::NewSegment(string name, Buffer *first, Buffer *last)
{
	Segment *segment = new Segment(this, name, first, last,
			segments.size());
	segments.push_back(segment);
	return segment;
}


Section *File::NewSection(string name, Buffer *first, Buffer *last)
{
	Buffer *shstrtab_buffer;
	Section *section;

	/* Add to list and set index */
	section = new Section(this, name, first, last, sections.size());
	sections.push_back(section);

	/* Store section name */
	if (sections.size() > 2)
	{
		shstrtab_buffer = buffers[1];
		section->info.sh_name = shstrtab_buffer->Tell();
		shstrtab_buffer->Write(name.c_str(), name.length() + 1);
	}

	/* Return section */
	return section;
}


SymbolTable *File::NewSymbolTable(string symtab, string strtab)
{
	SymbolTable *symbol_table = new SymbolTable(this, symtab, strtab);
	symbol_tables.push_back(symbol_table);
	return symbol_table;
}


void File::Generate(ostream& os)
{
	Section *section;
	Segment *segment;
	Buffer *buffer;

	unsigned int phtab_size;
	unsigned int shtab_size;
	unsigned int buf_offset;

	unsigned int i;
	unsigned int j;

	/* ELF Magic Characters */
	info.e_ident[0] = ELFMAG0;
	info.e_ident[1] = ELFMAG1;
	info.e_ident[2] = ELFMAG2;
	info.e_ident[3] = ELFMAG3;

	/* Set remaining e_ident properties - e_ident[7-15] is padding */
	info.e_ident[4] = ELFCLASS32;
	info.e_ident[5] = ELFDATA2LSB;
	info.e_ident[6] = EV_CURRENT;
	info.e_ident[16] = EI_NIDENT;
	info.e_type = ET_EXEC;
	
	/* Set ELF Header Properties for sections. Use elf.h structs for
	 * shentsize and ehsize */
	info.e_shentsize = sizeof(Elf32_Shdr);
	info.e_shnum = sections.size();
	info.e_shstrndx = 1;
	info.e_ehsize = sizeof(Elf32_Ehdr);

	/* Set ELF Header Properties for segments - If there are no segments,
	 * even the phentsize should be zero */
	if (segments.size())
	{	
		info.e_phoff = sizeof(Elf32_Ehdr);
		info.e_phnum = segments.size();
		info.e_phentsize = sizeof(Elf32_Phdr);
	}
	
	/* Calculate Program Header and Section Header table size - Actual
	 * header tables are created at the end */
	phtab_size = sizeof(Elf32_Phdr) * segments.size();
	shtab_size = sizeof(Elf32_Shdr) * sections.size();

	/* Find segment offsets */
	for (auto it = segments.begin(); it != segments.end(); ++it)
	{
		segment = *it;
		buf_offset = 0;
		for (j = 0; j < segment->first_buffer->index; j++)
		{
			buffer = buffers[j];
			buf_offset += buffer->Tell();
		}
		
		/* Add up all offsets to find segment offset */
		segment->info.p_offset = sizeof(Elf32_Ehdr) +
				phtab_size + buf_offset;
	}
		
	/* Find actual segment sizes */
	for (auto it = segments.begin(); it != segments.end(); ++it)
	{
		segment = *it;
		for (j = segment->first_buffer->index;
				j <= segment->last_buffer->index; j++)
		{
			buffer = buffers[j];
			segment->info.p_filesz += buffer->Tell();
			if (segment->info.p_type == PT_LOAD)
				segment->info.p_memsz += buffer->Tell();
		}
	}
	
	/* Find section offsets */
	for (auto it = sections.begin(); it != sections.end(); ++it)
	{
		section = *it;
		buf_offset = 0;
		for (j = 0; j < section->first_buffer->index; j++)
		{
			buffer = buffers[j];
			buf_offset += buffer->Tell();
		}
		section->info.sh_offset = sizeof(Elf32_Ehdr) +
				phtab_size + buf_offset;
	}
	
	/* Null section should not have an offset */
	section = sections[0];
	section->info.sh_offset = 0;
		
	/* Find actual section sizes */
	for (auto it = sections.begin(); it != sections.end(); ++it)
	{
		section = *it;
		for (j = section->first_buffer->index;
				j <= section->last_buffer->index; j++)
		{
			buffer = buffers[j];
			section->info.sh_size += buffer->Tell();
		}
	}

	/* Create Section Header Table */
	Elf32_Shdr shtab[sections.size()];
	for (i = 0; i < sections.size(); i++)
		shtab[i] = sections[i]->info;

	/* Create program header table */
	Elf32_Phdr phtab[segments.size()];
	for (i = 0; i < segments.size(); i++)
		phtab[i] = segments[i]->info;
	
	/* Calculate total size of buffers. Needed to calculate offset 
	 * for section header table. Section header table is typically 
	 * at the end (though not required). */
	buf_offset = 0;
	for (i = 0; i < buffers.size(); i++)
		buf_offset += buffers[i]->Tell();
	info.e_shoff = sizeof(Elf32_Ehdr) + phtab_size + buf_offset;
	
	/* Write headers, sections, etc. into buffer */
	os.write((char *) &info, sizeof(Elf32_Ehdr));
	os.write((char *) &phtab, phtab_size);
	for (i = 0; i < buffers.size(); i++)
		os << buffers[i]->GetStream().rdbuf();
	os.write((char *) &shtab, shtab_size);
}

