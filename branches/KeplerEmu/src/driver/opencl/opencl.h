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

#ifndef DRIVER_OPENCL_OPENCL_H
#define DRIVER_OPENCL_OPENCL_H

#include <driver/common/driver.h>
#include <lib/class/class.h>
#include <arch/southern-islands/emu/ndrange.h>


/*
 * Class 'OpenclDriver'
 */

CLASS_BEGIN(OpenclDriver, Driver)

	/* Device emulators */
	SIEmu *si_emu;

	/* Device timing simulators */
	X86Cpu *x86_cpu;
	SIGpu *si_gpu;
	int fused : 1;

	/* List of Southern Islands programs and kernels */
	struct list_t *si_program_list;
	struct list_t *si_kernel_list;
	struct list_t *si_ndrange_list;

	/* Count of current OpenCL ND-Ranges executing for this driver */
	int ndranges_running;

CLASS_END(OpenclDriver)

void OpenclDriverCreate(OpenclDriver *self, X86Emu *x86_emu, SIEmu *si_emu);
void OpenclDriverDestroy(OpenclDriver *self);

void OpenclDriverRequestWork(OpenclDriver *self, SINDRange *ndrange);
void OpenclDriverNDRangeComplete(OpenclDriver *self, SINDRange *ndrange);




/*
 * Public
 */

#define opencl_debug(...) debug(opencl_debug_category, __VA_ARGS__)
extern int opencl_debug_category;

int OpenclDriverCall(X86Context *ctx);

#endif

