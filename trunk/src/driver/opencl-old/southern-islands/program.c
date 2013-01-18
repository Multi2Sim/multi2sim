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

#include <assert.h>

#include <arch/southern-islands/emu/emu.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "mem.h"
#include "opencl.h"
#include "program.h"
#include "repo.h"


struct si_opencl_program_t *si_opencl_program_create()
{
	struct si_opencl_program_t *program;
	int i;

	/* Initialize */
	program = xcalloc(1, sizeof(struct si_opencl_program_t));
	program->id = si_opencl_repo_new_object_id(si_emu->opencl_repo,
		si_opencl_object_program);
	program->ref_count = 1;

	/* Constant buffers encoded in ELF file */
	program->constant_buffer_list = list_create_with_size(25);
	for(i = 0; i < 25; i++) 
		list_add(program->constant_buffer_list, NULL);

	/* Return */
	si_opencl_repo_add_object(si_emu->opencl_repo, program);
	return program;
}


void si_opencl_program_free(struct si_opencl_program_t *program)
{
	/* Free lists */
	list_free(program->constant_buffer_list);

	if (program->elf_file)
		elf_file_free(program->elf_file);
	si_opencl_repo_remove_object(si_emu->opencl_repo, program);
	free(program);
}


static char *err_si_opencl_southern_islands_format =
	"\tYour application tried to load a pre-compiled OpenCL kernel binary which\n"
	"\tdoes not contain code in the Southern Islands ISA. Please, check that the\n"
	"\toff-line compilation of your kernel targets this GPU architecture supported\n"
	"\tby Multi2Sim.\n";


void si_opencl_program_build(struct si_opencl_program_t *program)
{
	/* Open ELF file and check that it corresponds to a Southern Islands pre-compiled kernel */
	assert(program->elf_file);
	if (program->elf_file->header->e_machine != 0x3fd)
		fatal("%s: invalid binary file.\n%s", __FUNCTION__,
				err_si_opencl_southern_islands_format);
}


/* Look for a symbol name in program binary and read it from its corresponding section.
 * The contents pointed to by the symbol are stored in an ELF buffer.
 * No allocation happens here, the target buffer will just point to the contents of
 * an existing section. */

static char *err_si_opencl_elf_symbol =
	"\tThe ELF file analyzer is trying to find a name in the ELF symbol table.\n"
	"\tIf it is not found, it probably means that your application is requesting\n"
	"\texecution of a kernel function that is not present in the encoded binary.\n"
	"\tPlease, check the parameters passed to the 'clCreateKernel' function in\n"
	"\tyour application.\n"
	"\tThis could be also a symptom of compiling an OpenCL kernel source on a\n"
	"\tmachine with an installation of the APP SDK (using 'm2s-opencl-kc') but\n"
	"\twith an incorrect or missing installation of the GPU driver. In this case,\n"
	"\tthe tool will still compile the kernel into LLVM, but the ISA section will\n"
	"\tbe missing in the kernel binary.\n";

void si_opencl_program_read_symbol(struct si_opencl_program_t *program, char *symbol_name,
	struct elf_buffer_t *buffer)
{
	struct elf_file_t *elf_file;
	struct elf_symbol_t *symbol;
	struct elf_section_t *section;
	
	/* Look for symbol */
	elf_file = program->elf_file;
	assert(elf_file);
	symbol = elf_symbol_get_by_name(elf_file, symbol_name);
	if (!symbol)
		fatal("%s: ELF symbol '%s' not found.\n%s", __FUNCTION__,
			symbol_name, err_si_opencl_elf_symbol);
	
	/* Get section where the symbol is pointing */
	section = list_get(elf_file->section_list, symbol->section);
	assert(section);
	if (symbol->value + symbol->size > section->header->sh_size)
		fatal("%s: ELF symbol '%s' exceeds section '%s' boundaries.\n%s",
			__FUNCTION__, symbol->name, section->name, err_si_opencl_elf_symbol);

	/* Update buffer */
	buffer->ptr = section->buffer.ptr + symbol->value;
	buffer->size = symbol->size;
	buffer->pos = 0;
}


void si_opencl_program_initialize_constant_buffers(struct si_opencl_program_t *program)
{
	struct elf_file_t *elf_file;
	struct elf_symbol_t *elf_symbol;
	struct elf_buffer_t elf_buffer;
	struct si_opencl_mem_t *mem;
	char symbol_name[MAX_STRING_SIZE];
	int i;

	elf_file = program->elf_file;

	/* We can't tell how many constant buffers exist in advance, but we
	 * know they should be enumerated, starting with '2'.  This loop
	 * searches until a constant buffer matching the format is not found. */
	for (i = 2; i < 25; i++) 
	{
		/* Create string of symbol name */
		sprintf(symbol_name, "__OpenCL_%d_global", i);

		/* Check to see if symbol exists */
		elf_symbol = elf_symbol_get_by_name(elf_file, symbol_name);
                if (elf_symbol == NULL) {

			break;
		}
		si_opencl_debug("  constant buffer '%s' found with size %d\n",
			elf_symbol->name, elf_symbol->size);

		/* Read the elf symbol into a buffer */
		si_opencl_program_read_symbol(program, elf_symbol->name, &elf_buffer);

		/* Create a memory object and copy the constant buffer data to it */
		mem = si_opencl_mem_create();
		mem->type = 0;  /* FIXME */
		mem->size = elf_buffer.size;
		mem->flags = 0; /* TODO Change to CL_MEM_READ_ONLY */
		mem->host_ptr = 0;

		/* Assign position in device global memory */
		mem->device_ptr = si_emu->global_mem_top;
		si_emu->global_mem_top += mem->size;

		/* Copy constant buffer into device memory */
		mem_write(si_emu->global_mem, mem->device_ptr, mem->size, elf_buffer.ptr);

		/* Add the memory object to the constant buffer list */
		list_set(program->constant_buffer_list, i, mem);
	}
} 

