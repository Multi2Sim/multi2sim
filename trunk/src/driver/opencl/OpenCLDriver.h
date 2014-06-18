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

#include <src/driver/common/SI.h>
#include <src/arch/southern-islands/driver/Program.h>
#include <src/arch/southern-islands/driver/Kernel.h>

// Forward declaration
namespace x86
{
	class Asm;
	class Emu;
	class Context;
}  // namespace x86

namespace SI
{
	class Emu;
	class NDRange;
	class Program;
	class Kernel;
}  // namespace SI

namespace Driver
{

/// OpenCL Driver for SI
class OpenCLSIDriver : public virtual SICommon
{
	// Unique instance of OpenCL Driver
	static std::unique_ptr<OpenCLSIDriver> instance;

	// Private constructor. The only possible instance of the OpenCL Driver
	// can be obtained with a call to getInstance()
	OpenCLSIDriver();

	// List of Southern Islands programs and kernels
	std::vector<std::unique_ptr<SI::Program>> programs;
	std::vector<std::unique_ptr<SI::Kernel>> kernels;

	// Count of current OpenCL ND-Ranges executing for this driver 
	int ndranges_running;

public:

	~OpenCLSIDriver();

	// Driver verision information
	static const unsigned major = 7;
	static const unsigned minor = 2652;

	/// Get the only instance of the OpenCL Driver. If the instance does not
	/// exist yet, it will be created, and will remain allocated until the
	/// end of the execution.
	static OpenCLSIDriver *getInstance();

	/// OpenCL driver call
	int DriverCall(x86::Context *ctx, int abi_code);

	/// Getters
	///
	/// Get SI functional emulator
	SI::Emu *getEmuGpu() const { return si_emu; }

	/// Get count of program in list
	int getProgramCount() const { return programs.size(); }

	/// Get a pointer of a program by id
	SI::Program *getProgramById(unsigned id) { return programs[id].get(); }

	/// Get count of kernel in list
	int getKernelCount() const { return kernels.size(); }

	/// Get a pointer of a kernel by id
	SI::Kernel *getKernelById(unsigned id) {return kernels[id].get(); }

	/// Get count of current OpenCL ND-Ranges executing
	int getNDrangeRunning() const { return ndranges_running; }

	/// Increment count of NDRanges that are currently running
	void incNDRangeRunning() { ndranges_running++; }

	/// Decrease count of NDRanges that are currently running 
	void decNDRangeRunning() { ndranges_running--; }

	/// This function is called when all work groups from an ND-Range have
	/// been scheduled (i.e., ndrange->waiting_work_groups is empty)
	/// \param ndrange  
	void RequestWork(SI::NDRange *ndrange);

	/// This function is called when all work groups from an ND-Range have
	/// been scheduled and completed (i.e., ndrange->waiting_work_groups and 
	/// ndrange->running_work_groups are both empty)
	/// \param ndrange 
	void NDRangeComplete(SI::NDRange *ndrange);

	/// Add program to program list
	void AddProgram(std::unique_ptr<SI::Program> program);

	/// Add kernel to kernel list
	void AddKernel(std::unique_ptr<SI::Kernel> kernel);
};	

}  // namespace Driver

#endif
