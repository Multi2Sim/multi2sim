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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>

#include "array.h"
#include "elf-writer.h"
#include "list.h"
#include "string.h"


/*
 * Class 'ELFWriterBuffer'
 */

#define ELF_WRITER_BUFFER_SIZE  (1<<10)

void ELFWriterBufferCreate(ELFWriterBuffer *self)
{
	/* Initialize */
	self->index = -1;
	self->alloc_size = ELF_WRITER_BUFFER_SIZE;
	self->ptr = xcalloc(1, ELF_WRITER_BUFFER_SIZE);

	/* Virtual functions */
	asObject(self)->Dump = ELFWriterBufferDump;
}


void ELFWriterBufferDestroy(ELFWriterBuffer *self)
{
	free(self->ptr);
}


void ELFWriterBufferDump(Object *self, FILE *f)
{
	ELFWriterBuffer *buffer = asELFWriterBuffer(self);
	int i;
	int j;
	unsigned char c;

	/* Empty buffer */
	if (!buffer->size)
	{
		fprintf (f, "\nBuffer is empty\n\n");
		return;
	}

	/* Print buffer */
	for (i = 0; i < buffer->size; i += 16)
	{
		/* Print offset */
		fprintf(f, "%08x ", i);

		/* Print hex bytes */
		for (j = 0; j < 16; j++)
		{
			/* Space every 8 bytes */
			if (!(j % 8))
				fprintf(f, " ");

			/* Print hex values */
			if (i + j < buffer->size)
			{
				c = * (unsigned char *) (buffer->ptr + i + j);
				fprintf(f, "%02x ", c);
			}
			else
			{
				fprintf(f, "   ");
			}
		}

		/* Print character bytes */
		fprintf(f, " |");
		for (j = 0; j < 16; j++)
		{
			/* End of buffer reached */
			if (i + j >= buffer->size)
				break;

			/* Print character */
			c = * (unsigned char *) (buffer->ptr + i + j);
			fprintf(f, "%c", IN_RANGE(c, 32, 127) ? c : '.');
		}
		fprintf(f, "|\n");
	}
	
	fprintf(f, "\n\n");
}


void ELFWriterBufferWrite(ELFWriterBuffer *self, void *ptr, unsigned int size)
{
	int new_size;

	/* Grow allocated buffer if new content exceeds the current size */
	new_size = MAX(self->offset + size, self->size);
	while (new_size > self->alloc_size)
	{
		self->ptr = xrealloc(self->ptr, self->alloc_size * 2);
		self->alloc_size = self->alloc_size * 2;
		memset(self->ptr + self->size, 0, self->alloc_size - self->size);
	}

	/* Write content to buffer */
	memcpy(self->ptr + self->offset, ptr, size);
	self->size = new_size;
	self->offset = self->offset + size;
}


unsigned int ELFWriterBufferRead(ELFWriterBuffer *self, void *ptr, unsigned int size)
{
	/* Truncate size to available data after current offset */
	size = MAX(size, self->size - self->offset);

	/* Read content */
	memcpy(ptr, self->ptr + self->offset, size);
	self->offset += size;

	/* Return the number of read bytes */
	return size;
}


unsigned int ELFWriterBufferSeek(ELFWriterBuffer *self, unsigned int offset)
{
	self->offset = MAX(offset, self->size);
	return self->offset;
}


int ELFWriterBufferWriteToFile(ELFWriterBuffer *self, FILE *f)
{
	return fwrite(self->ptr, 1, self->size, f);
}


int ELFWriterBufferReadFromFile(ELFWriterBuffer *self, FILE *f)
{
	long file_size;
	void *buf;
	
	/* Get file size */
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (file_size < 0)
		return 0;

	/* Read file */
	buf = xmalloc(file_size);
	file_size = fread(buf, 1, file_size, f);

	/* Write into ELF buffer */
	ELFWriterBufferWrite(self, buf, file_size);

	/* Free temporary buffer */
	free(buf);
	return file_size;
}




/*
 * Class 'ELFWriterSection'
 */

void ELFWriterSectionCreate(ELFWriterSection *self, char *name,
		ELFWriterBuffer *first_buffer,
		ELFWriterBuffer *last_buffer)
{
	/* Initialize */
	self->name = new(String, name);
	self->first_buffer = first_buffer;
	self->last_buffer = last_buffer;
	self->index = -1;

	/* Virtual functions */
	asObject(self)->Dump = ELFWriterSectionDump;
}

		
void ELFWriterSectionDestroy(ELFWriterSection *self)
{
	delete(self->name);
}


void ELFWriterSectionDump(Object *self, FILE *f)
{
	ELFWriterSection *section = asELFWriterSection(self);
	ELFWriter *writer;
	ELFWriterBuffer *buffer;

	int first_index;
	int last_index;
	int i;

	/* Get ELF writer */
	writer = section->writer;
	if (!writer)
		panic("%s: section not added to an ELF writer object",
				__FUNCTION__);

	/* Buffers must have been inserted to the ELF file */
	first_index = section->first_buffer->index;
	last_index = section->last_buffer->index;
	if (first_index < 0 || last_index < 0)
		fatal("%s: buffers in section not added to ELF file",
				__FUNCTION__);

	/* Dump section */
	fprintf(f, "Dumping Section: %s\n", section->name->text);
	for (i = first_index; i <= last_index; i++)
	{
		fprintf(f, "  Dumping Buffer %d:\n", i - first_index + 1);
		buffer = asELFWriterBuffer(ArrayGet(writer->buffer_array, i));
		ELFWriterBufferDump(asObject(buffer), f);
	}
}




/*
 * Class 'ELFWriterSegment'
 */

void ELFWriterSegmentCreate(ELFWriterSegment *self, char *name,
		ELFWriterBuffer *first_buffer, ELFWriterBuffer *last_buffer)
{
	/* Initialize */
	self->name = new(String, name);
	self->first_buffer = first_buffer;
	self->last_buffer = last_buffer;
	self->index = -1;

	/* Virtual functions */
	asObject(self)->Dump = ELFWriterSegmentDump;
}


void ELFWriterSegmentDestroy(ELFWriterSegment *self)
{
	delete(self->name);
}


void ELFWriterSegmentDump(Object *self, FILE *f)
{
	ELFWriterSegment *segment = asELFWriterSegment(self);
	ELFWriterBuffer *buffer;
	ELFWriter *writer;

	int first_index;
	int last_index;
	int i;

	/* Get writer object */
	writer = segment->writer;
	if (!writer)
		panic("%s: segment not added to an ELF writer object",
				__FUNCTION__);

	/* Buffers must have been inserted to the ELF file */
	first_index = segment->first_buffer->index;
	last_index = segment->last_buffer->index;
	if (first_index < 0 || last_index < 0)
		fatal("%s: buffers in segment not added to ELF file",
				__FUNCTION__);

	/* Dump segment */
	fprintf(f, "Dumping Segment: %s\n", segment->name->text);
	for (i = first_index; i <= last_index; i++)
	{
		fprintf(f, "  Dumping Buffer %d:\n", i - first_index + 1);
		buffer = asELFWriterBuffer(ArrayGet(writer->buffer_array, i));
		ELFWriterBufferDump(asObject(buffer), f);
	}
}




/*
 * Class 'ELFWriterSymbol'
 */

void ELFWriterSymbolCreate(ELFWriterSymbol *self, char *name)
{
	self->name = new(String, name);
}


void ELFWriterSymbolDestroy(ELFWriterSymbol *self)
{
	delete(self->name);
}




/*
 * Class 'ELFWriterSymbolTable'
 */

void ELFWriterSymbolTableCreate(ELFWriterSymbolTable *self, char *symtab_name,
		char *strtab_name)
{
	ELFWriterSymbol *null_symbol;

	/* Initialize */
	self->symbol_table_buffer = new(ELFWriterBuffer);
	self->string_table_buffer = new(ELFWriterBuffer);
	self->symbol_list = new(List);

	/* Create one section for the symbol table, another for the string
	 * table, and associated them with the created buffers. */
	self->symbol_table_section = new(ELFWriterSection, symtab_name,
			self->symbol_table_buffer, self->symbol_table_buffer);
	self->string_table_section = new(ELFWriterSection, strtab_name,
			self->string_table_buffer, self->string_table_buffer);
	
	/* Set symtab and strtab properties */
	self->symbol_table_section->header.sh_type = SHT_SYMTAB;
	self->string_table_section->header.sh_type = SHT_STRTAB;
	self->symbol_table_section->header.sh_entsize = sizeof(Elf32_Sym);
	
	/* Set first entry of list to be a null symbol */
	null_symbol = new(ELFWriterSymbol, "");
	ELFWriterSymbolTableAdd(self, null_symbol);
}


void ELFWriterSymbolTableDestroy(ELFWriterSymbolTable *self)
{
	ListDeleteObjects(self->symbol_list);
	delete(self->symbol_list);
}


void ELFWriterSymbolTableAdd(ELFWriterSymbolTable *self,
		ELFWriterSymbol *symbol)
{
	/* Add symbol to symbol list */
	ListAdd(self->symbol_list, asObject(symbol));
		
	/* Update offset for symbol name, pointing to the current end of the
	 * strtab buffer. */
	symbol->symbol.st_name = self->string_table_buffer->size;

	/* Write symbol into symtab buffer and symbol name into the strtab buffer */
	ELFWriterBufferWrite(self->symbol_table_buffer, &symbol->symbol,
			sizeof(Elf32_Sym));
	ELFWriterBufferWrite(self->string_table_buffer, symbol->name->text,
			symbol->name->length + 1);
}



/*
 * Class 'ELFWriter'
 */


void ELFWriterCreate(ELFWriter *self)
{
	ELFWriterBuffer *null_buffer;
	ELFWriterBuffer *shstrtab;
	ELFWriterSection *null_section;
	ELFWriterSection *shstrtab_section;

	/* Initialize */
	self->section_array = new(Array);
	self->segment_array = new(Array);
	self->symbol_table_list = new(List);
	self->buffer_array = new(Array);

	/* Create null buffer for null section */
	null_buffer = new(ELFWriterBuffer);
	ELFWriterAddBuffer(self, null_buffer);

	/* Create null section - will be first entry in shstrtab */
	null_section = new(ELFWriterSection, "", null_buffer, null_buffer);
	ELFWriterAddSection(self, null_section);

	/* Create buffer for string table */
	shstrtab = new(ELFWriterBuffer);
	ELFWriterAddBuffer(self, shstrtab);

	/* Create section for shstrtab */
	shstrtab_section = new(ELFWriterSection, ".shstrtab", shstrtab, shstrtab);
	shstrtab_section->header.sh_name = 1;
	shstrtab_section->header.sh_type = SHT_STRTAB;
	ELFWriterAddSection(self, shstrtab_section);

	/* Write null section and shstrtab names into buffer */
	ELFWriterBufferWrite(shstrtab, null_section->name->text,
			null_section->name->length + 1);
	ELFWriterBufferWrite(shstrtab, shstrtab_section->name->text,
			shstrtab_section->name->length + 1);
}


void ELFWriterDestroy(ELFWriter *self)
{
	/* Free buffer array */
	ArrayDeleteObjects(self->buffer_array);
	delete(self->buffer_array);

	/* Free section array */
	ArrayDeleteObjects(self->section_array);
	delete(self->section_array);
	
	/* Free segment array */
	ArrayDeleteObjects(self->segment_array);
	delete(self->segment_array);
	
	/* Free symbol table list */
	ListDeleteObjects(self->symbol_table_list);
	delete(self->symbol_table_list);
}


void ELFWriterAddBuffer(ELFWriter *self, ELFWriterBuffer *buffer)
{
	ArrayAdd(self->buffer_array, asObject(buffer));
	buffer->index = self->buffer_array->count - 1;
}


void ELFWriterAddSegment(ELFWriter *self, ELFWriterSegment *segment)
{
	ArrayAdd(self->segment_array, asObject(segment));
	segment->index = self->segment_array->count - 1;
}


void ELFWriterAddSection(ELFWriter *self, ELFWriterSection *section)
{
	ELFWriterBuffer *shstrtab;

	/* Add to list and set index */
	ArrayAdd(self->section_array, asObject(section));
	section->index = self->section_array->count - 1;

	/* Store section name */
	if (self->buffer_array->count > 2)
	{
		shstrtab = asELFWriterBuffer(ArrayGet(self->buffer_array, 1));
		section->header.sh_name = shstrtab->size;
		ELFWriterBufferWrite(shstrtab, section->name->text,
				section->name->length + 1);
	}
}


void ELFWriterAddSymbolTable(ELFWriter *self, ELFWriterSymbolTable *table)
{
	ListAdd(self->symbol_table_list, asObject(table));

	/* Add symtab and strtab buffers and sections from the symbol table into
	 * the ELF self buffer and section list. */
	ELFWriterAddBuffer(self, table->symbol_table_buffer);
	ELFWriterAddBuffer(self, table->string_table_buffer);
	ELFWriterAddSection(self, table->symbol_table_section);
	ELFWriterAddSection(self, table->string_table_section);
	
	/* Now that the sections have acquired an index in the section list, we
	 * can make the symtab section point to the strtab section. */
	table->symbol_table_section->header.sh_link =
			table->string_table_section->index;
}


void ELFWriterGenerate(ELFWriter *self, ELFWriterBuffer *out_buffer)
{
	ELFWriterSection *section;
	ELFWriterSegment *segment;
	ELFWriterBuffer *buffer;
	Elf32_Ehdr *elf_header;

	unsigned int phtab_size;
	unsigned int shtab_size;
	unsigned int buf_offset;

	int i;
	int j;

	/* Set up ELF Header */
	elf_header = &self->header;
	
	/* ELF Magic Characters */
	elf_header->e_ident[0] = ELFMAG0;
	elf_header->e_ident[1] = ELFMAG1;
	elf_header->e_ident[2] = ELFMAG2;
	elf_header->e_ident[3] = ELFMAG3;

	/* Set remaining e_ident properties - e_ident[7-15] is padding */
	elf_header->e_ident[4] = ELFCLASS32;
	elf_header->e_ident[5] = ELFDATA2LSB;
	elf_header->e_ident[6] = EV_CURRENT;
	elf_header->e_ident[16] = EI_NIDENT;
	elf_header->e_type = ET_EXEC;
	
	/* Set ELF Header Properties for sections. Use elf.h structs for
	 * shentsize and ehsize */
	elf_header->e_shentsize = sizeof(Elf32_Shdr);
	elf_header->e_shnum = self->section_array->count;
	elf_header->e_shstrndx = 1;
	elf_header->e_ehsize = sizeof(Elf32_Ehdr);

	/* Set ELF Header Properties for segments - If there are no segments,
	 * even the phentsize should be zero */
	if (self->segment_array->count)
	{	
		elf_header->e_phoff = sizeof(Elf32_Ehdr);
		elf_header->e_phnum = self->segment_array->count;
		elf_header->e_phentsize = sizeof(Elf32_Phdr);
	}
	
	/* Calculate Program Header and Section Header table size - Actual
	 * header tables are created at the end */
	phtab_size = sizeof(Elf32_Phdr) * self->segment_array->count;
	shtab_size = sizeof(Elf32_Shdr) * self->section_array->count;

	/* Find segment offsets */
	ArrayForEach(self->segment_array, segment, ELFWriterSegment)
	{
		buf_offset = 0;
		for (j = 0; j < segment->first_buffer->index; j++)
		{
			buffer = asELFWriterBuffer(ArrayGet(
					self->buffer_array, j));
			buf_offset += buffer->size;
		}
		
		/* Add up all offsets to find segment offset */
		segment->header.p_offset = sizeof(Elf32_Ehdr) +
				phtab_size + buf_offset;
	}
		
	/* Find actual segment sizes */
	ArrayForEach(self->segment_array, segment, ELFWriterSegment)
	{
		for (j = segment->first_buffer->index;
				j <= segment->last_buffer->index; j++)
		{
			buffer = asELFWriterBuffer(ArrayGet(
					self->buffer_array, j));
			segment->header.p_filesz += buffer->size;
			if (segment->header.p_type == PT_LOAD)
				segment->header.p_memsz += buffer->size;
		}
	}
	
	/* Find section offsets */
	ArrayForEach(self->section_array, section, ELFWriterSection)
	{
		buf_offset = 0;
		for (j = 0; j < section->first_buffer->index; j++)
		{
			buffer = asELFWriterBuffer(ArrayGet(
					self->buffer_array, j));
			buf_offset += buffer->size;
		}

		section->header.sh_offset = sizeof(Elf32_Ehdr) +
				phtab_size + buf_offset;
	}
	
	/* Null section should not have an offset */
	section = asELFWriterSection(ArrayGet(self->section_array, 0));
	assert(section);
	section->header.sh_offset = 0;
		
	/* Find actual section sizes */
	ArrayForEach(self->section_array, section, ELFWriterSection)
	{
		for (j = section->first_buffer->index;
				j <= section->last_buffer->index; j++)
		{
			buffer = asELFWriterBuffer(ArrayGet(
					self->buffer_array, j));
			section->header.sh_size += buffer->size;
		}
	}

	/* Create Section Header Table */
	Elf32_Shdr shtab[self->section_array->count];
	
	i = 0;
	/* Enter section headers into section header string table */
	ArrayForEach(self->section_array, section, ELFWriterSection)
	{
		shtab[i] = section->header;
		i++;
	}

	/* Create program header table */
	Elf32_Phdr phtab[self->segment_array->count];
	i = 0;
	ArrayForEach(self->segment_array, segment, ELFWriterSegment)
	{
		phtab[i] = segment->header;
		i++;
	}
	
	/* Calculate total size of buffers. Needed to calculate offset 
	 * for section header table. Section header table is typically 
	 * at the end (though not required ). */
	buf_offset = 0;
	ArrayForEach(self->buffer_array, buffer, ELFWriterBuffer)
		buf_offset += buffer->size;
	self->header.e_shoff = sizeof(Elf32_Ehdr) + phtab_size + buf_offset;
	
	/* Write headers, sections, etc. into buffer */
	ELFWriterBufferWrite(out_buffer, &self->header, sizeof(Elf32_Ehdr));
	ELFWriterBufferWrite(out_buffer, phtab, phtab_size);
	ArrayForEach(self->buffer_array, buffer, ELFWriterBuffer)
		ELFWriterBufferWrite(out_buffer, buffer->ptr, buffer->size);
	ELFWriterBufferWrite(out_buffer, shtab, shtab_size);
}

