/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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


#include <assert.h>
#include <sys/stat.h>

#include <lib/mhandle/mhandle.h>

#include "debug.h"
#include "elf-format.h"
#include "list.h"



/*
 * Variables
 */

int elf_debug_category;

static char *elf_err_64bit =
	"\tThe ELF file being loaded is a 64-bit file, currently not supported\n"
	"\tby Multi2Sim. If you are compiling your own source code on a 64-bit\n"
	"\tmachine, please use the '-m32' flag in the gcc command-line. If you\n"
	"\tget compilation errors related with missing '.h' files, check that\n"
	"\tthe 32-bit gcc package associated with your Linux distribution is\n"
	"\tinstalled.\n";



/*
 * ELF Buffer
 */


/* Select new position in buffer */
void elf_buffer_seek(struct elf_buffer_t *elf_buffer, int pos)
{
	if (pos < 0 || pos > elf_buffer->size)
		fatal("%s: invalid position in buffer (%d)", __FUNCTION__, pos);
	elf_buffer->pos = pos;
}


/* Return a pointer to the current position */
void *elf_buffer_tell(struct elf_buffer_t *buffer)
{
	return buffer->ptr + buffer->pos;
}


/* Read a line from 'buffer'.
 * The line is placed in 'str', which is a buffer of 'size' bytes.
 * The function returns the number of bytes read. */
int elf_buffer_read_line(struct elf_buffer_t *buffer, char *str, int size)
{
	int read_count = 0;
	int write_count = 0;

	for (;;) {

		/* If only 1 byte left, use it for null termination */
		if (size == 1) {
			*str = '\0';
			break;
		}

		/* Copy one character */
		read_count = elf_buffer_read(buffer, str, 1);
		if (!read_count) {
			*str = '\0';
			break;
		}
		if (*str == '\n' || *str == '\0') {
			*str = '\0';
			break;
		}

		/* Advance string */
		size--;
		str++;
		write_count++;
	}

	return write_count;
}


/* Read 'size' bytes from ELF buffer, and store them in '*ptr'.
 *  -If 'size' is greater than the number of bytes left in the buffer, only the
 *   remaining contents of the buffer will be read.
 *  -The current position in the buffer is moved forward.
 *  -If 'ptr' is NULL, only the position in the buffer will be moved, without
 *   actually reading the contents of the buffer.
 *  -The function returns the number of bytes read.
 */
int elf_buffer_read(struct elf_buffer_t *buffer, void *ptr, int size)
{
	int left;

	/* Truncate size to read */
	left = buffer->size - buffer->pos;
	assert(left >= 0);
	if (size > left)
		size = left;

	/* Read bytes */
	if (ptr)
		memcpy(ptr, buffer->ptr + buffer->pos, size);

	/* Advance pointer and return read bytes */
	buffer->pos += size;
	return size;
}


/* Dump contents of buffer into file 'f'. The buffer is dumped at the current
 * position of the file. */
void elf_buffer_dump(struct elf_buffer_t *buffer, FILE *f)
{
	size_t size;

	size = fwrite(buffer->ptr, 1, buffer->size, f);
	if (size != buffer->size)
		fatal("%s: couldn't dump buffer contents",
			__FUNCTION__);
}




/*
 * ELF Symbol Table
 */


static int elf_symbol_compare(const void *a, const void *b)
{
	const struct elf_symbol_t *symbol_a = a;
	const struct elf_symbol_t *symbol_b = b;
	int bind_a;
	int bind_b;

	if (symbol_a->value < symbol_b->value)
		return -1;
	else if (symbol_a->value > symbol_b->value)
		return 1;
	else
	{
		/* Sort symbol with the same address as per their
		 * ST_BIND field in st_info (bits 4 to 8) */
		bind_a = (symbol_a->info >> 4) & 0xf;
		bind_b = (symbol_b->info >> 4) & 0xf;

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
}


/* Add symbols in a section into the symbol table */
static void elf_file_read_symbol_section(struct elf_file_t *elf_file, struct elf_section_t *section)
{
	int i, count;
	Elf32_Sym *sym;
	struct elf_section_t *symbol_names_section;
	struct elf_symbol_t *symbol;

	/* Read symbol table section */
	symbol_names_section = list_get(elf_file->section_list, section->header->sh_link);
	assert(symbol_names_section);
	elf_debug("  section '%s' is symbol table with names in section '%s'\n",
		section->name, symbol_names_section->name);

	/* Insert symbols */
	count = section->header->sh_size / sizeof(Elf32_Sym);
	for (i = 0; i < count; i++) {

		/* Read symbol */
		sym = (Elf32_Sym *) section->buffer.ptr + i;
		if (* (char *) (symbol_names_section->buffer.ptr + sym->st_name) == '\0')
			continue;

		/* Create symbol */
		symbol = xcalloc(1, sizeof(struct elf_symbol_t));
		symbol->value = sym->st_value;
		symbol->size = sym->st_size;
		symbol->section = sym->st_shndx;
		symbol->name = symbol_names_section->buffer.ptr + sym->st_name;
		symbol->info = sym->st_info;
		symbol->visibility = sym->st_other;
		assert(sym->st_name < symbol_names_section->buffer.size);

		/* Add symbol to list */
		list_add(elf_file->symbol_table, symbol);
	}
}


static void elf_file_read_symbol_table(struct elf_file_t *elf_file)
{
	struct elf_section_t *section;
	struct elf_symbol_t *symbol;
	int i;

	/* Create symbol table */
	elf_file->symbol_table = list_create();

	/* Load symbols from sections */
	elf_debug("Symbol table:\n");
	for (i = 0; i < list_count(elf_file->section_list); i++) {
		section = list_get(elf_file->section_list, i);
		if (section->header->sh_type == 2 || section->header->sh_type == 11)
			elf_file_read_symbol_section(elf_file, section);
	}

	/* Sort symbol table */
	list_sort(elf_file->symbol_table, elf_symbol_compare);

	/* Dump */
	elf_debug("\n");
	elf_debug("%-40s %-15s %-12s %-12s\n", "name", "section", "value", "size");
	for (i = 0; i < 80; i++)
		elf_debug("-");
	elf_debug("\n");
	for (i = 0; i < list_count(elf_file->symbol_table); i++)
	{
		char section_name[15];

		symbol = list_get(elf_file->symbol_table, i);
		section = list_get(elf_file->section_list, symbol->section);
		if (section)
			snprintf(section_name, sizeof(section_name), "%s", section->name);
		else
			snprintf(section_name, sizeof(section_name), "%d", symbol->section);
		elf_debug("%-40s %-15s 0x%-10x %-12d\n", symbol->name, section_name, symbol->value, symbol->size);
	}
	elf_debug("\n");
}


struct elf_symbol_t *elf_symbol_get_by_address(struct elf_file_t *elf_file,
	unsigned int addr, unsigned int *offset_ptr)
{
	int min, max, mid;
	struct elf_symbol_t *symbol;
	struct elf_symbol_t *prev_symbol;

	/* Empty symbol table */
	if (!list_count(elf_file->symbol_table))
		return NULL;

	/* All symbols in the table have a higher address */
	symbol = list_get(elf_file->symbol_table, 0);
	if (addr < symbol->value)
		return NULL;

	/* Binary search */
	min = 0;
	max = list_count(elf_file->symbol_table);
	while (min + 1 < max) {
		mid = (max + min) / 2;
		symbol = list_get(elf_file->symbol_table, mid);
		if (symbol->value > addr)
			max = mid;
		else if (symbol->value < addr)
			min = mid;
		else {
			min = mid;
			break;
		}
	}

	/* Invalid symbol */
	symbol = list_get(elf_file->symbol_table, min);
	if (!symbol->value)
		return NULL;

	/* Go backwards to find first symbol with that address */
	for (;;)
	{
		min--;
		prev_symbol = list_get(elf_file->symbol_table, min);
		if (!prev_symbol || prev_symbol->value != symbol->value)
			break;
		symbol = prev_symbol;
	}

	/* Return the symbol and its address */
	if (offset_ptr)
		*offset_ptr = addr - symbol->value;
	return symbol;
}


struct elf_symbol_t *elf_symbol_get_by_name(struct elf_file_t *elf_file, char *name)
{
	struct elf_symbol_t *symbol;
	int i;

	for (i = 0; i < list_count(elf_file->symbol_table); i++) {
		symbol = list_get(elf_file->symbol_table, i);
		if (!strcmp(symbol->name, name))
			return symbol;
	}
	return NULL;
}


int elf_symbol_read_content(struct elf_file_t *elf_file, struct elf_symbol_t *symbol,
		struct elf_buffer_t *elf_buffer)
{
	struct elf_section_t *section;

	/* Initialize buffer */
	assert(elf_buffer);
	elf_buffer->ptr = NULL;
	elf_buffer->size = 0;
	elf_buffer->pos = 0;

	/* Get section where the symbol is pointing */
	section = list_get(elf_file->section_list, symbol->section);
	if (!section || symbol->value + symbol->size > section->header->sh_size)
		return 0;

	/* Update buffer */
	elf_buffer->ptr = section->buffer.ptr + symbol->value;
	elf_buffer->size = symbol->size;
	elf_buffer->pos = 0;
	return 1;
}




/*
 * ELF File
 */

static void elf_file_read_elf_header(struct elf_file_t *elf_file)
{
	struct elf_buffer_t *buffer;
	Elf32_Ehdr *elf_header;
	int count;

	/* Read ELF header */
	buffer = &elf_file->buffer;
	elf_file->header = buffer->ptr;
	elf_header = elf_file->header;
	count = elf_buffer_read(buffer, NULL, sizeof(Elf32_Ehdr));
	if (count < EI_NIDENT)
		fatal("%s: not a valid ELF file\n", elf_file->path);

	/* Check magic characters */
	if (elf_header->e_ident[0] != 0x7f
		|| elf_header->e_ident[1] != 'E'
		|| elf_header->e_ident[2] != 'L'
		|| elf_header->e_ident[3] != 'F')
		fatal("%s: not a valid ELF file\n", elf_file->path);

	/* Check for 32-bit executable (field e_ident[EI_CLASS]) */
	if (elf_header->e_ident[4] == 2)
		fatal("%s: not supported architecture.\n%s",
			elf_file->path, elf_err_64bit);
	
	/* Check that header size is correct */
	if (elf_header->e_ehsize != sizeof(Elf32_Ehdr))
		fatal("%s: invalid ELF header size", elf_file->path);

	/* Check endianness */
	if (elf_header->e_ident[5] != 1)
		fatal("%s: ELF file endianness mismatch", elf_file->path);

	/* Debug */
	elf_debug("ELF header:\n");
	elf_debug("  ehdr.e_ident: EI_CLASS=%d, EI_DATA=%d, EI_VERSION=%d\n",
		elf_header->e_ident[4], elf_header->e_ident[5], elf_header->e_ident[6]);
	elf_debug("  ehdr.e_type: %d\n", elf_header->e_type);
	elf_debug("  ehdr.e_machine: %u\n", elf_header->e_machine);
	elf_debug("  ehdr.e_entry: 0x%x (program entry point)\n", elf_header->e_entry);
	elf_debug("  ehdr.e_phoff: %u (program header table offset)\n", elf_header->e_phoff);
	elf_debug("  ehdr.e_shoff: %u (section header table offset)\n", elf_header->e_shoff);
	elf_debug("  ehdr.e_phentsize: %u\n", elf_header->e_phentsize);
	elf_debug("  ehdr.e_phnum: %u\n", elf_header->e_phnum);
	elf_debug("  ehdr.e_shentsize: %u\n", elf_header->e_shentsize);
	elf_debug("  ehdr.e_shnum: %u\n", elf_header->e_shnum);
	elf_debug("  ehdr.e_shstrndx: %u\n", (uint32_t) elf_header->e_shstrndx);
	elf_debug("\n");
}


static void elf_file_read_section_headers(struct elf_file_t *elf_file)
{
	int i, count;

	struct elf_buffer_t *buffer;
	struct elf_section_t *section;
	Elf32_Ehdr *elf_header;

	/* Create section list */
	elf_file->section_list = list_create();

	/* Check section size and number */
	buffer = &elf_file->buffer;
	elf_header = elf_file->header;
	if (!elf_header->e_shnum || elf_header->e_shentsize != sizeof(Elf32_Shdr))
		fatal("%s: number of sections is 0 or section size is not %d",
			elf_file->path, (int) sizeof(Elf32_Shdr));

	/* Read section headers */
	elf_buffer_seek(buffer, elf_header->e_shoff);
	for (i = 0; i < elf_header->e_shnum; i++)
	{
		/* Allocate section */
		section = xcalloc(1, sizeof(struct elf_section_t));
		section->header = elf_buffer_tell(buffer);

		/* Advance buffer */
		count = elf_buffer_read(buffer, NULL, sizeof(Elf32_Shdr));
		if (count < sizeof(Elf32_Shdr))
			fatal("%s: unexpected end of file while reading section headers", elf_file->path);

		/* Get section contents, if section type is not SHT_NOBITS (8) */
		if (section->header->sh_type != 8)
		{
			section->buffer.ptr = buffer->ptr + section->header->sh_offset;
			section->buffer.size = section->header->sh_size;
			section->buffer.pos = 0;
			assert(section->buffer.ptr >= buffer->ptr);
			if (section->buffer.ptr + section->buffer.size > buffer->ptr + buffer->size)
				fatal("section %d out of the ELF boundaries (offs=0x%x, size=%u, ELF_size=%u)",
						i, section->header->sh_offset, section->header->sh_size, buffer->size);
		}

		/* Add section to list */
		list_add(elf_file->section_list, section);
	}

	/* Read string table, and update section names */
	assert(elf_header->e_shstrndx < elf_header->e_shnum);
	elf_file->string_table = list_get(elf_file->section_list, elf_header->e_shstrndx);
	assert(elf_file->string_table->header->sh_type == 3);
	for (i = 0; i < list_count(elf_file->section_list); i++) {
		section = list_get(elf_file->section_list, i);
		section->name = elf_file->string_table->buffer.ptr + section->header->sh_name;
	}


	/* Dump section headers */
	elf_debug("Section headers:\n");
	elf_debug("idx type flags addr       offset     size      link     name\n");
	for (i = 0; i < 80; i++)
		elf_debug("-");
	elf_debug("\n");
	for (i = 0; i < list_count(elf_file->section_list); i++) {
		section = list_get(elf_file->section_list, i);
		elf_debug("%-3d %-4u %-5u 0x%-8x 0x%-8x %-9u %-8u %s\n", i,
			section->header->sh_type,
			section->header->sh_flags,
			section->header->sh_addr,
			section->header->sh_offset,
			section->header->sh_size,
			section->header->sh_link,
			section->name);
	}
	elf_debug("\n");
}


static void elf_file_read_program_headers(struct elf_file_t *elf_file)
{
	struct elf_buffer_t *buffer;
	struct elf_program_header_t *program_header;
	Elf32_Ehdr *elf_header;

	int count;
	int i;

	/* Create program header list */
	buffer = &elf_file->buffer;
	elf_header = elf_file->header;
	elf_file->program_header_list = list_create();
	if (!elf_header->e_phnum)
		return;

	/* Check program header size */
	if (elf_header->e_phentsize != sizeof(Elf32_Phdr))
		fatal("%s: program header size %d (should be %d)",
			elf_file->path, elf_header->e_phentsize, (int) sizeof(Elf32_Phdr));

	/* Read program headers */
	elf_buffer_seek(buffer, elf_header->e_phoff);
	for (i = 0; i < elf_header->e_phnum; i++)
	{
		/* Allocate program header */
		program_header = xcalloc(1, sizeof(struct elf_program_header_t));
		program_header->header = elf_buffer_tell(buffer);

		/* Advance buffer */
		count = elf_buffer_read(buffer, NULL, sizeof(Elf32_Phdr));
		if (count < sizeof(Elf32_Phdr))
			fatal("%s: unexpected end of file while reading program headers", elf_file->path);

		/* Add program header to list */
		list_add(elf_file->program_header_list, program_header);
	}

	/* Dump program headers */
	elf_debug("Program headers:\n");
	elf_debug("idx type       offset     vaddr      paddr      filesz    memsz     flags  align\n");
	for (i = 0; i < 80; i++)
		elf_debug("-");
	elf_debug("\n");
	for (i = 0; i < list_count(elf_file->program_header_list); i++) {
		program_header = list_get(elf_file->program_header_list, i);
		elf_debug("%-3d 0x%-8x 0x%-8x 0x%-8x 0x%-8x %-9u %-9u %-6u %u\n", i,
			program_header->header->p_type,
			program_header->header->p_offset,
			program_header->header->p_vaddr,
			program_header->header->p_paddr,
			program_header->header->p_filesz,
			program_header->header->p_memsz,
			program_header->header->p_flags,
			program_header->header->p_align);
	}
	elf_debug("\n");
}


static struct elf_file_t *elf_file_create_from_allocated_buffer(void *buffer, int size, char *path)
{
	struct elf_file_t *elf_file;

	/* Create buffer */
	elf_debug("**\n** Loading ELF file\n** %s\n**\n\n", path);
	elf_file = xcalloc(1, sizeof(struct elf_file_t));

	/* Initialize buffer */
	elf_file->path = xstrdup(path ? path : "");
	elf_file->buffer.ptr = buffer;
	elf_file->buffer.size = size;
	elf_file->buffer.pos = 0;

	/* Read ELF file contents */
	elf_file_read_elf_header(elf_file);
	elf_file_read_section_headers(elf_file);
	elf_file_read_program_headers(elf_file);
	elf_file_read_symbol_table(elf_file);

	/* Return */
	elf_debug("\n\n\n");
	return elf_file;
}


struct elf_file_t *elf_file_create_from_buffer(void *ptr, int size, char *name)
{
	struct elf_file_t *elf_file;
	void *ptr_copy;

	/* Make a copy of the buffer */
	ptr_copy = xmalloc(size);
	memcpy(ptr_copy, ptr, size);

	/* Create ELF */
	elf_file = elf_file_create_from_allocated_buffer(ptr_copy, size, name);
	return elf_file;
}


struct elf_file_t *elf_file_create_from_path(char *path)
{
	struct elf_file_t *elf_file;
	struct stat st;
	FILE *f;

	void *buffer;
	int size;
	int count;

	/* Get file size */
	if (stat(path, &st))
		fatal("'%s': path not found", path);
	size = st.st_size;

	/* Open file */
	f = fopen(path, "rt");
	if (!f)
		fatal("'%s': cannot open file", path);

	/* Read file contents */
	buffer = xmalloc(size);
	count = fread(buffer, 1, size, f);
	if (count != size)
		fatal("'%s': error reading file contents", path);

	/* Create ELF file */
	elf_file = elf_file_create_from_allocated_buffer(buffer, size, path);
	return elf_file;
}


void elf_file_free(struct elf_file_t *elf_file)
{
	/* Free symbol table */
	while (list_count(elf_file->symbol_table))
		free(list_remove_at(elf_file->symbol_table, 0));
	list_free(elf_file->symbol_table);

	/* Free section list */
	while (list_count(elf_file->section_list))
		free(list_remove_at(elf_file->section_list, 0));
	list_free(elf_file->section_list);

	/* Free program header list */
	while (list_count(elf_file->program_header_list))
		free(list_remove_at(elf_file->program_header_list, 0));
	list_free(elf_file->program_header_list);

	/* Free rest */
	free(elf_file->buffer.ptr);
	free(elf_file->path);
	free(elf_file);
}


/* Read the ELF header from a file, without creating an entire ELF object.
 * This function is useful to pre-process an ELF file before deciding a
 * specific action for it that depends on a field of the header. */
void elf_file_read_header(char *path, Elf32_Ehdr *ehdr)
{
	int count;

	FILE *f;

	/* Open file */
	f = fopen(path, "rb");
	if (!f)
		fatal("%s: cannot open file", path);

	/* Read header */
	count = fread(ehdr, sizeof(Elf32_Ehdr), 1, f);
	if (count != 1)
		fatal("%s: invalid ELF file", path);

	/* Check that file is a valid ELF file */
	if (strncmp((char *) ehdr->e_ident, ELFMAG, 4))
		fatal("%s: invalid ELF file", path);

	/* Check that ELF file is a 32-bit object */
	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		fatal("%s: 64-bit ELF not supported.\n%s",
			path, elf_err_64bit);

	/* Close file */
	fclose(f);
}
