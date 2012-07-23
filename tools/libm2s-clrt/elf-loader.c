#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <debug.h>




/*
 * Private functions
 */
Elf32_Shdr *get_section_header(void *elf, char *name)
{
	int i;
	char *strtab_start;
	Elf32_Ehdr *header = (Elf32_Ehdr *)elf;
	
	assert(header->e_shentsize == sizeof (Elf32_Shdr));
	
	Elf32_Shdr *section = (Elf32_Shdr *) ((char *) header + header->e_shoff);
	Elf32_Shdr *shstr = section + header->e_shstrndx;
	assert(shstr->sh_type == SHT_STRTAB);
	
	strtab_start = (char *) header + shstr->sh_offset;
	
	for (i = 0; i < header->e_shnum; i++)
		if (!strcmp(strtab_start + section[i].sh_name, name))
			return section + i;
	return NULL;
}


void *get_inner_elf_addr(void *outer_elf)
{
	Elf32_Shdr *text = get_section_header(outer_elf, ".text");
	assert(text);
	assert(text->sh_type == SHT_PROGBITS);
	return (char *) outer_elf + text->sh_offset;	
}


void *get_function_info(void *elf, const char *name, size_t **metadata, int *meta_size)
{
	void *addr;
	int i;
	int dynsect_size;
	int strtab_size;
	char *fullname;
	char *metaname;
	char *strtab_start;
	Elf32_Ehdr *elfheader;
	Elf32_Shdr *section;
	Elf32_Sym *dynsect_start;

	fullname = (char *) malloc(strlen(name) + 100);
	if (fullname == NULL)
		fatal("%s: out of memory", __FUNCTION__);
	metaname = (char *) malloc(strlen(name) + 100);
	if (metaname == NULL)
		fatal("%s: out of memory", __FUNCTION__);

	sprintf(fullname, "__OpenCL_%s_kernel", name);
	sprintf(metaname, "__OpenCL_%s_metadata", name);
	
	elfheader = (Elf32_Ehdr *) elf;

	/* fprintf(stderr, "Start of section header: %d\n", elfheader->e_shoff); */
	assert(elfheader->e_shentsize == sizeof (Elf32_Shdr));

	section = (Elf32_Shdr *) ((char *) elfheader + elfheader->e_shoff);	
	dynsect_start = NULL;
	dynsect_size = 0;

	strtab_start = NULL;
	strtab_size = 0;
	for (i = 0; i < elfheader->e_shnum; i++)
	{
		if (section[i].sh_type == SHT_DYNSYM)
		{
			dynsect_start = (Elf32_Sym *) ((char *) elfheader + section[i].sh_offset);
			dynsect_size = section[i].sh_size;
		}

		if (section[i].sh_type == SHT_STRTAB)
		{
			strtab_start = (char *) elfheader + section[i].sh_offset;
			strtab_size = section[i].sh_size;
		}

		if (strtab_start != 0 && dynsect_start != 0)
			break;
	}
	
	assert(dynsect_start != NULL);
	assert(dynsect_size % sizeof (Elf32_Sym) == 0);

	assert(strtab_start != NULL);
	assert(strtab_start[0] == '\0');
	assert(strtab_start[strtab_size - 1] == '\0');

	addr = NULL;
	*metadata = NULL;
	Elf32_Shdr *data = get_section_header(elfheader, ".data");
	for (i = 0; i < dynsect_size / sizeof (Elf32_Sym); i++)
	{
		char *curstr;

		curstr = strtab_start + dynsect_start[i].st_name;
		if (!strcmp(fullname, curstr))
			addr = (void *) ((char *) elfheader + dynsect_start[i].st_value);
		if (!strcmp(metaname, curstr))
		{
			*metadata = (size_t *) ((char *) elfheader + dynsect_start[i].st_value + data->sh_offset - data->sh_addr);
			*meta_size = dynsect_start[i].st_size;
		}
	}

	free(fullname);
	free(metaname);
	return addr;
}

