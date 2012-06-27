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

#include <fermi-emu.h>
#include <x86-emu.h>



/*
 * Global variables
 */


struct frm_emu_t *frm_emu;

long long frm_emu_max_cycles = 0;
long long frm_emu_max_inst = 0;
int frm_emu_max_functions = 0;

enum frm_emu_kind_t frm_emu_kind = frm_emu_functional;

char *frm_emu_cuda_binary_name = "";
char *frm_emu_report_file_name = "";
FILE *frm_emu_report_file = NULL;

int frm_emu_warp_size = 32;



/*
 * Fermi Emulator
 */


void frm_emu_init(void)
{
        /* Allocate */
        frm_emu = calloc(1, sizeof(struct frm_emu_t));
        if (!frm_emu)
                fatal("%s: out of memory", __FUNCTION__);

        frm_emu->const_mem = mem_create();
        frm_emu->const_mem->safe = 0;
        frm_emu->global_mem = mem_create();
        frm_emu->global_mem->safe = 0;

	frm_disasm_init();
	frm_isa_init();
	//frm_emu_disasm("data/vectorAdd_kernel.cubin");

        /* Create device */
        frm_cuda_object_list = linked_list_create();
        frm_cuda_device_create();
}


void frm_emu_done(void)
{
	/* Free CUDA object list */
	frm_cuda_object_free_all();
	linked_list_free(frm_cuda_object_list);

	frm_isa_done();

        mem_free(frm_emu->const_mem);
        mem_free(frm_emu->global_mem);
        free(frm_emu);
}




/* 
 * Fermi disassembler
 */


void frm_emu_disasm(char *path)
{
	struct elf_file_t *elf_file;
	struct elf_section_t *section;
	int inst_index;
	char inst_str[MAX_STRING_SIZE];
	int i;

	/* Initialization */
	frm_disasm_init();

	/* Find .text section which saves instruction bits */
	elf_file = elf_file_create_from_path(path);

	for (i = 0; i < list_count(elf_file->section_list); ++i)
	{
		section = (struct elf_section_t *)list_get(elf_file->section_list, i);
		//FILE *fp = fopen(section->name, "w");
		fprintf(stdout, "sec: %s\n", section->name);
		int j;
		for (j = 0; j < section->buffer.size; ++j) {
			if (j != 0 && j%8 == 0)
				fprintf(stdout, "\n");
			fprintf(stdout, "%02x", *((unsigned char *)(section->buffer.ptr)+j));
		}
		fprintf(stdout, "\n");
		//fclose(fp);
	}
	for (i = 0; i < list_count(elf_file->section_list); ++i)
	{
		section = (struct elf_section_t *)list_get(elf_file->section_list, i);
		if (!strncmp(section->name, ".text", 5))
			break;
	}
	if (i == list_count(elf_file->section_list))
		fatal(".text section not found!\n");

	/* Decode and dump instructions */
	for (inst_index = 0; inst_index < section->buffer.size/8; ++inst_index)
	{
		frm_inst_hex_dump(stdout, (unsigned char*)(section->buffer.ptr), inst_index);
		frm_inst_dump(stdout, inst_str, MAX_STRING_SIZE, (unsigned char*)(section->buffer.ptr), inst_index);
	}

	/* Free external ELF */
	elf_file_free(elf_file);
}

