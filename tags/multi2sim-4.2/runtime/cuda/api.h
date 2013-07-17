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

#ifndef RUNTIME_CUDA_API_H
#define RUNTIME_CUDA_API_H


/* Debug */
void cuda_debug(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

/* Syscall code */
#define CUDA_SYS_CODE 328

/* List of CUDA calls */
enum cuda_call_t
{
	cuda_call_invalid,
#define CUDA_DEFINE_CALL(name) cuda_call_##name,
#include "../../src/driver/cuda/cuda.dat"
#undef CUDA_DEFINE_CALL
	cuda_call_count
};

/* For CUDA runtime */
#define __dv(v)

/* For CUDA driver */
extern struct cuda_device_t *device;

/* CUDA lists */
extern struct list_t *context_list;
extern struct list_t *device_list;
extern struct list_t *module_list;
extern struct list_t *function_list;


#endif

