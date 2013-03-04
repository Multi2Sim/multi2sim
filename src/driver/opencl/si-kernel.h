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

#ifndef DRIVER_OPENCL_SI_KERNEL_H
#define DRIVER_OPENCL_SI_KERNEL_H

#include <lib/util/elf-format.h>


/*
 * Kernel List
 */

extern struct list_t *opencl_si_kernel_list;

void opencl_si_kernel_list_init(void);
void opencl_si_kernel_list_done(void);




/*
 * Argument
 */

struct opencl_si_arg_t
{
};

struct opencl_si_arg_t *opencl_si_arg_create(void);
void opencl_si_arg_free(struct opencl_si_arg_t *arg);



/*
 * Kernel
 */


struct opencl_si_kernel_t
{
	int id;
	char *name;

	/* Program that kernel belongs to */
	struct opencl_si_program_t *program;

	/* List of kernel arguments. Each element of the list is of type
	 * 'struct opencl_si_arg_t'. */
	struct list_t *arg_list;

	/* Excerpts of program binary */
	struct elf_buffer_t metadata_buffer;
	struct elf_buffer_t header_buffer;
	struct elf_buffer_t kernel_buffer;
};

struct opencl_si_kernel_t *opencl_si_kernel_create(struct opencl_si_program_t *program,
		char *name);
void opencl_si_kernel_free(struct opencl_si_kernel_t *kernel);

#endif

