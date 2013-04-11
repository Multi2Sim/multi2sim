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


/* Version */
#define CUDA_VERSION_MAJOR 1
#define CUDA_VERSION_MINOR 850
struct cuda_version_t
{
	int major;
	int minor;
};

/* Debug */
extern int cuda_debug;
#define cuda_debug_print(stream, ...) (cuda_debug ? fprintf((stream), \
			__VA_ARGS__) : (void) 0)
#define cuda_debug_print(stream, ...) (cuda_debug ? fprintf((stream), \
			__VA_ARGS__) : (void) 0)

/* Error */
extern char *cuda_err_not_impl;
extern char *cuda_rt_err_not_impl;
extern char *cuda_err_version;
extern char *cuda_err_native;
extern char *cuda_rt_err_native;
#define __CUDA_NOT_IMPL__  warning("%s: not implemented.\n%s", __FUNCTION__, \
		cuda_err_not_impl);
#define __CUDART_NOT_IMPL__  warning("%s: not implemented.\n%s", __FUNCTION__, \
		cuda_rt_err_not_impl);

/* System Call Code */
#define CUDA_SYS_CODE 328

/* List of CUDA Calls */
enum cuda_call_t
{
	cuda_call_invalid,
#define CUDA_DEFINE_CALL(name) cuda_call_##name,
#include "../../src/driver/cuda/cuda.dat"
#undef CUDA_DEFINE_CALL
	cuda_call_count
};

/* For CUDA runtime */
extern CUmodule module;
extern CUfunction function;
extern unsigned long long int *inst_buffer;
extern unsigned int inst_buffer_size;
extern unsigned int num_gpr_used;
#define __dv(v)
struct __fatDeviceText
{
	int m;
	int v;
	const unsigned long long* d;
	char* f;
};

/* For CUDA driver */
extern struct cuda_device_t *device;

/* CUDA lists */
extern struct list_t *context_list;
extern struct list_t *device_list;
extern struct list_t *module_list;
extern struct list_t *function_list;


#endif

