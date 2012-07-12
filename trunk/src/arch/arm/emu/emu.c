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


void arm_elf_function_symbol(struct elf_file_t *elf_file, unsigned int inst_addr)
{
	unsigned int i;
	struct elf_symbol_t *symbol;
	for ( i = 0; i < list_count(elf_file->symbol_table); i++)
	{
		symbol = (struct elf_symbol_t* )list_get(elf_file->symbol_table, i);
		if(symbol->value == inst_addr)
		{
			if((!strncmp(symbol->name, "$",1)))
			{
				continue;
			}
			else
			{
				printf ("\n%08x <%s>\n", symbol->value, symbol->name);
				break;
			}
		}
	}
}

unsigned int arm_dump_word_symbol(struct elf_file_t *elf_file, unsigned int inst_addr, void *inst_ptr)
{
	struct elf_symbol_t *symbol;
	unsigned int word_flag;
 	symbol = elf_symbol_get_by_address(elf_file, inst_addr,	NULL);

	if((!strncmp(symbol->name, "$d",2)))
	{
		printf (".word   0x%08x\n", *(unsigned int *)inst_ptr);
		word_flag = 1;
	}
	else
	{
		word_flag = 0;
	}

	return (word_flag);
}

/*
 * Arm disassembler
 */
void arm_emu_disasm(char *path)
{
	struct elf_file_t *elf_file;
	struct elf_section_t *section;

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

		arm_elf_function_symbol(elf_file, (section->header->sh_addr + inst_index));

		arm_inst_hex_dump(stdout, inst_ptr, (section->header->sh_addr + inst_index));

		if (!arm_dump_word_symbol(
			elf_file, (section->header->sh_addr + inst_index),
			inst_ptr))
			arm_inst_dump(stdout, inst_str, MAX_STRING_SIZE, inst_ptr , inst_index,
				(section->header->sh_addr + inst_index));

		inst_index += 4;
	}

	/* Free external ELF */
	elf_file_free(elf_file);
}
