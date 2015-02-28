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


#include "api.h"

struct CUfunc_st
{
	int id;

	char *name;

	/* CUDA function binary */
	struct elf_file_t *bin;

	/* Instruction buffer, i.e., .text section in cubin */
	unsigned inst_buf_size;
	unsigned long long *inst_buf;

	/* Mandatory fields according to NVIDIA's documentation */
	int maxThreadsPerBlock;
	size_t sharedSizeBytes;
	size_t constSizeBytes;
	size_t localSizeBytes;
	int numRegs;
	int ptxVersion;
	int binaryVersion;
	CUfunc_cache cache_config;

	/* Used to find the function in the function list */
	unsigned host_func_ptr;
};

CUfunction cuda_function_create(CUmodule module, const char *function_name);
void cuda_function_free(CUfunction function);


#endif

