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

#ifndef DRIVER_OPENCL_SI_DRIVER_H
#define DRIVER_OPENCL_SI_DRIVER_H

#include <string>

#include <src/arch/x86/asm/Asm.h>
#include <src/arch/x86/emu/Emu.h>
#include <src/arch/southern-islands/emu/Emu.h>
#include <src/arch/southern-islands/emu/NDRange.h>
#include <src/arch/southern-islands/emu/WorkGroup.h>
#include <src/arch/southern-islands/emu/Wavefront.h>
#include <src/arch/southern-islands/emu/WorkItem.h>

#include "southern-islands/Kernel.h"
#include "southern-islands/Program.h"

namespace Driver
{

class OpenCLDriver
{
	// Driver verision information
	static const unsigned major = 5;
	static const unsigned minor = 2173;

	// Unique instance of Opencl Driver
	static std::unique_ptr<OpenCLDriver> instance;

	// Private constructor. The only possible instance of the OpenCL Driver
	// can be obtained with a call to getInstance()
	OpenCLDriver();

	// Device emulators
	const SI::Emu *si_emu;
	const x86::Emu *x86_emu;

	// Device timing simulators
	// SI::Gpu *gpu;
	// x86::Cpu *cpu;

	// List of Southern Islands programs and kernels
	std::vector<std::unique_ptr<SI::Program>> programs;
	std::vector<std::unique_ptr<SI::Kernel>> kernels;
	std::vector<std::unique_ptr<SI::NDRange>> ndranges;

public:

	/// Get the only instance of the OpenCL Driver. If the instance does not
	/// exist yet, it will be created, and will remain allocated until the
	/// end of the execution.
	static OpenCLDriver *getInstance();

	/// This function is called when all work groups from an ND-Range have
	/// been scheduled (i.e., ndrange->waiting_work_groups is empty)
	/// \param ndrange  
	void RequestWork(SI::NDRange *ndrange);

	/// This function is called when all work groups from an ND-Range have
	/// been scheduled and completed (i.e., ndrange->waiting_work_groups and 
	/// ndrange->running_work_groups are both empty)
	/// \param ndrange 
	void NDRangeComplete(SI::NDRange *ndrange);

	/// OpenCL driver call
	int DriverCall();

};	

}  // namespace Driver

#endif
