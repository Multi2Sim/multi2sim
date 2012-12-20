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

#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>

#include "debug.h"
#include "mhandle.h"


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



