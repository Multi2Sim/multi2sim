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

#ifndef RUNTIME_CUDA_FUNCTION_H
#define RUNTIME_CUDA_FUNCTION_H


extern struct list_t *function_list;

struct CUfunc_st
{
	int id;
	int ref_count;

	char *name;
	unsigned int module_id;

	unsigned long long int *inst_buffer;
	unsigned int inst_buffer_size;

	struct list_t *arg_list;
	CUdeviceptr **arg_array;

	int global_sizes[3];  /* #thread-blocks in a grid */
	int local_sizes[3];
};

CUfunction cuda_function_create(CUmodule module, const char *function_name);
void cuda_function_free(CUfunction function);


#endif

