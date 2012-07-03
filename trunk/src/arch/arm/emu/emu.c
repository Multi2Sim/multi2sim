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

#include <arm-emu.h>

/*
 * Arm disassembler
 */

void arm_emu_disasm(char *path)
{
	struct elf_file_t *elf_file;
	struct elf_section_t *section;
	struct elf_symbol_t *symbol;

	char inst_str[MAX_STRING_SIZE];
	int i;
	unsigned int inst_index;
	void *inst_ptr;

	/* Initialization */
	arm_disasm_init();

	/* Find .text section which saves instruction bits */
	elf_file = elf_file_create_from_path(path);

	for (i = 0; i < list_count(elf_file->section_list); ++i)
	{
		section = (struct elf_section_t *)list_get(elf_file->section_list, i);
		if (!strncmp(section->name, ".text", 5))
			break;
	}
	if (i == list_count(elf_file->section_list))
		fatal(".text section not found!\n");

	/* Decode and dump instructions */
	for (inst_ptr = section->buffer.ptr; inst_ptr < section->buffer.ptr +
			section->buffer.size; inst_ptr += 4)
	{
		symbol = elf_symbol_get_by_address(elf_file, (section->header->sh_addr + inst_index),
			NULL);
		if((section->header->sh_addr + inst_index) == (symbol->value))
		{
			printf ("\n%08x <%s>\n", symbol->value, symbol->name);
		}
		arm_inst_hex_dump(stdout, inst_ptr, (section->header->sh_addr + inst_index));
		arm_inst_dump(stdout, inst_str, MAX_STRING_SIZE, inst_ptr , inst_index);
		inst_index += 4;
	}

	/* Free external ELF */
	elf_file_free(elf_file);
}
