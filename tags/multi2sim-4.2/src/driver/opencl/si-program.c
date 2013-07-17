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


#include <arch/southern-islands/emu/emu.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "opencl.h"
#include "si-program.h"


/*
 * Program List
 */


struct list_t *opencl_si_program_list;

void opencl_si_program_list_init(void)
{
	/* Already initialized */
	if (opencl_si_program_list)
		return;

	/* Initialize and add one empty element */
	opencl_si_program_list = list_create();
	list_add(opencl_si_program_list, NULL);
}


void opencl_si_program_list_done(void)
{
	int index;
	struct opencl_si_program_t *program;

	/* Not initialized */
	if (!opencl_si_program_list)
		return;

	/* Free list of Southern Islands programs */
	LIST_FOR_EACH(opencl_si_program_list, index)
	{
		program = list_get(opencl_si_program_list, index);
		if (program)
			opencl_si_program_free(program);
	}
	list_free(opencl_si_program_list);
}




/*
 * Constant Buffer
 */

struct opencl_si_constant_buffer_t *opencl_si_constant_buffer_create(int id,
	unsigned int device_ptr, unsigned int size)
{
	struct opencl_si_constant_buffer_t *constant_buffer;

	/* Initialize */
	constant_buffer = xcalloc(1, sizeof(struct opencl_si_constant_buffer_t));
	constant_buffer->id = id;
	constant_buffer->device_ptr = device_ptr;
	constant_buffer->size = size;

	/* Return */
	return constant_buffer;
}


void opencl_si_constant_buffer_free(struct opencl_si_constant_buffer_t *constant_buffer)
{
	free(constant_buffer);
}




/*
 * Program
 */

static void opencl_si_program_initialize_constant_buffers(
		struct opencl_si_program_t *program)
{
	struct elf_file_t *elf_file;
	struct elf_buffer_t elf_buffer;
	struct elf_symbol_t *symbol;
	struct opencl_si_constant_buffer_t *constant_buffer;

	char symbol_name[MAX_STRING_SIZE];
	int i;

	elf_file = program->elf_file;
	assert(elf_file);

	/* Constant buffers encoded in ELF file */
	program->constant_buffer_list = list_create_with_size(25);
	for (i = 0; i < 25; i++) 
		list_add(program->constant_buffer_list, NULL);

	/* We can't tell how many constant buffers exist in advance, but we
	 * know they should be enumerated, starting with '2'.  This loop
	 * searches until a constant buffer matching the format is not 
	 * found. */
	for (i = 2; i < 25; i++) 
	{
		/* Create string of symbol name */
		sprintf(symbol_name, "__OpenCL_%d_global", i);

		/* Check to see if symbol exists */
		symbol = elf_symbol_get_by_name(elf_file, symbol_name);
		if (!symbol)
			break;

		/* Read the elf symbol into a buffer */
		elf_symbol_read_content(elf_file, symbol, &elf_buffer);
		opencl_debug("\tconstant buffer '%s' found with size %d\n",
			symbol->name, symbol->size);

		/* Copy constant buffer into device memory */
		mem_write(si_emu->video_mem, si_emu->video_mem_top,
			symbol->size, elf_buffer.ptr);

		/* Create buffer */
		constant_buffer = opencl_si_constant_buffer_create(i,
			si_emu->video_mem_top, symbol->size);
		si_emu->video_mem_top += symbol->size;

		/* Add the constant buffer to the list */
		list_set(program->constant_buffer_list, i, constant_buffer);
	}
} 

struct opencl_si_program_t *opencl_si_program_create(void)
{
	struct opencl_si_program_t *program;

	/* Initialize */
	program = xcalloc(1, sizeof(struct opencl_si_program_t));

	/* Insert in program list */
	opencl_si_program_list_init();
	list_add(opencl_si_program_list, program);
	program->id = list_count(opencl_si_program_list) - 1;

	/* Return */
	return program;
}


void opencl_si_program_free(struct opencl_si_program_t *program)
{
	int index;

	/* Free constant buffers */
	if (program->constant_buffer_list)
	{
		LIST_FOR_EACH(program->constant_buffer_list, index)
			opencl_si_constant_buffer_free(list_get(
					program->constant_buffer_list,
					index));
		list_free(program->constant_buffer_list);
	}

	/* ELF file */
	if (program->elf_file)
		elf_file_free(program->elf_file);

	/* Free program */
	free(program);
}


void opencl_si_program_set_binary(struct opencl_si_program_t *program,
		void *buf, unsigned int size)
{
	char name[MAX_STRING_SIZE];

	/* Already set */
	if (program->elf_file)
		fatal("%s: binary already set", __FUNCTION__);

	/* Load ELF binary from guest memory */
	snprintf(name, sizeof name, "program[%d].externalELF", program->id);
	program->elf_file = elf_file_create_from_buffer(buf, size, name);

	/* Initialize constant buffers */
	opencl_si_program_initialize_constant_buffers(program);
}

