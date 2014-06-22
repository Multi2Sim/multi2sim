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

#include <src/arch/x86/emu/Context.h>
#include <arch/x86/emu/Emu.h>
#include <arch/southern-islands/emu/Emu.h>
#include <arch/southern-islands/emu/NDRange.h>
#include <memory/Memory.h>
#include <driver/opengl/ABI.h>
#include <driver/opengl/OpenGLDriver.h>
#include <driver/opengl/southern-islands/SPI.h>
#include <lib/cpp/Misc.h>

#include "ABI.h"

using namespace misc;
using namespace mem;

namespace SI
{

/*
 * OpenGL ABI call #2 - si_mem_alloc
 *
 * Allocates memory in the Southern Islands device.
 *
 * @param unsigned int size
 *
 * 	Number of bytes to allocate.
 *
 * @return void *
 *
 *	The function returns a pointer in the device memory space. This pointer
 *	should not be dereferenced in the runtime, but instead passed to other
 *	ABI calls taking device pointers as input arguments.
 */

int OpenGLABIMemAllocImpl(x86::Context *ctx)
{
	Driver::OpenGLSIDriver *driver = Driver::OpenGLSIDriver::getInstance();
	x86::Regs &regs  = ctx->getRegs();
	SI::Emu *si_emu = SI::Emu::getInstance();
	
	// SI::Gpu *si_gpu = driver->si_gpu;

	// Arguments 
	unsigned size = regs.getEdx();
	x86::Emu::opencl_debug << misc::fmt("\tsize = %u\n", size);

	if (driver->isFused())
	{
		fatal("%s: GPU is set as a fused device, so the x86 "
			"allocator should be used", __FUNCTION__);
	}

	// if (si_gpu)
	// {
	// 	// Allocate starting from nearest page boundary 
	// 	if (si_emu->getVideoMemTop() & si_gpu->mmu->page_mask)
	// 	{
	// 		si_emu->incVideoMemTop(si_gpu->mmu->page_size -
	// 			(si_emu->video_mem_top & 
	// 			 si_gpu->mmu->page_mask));
	// 	}
	// }

	// Map new pages 
	mem::Memory &video_mem = si_emu->getVideoMem();
	video_mem.Map(si_emu->getVideoMemTop(), size,
			Memory::AccessRead |
			Memory::AccessWrite);

	// Virtual address of memory object 
	unsigned device_ptr = si_emu->getVideoMemTop();
	x86::Emu::opencl_debug << misc::fmt("\t%d bytes of device memory allocated at 0x%x\n",
		size, device_ptr);

	// For now, memory allocation in device memory is done by just 
	// incrementing a pointer to the top of the global memory space. 
	// Since memory deallocation is not implemented, "holes" in the 
	// memory space are not considered. 
	si_emu->incVideoMemTop(size);

	// Return device pointer 
	return device_ptr;
}

/*
 * OpenGL ABI call #3 - si_mem_read
 *
 * Read memory from Southern Islands device into host memory space.
 *
 * @param void *host_ptr
 *
 * 	Destination pointer in host memory space.
 *
 * @param void *device_ptr
 *
 * 	Source pointer in device memory space.
 *
 * @param unsigned int size
 *
 * 	Number of bytes to read.
 *
 * @return void
 *
 *	The function does not have any return value.
 */

int OpenGLABIMemReadImpl(x86::Context *ctx)
{
	Driver::OpenGLSIDriver *driver = Driver::OpenGLSIDriver::getInstance();
	SI::Emu *si_emu = SI::Emu::getInstance();
	// SI::Gpu *si_gpu = driver->si_gpu;

	x86::Regs &regs   = ctx->getRegs();
	mem::Memory &mem  = ctx->getMem();
	mem::Memory &video_mem = si_emu->getVideoMem();

	if (driver->isFused())
	{
		fatal("%s: GPU is set as a fused device, so the x86 "
			"memory operations should be used", __FUNCTION__);
	}

	// Arguments 
	unsigned host_ptr = regs.getEcx();
	unsigned device_ptr = regs.getEdx();
	unsigned size = regs.getEsi();
	x86::Emu::opencl_debug << misc::fmt("\thost_ptr = 0x%x, device_ptr = 0x%x, size = %d bytes\n",
			host_ptr, device_ptr, size);

	/* Check memory range */
	if (device_ptr + size > si_emu->getVideoMemTop())
		fatal("%s: accessing device memory not allocated",
				__FUNCTION__);

	// Read memory from device to host 
	char *buf = (char *)malloc(size);
	video_mem.Read(device_ptr, size, buf);
	mem.Write(host_ptr, size, buf);
	free(buf);

	// Return
	return 0;
}

int OpenGLABIMemWriteImpl(x86::Context *ctx)
{
	Driver::OpenGLSIDriver *driver = Driver::OpenGLSIDriver::getInstance();
	SI::Emu *si_emu = SI::Emu::getInstance();
	// SI::Gpu *si_gpu = driver->si_gpu;

	x86::Regs &regs   = ctx->getRegs();
	mem::Memory &mem  = ctx->getMem();
	mem::Memory &video_mem = si_emu->getVideoMem();

	if (driver->isFused())
	{
		fatal("%s: GPU is set as a fused device, so the x86 "
			"memory operations should be used", __FUNCTION__);
	}

	// Arguments 
	unsigned device_ptr = regs.getEcx();
	unsigned host_ptr = regs.getEdx();
	unsigned size = regs.getEsi();
	x86::Emu::opencl_debug << misc::fmt("\tdevice_ptr = 0x%x, host_ptr = 0x%x, size = %d bytes\n",
			device_ptr, host_ptr, size);

	/* Check memory range */
	if (device_ptr + size > si_emu->getVideoMemTop())
		fatal("%s: accessing device memory not allocated",
				__FUNCTION__);

	// Read memory from host to device 
	char *buf = (char *)malloc(size);
	mem.Read(device_ptr, size, buf);
	video_mem.Write(host_ptr, size, buf);
	free(buf);

	// Return
	return 0;
}

/*
 * OpenGL ABI call #5 - si_mem_copy
 *
 * Copy memory across two different regions of the Southern Islands device
 * memory space.
 *
 * @param void *dest_ptr
 *
 * 	Destination pointer in device memory.
 *
 * @param void *src_ptr
 *
 * 	Source pointer in device memory.
 *
 * @param unsigned int size
 *
 * 	Number of bytes to read.
 *
 * @return void
 *
 *	The function does not have any return value.
 */

int OpenGLABIMemCopyImpl(x86::Context *ctx)
{
	Driver::OpenGLSIDriver *driver = Driver::OpenGLSIDriver::getInstance();
	SI::Emu *si_emu = SI::Emu::getInstance();
	// SI::Gpu *si_gpu = driver->si_gpu;

	x86::Regs &regs   = ctx->getRegs();
	mem::Memory &video_mem = si_emu->getVideoMem();

	if (driver->isFused())
	{
		fatal("%s: GPU is set as a fused device, so the x86 "
			"memory operations should be used", __FUNCTION__);
	}

	// Arguments 
	unsigned dest_ptr = regs.getEcx();
	unsigned src_ptr = regs.getEdx();
	unsigned size = regs.getEsi();
	x86::Emu::opencl_debug << misc::fmt("\tdest_ptr = 0x%x, src_ptr = 0x%x, size = %d bytes\n",
			dest_ptr, src_ptr, size);

	/* Check memory range */
	if (src_ptr + size > si_emu->getVideoMemTop() ||
			dest_ptr + size > si_emu->getVideoMemTop())
		fatal("%s: accessing device memory not allocated",
				__FUNCTION__);

	// Write memory from host to device 
	char *buf = (char *)malloc(size);
	video_mem.Read(src_ptr, size, buf);
	video_mem.Write(dest_ptr, size, buf);
	free(buf);

	// Return
	return 0;
}

/*
 * OpenGL ABI call #6 - si_mem_free
 *
 * Deallocated memory in Southern Islands global memory scope.
 *
 * @param unsigned int device_ptr
 *
 * 	Memory address in device global memory returned previously by a call to
 *	'si_mem_alloc'.
 *
 * @return void
 *
 *	No value is returned.
 */

int OpenGLABIMemFreeImpl(x86::Context *ctx)
{
	x86::Regs &regs   = ctx->getRegs();

	// Arguments 
	unsigned device_ptr = regs.getEcx();
	x86::Emu::opencl_debug << misc::fmt("\tdevice_ptr = %u\n", device_ptr);

	// For now, this call is ignored. No deallocation of global memory can
	// happen. 

	// Return
	return 0;
}

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
	Driver::OpenGLSIDriver *opengl_driver = Driver::OpenGLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();
	mem::Memory &mem = ctx->getMem();

	// Arguments
	unsigned program_id = regs.getEcx();
	unsigned name_ptr = regs.getEdx();

	ProgramGL *program = opengl_driver->getProgramById(program_id);
	if (!program)
		fatal("Invalid program ID %d", program_id);

	std::string name = mem.ReadString(name_ptr, 1024);
	unsigned attrib_loc = program->getAttribLoc(name);

	// Return
	return attrib_loc;
}

int OpenGLABIProgramGetUniformLoc(x86::Context *ctx)
{
	Driver::OpenGLSIDriver *opengl_driver = Driver::OpenGLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();
	mem::Memory &mem = ctx->getMem();

	// Arguments
	unsigned program_id = regs.getEcx();
	unsigned name_ptr = regs.getEdx();

	ProgramGL *program = opengl_driver->getProgramById(program_id);
	if (!program)
		fatal("Invalid program ID %d", program_id);

	std::string name = mem.ReadString(name_ptr, 1024);
	unsigned uniform_loc = program->getUniformLoc(name);

	// Return
	return uniform_loc;
}

int OpenGLABIShaderCreate(x86::Context *ctx)
{
	Driver::OpenGLSIDriver *opengl_driver = Driver::OpenGLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();

	// Arguments
	unsigned program_id = regs.getEcx();
	unsigned shader_id = regs.getEdx();
	unsigned shader_type = regs.getEsi();

	ProgramGL *program = opengl_driver->getProgramById(program_id);
	if (!program)
		fatal("Invalid program ID %d", program_id);

	program->BindShader(shader_id, (OpenGLSiShaderStage)shader_type);

	// Return
	return 0;
}

int OpenGLABIShaderFree(x86::Context *ctx)
{
	// No need, shader will be freed automatically

	// Return
	return 0;
}

int OpenGLABIShaderSetInput(x86::Context *ctx)
{
	Driver::OpenGLSIDriver *opengl_driver = Driver::OpenGLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();
	mem::Memory &mem = ctx->getMem();

	// FIXME: change runtime syscall parameters
	fatal("Runtime interface changed!, Check runtime!\n");

	// Arguments
	unsigned args[7];
	unsigned arg_ptr = regs.getEcx();

	// Arguments 
	mem.Read(arg_ptr, 7 * sizeof(unsigned), (char *)args);

	unsigned shader_id = args[0];
	unsigned device_ptr = args[1];
	unsigned num_elems = args[2];
	unsigned data_type = args[3];
	unsigned size = args[4];
	unsigned index = args[5];
	unsigned program_id = args[7];

	x86::Emu::opengl_debug << misc::fmt("\tprogram_id = %d, shader_id = %d,	device_ptr = 0x%x, num_elems = %d, type = %d, size = %d, index = %d\n",
		program_id, shader_id, device_ptr, num_elems, data_type, size, index);

	SI::ProgramGL *program= opengl_driver->getProgramById(program_id);
	SI::Shader *shader = program->getShaderByID(shader_id);
	shader->AddInput(index, size, device_ptr, (SIInputDataType)data_type, num_elems,
		false, 0, 0);

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
	Driver::OpenGLSIDriver *opengl_driver = Driver::OpenGLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();
	SI::SPI *spi = opengl_driver->getSPIModule();

	// Arguments 
	unsigned x = regs.getEcx();
	unsigned y = regs.getEdx();
	unsigned width = regs.getEsi();
	unsigned height = regs.getEdi();

	// Debug 
	x86::Emu::opengl_debug << misc::fmt("\tViewport x = %d, y = %d, width = %d, height = %d\n", 
		x, y, width, height);

	spi->setViewport(x, y, width, height);

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

