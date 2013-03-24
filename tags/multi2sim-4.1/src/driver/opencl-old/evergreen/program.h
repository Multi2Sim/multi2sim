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

#ifndef ARCH_EVERGREEN_EMU_OPENCL_PROGRAM_H
#define ARCH_EVERGREEN_EMU_OPENCL_PROGRAM_H

#include <lib/util/elf-format.h>


struct evg_opencl_program_t
{
	unsigned int id;
	int ref_count;

	unsigned int device_id;  /* Only one device allowed */
	unsigned int context_id;

	/* ELF binary */
	struct elf_file_t *elf_file;

	/* Constant buffers are shared by all kernels compiled in the
	 * same binary. This list is comprised of elf_buffers. */
	struct list_t *constant_buffer_list;
};

struct evg_opencl_program_t *evg_opencl_program_create(void);
void evg_opencl_program_free(struct evg_opencl_program_t *program);

void evg_opencl_program_build(struct evg_opencl_program_t *program);
void evg_opencl_program_initialize_constant_buffers(struct evg_opencl_program_t *program);

void evg_opencl_program_read_symbol(struct evg_opencl_program_t *program, char *symbol_name,
	struct elf_buffer_t *buffer);


#endif

