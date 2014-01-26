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

#ifndef DRIVER_CUDA_CUDA_H
#define DRIVER_CUDA_CUDA_H


#include <driver/common/driver.h>


/* Version */
struct cuda_version_t
{
	int major;
	int minor;
};
#define CUDA_VERSION_MAJOR 1
#define CUDA_VERSION_MINOR 1020

/* Debug */
extern int cuda_debug_category;
#define cuda_debug(...) debug(cuda_debug_category, __VA_ARGS__)

/* Error */
extern char *cuda_err_code;

/* Enumeration of ABI calls */
enum cuda_call_t
{
	cuda_call_invalid = 0,
#define CUDA_DEFINE_CALL(name) cuda_call_##name,
#include "cuda.dat"
#undef CUDA_DEFINE_CALL
	cuda_call_count
};


/*
 * Class 'CudaDriver'
 */

CLASS_BEGIN(CudaDriver, Driver)

	/* Device emulators */
	FrmEmu *frm_emu;
	KplEmu *kpl_emu;

CLASS_END(CudaDriver)

void CudaDriverCreate(CudaDriver *self, X86Emu *x86_emu, FrmEmu *frm_emu,
		KplEmu *kpl_emu);
void CudaDriverDestroy(CudaDriver *self);


/*
 * Public
 */

int CudaDriverCall(X86Context *ctx);


#endif

