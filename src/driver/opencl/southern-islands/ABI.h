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

#ifndef DRIVER_OPENCL_SI_ABI_H
#define DRIVER_OPENCL_SI_ABI_H

#include <string>

#include <src/arch/southern-islands/emu/Emu.h>
#include <src/arch/southern-islands/emu/NDRange.h>

#include "Program.h"
#include "Kernel.h"

using namespace SI;

namespace SI
{

/* List of OpenCL Runtime calls */
enum OpenCLABICall
{
	OpenCLABIInvalid = 0,
#define OPENCL_ABI_CALL(name, code) OpenCLABI##name = code,
#include "ABI.dat"
#undef OPENCL_ABI_CALL
	OpenCLABICallCount
};


class OpenCLDriver
{
	// Device emulators
	SI::Emu *si_emu;
	// x86::Emu *x86; // FIXME: move to common driver class?

	// Device timing simulators
	// SI::Gpu *gpu;
	// x86::Cpu *cpu;

	// List of Southern Islands programs and kernels
	std::vector<std::unique_ptr<Program>> programs;
	std::vector<std::unique_ptr<Kernel>> kernels;
	std::vector<std::unique_ptr<NDRange>> ndranges;

	// FIXME: OpenCL ABI calls
#define OPENCL_ABI_CALL(name, code) \
	int OpenCLABI##name##Impl(/*FIXME: X86Context *ctx*/);
#include "ABI.dat"
#undef OPENCL_ABI_CALL


public:
	OpenCLDriver(SI::Emu *si_emu);

	/// This function is called when all work groups from an ND-Range have
	/// been scheduled (i.e., ndrange->waiting_work_groups is empty)
	/// \param ndrange  
	void RequestWork(NDRange *ndrange);

	/// This function is called when all work groups from an ND-Range have
	/// been scheduled and completed (i.e., ndrange->waiting_work_groups and 
	/// ndrange->running_work_groups are both empty)
	/// \param ndrange 
	void NDRangeComplete(NDRange *ndrange);

	/// OpenCL driver call
	int DriverCall(/*x86Context *x86_ctx*/);

};	

}  // namespace Driver


#if 0
/*
 * Class 'OpenCLDriver'
 */

CLASS_BEGIN(OpenCLDriver, Driver)

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

CLASS_END(OpenCLDriver)

void OpenCLDriverCreate(OpenCLDriver *self, X86Emu *x86_emu, SIEmu *si_emu);
void OpenCLDriverDestroy(OpenCLDriver *self);

void OpenCLDriverRequestWork(OpenCLDriver *self, SINDRange *ndrange);
void OpenCLDriverNDRangeComplete(OpenCLDriver *self, SINDRange *ndrange);


/*
 * Public
 */

#define opencl_debug(...) debug(opencl_debug_category, __VA_ARGS__)
extern int opencl_debug_category;

int OpenCLDriverCall(X86Context *ctx);

#endif



#endif
