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
extern int cuda_enable_debug;

#define cuda_debug(stream, ...) (cuda_enable_debug ? fprintf((stream), __VA_ARGS__) : (void) 0)



/* System call for CUDA driver */
#define CUDA_SYS_CODE  328



/* List of CUDA driver calls */
enum cuda_call_t
{
	cuda_call_invalid,
#define CUDA_DEFINE_CALL(name) cuda_call_##name,
#include "../../src/driver/cuda/cuda.dat"
#undef CUDA_DEFINE_CALL
	cuda_call_count
};



/* CUDA context */
struct CUctx_st
{
	unsigned int id;
};



/* CUDA module */
struct CUmod_st
{
	unsigned int id;
};



/* CUDA function */
struct CUfunc_st
{
	unsigned int id;
};



/* CUDA stream */
struct CUstream_st
{
	unsigned int id;
};

#endif

