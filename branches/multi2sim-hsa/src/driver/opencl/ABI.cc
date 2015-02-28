/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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


#include <arch/x86/emu/Context.h>
#include <arch/x86/emu/Emu.h>
#include <memory/Memory.h>

#include "ABI.h"
#include "OpenCLDriver.h"

namespace Driver
{

std::string OpenCLABICallName[OpenCLABICallCount + 1] =
{
	"Invalid",
#define OPENCL_ABI_CALL(space, name, code) #space #name,
#include "ABI.dat"
#undef OPENCL_ABI_CALL
	"CallCount"
};

OpenCLABICallFuncPtr OpenCLABICallTable[OpenCLABICallCount + 1] =
{
	nullptr
};

int OpenCLABIInitImpl(x86::Context *ctx)
{
	x86::Regs &regs = ctx->getRegs();
	mem::Memory &mem = ctx->getMem();

	struct OpenCLVersion
	{
		int major;
		int minor;
	};
	OpenCLVersion version;

	// Arguments
	unsigned version_ptr = regs.getEdx();
	x86::Emu::opencl_debug << misc::fmt("\tversion_ptr=0x%x\n", version_ptr);

	// Return versions
	assert(sizeof(struct OpenCLVersion) == 8);
	version.major = OpenCLSIDriver::major;
	version.minor = OpenCLSIDriver::minor;
	mem.Write(version_ptr, sizeof version, (const char *)&version);

	// Debug
	x86::Emu::opencl_debug << misc::fmt("\tMulti2Sim OpenCL implementation in host: v. %d.%d.\n",
		OpenCLSIDriver::major, OpenCLSIDriver::minor);

	// Return Success
	return 0;
}

}  // namespace Driver
