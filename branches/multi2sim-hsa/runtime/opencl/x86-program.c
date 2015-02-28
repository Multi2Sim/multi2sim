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
#include <dlfcn.h>

#include "debug.h"
#include "device.h"
#include "elf-format.h"
#include "list.h"
#include "mhandle.h"
#include "program.h"
#include "x86-program.h"


struct opencl_x86_program_t *opencl_x86_program_create(
		struct opencl_program_t *parent,
		struct opencl_x86_device_t *device,
		void *binary,
		unsigned int length)
{
	struct opencl_x86_program_t *program;
	struct elf_file_t *elf_file;
	struct elf_section_t *section;

	int f;
	int found;
	int section_index;

	/* Initialize */
	program = xcalloc(1, sizeof(struct opencl_x86_program_t));
	program->type = opencl_runtime_type_x86;
	program->parent = parent;
	program->device = device;

	/* Load ELF binary */
	elf_file = elf_file_create_from_buffer((void *) binary, length,
			"OpenCL binary");

	/* Find '.text' section */
	found = 0;
	LIST_FOR_EACH(elf_file->section_list, section_index)
	{
		section = list_get(elf_file->section_list, section_index);
		if (!strcmp(section->name, ".text"))
		{
			found = 1;
			break;
		}
	}

	/* Section not found */
	if (!found)
		fatal("%s: no '.text' section in binary", __FUNCTION__);

	/* Extract section to temporary file */
	program->file_name = xstrdup("/tmp/tmp_XXXXXX");
	f = mkstemp(program->file_name);
	if (f == -1)
		fatal("%s: could not create temporary file", __FUNCTION__);
	if (write(f, section->buffer.ptr, section->buffer.size) != section->buffer.size)
		fatal("%s: could not write to temporary file", __FUNCTION__);
	close(f);

	/* Close ELF external binary */
	elf_file_free(elf_file);

	/* Load internal binary for dynamic linking */
	program->dlhandle = dlopen(program->file_name, RTLD_NOW);
	if (!program->dlhandle)
		fatal("%s: could not open ELF binary derived from program", __FUNCTION__);		

	/* Return */
	return program;
}


void opencl_x86_program_free(struct opencl_x86_program_t *program)
{
	assert(program->type == opencl_runtime_type_x86);

	dlclose(program->dlhandle);
	unlink(program->file_name);
	free(program->file_name);
	free(program);
}


int opencl_x86_program_valid_binary(
	void *device,
	void *binary,
	unsigned int length)
{
	Elf32_Ehdr *h = (Elf32_Ehdr *) binary;

	return h->e_machine == 0x7d2;
}

