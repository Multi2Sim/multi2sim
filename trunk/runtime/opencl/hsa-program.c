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
#include <sys/ioctl.h>

#include "elf-format.h"
#include "mhandle.h"
#include "hsa-program.h"


struct opencl_hsa_program_t *opencl_hsa_program_create(
		struct opencl_program_t *parent,
		struct opencl_si_device_t *device,
		void *binary,
		unsigned int length)
{
	struct opencl_hsa_program_t *program;

	/* Initialize */
	program = xcalloc(1, sizeof(struct opencl_hsa_program_t));
	program->type = opencl_runtime_type_hsa;
	program->elf_file = elf_file_create_from_buffer(binary, length,
		"BRIG Binary");

	/* Create program object in driver */
	program->id = ioctl(m2s_active_dev,
		HSAProgramCreate);

	/* Set program binary in driver */
	ioctl(m2s_active_dev, HSAProgramSetBinary,
		program->id, binary, length);

	/* Return */
	return program;
}

void opencl_hsa_program_free(struct opencl_hsa_program_t *program)
{
	assert(program->type == opencl_runtime_type_hsa);

	elf_file_free(program->elf_file);
	free(program);
}

/* Return true is a binary file is a valid HSA program binary. */
int opencl_si_program_valid_binary(void *device, void *binary,
	unsigned int length)
{
	// TODO: Check the constraints of an HSAIL Binary ELF Header
}
