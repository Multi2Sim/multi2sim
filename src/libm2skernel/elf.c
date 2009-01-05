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

struct elf_symbol_t {
	char *name;
	uint32_t value;
};

struct elf_file_t {
	FILE *f;
	void *shstr;  /* Section header string table */
	Elf32_Ehdr ehdr;  /* ELF header */
	Elf32_Shdr *shdr;  /* Section headers (array of ehdr.shnum elements) */
	Elf32_Phdr *phdr;  /* Program headers (array of ehdr.phnum elements) */
	uint32_t phdt_base;  /* Program header table base */

	/* Symbol table */
	int symtab_size;
	int symtab_count;
	struct elf_symbol_t *symtab;
};


static int elf_read_ehdr(struct elf_file_t *f)
{
	int count;
	count = fread(&f->ehdr, 1, sizeof(f->ehdr), f->f);

	/* Check magic characters */
	if (f->ehdr.e_ident[0] != 0x7f || f->ehdr.e_ident[1] != 'E' ||
		f->ehdr.e_ident[2] != 'L' || f->ehdr.e_ident[3] != 'F') {
		elf_debug("error: not an ELF file\n");
		return 0;
	}

	/* Check header size */
	if (count != sizeof(f->ehdr) || f->ehdr.e_ehsize != sizeof(f->ehdr)) {
		elf_debug("error: header size is not %d bytes\n", (int) sizeof(f->ehdr));
		return 0;
	}

	/* Check endianness */
	if (f->ehdr.e_ident[5] != 1) {
		elf_debug("error: not little endian\n");
		return 0;
	}

	/* Debug */
	elf_debug("ELF header:\n");
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

	return 1;
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


static int elf_read_symtab(struct elf_file_t *f, int section)
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
		return 0;

	/* Resize symbol table */
	count = f->shdr[section].sh_size / sizeof(Elf32_Sym);
	elf_debug("    %d symbols\n", count);
	if (f->symtab_count + count > f->symtab_size) {
		f->symtab_size = f->symtab_count + count;
		f->symtab = realloc(f->symtab, f->symtab_size * sizeof(struct elf_symbol_t));
		if (!f->symtab)
			return 1;
	}

	/* Insert symbols */
	for (i = 0; i < count; i++) {
		symbol = &f->symtab[f->symtab_count++];
		sym = (Elf32_Sym *) buf + i;
		symbol->value = sym->st_value;
		symbol->name = strdup(bufnames + sym->st_name);
	}

	/* Success */
	elf_section_free(buf);
	elf_section_free(bufnames);
	return 1;
}


static int elf_read_shdr(struct elf_file_t *f)
{
	int i, count;
	Elf32_Shdr *shdr;

	/* Allocate memory for section headers */
	f->shdr = calloc(f->ehdr.e_shnum, sizeof(Elf32_Shdr));
	if (!f->shdr) {
		elf_debug("out of memory\n");
		return 1;
	}

	/* Check section size and number */
	if (!f->ehdr.e_shnum || f->ehdr.e_shentsize != sizeof(Elf32_Shdr)) {
		elf_debug("error: number of sections is 0 or section size is not %d\n",
			(int) sizeof(Elf32_Shdr));
		goto error;
	}

	/* Read section headers */
	fseek(f->f, f->ehdr.e_shoff, SEEK_SET);
	for (i = 0; i < f->ehdr.e_shnum; i++) {
		shdr = &f->shdr[i];
		count = fread(shdr, 1, sizeof(Elf32_Shdr), f->f);
		if (count != sizeof(Elf32_Shdr)) {
			elf_debug("error: unexpected end of file reading shdr\n");
			goto error;
		}
	}

	/* Read section header string table */
	if (f->ehdr.e_shstrndx >= f->ehdr.e_shnum ||
		f->shdr[f->ehdr.e_shstrndx].sh_type != 3) {
		elf_debug("error: section %d is not a valid string table\n",
			f->ehdr.e_shstrndx);
		goto error;
	}
	f->shstr = elf_section_read(f, f->ehdr.e_shstrndx);
	if (!f->shstr)
		goto error;
	
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
		if (f->shdr[i].sh_type == 2)
			elf_read_symtab(f, i);
	if (f->symtab)
		qsort(f->symtab, f->symtab_count, sizeof(struct elf_symbol_t), elf_symbol_compare);
	elf_debug("\n");

	/* Success */
	return 1;

error:
	free(f->shdr);
	return 0;
}


static int elf_read_phdr(struct elf_file_t *f)
{
	int i, count;
	Elf32_Phdr *phdr;

	/* No program headers */
	if (!f->ehdr.e_phnum)
		return 0;

	/* Allocate memory for program headers */
	f->phdr = calloc(f->ehdr.e_phnum, sizeof(Elf32_Phdr));
	if (!f->phdr) {
		elf_debug("out of memory\n");
		return 1;
	}

	/* Check phdr size and number */
	if (f->ehdr.e_phentsize != sizeof(Elf32_Phdr)) {
		elf_debug("error: program header size is not %d\n",
			(int) sizeof(Elf32_Phdr));
		goto error;
	}

	/* Read program headers */
	fseek(f->f, f->ehdr.e_phoff, SEEK_SET);
	for (i = 0; i < f->ehdr.e_phnum; i++) {
		phdr = &f->phdr[i];
		count = fread(phdr, 1, sizeof(Elf32_Phdr), f->f);
		if (count != sizeof(Elf32_Phdr)) {
			elf_debug("error: unexpected end of file reading phdr\n");
			goto error;
		}

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
	
	/* Success */
	return 1;

error:
	free(f->shdr);
	return 0;
}


struct elf_file_t *elf_open(char *path)
{
	struct elf_file_t *f;

	/* Create structure */
	f = calloc(1, sizeof(struct elf_file_t));
	if (!f) {
		elf_debug("elf_open: out of memory\n");
		return NULL;
	}

	/* Open file */
	f->f = fopen(path, "rb");
	if (!f->f) {
		elf_debug("%s: cannot open path\n", path);
		return NULL;
	}
	elf_debug("\n%s: reading ELF file\n", path);

	/* Read header */
	if (!elf_read_ehdr(f))
		goto error;
	
	/* Read section headers */
	if (!elf_read_shdr(f))
		goto error;
	
	/* Read program headers */
	if (!elf_read_phdr(f))
		goto error;
	
	/* Return elf file structure */
	return f;

error:
	elf_close(f);
	return NULL;
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


void *elf_section_read(struct elf_file_t *f, int section)
{
	void *buf;
	Elf32_Shdr *shdr;
	int count;

	/* Allocate memory */
	if (section < 0 || section >= f->ehdr.e_shnum) {
		elf_debug("elf_section_read: section out of range\n");
		return NULL;
	}
	shdr = &f->shdr[section];
	buf = calloc(1, shdr->sh_size);
	if (!buf) {
		elf_debug("elf_section_read: out of memory\n");
		return NULL;
	}

	/* Section of type SHT_NOBITS */
	if (f->shdr[section].sh_type == 8) {
		memset(buf, 0, shdr->sh_size);
		return buf;
	}

	/* Copy section contents */
	fseek(f->f, shdr->sh_offset, SEEK_SET);
	count = fread(buf, 1, shdr->sh_size, f->f);
	if (count != shdr->sh_size) {
		elf_debug("elf_section_read: could not read section\n");
		free(buf);
		return NULL;
	}
	return buf;
}


void elf_section_free(void *buf)
{
	free(buf);
}


char *elf_get_symbol(struct elf_file_t *f, uint32_t addr, uint32_t *poffs)
{
	int min, max, mid;
	
	/* Binary search */
	if (!f->symtab_count)
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
	if (poffs)
		*poffs = addr - f->symtab[min].value;
	return f->symtab[min].name;
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

