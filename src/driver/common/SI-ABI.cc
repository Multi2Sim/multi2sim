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
#include <src/arch/x86/emu/Emu.h>
#include <src/arch/southern-islands/emu/Emu.h>
#include <src/arch/southern-islands/emu/NDRange.h>
#include <src/driver/opencl/OpenCLDriver.h>
#include <src/mem-system/Memory.h>

#include "SI-ABI.h"

using namespace misc;
using namespace mem;

namespace SI
{

/*
 * OpenCL ABI call #2 - si_mem_alloc
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

int SIABIMemAllocImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
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
		MemoryAccessRead | MemoryAccessWrite);

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
 * OpenCL ABI call #3 - si_mem_read
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

int SIABIMemReadImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
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

int SIABIMemWriteImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
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
 * OpenCL ABI call #5 - si_mem_copy
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

int SIABIMemCopyImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
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
 * OpenCL ABI call #6 - si_mem_free
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

int SIABIMemFreeImpl(x86::Context *ctx)
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

}  // namespace SI
