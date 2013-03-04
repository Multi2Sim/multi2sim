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

#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>

#include "si-kernel.h"



/*
 * Kernel List
 */


struct list_t *opencl_si_kernel_list;

void opencl_si_kernel_list_init(void)
{
	/* Already initialized */
	if (opencl_si_kernel_list)
		return;

	/* Initialize and add one empty element */
	opencl_si_kernel_list = list_create();
	list_add(opencl_si_kernel_list, NULL);
}


void opencl_si_kernel_list_done(void)
{
	struct opencl_si_kernel_t *kernel;
	int index;

	/* Not initialized */
	if (!opencl_si_kernel_list)
		return;

	/* Free list of Southern Islands kernels */
	LIST_FOR_EACH(opencl_si_kernel_list, index)
	{
		kernel = list_get(opencl_si_kernel_list, index);
		if (kernel)
			opencl_si_kernel_free(kernel);
	}
	list_free(opencl_si_kernel_list);
}




/*
 * Kernel
 */

struct opencl_si_kernel_t *opencl_si_kernel_create(struct opencl_si_program_t *program,
		char *name)
{
	struct opencl_si_kernel_t *kernel;

	/* Initialize */
	kernel = xcalloc(1, sizeof(struct opencl_si_kernel_t));
	kernel->name = xstrdup(name);
	kernel->program = program;

	/* Insert in kernel list */
	opencl_si_kernel_list_init();
	list_add(opencl_si_kernel_list, kernel);
	kernel->id = list_count(opencl_si_kernel_list) - 1;

	/* Return */
	return kernel;
}


void opencl_si_kernel_free(struct opencl_si_kernel_t *kernel)
{
	free(kernel->name);
	free(kernel);
}
