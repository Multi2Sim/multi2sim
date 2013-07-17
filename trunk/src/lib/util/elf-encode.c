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

#include "debug.h"
#include "elf-encode.h"
#include "list.h"
#include "misc.h"



/*
 * Buffer
 */

#define ELF_ENC_BUFFER_INIT_SIZE 1000

struct elf_enc_buffer_t *elf_enc_buffer_create(void)
{
	struct elf_enc_buffer_t *buffer;

	/* Initialize */
	buffer = xcalloc(1, sizeof(struct elf_enc_buffer_t));
	buffer->index = -1;
	buffer->alloc_size = ELF_ENC_BUFFER_INIT_SIZE;
	buffer->ptr = xcalloc(1, ELF_ENC_BUFFER_INIT_SIZE);

	/* Return */
	return buffer;
}


void elf_enc_buffer_free(struct elf_enc_buffer_t *buffer)
{
	free(buffer->ptr);
	free(buffer);
}


void elf_enc_buffer_write(struct elf_enc_buffer_t *buffer, void *ptr,
		unsigned int size)
{
	int new_size;

	/* Grow allocated buffer if new content exceeds the current size */
	new_size = MAX(buffer->offset + size, buffer->size);
	while (new_size > buffer->alloc_size)
	{
		buffer->ptr = xrealloc(buffer->ptr, buffer->alloc_size * 2);
		buffer->alloc_size = buffer->alloc_size * 2;
		memset(buffer->ptr + buffer->size, 0, buffer->alloc_size - buffer->size);
	}

	/* Write content to buffer */
	memcpy(buffer->ptr + buffer->offset, ptr, size);
	buffer->size = new_size;
	buffer->offset = buffer->offset + size;
}


void elf_enc_buffer_dump(struct elf_enc_buffer_t *buffer, FILE *f)
{
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


unsigned int elf_enc_buffer_read(struct elf_enc_buffer_t *buffer, void *ptr,
	unsigned int size)
{
	/* Truncate size to available data after current offset */
	size = MAX(size, buffer->size - buffer->offset);

	/* Read content */
	memcpy(ptr, buffer->ptr + buffer->offset, size);
	buffer->offset += size;

	/* Return the number of read bytes */
	return size;
}


unsigned int elf_enc_buffer_seek(struct elf_enc_buffer_t *buffer, unsigned int offset)
{
	buffer->offset = MAX(offset, buffer->size);
	return buffer->offset;
}


int elf_enc_buffer_write_to_file(struct elf_enc_buffer_t *buffer, FILE *f)
{
	return fwrite(buffer->ptr, 1, buffer->size, f);
}


int elf_enc_buffer_read_from_file(struct elf_enc_buffer_t *buffer, FILE *f)
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
	elf_enc_buffer_write(buffer, buf, file_size);

	/* Free temporary buffer */
	free(buf);
	return file_size;
}



/*
 * Section
 */

struct elf_enc_section_t *elf_enc_section_create(char *name, 
	struct elf_enc_buffer_t *first_buffer,
	struct elf_enc_buffer_t *last_buffer)
{
	struct elf_enc_section_t *section;

	/* Initialize */
	section = xcalloc(1, sizeof(struct elf_enc_section_t));
	section->name = xstrdup(name);
	section->first_buffer = first_buffer;
	section->last_buffer = last_buffer;
	section->index = -1;
	
	/* Return */
	return section;
}


void elf_enc_section_free(struct elf_enc_section_t *section)
{
	free(section->name);
	free(section);
}


void elf_enc_section_dump(struct elf_enc_section_t *section,
	struct elf_enc_file_t *file, FILE *f)
{

	struct elf_enc_buffer_t *buffer;

	int first_index;
	int last_index;
	int i;

	/* Buffers must have been inserted to the ELF file */
	first_index = section->first_buffer->index;
	last_index = section->last_buffer->index;
	if (first_index < 0 || last_index < 0)
		fatal("%s: buffers in section not added to ELF file",
				__FUNCTION__);

	/* Dump section */
	fprintf(f, "Dumping Section: %s\n", section->name);
	for (i = first_index; i <= last_index; i++)
	{
		fprintf(f, "  Dumping Buffer %d:\n", i - first_index + 1);
		buffer = list_get(file->buffer_list, i);
		elf_enc_buffer_dump(buffer, f);
	}
	
}




/*
 * Segment
 */

struct elf_enc_segment_t *elf_enc_segment_create(char *name, 
	struct elf_enc_buffer_t *first_buffer,
	struct elf_enc_buffer_t *last_buffer)
{
	struct elf_enc_segment_t *segment;

	/* Initialize */
	segment = xcalloc(1, sizeof(struct elf_enc_segment_t));
	segment->name = xstrdup(name);
	segment->first_buffer = first_buffer;
	segment->last_buffer = last_buffer;
	segment->index = -1;

	/* Return */
	return segment;
}


void elf_enc_segment_free(struct elf_enc_segment_t *segment)
{
	free(segment->name);
	free(segment);
}


void elf_enc_segment_dump(struct elf_enc_segment_t *segment,
	struct elf_enc_file_t *file, FILE *f)
{
	struct elf_enc_buffer_t *buffer;

	int first_index;
	int last_index;
	int i;

	/* Buffers must have been inserted to the ELF file */
	first_index = segment->first_buffer->index;
	last_index = segment->last_buffer->index;
	if (first_index < 0 || last_index < 0)
		fatal("%s: buffers in segment not added to ELF file",
				__FUNCTION__);

	/* Dump segment */
	fprintf(f, "Dumping Segment: %s\n", segment->name);
	for (i = first_index; i <= last_index; i++)
	{
		fprintf(f, "  Dumping Buffer %d:\n", i - first_index + 1);
		buffer = list_get(file->buffer_list, i);
		elf_enc_buffer_dump(buffer, f);
	}
}



/*
 * Symbol
 */

struct elf_enc_symbol_t *elf_enc_symbol_create(char *name)
{
	struct elf_enc_symbol_t *symbol;
	
	symbol = xcalloc(1, sizeof(struct elf_enc_symbol_t));
	symbol->name = xstrdup(name);

	return symbol;
}


void elf_enc_symbol_free(struct elf_enc_symbol_t *symbol)
{
	free(symbol->name);
	free(symbol);
}




/*
 * Symbol Table
 */

struct elf_enc_symbol_table_t *elf_enc_symbol_table_create(char *symtab_name,
		char *strtab_name)
{
	struct elf_enc_symbol_table_t *table;
	struct elf_enc_symbol_t *null_symbol;

	/* Initialize */
	table = xcalloc(1, sizeof(struct elf_enc_symbol_table_t));
	table->symbol_table_buffer = elf_enc_buffer_create();
	table->string_table_buffer = elf_enc_buffer_create();
	table->symbol_list = list_create();

	/* Create one section for the symbol table, another for the string
	 * table, and associated them with the created buffers. */
	table->symbol_table_section = elf_enc_section_create(symtab_name,
			table->symbol_table_buffer, table->symbol_table_buffer);
	table->string_table_section = elf_enc_section_create(strtab_name,
			table->string_table_buffer, table->string_table_buffer);
	
	/* Set symtab and strtab properties */
	table->symbol_table_section->header.sh_type = SHT_SYMTAB;
	table->string_table_section->header.sh_type = SHT_STRTAB;
	table->symbol_table_section->header.sh_entsize = sizeof(Elf32_Sym);
	
	/* Set first entry of list to be a null symbol */
	null_symbol = elf_enc_symbol_create("");
	elf_enc_symbol_table_add(table, null_symbol);

	/* Return */
	return table;
}


void elf_enc_symbol_table_free(struct elf_enc_symbol_table_t *table)
{
	int i;
	
	/* Traverse symbol list and free symbols */
	LIST_FOR_EACH(table->symbol_list, i)
		elf_enc_symbol_free(list_get(table->symbol_list, i));
	
	/* Free list and table */
	list_free(table->symbol_list);
	free(table);
}


void elf_enc_symbol_table_add(struct elf_enc_symbol_table_t *table,
	struct elf_enc_symbol_t *symbol)
{
	/* Add symbol to symbol list */
	list_add(table->symbol_list, symbol);	
			
		
	/* Update offset for symbol name, pointing to the current end of the
	 * strtab buffer. */
	symbol->symbol.st_name = table->string_table_buffer->size;

	/* Write symbol into symtab buffer and symbol name into the strtab buffer */
	elf_enc_buffer_write(table->symbol_table_buffer,
			&symbol->symbol, sizeof(Elf32_Sym));
	elf_enc_buffer_write(table->string_table_buffer,
			symbol->name, strlen(symbol->name) + 1);
}



/*
 * ELF File
 */

struct elf_enc_file_t *elf_enc_file_create(void)
{
	struct elf_enc_file_t *file;

	/* Initialize */
	file = xcalloc(1, sizeof(struct elf_enc_file_t));
	file->section_list = list_create();
	file->segment_list = list_create();
	file->symbol_table_list = list_create();
	file->buffer_list = list_create();

	/* Create null buffer for null section */
	struct elf_enc_buffer_t *null_buffer;
	null_buffer = elf_enc_buffer_create();
	elf_enc_file_add_buffer(file, null_buffer);

	/* Create null section - will be first entry in shstrtab */
	struct elf_enc_section_t *null_section;
	null_section = elf_enc_section_create("", null_buffer, null_buffer);
	elf_enc_file_add_section(file, null_section);

	/* Create buffer for string table */
	struct elf_enc_buffer_t *shstrtab;
	shstrtab = elf_enc_buffer_create();
	elf_enc_file_add_buffer(file, shstrtab);

	/* Create section for shstrtab */
	struct elf_enc_section_t *shstrtab_section;
	shstrtab_section = elf_enc_section_create(".shstrtab", shstrtab, shstrtab);
	shstrtab_section->header.sh_name = 1;
	shstrtab_section->header.sh_type = SHT_STRTAB;
	elf_enc_file_add_section(file, shstrtab_section);

	/* Write null section and shstrtab names into buffer */
	elf_enc_buffer_write(shstrtab, null_section->name, strlen(null_section->name) + 1);
	elf_enc_buffer_write(shstrtab, shstrtab_section->name, strlen(shstrtab_section->name) + 1);

	/* Return */
	return file;
}


void elf_enc_file_free(struct elf_enc_file_t *file)
{
	int i;

	/* Free buffer list */
	LIST_FOR_EACH(file->buffer_list, i)
		elf_enc_buffer_free(list_get(file->buffer_list, i));
	list_free(file->buffer_list);

	/* Free section list */
	LIST_FOR_EACH(file->section_list, i)
		elf_enc_section_free(list_get(file->section_list, i));
	list_free(file->section_list);
	
	/* Free segment list */
	LIST_FOR_EACH(file->segment_list, i)
		elf_enc_segment_free(list_get(file->segment_list, i));
	list_free(file->segment_list);
	
	/* Free symbol table list */
	LIST_FOR_EACH(file->symbol_table_list, i)
		elf_enc_symbol_table_free(list_get(file->symbol_table_list, i));
	list_free(file->symbol_table_list);

	/* Free ELF file */
	free(file);	
}


void elf_enc_file_add_buffer(struct elf_enc_file_t *file,
	struct elf_enc_buffer_t *buffer)
{
	int count;

	/* Add to list and set index */
	count = list_count(file->buffer_list);
	list_add(file->buffer_list, buffer);
	buffer->index = count;
}


void elf_enc_file_add_section(struct elf_enc_file_t *file,
	struct elf_enc_section_t *section)
{
	int count;
	struct elf_enc_buffer_t *shstrtab;

	/* Add to list and set index */
	count = list_count(file->section_list);
	list_add(file->section_list, section);
	section->index = count;

	if (list_count(file->buffer_list) > 2)
	{
		shstrtab = list_get(file->buffer_list, 1);
		section->header.sh_name = shstrtab->size;
		elf_enc_buffer_write(shstrtab, section->name, strlen(section->name) + 1);
	}
}


void elf_enc_file_add_segment(struct elf_enc_file_t *file,
	struct elf_enc_segment_t *segment)
{
	int count;

	/* Add to list and set index */
	count = list_count(file->segment_list);
	list_add(file->segment_list, segment);
	segment->index = count;
}


void elf_enc_file_add_symbol_table(struct elf_enc_file_t *file, 
	struct elf_enc_symbol_table_t *table)
{
	list_add(file->symbol_table_list, table);

	/* Add symtab and strtab buffers and sections from the symbol table into
	 * the ELF file buffer and section list. */
	elf_enc_file_add_buffer(file, table->symbol_table_buffer);
	elf_enc_file_add_buffer(file, table->string_table_buffer);
	elf_enc_file_add_section(file, table->symbol_table_section);
	elf_enc_file_add_section(file, table->string_table_section);
	
	/* Now that the sections have acquired an index in the section list, we
	 * can make the symtab section point to the strtab section. */
	table->symbol_table_section->header.sh_link =
			table->string_table_section->index;
		
}


void elf_enc_file_generate(struct elf_enc_file_t *file,
		struct elf_enc_buffer_t *bin_buffer)
{
	struct elf_enc_section_t *section;
	struct elf_enc_segment_t *segment;
	struct elf_enc_buffer_t *buffer;
	//struct elf_enc_buffer_t *shstrtab;

	unsigned int phtab_size;
	unsigned int shtab_size;
	unsigned int buf_offset;
	int i;
	int j;

	Elf32_Ehdr *elf_header;

	
	/* Set up ELF Header */
	elf_header = &file->header;
	
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
	
	
	/* Set ELF Header Properties for sections. Use elf.h structs for shentsize and ehsize */
	elf_header->e_shentsize = sizeof(Elf32_Shdr);
	elf_header->e_shnum = list_count(file->section_list);
	elf_header->e_shstrndx = 1;
	elf_header->e_ehsize = sizeof(Elf32_Ehdr);

	/* Set ELF Header Properties for segments - If there are no segments, even the phentsize should be zero */
	if (list_count(file->segment_list) != 0)
	{	
		elf_header->e_phoff = sizeof(Elf32_Ehdr);
		elf_header->e_phnum = list_count(file->segment_list);
		elf_header->e_phentsize = sizeof(Elf32_Phdr);
	}
	
	/* Calculate Program Header and Section Header table size - Actual header tables are created at the end */
	phtab_size = sizeof(Elf32_Phdr) * list_count(file->segment_list);
	shtab_size = sizeof(Elf32_Shdr) * list_count(file->section_list);

	/* Section Header String Table - typically first section (though not required)
	shstrtab = list_get(file->buffer_list, 1);
	assert(shstrtab);
	*/


	/* Write the names of each section into shstrtab buffer 
	for (i = 0; i < list_count(file->section_list); i++)
	{
		section = list_get(file->section_list, i);
		section->header.sh_name = shstrtab->size;
		elf_enc_buffer_write(shstrtab, section->name, strlen(section->name) + 1);
	}
	*/
	
	/* TODO Get rid of when done testing */
	//elf_enc_buffer_dump(shstrtab, stdout); 


	/* Find segment offsets */
	for(i = 0; i < list_count(file->segment_list); i++)
	{
		segment = list_get(file->segment_list, i);
	
		buf_offset = 0;

		for (j = 0; j < segment->first_buffer->index; j++)
		{
			buffer = list_get(file->buffer_list, j);
			buf_offset += buffer->size;
		}
		
		/* Add up all offsets to find segment offset */
		segment->header.p_offset = sizeof(Elf32_Ehdr) + phtab_size + buf_offset;
	}
		
	/* Find actual segment sizes */
	for(i = 0; i < list_count(file->segment_list); i++)
	{
		segment = list_get(file->segment_list, i);

		for(j = segment->first_buffer->index; j <= segment->last_buffer->index; j++)
		{
			buffer = list_get(file->buffer_list, j);
			segment->header.p_filesz += buffer->size;
			if (segment->header.p_type == PT_LOAD)
				segment->header.p_memsz += buffer->size;
		}
	}
	

	/* Find section offsets */
	for(i = 0; i < list_count(file->section_list); i++)
	{
		section = list_get(file->section_list, i);
	
		buf_offset = 0;

		for (j = 0; j < section->first_buffer->index; j++)
		{
			buffer = list_get(file->buffer_list, j);
			buf_offset += buffer->size;
		}

		section->header.sh_offset = sizeof(Elf32_Ehdr) + phtab_size + buf_offset;
	}
	
	/* Null section should not have an offset */
	section = list_get(file->section_list, 0);
	section->header.sh_offset = 0;
		
	/* Find actual section sizes */
	for(i = 0; i < list_count(file->section_list); i++)
	{
		section = list_get(file->section_list, i);

		for(j = section->first_buffer->index; j<=section->last_buffer->index; j++)
		{
			buffer = list_get(file->buffer_list, j);
			section->header.sh_size += buffer->size;
		}
	}



	/* Create Section Header Table */

	Elf32_Shdr shtab[list_count(file->section_list)];
	
	/* Enter section headers into section header string table */
	for(i = 0; i < list_count(file->section_list); i++)
	{
		section = list_get(file->section_list, i);
		shtab[i] = section->header;
	}
	

	/* Create program header table */
	Elf32_Phdr phtab[list_count(file->segment_list)];

	for(i = 0; i < list_count(file->segment_list); i++)
	{
		segment = list_get(file->segment_list, i);
		phtab[i] = segment->header;
	}

	
	/* Calculate total size of buffers. Needed to calculate offset 
	 * for section header table. Section header table is typically 
	 * at the end (though not required ). */

	buf_offset = 0;

	for(i = 0; i < list_count(file->buffer_list); i++)
	{
		buffer = list_get(file->buffer_list, i);
		buf_offset += buffer->size;
	}

	file->header.e_shoff = sizeof(Elf32_Ehdr) + phtab_size + buf_offset;
	

	/* Write headers, sections, etc. into buffer */

	elf_enc_buffer_write(bin_buffer, &file->header, sizeof(Elf32_Ehdr));

	elf_enc_buffer_write(bin_buffer, phtab, phtab_size);

	for(i = 0; i < list_count(file->buffer_list); i++)
	{
		buffer = list_get(file->buffer_list, i);
		elf_enc_buffer_write(bin_buffer, buffer->ptr, buffer->size);
	}
	
	elf_enc_buffer_write(bin_buffer, shtab, shtab_size);
}

