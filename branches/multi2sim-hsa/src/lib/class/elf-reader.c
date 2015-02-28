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

#include <sys/stat.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "array.h"
#include "elf-reader.h"
#include "list.h"
#include "string.h"


/*
 * Class 'ELFBuffer'
 */

void ELFBufferCreate(ELFBuffer *self, void *ptr, int size)
{
	/* Initialize */
	self->ptr = ptr;
	self->size = size;
}


void ELFBufferDestroy(ELFBuffer *self)
{
}


void ELFBufferDump(Object *self, FILE *f)
{
	ELFBuffer *buffer = asELFBuffer(self);
	size_t size;

	size = fwrite(buffer->ptr, 1, buffer->size, f);
	if (size != buffer->size)
		fatal("%s: couldn't dump ELF buffer content",
				__FUNCTION__);
}


void ELFBufferSeek(ELFBuffer *self, int pos)
{
	if (pos < 0 || pos > self->size)
		fatal("%s: invalid position in ELF buffer (%d)",
				__FUNCTION__, pos);
	self->pos = pos;
}


void *ELFBufferTell(ELFBuffer *self)
{
	return self->ptr + self->pos;
}


int ELFBufferIsEnd(ELFBuffer *self)
{
	return self->pos == self->size;
}


int ELFBufferRead(ELFBuffer *self, void *ptr, int size)
{
	int left;

	/* Truncate size to read */
	left = self->size - self->pos;
	assert(left >= 0);
	if (size > left)
		size = left;

	/* Read bytes */
	if (ptr)
		memcpy(ptr, self->ptr + self->pos, size);

	/* Advance pointer and return read bytes */
	self->pos += size;
	return size;
}


String *ELFBufferReadLine(ELFBuffer *self)
{
	char *left;
	char *right;
	char *end;

	String *string;

	/* Nothing to read */
	if (self->pos == self->size)
		return NULL;

	/* Initialize string boundaries */
	left = right = self->ptr + self->pos;
	end = self->ptr + self->size;

	/* Find end of string */
	while (right < end && *right != '\n')
	{
		right++;
		self->pos++;
	}

	/* Create string */
	string = new_ctor(String, CreateMaxLength, left, right - left);

	/* Skip newline character */
	if (right < end)
		self->pos++;
	
	/* Return new string */
	return string;
}



/*
 * Class 'ELFSection'
 */


void ELFSectionCreate(ELFSection *self)
{
}


void ELFSectionDestroy(ELFSection *self)
{
	if (self->name)
		delete(self->name);
	if (self->buffer)
		delete(self->buffer);
}



/*
 * Class 'ELFProgramHeader'
 */


void ELFProgramHeaderCreate(ELFProgramHeader *self)
{
}


void ELFProgramHeaderDestroy(ELFProgramHeader *self)
{
}



/*
 * Class 'ELFSymbol'
 */

void ELFSymbolCreate(ELFSymbol *self)
{
	/* Virtual functions */
	asObject(self)->Compare = ELFSymbolCompare;
}


void ELFSymbolDestroy(ELFSymbol *self)
{
}


int ELFSymbolCompare(Object *self, Object *object)
{
	ELFSymbol *symbol_a = asELFSymbol(self);
	ELFSymbol *symbol_b = asELFSymbol(object);
	
	int bind_a;
	int bind_b;

	if (symbol_a->sym->st_value < symbol_b->sym->st_value)
		return -1;

	if (symbol_a->sym->st_value > symbol_b->sym->st_value)
		return 1;

	/* Sort symbol with the same address as per their
	 * ST_BIND field in st_info (bits 4 to 8) */
	bind_a = (symbol_a->sym->st_info >> 4) & 0xf;
	bind_b = (symbol_b->sym->st_info >> 4) & 0xf;

	if (bind_a < bind_b)
		return -1;
	else if (bind_a > bind_b)
		return 1;
	else
	{
		/* Sort alphabetically */
		return strcmp(symbol_a->name, symbol_b->name);
	}
}




/*
 * Class 'ELFReader'
 */

static char *elf_reader_err_64bit =
	"\tThe ELF file being loaded is a 64-bit file, currently not supported\n"
	"\tby Multi2Sim. If you are compiling your own source code on a 64-bit\n"
	"\tmachine, please use the '-m32' flag in the gcc command-line. If you\n"
	"\tget compilation errors related with missing '.h' files, check that\n"
	"\tthe 32-bit gcc package associated with your Linux distribution is\n"
	"\tinstalled.\n";

static void __ELFReaderReadHeader(ELFReader *self)
{
	ELFBuffer *buffer;
	Elf32_Ehdr *header;
	int count;

	/* Read ELF header */
	buffer = self->buffer;
	self->header = buffer->ptr;
	header = self->header;
	count = ELFBufferRead(buffer, NULL, sizeof(Elf32_Ehdr));
	if (count < EI_NIDENT)
		fatal("%s: not a valid ELF file\n", self->path->text);

	/* Check magic characters */
	if (header->e_ident[0] != 0x7f
			|| header->e_ident[1] != 'E'
			|| header->e_ident[2] != 'L'
			|| header->e_ident[3] != 'F')
		fatal("%s: not a valid ELF file\n", self->path->text);

	/* Check for 32-bit executable (field e_ident[EI_CLASS]) */
	if (header->e_ident[4] == 2)
		fatal("%s: not supported architecture.\n%s",
			self->path->text, elf_reader_err_64bit);
	
	/* Check that header size is correct */
	if (header->e_ehsize != sizeof(Elf32_Ehdr))
		fatal("%s: invalid ELF header size", self->path->text);

	/* Check endianness */
	if (header->e_ident[5] != 1)
		fatal("%s: ELF file endianness mismatch", self->path->text);

	/* Debug */
	elf_reader_debug("ELF header:\n");
	elf_reader_debug("  ehdr.e_ident: EI_CLASS=%d, EI_DATA=%d, EI_VERSION=%d\n",
			header->e_ident[4], header->e_ident[5], header->e_ident[6]);
	elf_reader_debug("  ehdr.e_type: %d\n", header->e_type);
	elf_reader_debug("  ehdr.e_machine: %u\n", header->e_machine);
	elf_reader_debug("  ehdr.e_entry: 0x%x\n", header->e_entry);
	elf_reader_debug("  ehdr.e_phoff: %u\n", header->e_phoff);
	elf_reader_debug("  ehdr.e_shoff: %u\n", header->e_shoff);
	elf_reader_debug("  ehdr.e_phentsize: %u\n", header->e_phentsize);
	elf_reader_debug("  ehdr.e_phnum: %u\n", header->e_phnum);
	elf_reader_debug("  ehdr.e_shentsize: %u\n", header->e_shentsize);
	elf_reader_debug("  ehdr.e_shnum: %u\n", header->e_shnum);
	elf_reader_debug("  ehdr.e_shstrndx: %u\n",
			(unsigned int) header->e_shstrndx);
	elf_reader_debug("\n");
}


static void ELFReaderReadSections(ELFReader *self)
{
	ELFBuffer *buffer;
	ELFSection *section;
	Elf32_Ehdr *header;

	int count;
	int index;
	int i;

	/* Check section size and number */
	buffer = self->buffer;
	header = self->header;
	if (!header->e_shnum || header->e_shentsize != sizeof(Elf32_Shdr))
		fatal("%s: number of sections is 0 or section size is not %d",
			self->path->text, (int) sizeof(Elf32_Shdr));

	/* Read section headers */
	ELFBufferSeek(buffer, header->e_shoff);
	for (i = 0; i < header->e_shnum; i++)
	{
		/* Create section */
		section = new(ELFSection);
		section->header = ELFBufferTell(buffer);
		section->buffer = new(ELFBuffer, NULL, 0);

		/* Advance buffer */
		count = ELFBufferRead(buffer, NULL, sizeof(Elf32_Shdr));
		if (count < sizeof(Elf32_Shdr))
			fatal("%s: unexpected end of file while reading "
					"section headers", self->path->text);

		/* Get section contents, if section type is not SHT_NOBITS
		 * (sh_type = 8). */
		if (section->header->sh_type != 8)
		{
			section->buffer->ptr = buffer->ptr +
					section->header->sh_offset;
			section->buffer->size = section->header->sh_size;
			section->buffer->pos = 0;
			assert(section->buffer->ptr >= buffer->ptr);
			if (section->buffer->ptr + section->buffer->size >
					buffer->ptr + buffer->size)
				fatal("section %d out of the ELF boundaries "
					"(offs=0x%x, size=%u, ELF_size=%u)",
					i, section->header->sh_offset,
					section->header->sh_size,
					buffer->size);
		}

		/* Add section to list */
		ArrayAdd(self->section_array, asObject(section));
	}

	/* Read string table */
	assert(header->e_shstrndx < header->e_shnum);
	self->string_table = asELFSection(ArrayGet(self->section_array,
			header->e_shstrndx));
	assert(self->string_table->header->sh_type == 3);

	/* Section names */
	ArrayForEach(self->section_array, section, ELFSection)
		section->name = new(String, self->string_table->buffer->ptr
				+ section->header->sh_name);


	/* Dump section headers */
	elf_reader_debug("Section headers:\n");
	elf_reader_debug("idx type flags addr       offset     size      link     name\n");
	for (i = 0; i < 80; i++)
		elf_reader_debug("-");
	elf_reader_debug("\n");
	index = 0;
	ArrayForEach(self->section_array, section, ELFSection)
	{
		elf_reader_debug("%-3d %-4u %-5u 0x%-8x 0x%-8x %-9u %-8u %s\n",
			index,
			section->header->sh_type,
			section->header->sh_flags,
			section->header->sh_addr,
			section->header->sh_offset,
			section->header->sh_size,
			section->header->sh_link,
			section->name->text);
		index++;
	}
	elf_reader_debug("\n");
}


static void ELFReaderReadProgramHeaders(ELFReader *self)
{
	ELFBuffer *buffer;
	ELFProgramHeader *program_header;
	Elf32_Ehdr *header;

	int count;
	int index;
	int i;

	/* Create program header list */
	buffer = self->buffer;
	header = self->header;
	if (!header->e_phnum)
		return;

	/* Check program header size */
	if (header->e_phentsize != sizeof(Elf32_Phdr))
		fatal("%s: program header size %d (should be %d)",
				self->path->text, header->e_phentsize,
				(int) sizeof(Elf32_Phdr));

	/* Read program headers */
	ELFBufferSeek(buffer, header->e_phoff);
	for (i = 0; i < header->e_phnum; i++)
	{
		/* Create program header */
		program_header = new(ELFProgramHeader);
		program_header->header = ELFBufferTell(buffer);

		/* Advance buffer */
		count = ELFBufferRead(buffer, NULL, sizeof(Elf32_Phdr));
		if (count < sizeof(Elf32_Phdr))
			fatal("%s: unexpected end of file while reading "
				"program headers", self->path->text);

		/* Add program header to list */
		ListAdd(self->program_header_list, asObject(program_header));
	}

	/* Dump program headers */
	elf_reader_debug("Program headers:\n");
	elf_reader_debug("idx type       offset     vaddr      paddr      filesz    memsz     flags  align\n");
	for (i = 0; i < 80; i++)
		elf_reader_debug("-");
	elf_reader_debug("\n");
	index = 0;
	ListForEach(self->program_header_list, program_header, ELFProgramHeader)
	{
		elf_reader_debug("%-3d 0x%-8x 0x%-8x 0x%-8x 0x%-8x "
			"%-9u %-9u %-6u %u\n",
			index,
			program_header->header->p_type,
			program_header->header->p_offset,
			program_header->header->p_vaddr,
			program_header->header->p_paddr,
			program_header->header->p_filesz,
			program_header->header->p_memsz,
			program_header->header->p_flags,
			program_header->header->p_align);
		index++;
	}
	elf_reader_debug("\n");
}


static void ELFReaderReadSymbolSection(ELFReader *self, ELFSection *section)
{
	Elf32_Sym *sym;
	ELFSection *symbol_names_section;
	ELFSymbol *symbol;
	
	int i;
	int count;

	/* Read symbol table section */
	symbol_names_section = asELFSection(ArrayGet(self->section_array,
			section->header->sh_link));
	assert(symbol_names_section);
	elf_reader_debug("  section '%s' is symbol table with names in section '%s'\n",
		section->name->text, symbol_names_section->name->text);

	/* Insert symbols */
	count = section->header->sh_size / sizeof(Elf32_Sym);
	for (i = 0; i < count; i++)
	{
		/* Read symbol. Don't create symbol if name is empty. */
		sym = (Elf32_Sym *) section->buffer->ptr + i;
		if (* (char *) (symbol_names_section->buffer->ptr
				+ sym->st_name) == '\0')
			continue;

		/* Create symbol */
		symbol = new(ELFSymbol);
		symbol->sym = sym;
		symbol->name = symbol_names_section->buffer->ptr + sym->st_name;
		assert(sym->st_name < symbol_names_section->buffer->size);

		/* Add symbol to list */
		ArrayAdd(self->symbol_array, asObject(symbol));
	}
}


static void ELFReaderReadSymbols(ELFReader *self)
{
	ELFSection *section;
	ELFSymbol *symbol;
	int i;

	/* Load symbols from sections */
	elf_reader_debug("Symbol table:\n");
	ArrayForEach(self->section_array, section, ELFSection)
		if (section->header->sh_type == 2 ||
				section->header->sh_type == 11)
			ELFReaderReadSymbolSection(self, section);

	/* Sort symbol table */
	ArraySort(self->symbol_array);

	/* Dump */
	elf_reader_debug("\n");
	elf_reader_debug("%-40s %-15s %-12s %-12s\n",
			"name", "section", "value", "size");
	for (i = 0; i < 80; i++)
		elf_reader_debug("-");
	elf_reader_debug("\n");
	ArrayForEach(self->symbol_array, symbol, ELFSymbol)
	{
		char section_name[15];

		section = asELFSection(ArrayGet(self->section_array,
				symbol->sym->st_shndx));
		if (section)
			snprintf(section_name, sizeof section_name ,
					"%s", section->name->text);
		else
			snprintf(section_name, sizeof section_name,
					"%d", symbol->sym->st_shndx);
		elf_reader_debug("%-40s %-15s 0x%-10x %-12d\n",
				symbol->name,
				section_name,
				symbol->sym->st_value,
				symbol->sym->st_size);
	}
	elf_reader_debug("\n");
}


static void ELFReaderDoCreate(ELFReader *self)
{
	/* Initialize */
	self->section_array = new(Array);
	self->program_header_list = new(List);
	self->symbol_array = new(Array);
	
	/* Interpret file */
	__ELFReaderReadHeader(self);
	ELFReaderReadSections(self);
	ELFReaderReadProgramHeaders(self);
	ELFReaderReadSymbols(self);
}


void ELFReaderCreate(ELFReader *self, const char *path)
{
	struct stat st;
	FILE *f;

	void *ptr;
	int size;
	int count;

	/* Save path */
	self->path = new(String, path);

	/* Get file size */
	if (stat(path, &st))
		fatal("%s: path not found", path);
	size = st.st_size;

	/* Open file */
	f = fopen(path, "rt");
	if (!f)
		fatal("%s: cannot open file", path);

	/* Read file contents */
	ptr = xmalloc(size);
	count = fread(ptr, 1, size, f);
	if (count != size)
		fatal("%s: error reading file contents", path);
	
	/* Create buffer */
	self->buffer = new(ELFBuffer, ptr, size);

	/* Common constructor */
	ELFReaderDoCreate(self);
}


void ELFReaderCreateFromBuffer(ELFReader *self, void *ptr, int size)
{
	void *new_ptr;

	/* Initialize */
	self->path = new(String, "");

	/* Allocate new buffer */
	new_ptr = xcalloc(1, size);
	memcpy(new_ptr, ptr, size);
	self->buffer = new(ELFBuffer, new_ptr, size);

	/* Common constructor */
	ELFReaderDoCreate(self);
}


void ELFReaderDestroy(ELFReader *self)
{
	delete(self->path);

	free(self->buffer->ptr);
	delete(self->buffer);

	ArrayDeleteObjects(self->section_array);
	delete(self->section_array);

	ListDeleteObjects(self->program_header_list);
	delete(self->program_header_list);

	ArrayDeleteObjects(self->symbol_array);
	delete(self->symbol_array);
}


ELFSymbol *ELFReaderGetSymbolByAddress(ELFReader *self, unsigned int address,
		unsigned int *offset_ptr)
{
	ELFSymbol *symbol;
	ELFSymbol *prev_symbol;
	
	int min;
	int max;
	int mid;

	/* Empty symbol table */
	if (!self->symbol_array->count)
		return NULL;

	/* All symbols in the table have a higher address */
	symbol = asELFSymbol(ArrayGet(self->symbol_array, 0));
	if (address < symbol->sym->st_value)
		return NULL;

	/* Binary search */
	min = 0;
	max = self->symbol_array->count;
	while (min + 1 < max)
	{
		mid = (max + min) / 2;
		symbol = asELFSymbol(ArrayGet(self->symbol_array, mid));
		if (symbol->sym->st_value > address)
		{
			max = mid;
		}
		else if (symbol->sym->st_value < address)
		{
			min = mid;
		}
		else
		{
			min = mid;
			break;
		}
	}

	/* Invalid symbol */
	symbol = asELFSymbol(ArrayGet(self->symbol_array, min));
	if (!symbol->sym->st_value)
		return NULL;

	/* Go backwards to find first symbol with that address */
	for (;;)
	{
		min--;
		prev_symbol = asELFSymbol(ArrayGet(self->symbol_array, min));
		if (!prev_symbol || prev_symbol->sym->st_value !=
				symbol->sym->st_value)
			break;
		symbol = prev_symbol;
	}

	/* Return the symbol and its address */
	if (offset_ptr)
		*offset_ptr = address - symbol->sym->st_value;
	return symbol;
}


ELFSymbol *ELFReaderGetSymbolByName(ELFReader *self, char *name)
{
	ELFSymbol *symbol;

	/* Search */
	ArrayForEach(self->symbol_array, symbol, ELFSymbol)
		if (!strcmp(symbol->name, name))
			return symbol;
	
	/* Not found */
	return NULL;
}


ELFBuffer *ELFReaderReadSymbolContent(ELFReader *self, ELFSymbol *symbol)
{
	ELFSection *section;
	ELFBuffer *buffer;

	/* Get section where the symbol is pointing */
	section = asELFSection(ArrayGet(self->section_array,
			symbol->sym->st_shndx));
	if (!section || symbol->sym->st_value + symbol->sym->st_size
			> section->header->sh_size)
		return NULL;

	/* Create buffer object */
	buffer = new(ELFBuffer, section->buffer->ptr + symbol->sym->st_value,
			symbol->sym->st_size);
	return buffer;
}






/*
 * Public
 */

int elf_reader_debug_category;


void ELFFileReadHeader(char *path, Elf32_Ehdr *ehdr_ptr)
{
	int count;

	FILE *f;

	/* Open file */
	f = fopen(path, "rb");
	if (!f)
		fatal("%s: cannot open file", path);

	/* Read header */
	assert(ehdr_ptr);
	count = fread(ehdr_ptr, sizeof(Elf32_Ehdr), 1, f);
	if (count != 1)
		fatal("%s: invalid ELF file", path);

	/* Check that file is a valid ELF file */
	if (strncmp((char *) ehdr_ptr->e_ident, ELFMAG, 4))
		fatal("%s: invalid ELF file", path);

	/* Check that ELF file is a 32-bit object */
	if (ehdr_ptr->e_ident[EI_CLASS] == ELFCLASS64)
		fatal("%s: 64-bit ELF not supported.\n%s",
			path, elf_reader_err_64bit);

	/* Close file */
	fclose(f);
}
