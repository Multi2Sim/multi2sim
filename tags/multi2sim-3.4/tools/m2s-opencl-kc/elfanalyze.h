#ifndef ELFANALYZE_H
#define ELFANALYZE_H

#include <stdio.h>
#include <stdint.h>
#include <elf.h>

#define elf_debug(...) fprintf(elf_debug_file, __VA_ARGS__)
extern FILE *elf_debug_file;

struct elf_symbol_t {
	char *name;
	uint32_t value;
	uint32_t size;
	int section;
};

struct elf_file_t {
	FILE *f;
	char path[300];
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

struct elf_file_t *elf_open(char *path);
void elf_close(struct elf_file_t *f);

int elf_section_count(struct elf_file_t *f);
int elf_section_info(struct elf_file_t *f, int section,
	char **pname, uint32_t *paddr, uint32_t *poffset,
	uint32_t *psize, uint32_t *pflags);
void elf_section_flags_dump(FILE *f, int flags);
void *elf_section_read(struct elf_file_t *f, int section);
void elf_section_free(void *buf);

void *elf_phdt(struct elf_file_t *f);
uint32_t elf_phdt_base(struct elf_file_t *f);
uint32_t elf_phdr_count(struct elf_file_t *f);
uint32_t elf_phdr_size(struct elf_file_t *f);

char *elf_get_symbol(struct elf_file_t *f, uint32_t addr, uint32_t *poffs);
uint32_t elf_get_entry(struct elf_file_t *f);
int elf_merge_symtab(struct elf_file_t *f, struct elf_file_t *src);

#endif

