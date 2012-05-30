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


#ifndef M2S_CUDA_H
#define M2S_CUDA_H

#include <cuda.h>

/* System call for CUDA driver */
#define FRM_CUDA_SYS_CODE  328


/* List of CUDA driver calls */
enum frm_cuda_call_t
{
	frm_cuda_call_invalid,
#define FRM_CUDA_DEFINE_CALL(name, code) frm_cuda_call_##name = code,
#include "../../src/arch/fermi/emu/cuda.dat"
#undef FRM_CUDA_DEFINE_CALL
	frm_cuda_call_count
};


/* CUDA Data Structures */
struct CUctx_st
{
	unsigned int id;
};


struct CUfunc_st
{
	unsigned int id;
	char name[1024];
};


struct CUmod_st
{
	unsigned int id;
	struct CUfunc_st func[1024];
};


#endif

