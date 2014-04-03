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

#include <arch/x86/emu/Context.h>
#include <arch/x86/emu/Emu.h>
#include <arch/southern-islands/emu/Emu.h>
#include <arch/southern-islands/emu/NDRange.h>
#include <mem-system/Memory.h>
#include <driver/opengl/ABI.h>
#include <driver/opengl/OpenGLDriver.h>
#include <lib/cpp/Misc.h>

#include "ABI.h"

using namespace misc;

namespace SI
{

/*
 * OpenGL ABI call #8 - si_program_create
 *
 * Create a Southern Islands program object
 *
 * @return int
 *
 *	
 	No value is returned.
 */

int OpenGLABIProgramCreate(x86::Context *ctx)
{
	Driver::OpenGLSIDriver *opengl_driver = Driver::OpenGLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();

	// Arguments
	unsigned program_id = regs.getEcx();

	// Called from runtime API glUseProgram()
	// it is guaranteed to be used later
	std::unique_ptr<ProgramGL> program(new ProgramGL(program_id));

	// Add to program list in OpenGL SI driver
	opengl_driver->AddProgram(std::move(program));

	// Return
	return 0;
}

int OpenGLABIProgramFree(x86::Context *ctx)
{
	Driver::OpenGLSIDriver *opengl_driver = Driver::OpenGLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();

	// Arguments
	unsigned program_id = regs.getEcx();

	opengl_driver->RemoveProgramById(program_id);

	// Return
	return 0;
}

int OpenGLABIProgramSetBinary(x86::Context *ctx)
{
	Driver::OpenGLSIDriver *opengl_driver = Driver::OpenGLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();
	mem::Memory &mem = ctx->getMem();

	// Arguments
	unsigned program_id = regs.getEcx();
	unsigned bin_ptr = regs.getEdx();
	unsigned bin_size = regs.getEsi();

	ProgramGL *program = opengl_driver->getProgramById(program_id);
	if (!program)
		fatal("Invalid program ID %d", program_id);

	char *buffer = (char *)calloc(1, bin_size);
	mem.Read(bin_ptr, bin_size, buffer);
	program->SetBinary(buffer, bin_size);
	free(buffer);

	// Return
	return 0;
}

int OpenGLABIProgramGetAttribLoc(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABIProgramGetUniformLoc(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABIShaderCreate(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABIShaderFree(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABIShaderSetInput(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABINdrangeCreate(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABINdrangeGetNumBufferEntries(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABINdrangeSendWorkGroups(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABINdrangeFinish(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABINdrangePassMemObjs(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABIViewport(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABIRaster(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenGLABIRasterFinish(x86::Context *ctx)
{
	// Return
	return 0;
}

}  // namespace SI