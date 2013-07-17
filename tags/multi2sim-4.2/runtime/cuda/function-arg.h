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

#ifndef RUNTIME_CUDA_FUNCTION_ARG_H
#define RUNTIME_CUDA_FUNCTION_ARG_H


struct cuda_function_arg_t
{
	void *ptr;
	int size;
	int offset;
};

struct cuda_function_arg_t *cuda_function_arg_create(const void *ptr, 
		int size, int offset);
void cuda_function_arg_free(CUfunction function, 
		struct cuda_function_arg_t *arg);


#endif

