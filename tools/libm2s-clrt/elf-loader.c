#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <util/debug.h>
#include <dlfcn.h>

/*
 * Private functions
 */
Elf32_Shdr *get_section_header(const void *elf, char *name)
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


void *get_inner_elf_addr(const unsigned char *outer_elf, uint32_t *size)
{
	Elf32_Shdr *text = get_section_header(outer_elf, ".text");
	assert(text);
	assert(text->sh_type == SHT_PROGBITS);
	if (size)
		*size = text->sh_size;
	return (char *) outer_elf + text->sh_offset;	
}


void *get_function_info(void *handle, const char *name, size_t **metadata)
{
	void *addr;
	char *fullname;
	char *metaname;

	fullname = (char *) malloc(strlen(name) + 100);
	if (!fullname)
		fatal("%s: out of memory", __FUNCTION__);
	metaname = (char *) malloc(strlen(name) + 100);
	if (!metaname)
		fatal("%s: out of memory", __FUNCTION__);

	sprintf(fullname, "__OpenCL_%s_kernel", name);
	sprintf(metaname, "__OpenCL_%s_metadata", name);

	*metadata = (size_t *)dlsym(handle, metaname);
	addr = dlsym(handle, fullname);

	free(fullname);
	free(metaname);
	return addr;
}

