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

#include <arch/x86/asm/Asm.h>
#include <arch/x86/emu/Emu.h>
#include <arch/southern-islands/emu/WorkGroup.h>
#include <arch/southern-islands/emu/Wavefront.h>
#include <arch/southern-islands/emu/WorkItem.h>

#include "ABI.h"
#include "OpenCLDriver.h"

namespace Driver
{
	
std::unique_ptr<OpenCLDriver> OpenCLDriver::instance;

OpenCLDriver::OpenCLDriver()
{
	// Obtain instance of emulators
	// si_emu = SI::Emu::getInstance();
}

OpenCLDriver *OpenCLDriver::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new OpenCLDriver());
	return instance.get();
}

void OpenCLDriver::RequestWork(SI::NDRange *ndrange)
{
	// FIXME
}

void OpenCLDriver::NDRangeComplete(SI::NDRange *ndrange)
{
	// FIXME
}

int OpenCLDriver::DriverCall()
{
	// FIXME
	// struct x86_regs_t *regs = ctx->regs;

	// Variables

	// Function code
	// int code = regs->ebx;
	// if (code <= OpenCLABIInvalid || code >= OpenCLABICallCount)
	// 	fatal("%s: invalid OpenCL ABI call (code %d).\n%s",
	// 		__FUNCTION__, code, opencl_err_abi_call);

	// Debug
	// x86_sys_debug("  %s (code %d)\n", OpenCLABICallName[code], code);
	// opencl_debug("OpenCL ABI call '%s' (code %d)\n",
		// OpenCLABICallName[code], code);

	// Call OpenCL Runtime function
	// assert(OpenCLABICallTable[code]);
	// int ret = OpenCLABICallTable[code]();

	// Return value
	// return ret;
	return 0;
}

}  // namespace Driver
