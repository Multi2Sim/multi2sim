/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include "m2skernel.h"

int elf_debug_category;

static void elf_read_ehdr(struct elf_file_t *f)
{
	int count;
	count = fread(&f->ehdr, 1, sizeof(f->ehdr), f->f);

	/* Check magic characters */
	if (f->ehdr.e_ident[0] != 0x7f || f->ehdr.e_ident[1] != 'E' ||
		f->ehdr.e_ident[2] != 'L' || f->ehdr.e_ident[3] != 'F')
		fatal("%s: not a valid ELF file\n", f->path);
	
	/* Check for 32-bit executable (field e_ident[EI_CLASS]) */
	if (f->ehdr.e_ident[4] == 2)
		fatal("%s: this is a 64-bit executable, which is not supported in Multi2Sim. "
			"If you are compiling your own sources on an x86_64 machine, try "
			"using the '-m32' flag in the gcc command-line. If you get compilation "
			"errors about not found '.h' files, check that the 32-bit gcc package "
			"associated with your Linux distribution is installed.",
			f->path);

	/* Check header size */
	if (count != sizeof(f->ehdr) || f->ehdr.e_ehsize != sizeof(f->ehdr))
		fatal("%s: header size is not %d bytes long", f->path, (int) sizeof(f->ehdr));

	/* Check endianness */
	if (f->ehdr.e_ident[5] != 1)
		fatal("%s: ELF file endianness mismatch", f->path);

	/* Debug */
	elf_debug("ELF header:\n");
	elf_debug("  ehdr.e_ident: EI_CLASS=%d, EI_DATA=%d, EI_VERSION=%d\n",
		f->ehdr.e_ident[4], f->ehdr.e_ident[5], f->ehdr.e_ident[6]);
	elf_debug("  ehdr.e_type: %d\n", f->ehdr.e_type);
	elf_debug("  ehdr.e_machine: %u\n", f->ehdr.e_machine);
	elf_debug("  ehdr.e_entry: 0x%x (program entry point)\n", f->ehdr.e_entry);
	elf_debug("  ehdr.e_phoff: %u (program header table offset)\n", f->ehdr.e_phoff);
	elf_debug("  ehdr.e_shoff: %u (section header table offset)\n", f->ehdr.e_shoff);
	elf_debug("  ehdr.e_phentsize: %u\n", f->ehdr.e_phentsize);
	elf_debug("  ehdr.e_phnum: %u\n", f->ehdr.e_phnum);
	elf_debug("  ehdr.e_shentsize: %u\n", f->ehdr.e_shentsize);
	elf_debug("  ehdr.e_shnum: %u\n", f->ehdr.e_shnum);
	elf_debug("  ehdr.e_shstrndx: %u\n", (uint32_t) f->ehdr.e_shstrndx);
	elf_debug("\n");
}


static int elf_symbol_compare(const void *a, const void *b)
{
	struct elf_symbol_t *symbol_a = (struct elf_symbol_t *) a;
	struct elf_symbol_t *symbol_b = (struct elf_symbol_t *) b;
	if (symbol_a->value < symbol_b->value)
		return -1;
	else if (symbol_a->value > symbol_b->value)
		return 1;
	else
		return 0;
}


static void elf_read_symtab(struct elf_file_t *f, int section)
{
	void *buf, *bufnames;
	int i, count;
	Elf32_Sym *sym;
	struct elf_symbol_t *symbol;

	/* Read symbol table section */
	elf_debug("  section %d is symbol table with names in section %d\n",
		section, f->shdr[section].sh_link);
	buf = elf_section_read(f, section);
	bufnames = elf_section_read(f, f->shdr[section].sh_link);
	if (!buf || !bufnames)
		fatal("%s: bogus symbol table", f->path);

	/* Resize symbol table */
	count = f->shdr[section].sh_size / sizeof(Elf32_Sym);
	elf_debug("    %d symbols\n", count);
	if (f->symtab_count + count > f->symtab_size) {
		f->symtab_size = f->symtab_count + count;
		f->symtab = realloc(f->symtab, f->symtab_size * sizeof(struct elf_symbol_t));
		if (!f->symtab)
			fatal("%s: out of memory resizing symbol table", f->path);
	}

	/* Insert symbols */
	for (i = 0; i < count; i++) {
		sym = (Elf32_Sym *) buf + i;
		if (* (char *) (bufnames + sym->st_name) == '\0')
			continue;
		symbol = &f->symtab[f->symtab_count++];
		symbol->value = sym->st_value;
		symbol->size = sym->st_size;
		symbol->section = sym->st_shndx;
		symbol->name = strdup(bufnames + sym->st_name);
		if (!symbol->name)
			fatal("%s: out of memory duplicating symbol name", f->path);
	}

	/* Success */
	elf_free_buffer(buf);
	elf_free_buffer(bufnames);
}


static void elf_read_shdr(struct elf_file_t *f)
{
	int i, count;
	Elf32_Shdr *shdr;

	/* Allocate memory for section headers */
	f->shdr = calloc(f->ehdr.e_shnum, sizeof(Elf32_Shdr));
	if (!f->shdr)
		fatal("%s: out of memory allocating headers", f->path);

	/* Check section size and number */
	if (!f->ehdr.e_shnum || f->ehdr.e_shentsize != sizeof(Elf32_Shdr))
		fatal("%s: number of sections is 0 or section size is not %d",
			f->path, (int) sizeof(Elf32_Shdr));

	/* Read section headers */
	fseek(f->f, f->ehdr.e_shoff, SEEK_SET);
	for (i = 0; i < f->ehdr.e_shnum; i++) {
		shdr = &f->shdr[i];
		count = fread(shdr, 1, sizeof(Elf32_Shdr), f->f);
		if (count != sizeof(Elf32_Shdr))
			fatal("%s: unexpected end of file reading shdr", f->path);
	}

	/* Read section header string table */
	if (f->ehdr.e_shstrndx >= f->ehdr.e_shnum ||
		f->shdr[f->ehdr.e_shstrndx].sh_type != 3)
		fatal("%s: section %d is not a valid string table",
			f->path, f->ehdr.e_shstrndx);
	f->shstr = elf_section_read(f, f->ehdr.e_shstrndx);
	if (!f->shstr)
		fatal("%s: could not read section header string table", f->path);
	
	/* Dump section headers */
	elf_debug("Section headers:\n");
	elf_debug("idx type flags addr       offset     size      link     name\n");
	for (i = 0; i < 80; i++)
		elf_debug("-");
	elf_debug("\n");
	for (i = 0; i < f->ehdr.e_shnum; i++) {
		shdr = &f->shdr[i];
		elf_debug("%-3d %-4u %-5u 0x%-8x 0x%-8x %-9u %-8u %s\n", i,
			shdr->sh_type,
			shdr->sh_flags,
			shdr->sh_addr,
			shdr->sh_offset,
			shdr->sh_size,
			shdr->sh_link,
			(char *) (f->shstr + shdr->sh_name));
	}
	elf_debug("\n");
	
	/* Load symbol table and sort it by address */
	elf_debug("Symbol table:\n");
	for (i = 0; i < f->ehdr.e_shnum; i++)
		if (f->shdr[i].sh_type == 2 || f->shdr[i].sh_type == 11)
			elf_read_symtab(f, i);
	if (f->symtab)
		qsort(f->symtab, f->symtab_count, sizeof(struct elf_symbol_t), elf_symbol_compare);
	elf_debug("\n");
}


static void elf_read_phdr(struct elf_file_t *f)
{
	int i, count;
	Elf32_Phdr *phdr;

	/* No program headers */
	if (!f->ehdr.e_phnum)
		return;

	/* Allocate memory for program headers */
	f->phdr = calloc(f->ehdr.e_phnum, sizeof(Elf32_Phdr));
	if (!f->phdr)
		fatal("%s: out of memory allocating program headers", f->path);

	/* Check phdr size and number */
	if (f->ehdr.e_phentsize != sizeof(Elf32_Phdr))
		fatal("%s: program header size is not %d", f->path, (int) sizeof(Elf32_Phdr));

	/* Read program headers */
	fseek(f->f, f->ehdr.e_phoff, SEEK_SET);
	for (i = 0; i < f->ehdr.e_phnum; i++) {
		phdr = &f->phdr[i];
		count = fread(phdr, 1, sizeof(Elf32_Phdr), f->f);
		if (count != sizeof(Elf32_Phdr))
			fatal("%s: unexpected end of file reading phdr", f->path);

		/* Program header PT_PHDR, specifying location and
		 * size of the program header table itself. */
		if (phdr->p_type == PT_PHDR)
			f->phdt_base = phdr->p_vaddr;
	}

	/* Dump program headers */
	elf_debug("Program headers:\n");
	elf_debug("idx type       offset     vaddr      paddr      filesz    memsz     flags  align\n");
	for (i = 0; i < 80; i++)
		elf_debug("-");
	elf_debug("\n");
	for (i = 0; i < f->ehdr.e_phnum; i++) {
		phdr = &f->phdr[i];
		elf_debug("%-3d 0x%-8x 0x%-8x 0x%-8x 0x%-8x %-9u %-9u %-6u %u\n", i,
			phdr->p_type,
			phdr->p_offset,
			phdr->p_vaddr,
			phdr->p_paddr,
			phdr->p_filesz,
			phdr->p_memsz,
			phdr->p_flags,
			phdr->p_align);
	}
	elf_debug("\n");
}


struct elf_file_t *elf_open(char *path)
{
	struct elf_file_t *elf;

	/* Create structure */
	elf = calloc(1, sizeof(struct elf_file_t));
	if (!elf)
		fatal("elf_open: out of memory\n");

	/* Open file */
	elf->f = fopen(path, "rb");
	if (!elf->f)
		fatal("%s: cannot open executable file", path);
	if (strlen(path) >= sizeof(elf->path))
		fatal("%s: executable file path too long", path);
	strcpy(elf->path, path);

	/* Get file size */
	fseek(elf->f, 0, SEEK_END);
	elf->size = ftell(elf->f);
	fseek(elf->f, 0, SEEK_SET);

	/* Read header, section headers, and program headers. */
	elf_debug("\n%s: reading ELF file\n", path);
	elf_read_ehdr(elf);
	elf_read_shdr(elf);
	elf_read_phdr(elf);
	
	/* Return elf file structure */
	return elf;
}


void elf_close(struct elf_file_t *f)
{
	int i;

	fclose(f->f);
	if (f->shdr)
		free(f->shdr);
	if (f->shstr)
		free(f->shstr);
	if (f->phdr)
		free(f->phdr);

	/* Free symbol table */
	if (f->symtab) {
		for (i = 0; i < f->symtab_count; i++)
			free(f->symtab[i].name);
		free(f->symtab);
	}

	/* Free ELF file struct */
	free(f);
}






/*
 * Read from ELF file
 */

void *elf_read_buffer(struct elf_file_t *elf, uint32_t offset, uint32_t size)
{
	void *buf;
	int count;

	/* Check bounds */
	if (offset + size > elf->size) {
		elf_debug("%s: invalid offset/size\n", __FUNCTION__);
		return NULL;
	}
	
	/* Allocate buffer */
	buf = calloc(1, size);
	if (!buf) {
		elf_debug("%s: out of memory\n", __FUNCTION__);
		return NULL;
	}

	/* Copy file contents */
	fseek(elf->f, offset, SEEK_SET);
	count = fread(buf, 1, size, elf->f);
	if (count != size) {
		elf_debug("%s: could not read in file\n", __FUNCTION__);
		free(buf);
		return NULL;
	}

	/* Return read buffer */
	return buf;
}


void elf_free_buffer(void *buf)
{
	free(buf);
}




/*
 * Read from sections
 */

void *elf_section_read_offset(struct elf_file_t *elf, int section, uint32_t offset, uint32_t size)
{
	Elf32_Shdr *shdr;

	/* Check bounds */
	if (section < 0 || section >= elf->ehdr.e_shnum) {
		elf_debug("%s: section out of range\n", __FUNCTION__);
		return NULL;
	}

	/* Check valid offset and size */
	shdr = &elf->shdr[section];
	if (offset + size > shdr->sh_size || !size) {
		elf_debug("%s: invalid offset/size\n", __FUNCTION__);
		return NULL;
	}

	/* If section type is SHT_NOBITS, return zeroed buffer */
	if (elf->shdr[section].sh_type == 8)
		return calloc(1, size);
	
	/* Read section from ELF file */
	return elf_read_buffer(elf, shdr->sh_offset + offset, size);
}


void *elf_section_read(struct elf_file_t *elf, int section)
{
	Elf32_Shdr *shdr;

	/* Check bounds */
	if (section < 0 || section >= elf->ehdr.e_shnum) {
		elf_debug("%s: section out of range\n", __FUNCTION__);
		return NULL;
	}

	/* Call partial read */
	shdr = &elf->shdr[section];
	return elf_section_read_offset(elf, section, 0, shdr->sh_size);
}


int elf_section_count(struct elf_file_t *f)
{
	return f->ehdr.e_shnum;
}


int elf_section_info(struct elf_file_t *f, int section,
	char **pname, uint32_t *paddr, uint32_t *psize, uint32_t *pflags)
{
	if (section < 0 || section >= f->ehdr.e_shnum) {
		elf_debug("elf_section_size: section out of range\n");
		return 0;
	}
	if (pname)
		*pname = f->shstr + f->shdr[section].sh_name;
	if (paddr)
		*paddr = f->shdr[section].sh_addr;
	if (psize)
		*psize = f->shdr[section].sh_size;
	if (pflags)
		*pflags = f->shdr[section].sh_flags;
	return 1;
}


uint32_t elf_section_addr(struct elf_file_t *f, int section)
{
	if (section < 0 || section >= f->ehdr.e_shnum) {
		elf_debug("elf_section_size: section out of range\n");
		return 0;
	}
	return f->shdr[section].sh_addr;
}




/*
 * ELF symbols
 */

struct elf_symbol_t *elf_get_symbol_by_address(struct elf_file_t *f, uint32_t addr, uint32_t *poffs)
{
	int min, max, mid;
	
	/* Binary search */
	if (!f->symtab_count)
		return NULL;
	if (addr < f->symtab[0].value)
		return NULL;
	min = 0;
	max = f->symtab_count;
	while (min + 1 < max) {
		mid = (max + min) / 2;
		if (f->symtab[mid].value > addr)
			max = mid;
		else if (f->symtab[mid].value < addr)
			min = mid;
		else {
			min = mid;
			break;
		}
	}
	
	/* Go backwards to find appropriate symbol */
	while (min > 0 && f->symtab[min].value == f->symtab[min - 1].value)
		min--;
	if (!f->symtab[min].value)
		return NULL;
	if (poffs)
		*poffs = addr - f->symtab[min].value;
	return &f->symtab[min];
}


struct elf_symbol_t *elf_get_symbol_by_name(struct elf_file_t *f, char *name)
{
	int i;
	for (i = 0; i < f->symtab_count; i++)
		if (!strcmp(f->symtab[i].name, name))
			return &f->symtab[i];
	return NULL;
}


/* Add to the destination elf file all symbols read in the source elf file.
 * Return 0 if operation failed. */
int elf_merge_symtab(struct elf_file_t *f, struct elf_file_t *src)
{
	int i;
	struct elf_symbol_t *srcsym, *dstsym;

	/* Resize table */
	if (f->symtab_count + src->symtab_count > f->symtab_size) {
		f->symtab_size = f->symtab_count + src->symtab_count;
		f->symtab = realloc(f->symtab, f->symtab_size * sizeof(struct elf_symbol_t));
		if (!f->symtab)
			return 0;
	}

	/* Add symbols */
	for (i = 0; i < src->symtab_count; i++) {
		srcsym = &src->symtab[i];
		dstsym = &f->symtab[f->symtab_count++];
		dstsym->value = srcsym->value;
		dstsym->name = strdup(srcsym->name);
		if (!dstsym->name)
			return 0;
	}

	/* Sort table */
	qsort(f->symtab, f->symtab_count, sizeof(struct elf_symbol_t), elf_symbol_compare);
	return 1;
}


uint32_t elf_get_entry(struct elf_file_t *f)
{
	return f->ehdr.e_entry;
}


void *elf_phdt(struct elf_file_t *f)
{
	return f->phdr;
}

uint32_t elf_phdt_base(struct elf_file_t *f)
{
	return f->phdt_base;
}


uint32_t elf_phdr_count(struct elf_file_t *f)
{
	return f->ehdr.e_phnum;
}


uint32_t elf_phdr_size(struct elf_file_t *f)
{
	return f->ehdr.e_phentsize;
}

