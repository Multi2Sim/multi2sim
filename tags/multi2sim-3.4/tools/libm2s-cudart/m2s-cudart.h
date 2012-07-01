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


#ifndef M2S_CUDART_H
#define M2S_CUDART_H

#include <cuda_runtime_api.h>

/* System call for CUDA runtime */
#define FRM_CUDART_SYS_CODE  329


/* List of CUDA runtime calls */
enum frm_cudart_call_t
{
	frm_cudart_call_invalid,
#define FRM_CUDART_DEFINE_CALL(name, code) frm_cudart_call_##name = code,
#include "../../src/arch/fermi/emu/cudart.dat"
#undef FRM_CUDA_DEFINE_CALL
	frm_cuda_callrt_count
};

#endif

