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
#include "OpenGLDriver.h"

using namespace SI;

namespace Driver
{

int OpenGLABIInitImpl(x86::Context *ctx)
{
	x86::Regs &regs = ctx->getRegs();
	mem::Memory &mem = ctx->getMem();

	struct OpenGLVersion
	{
		int major;
		int minor;
	};
	OpenGLVersion version;

	// Arguments
	unsigned version_ptr = regs.getEcx();
	x86::Emu::opengl_debug << misc::fmt("\tversion_ptr=0x%x\n", version_ptr);

	// Return versions
	assert(sizeof(OpenGLVersion) == 8);
	version.major = OpenGLSIDriver::major;
	version.minor = OpenGLSIDriver::minor;
	mem.Write(version_ptr, sizeof version, (const char *)&version);

	// Debug
	x86::Emu::opengl_debug << misc::fmt("\tMulti2Sim OpenGL implementation in host: v. %d.%d.\n",
		OpenGLSIDriver::major, OpenGLSIDriver::minor);
	x86::Emu::opengl_debug << misc::fmt("\tMulti2Sim OpenGL Runtime in guest: v. %d.%d.\n",
		version.major, version.minor);	

	// Return Success
	return 0;
}

}  // namespace Driver
