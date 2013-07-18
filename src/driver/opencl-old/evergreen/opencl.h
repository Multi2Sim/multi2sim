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

#ifndef ARCH_EVERGREEN_EMU_OPENCL_H
#define ARCH_EVERGREEN_EMU_OPENCL_H

#include <lib/util/class.h>


/* Function tables */
extern char *evg_opencl_func_name[];
extern int evg_opencl_func_argc[];

/* Debugging */
#define evg_opencl_debug(...) debug(evg_opencl_debug_category, __VA_ARGS__)
extern int evg_opencl_debug_category;

/* Some constants */
#define EVG_OPENCL_FUNC_FIRST  1000
#define EVG_OPENCL_FUNC_LAST  1073
#define EVG_OPENCL_FUNC_COUNT  (EVG_OPENCL_FUNC_LAST - EVG_OPENCL_FUNC_FIRST + 1)
#define EVG_OPENCL_MAX_ARGS  14

int evg_opencl_abi_call(X86Context *ctx);

int evg_opencl_abi_read_args(X86Context *ctx, int *argc_ptr,
		void *argv_ptr, int argv_size);
void evg_opencl_abi_return(X86Context *ctx, int value);

#endif

