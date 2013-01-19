/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef DRIVER_CUDA_FUNCTION_H
#define DRIVER_CUDA_FUNCTION_H

#include <lib/util/elf-format.h>
#include <lib/util/string.h>


struct frm_cuda_function_t
{
	int id;
	char name[MAX_STRING_SIZE];  /* FIXME */
	int ref_count;

	unsigned int module_id;
	struct list_t *arg_list;

	/* FIXME */
	struct elf_buffer_t function_buffer;

	/* Number of work dimensions */
	int work_dim;

	/* 3D Counters */
	int global_size3[3];  /* Total number of work_items */
	int local_size3[3];  /* Number of work_items in a group */
	int group_count3[3];  /* Number of work_item groups */

	/* 1D Counters. Each counter is equal to the multiplication
	 * of each component in the corresponding 3D counter. */
	int global_size;
	int local_size;
	int group_count;

	/* State of the running function */
	struct frm_grid_t *grid;
};

struct frm_cuda_function_t *frm_cuda_function_create(void);
void frm_cuda_function_free(struct frm_cuda_function_t *function);
void frm_cuda_function_load(struct frm_cuda_function_t *function, char *function_name);


#endif

