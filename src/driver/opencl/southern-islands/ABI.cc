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
#include <src/mem-system/Memory.h>

#include <src/driver/opencl/ABI.h>
#include <src/driver/opencl/OpenCLDriver.h>

#include "ABI.h"
#include "Program.h"
#include "Kernel.h"

using namespace misc;
using namespace mem;

namespace SI
{

static const unsigned MaxWorkGroupBufferSize = 1024*1024;

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

int OpenCLABIMemAllocImpl(x86::Context *ctx)
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

int OpenCLABIMemReadImpl(x86::Context *ctx)
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

int OpenCLABIMemWriteImpl(x86::Context *ctx)
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

int OpenCLABIMemCopyImpl(x86::Context *ctx)
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

int OpenCLABIMemFreeImpl(x86::Context *ctx)
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
 * OpenCL ABI call #7 - ProgramCreate
 *
 * Create a Southern Islands program object and return a unique identifier
 * for it.
 *
 * @return int
 *
 *	Unique program ID.
 */

int OpenCLABIProgramCreateImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();

	// Create program 
	int program_count = driver->getProgramCount();
	std::unique_ptr<Program> program(new Program(program_count));

	// Debug
	x86::Emu::opencl_debug << misc::fmt(
		"\tnew program ID = %d\n", program->getId());

	// And add it to list of SI programs in OpenCL SI driver
	driver->AddProgram(std::move(program));

	// Return
	return 0;
}

/*
 * OpenCL ABI call #8 - ProgramSetBinary
 *
 * Associate a binary to a Southern Islands program.
 *
 * @param int program_id
 *
 * 	Program ID, as returned by a previous ABI call to 'si_program_create'.
 *
 * @param void *buf
 *
 * 	Pointer to the memory space where the program binary can be found.
 *
 * @param unsigned int size
 *
 * 	Size of the program binary
 *
 * @return void
 *
 *	No return value.
 */

int OpenCLABIProgramSetBinaryImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();

	x86::Regs &regs  = ctx->getRegs();
	mem::Memory &mem = ctx->getMem();

	// Arguments 
	int program_id = (int)regs.getEcx();
	unsigned bin_ptr = regs.getEdx();
	unsigned bin_size = regs.getEsi();
	x86::Emu::opencl_debug << misc::fmt(
		"\tprogram_id=%d, bin_ptr=0x%x, size=%u\n",	program_id, bin_ptr, bin_size);

	// Get program 
	Program *program = driver->getProgramById(program_id);
	if (!program)
		fatal("%s: invalid program ID (%d)",
				__FUNCTION__, program_id);

	// Set the binary 
	char *buf = (char *)calloc(1, bin_size);
	mem.Read(bin_ptr, bin_size, buf);
	program->SetBinary(buf, bin_size);
	free(buf);

	// No return value 
	return 0;
}

/*
 * OpenCL ABI call #9 - si_kernel_create
 *
 * Create a Southern Islands kernel object and return a unique identifier
 * for it.
 *
 * @param int program_id
 *
 * 	Program ID, as returned by ABI call 'si_program_create'
 *
 * @param char *func_name
 *
 * 	Kernel function name in the program.
 *
 * @return int
 *
 *	Unique kernel ID.
 */

int OpenCLABIKernelCreateImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();

	x86::Regs &regs  = ctx->getRegs();
	mem::Memory &mem = ctx->getMem();

	// Arguments 
	int program_id         = (int)regs.getEcx();
	unsigned func_name_ptr = regs.getEdx();
	x86::Emu::opencl_debug << misc::fmt(
		"\tprogram_id=%d, func_name_ptr=0x%x\n", program_id, func_name_ptr);

	// Read function name 
	std::string func_name = mem.ReadString(func_name_ptr);
	x86::Emu::opencl_debug << misc::fmt(
		"\tfunc_name='%s'\n", func_name.c_str());

	// Get program object 
	Program *program = driver->getProgramById(program_id);
	if (!program)
		fatal("%s: invalid program ID (%d)",
				__FUNCTION__, program_id);

	// Create kernel 
	std::unique_ptr<Kernel> kernel(new Kernel(driver->getKernelCount(),
			func_name, program));
	x86::Emu::opencl_debug << misc::fmt(
		"\tnew kernel ID = %d\n", kernel->getId());

	// Add to kernel list
	driver->AddKernel(std::move(kernel));

	// Return kernel ID 
	return kernel->getId();
}

/*
 * OpenCL ABI call #10 - si_kernel_set_arg_value
 *
 * Set a kernel argument with a basic type (cl_char, cl_int, cl_float, ...).
 *
 * @param int kernel_id
 *
 * 	Kernel ID, as returned by ABI call 'si_kernel_create'
 *
 * @param int index
 *
 * 	Argument index to set.
 *
 * @param void *host_ptr
 *
 *	Address in host memory containing the value of the argument. The memory
 * 	pointed to by this variable will be copied internally, keeping a copy of
 * 	the argument for future use.
 *
 * @param unsigned int size
 *
 * 	Argument size. This size must match the size encoded in the kernel
 * 	metadata for this particular argument.
 *
 * @return int
 *
 *	Unique kernel ID.
 */

int OpenCLABIKernelSetArgValueImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();

	x86::Regs &regs   = ctx->getRegs();
	mem::Memory &mem  = ctx->getMem();

	// Arguments 
	int kernel_id     = (int)regs.getEcx();
	unsigned index    = regs.getEdx();
	unsigned host_ptr = regs.getEsi();
	int size          = (int)regs.getEdi();
	x86::Emu::opencl_debug << misc::fmt(
		"\tkernel_id=%d, index=%d\n", kernel_id, index);
	x86::Emu::opencl_debug << misc::fmt(
		"\thost_ptr=0x%x, size=%u\n", host_ptr, size);

	// Get kernel 
	Kernel *kernel = driver->getKernelById(kernel_id);
	if (!kernel)
		fatal("%s: invalid kernel ID (%d)",
				__FUNCTION__, kernel_id);

	// Get argument 
	ArgValue *arg = dynamic_cast<ArgValue *>(kernel->getArgByIndex(index));
	if (!arg || arg->getType() != ArgTypeValue)
		fatal("%s: invalid argument %d type",
				__FUNCTION__, index);

	// Check valid size 
	if (size != arg->getSize())
		fatal("%s: argument %d: size %d expected, %d found",
				__FUNCTION__, index, arg->getSize(), size);

	// Save value 
	fatal("FIXME: OpenCLABIKernelSetArgValueImpl()");
	char *value_ptr = (char *)calloc(1, size);
	mem.Read(host_ptr, size, value_ptr);
	arg->setValue(value_ptr);
	free(value_ptr);

	// No return value 
	return 0;
}

/*
 * OpenCL ABI call #11 - si_kernel_set_arg_pointer
 *
 * Set a kernel argument of type 'cl_mem', or local memory. In general, any
 * argument that uses the 'pointer' name as first token in the metadata entry of
 * the kernel binary.
 *
 * @param int kernel_id
 *
 * 	Kernel ID, as returned by ABI call 'si_kernel_create'
 *
 * @param int index
 *
 * 	Argument index to set.
 *
 * @param void *device_ptr
 *
 *	If the argument represents a 'cl_mem' object in global memory, pointer
 *	to device memory containing the data, as returned by a previous call to
 *	'si_mem_alloc'.
 *	If the argument is a variable in local memory, the purpose of the call
 *	is just allocating space for it, so this value should be NULL.
 *
 * @param unsigned int size
 *
 *	If the argument represents a 'cl_mem' object, size allocated in global
 *	memory for the object.
 *	If the argument is a variable in local memory, number of bytes to be
 *	allocated in the device local memory.
 *
 * @return int
 *
 *	Unique kernel ID.
 */

int OpenCLABIKernelSetArgPointerImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
	x86::Regs &regs  = ctx->getRegs();

	// Arguments 
	int kernel_id       = (int)regs.getEcx();
	unsigned index      = regs.getEdx();
	unsigned device_ptr = regs.getEsi();
	int size            = (int)regs.getEdi();
	x86::Emu::opencl_debug << misc::fmt(
		"\tkernel_id=%d, index=%d\n", kernel_id, index);
	x86::Emu::opencl_debug << misc::fmt(
		"\tdevice_ptr=0x%x, size=%u\n", device_ptr, size);

	// Get kernel 
	Kernel *kernel = driver->getKernelById(kernel_id);
	if (!kernel)
		fatal("%s: invalid kernel ID (%d)",
				__FUNCTION__, kernel_id);

	// Get argument 
	ArgPointer *arg = dynamic_cast<ArgPointer *>(kernel->getArgByIndex(index));
	if (!arg || arg->getType() != ArgTypePointer)
		fatal("%s: invalid argument %d type",
				__FUNCTION__, index);

	// Check valid size 
	if (size != arg->getSize())
		fatal("%s: argument %d: size %d expected, %d found",
				__FUNCTION__, index, arg->getSize(), size);

	// Save value 
	arg->setSetFlag(true);
	arg->setSize(size);
	arg->setDevicePtr(device_ptr);

	// No return value 
	return 0;
}

/*
 * OpenCL ABI call #12 - si_kernel_set_arg_image
 *
 * Set a kernel argument of type 'cl_image'.
 *
 * @param int kernel_id
 *
 * 	Kernel ID, as returned by ABI call 'si_kernel_create'
 *
 * @param int index
 *
 * 	Argument index to set.
 *
 * @param [...] FIXME - not decided yet
 *
 * @return int
 *
 *	Unique kernel ID.
 */

int OpenCLABIKernelSetArgImageImpl(x86::Context *ctx)
{
	x86::Regs &regs  = ctx->getRegs();

	// Arguments 
	int kernel_id = (int)regs.getEcx();
	unsigned index = regs.getEdx();
	x86::Emu::opencl_debug << misc::fmt("\tkernel_id=%d, index=%d\n", kernel_id, index);

	// Not yet 
	fatal("%s: not implemented", __FUNCTION__);

	// No return value 
	return 0;
}

/*
 * OpenCL ABI call #13 - si_kernel_set_arg_sampler
 *
 * Set a kernel argument of type 'cl_sampler'.
 *
 * @param int kernel_id
 *
 * 	Kernel ID, as returned by ABI call 'si_kernel_create'
 *
 * @param int index
 *
 * 	Argument index to set.
 *
 * @param [...] FIXME - not decided yet
 *
 * @return int
 *
 *	Unique kernel ID.
 */

int OpenCLABIKernelSetArgSamplerImpl(x86::Context *ctx)
{
	x86::Regs &regs  = ctx->getRegs();

	// Arguments 
	int kernel_id = (int)regs.getEcx();
	unsigned index = regs.getEdx();
	x86::Emu::opencl_debug << misc::fmt("\tkernel_id=%d, index=%d\n", kernel_id, index);

	// Not yet 
	fatal("%s: not implemented", __FUNCTION__);

	// No return value 
	return 0;
}


int OpenCLABINDRangeCreateImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
	SI::Emu *si_emu = SI::Emu::getInstance();
	// SI::Gpu *si_gpu = driver->si_gpu;

	x86::Regs &regs   = ctx->getRegs();
	mem::Memory &mem  = ctx->getMem();

	unsigned int global_offset[3];
	unsigned int global_size[3];
	unsigned int local_size[3];

	if (driver->isFused())
		si_emu->setGlobalMem(&(ctx->getMem()));

	// Arguments 
	int kernel_id = (int)regs.getEcx();
	int work_dim = (int)regs.getEdx();
	unsigned global_offset_ptr = regs.getEsi();
	unsigned global_size_ptr = regs.getEdi();
	unsigned local_size_ptr = regs.getEbp();
	x86::Emu::opencl_debug << misc::fmt("\tkernel_id=%d, work_dim=%d\n", 
		kernel_id, work_dim);
	x86::Emu::opencl_debug << misc::fmt("\tglobal_offset_ptr=0x%x, global_size_ptr=0x%x, "
		"local_size_ptr=0x%x\n", global_offset_ptr, global_size_ptr, local_size_ptr);
	
	// Debug 
	assert(work_dim >= 1 && work_dim <= 3);
	mem.Read(global_offset_ptr, work_dim * 4, (char *)global_offset);
	mem.Read(global_size_ptr, work_dim * 4, (char *)global_size);
	mem.Read(local_size_ptr, work_dim * 4, (char *)local_size);
	for (int i = 0; i < work_dim; i++)
		x86::Emu::opencl_debug << misc::fmt("\tglobal_offset[%d] = %u\n", i, global_offset[i]);
	for (int i = 0; i < work_dim; i++)
		x86::Emu::opencl_debug << misc::fmt("\tglobal_size[%d] = %u\n", i, global_size[i]);
	for (int i = 0; i < work_dim; i++)
		x86::Emu::opencl_debug << misc::fmt("\tlocal_size[%d] = %u\n", i, local_size[i]);

	// Get kernel 
	SI::Kernel *kernel = driver->getKernelById(kernel_id);
	if (!kernel)
		fatal("%s: invalid kernel ID (%d)", __FUNCTION__, kernel_id);

	// Create ND-Range 
	std::unique_ptr<NDRange> ndrange(new NDRange(driver->getEmuGpu()));
	x86::Emu::opencl_debug << misc::fmt("\tcreated ndrange %d\n", ndrange->getId());

	// Initialize address space ID.  Our current SVM implementation sets
	// the ndrange ASID to the CPU context's ASID 
	ndrange->setAddressSpaceIndex(x86::Emu::getInstance()->getAddressSpaceIndex());
	x86::Emu::opencl_debug << misc::fmt("\tndrange address space index = %d\n", 
		ndrange->getAddressSpaceIndex());

	// Initialize from kernel binary encoding dictionary
	ndrange->InitFromKernel(kernel);

	// FIXME
	// if (si_gpu)
	// 	SIGpuMapNDRange(si_gpu, ndrange);
	
	// Return ID of new nd-range 
	return ndrange->getId();
}

/*
 * OpenCL ABI call #15 - si_ndrange_get_num_buffer_entries
 *
 * Returns the number of available buffer entries in the waiting 
 * work-group queue.
 *
 * @param unsigned int *host_ptr
 *
 *	Location to be populated with the number of available 
 *	buffer entry slots.
 *
 * @return int
 *
 *	The function always returns 0.
 */

int OpenCLABINDRangeGetBufferEntriesImpl(x86::Context *ctx)
{
	// SI::Gpu *si_gpu = driver->si_gpu;

	x86::Regs &regs   = ctx->getRegs();
	mem::Memory &mem  = ctx->getMem();

	int available_buffer_entries;

	// Arguments 
	unsigned host_ptr = regs.getEcx();

	// if (si_gpu)
	// {
	// 	available_buffer_entries = 
	// 		MaxWorkGroupBufferSize -
	// 		list_count(si_gpu->waiting_work_groups);
	// }
	// else
	// {
		available_buffer_entries = 
			MaxWorkGroupBufferSize;
	// }

	x86::Emu::opencl_debug << misc::fmt("\tavailable buffer entries = %d\n", 
		available_buffer_entries);

	mem.Write(host_ptr, sizeof available_buffer_entries,
		(const char *)&available_buffer_entries);

	// Return
	return 0;
}

/*
 * OpenCL ABI call #16 - si_ndrange_send_work_groups
 *
 * Let's the driver know that work groups have been added to 
 * the queue.
 *
 * @param unsigned int ndrange_id
 *
 *	ID of the ND-Range
 *
 * @param unsigned int work_group_start
 *
 *	First work group to execute
 *
 * @param unsigned int work_group_count
 *
 *	Number of work groups to execute
 *
 * @return int
 *
 *	The function always returns 0.
 */

// bool OpenCLABINDRangeSendWorkGoupsCanWakeup()
// {
// 	// FIXME, x86 suspend mechanism
// 	return true;
// }

// int OpenCLABINDRangeSendWorkGoupsWakeup()
// {
// 	// FIXME, x86 suspend mechanism
// 	return 0;
// }

int OpenCLABINDRangeSendWorkGoupsImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();

	// Arguments 
	int ndrange_id = (int)regs.getEcx();
	unsigned work_group_start = regs.getEdx();
	unsigned work_group_count = regs.getEsi();

	NDRange *ndrange = driver->getNDRangeById(ndrange_id);
	if (!ndrange)
		fatal("%s: invalid ndrange ID (%d)", __FUNCTION__, ndrange_id);
	x86::Emu::opencl_debug << misc::fmt("\tndrange %d\n", ndrange->getId());

	assert(work_group_count <= MaxWorkGroupBufferSize -
		ndrange->getWaitingWorkgroupsCount());

	x86::Emu::opencl_debug << misc::fmt("\treceiving %d work groups: (%d) through (%d)\n",
		work_group_count, work_group_start, 
		work_group_start + work_group_count - 1);

	// Receive work groups (add them to the waiting queue) 
	for (unsigned work_group_id = work_group_start; 
		work_group_id < work_group_start + work_group_count; 
		work_group_id++)
		ndrange->AddWorkgroupIdToWaitingList(work_group_id);

	// Suspend x86 context until driver needs more work 
	// ctx->Suspend(OpenCLABINDRangeSendWorkGoupsCanWakeup, 
	// 	OpenCLABINDRangeSendWorkGoupsWakeup, x86::ContextSuspended);

	// Return
	return 0;
}

/*
 * OpenCL ABI call #17 - si_ndrange_finish
 *
 * Tells the driver that there are no more work groups to execute
 * from the ND-Range.
 *
 * @param int ndrange_id
 *
 *	ID of nd-range
 *
 * @return int
 *
 *	The function always returns 0.
 */

// bool OpenCLABINDRangeFinishCanWakeup()
// {
// 	// FIXME, x86 suspend mechanism
// 	return true;
// }

// int OpenCLABINDRangeFinishWakeup()
// {
// 	// FIXME, x86 suspend mechanism
// 	return 0;
// }

int OpenCLABINDRangeFinishImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();

	// Arguments 
	int ndrange_id = (int)regs.getEcx();

	NDRange *ndrange = driver->getNDRangeById(ndrange_id);
	if (!ndrange)
		fatal("%s: invalid ndrange ID (%d)", __FUNCTION__, ndrange_id);
	x86::Emu::opencl_debug << misc::fmt("\tndrange %d\n", ndrange->getId());

	ndrange->setLastWorkgroupSend(true);
	
	 // If no work-groups are left in the queues, remove the nd-range
	 // from the driver list 
	if (!ndrange->getRunningWorkgroupsCount() && 
		!ndrange->getWaitingWorkgroupsCount())
	{
		x86::Emu::opencl_debug << misc::fmt("\tnd-range %d finished\n", ndrange_id);
	}
	else
	{
		x86::Emu::opencl_debug << misc::fmt("\twaiting for nd-range %d to finish (blocking)\n", 
				ndrange_id);
	// Suspend x86 context until driver needs more work 
	// ctx->Suspend(OpenCLABINDRangeFinishCanWakeup, 
	// 	OpenCLABINDRangeFinishWakeup, x86::ContextSuspended);
	}

	// Return
	return 0;
}

/*
 * OpenCL ABI call #18 - si_ndrange_pass_mem_objs
 *
 * @param int ndrange_id
 *
 *	ID of nd-range
 *
 * @param int kernel_id
 *
 *	ID of kernel
 *
 * @param unsigned int *tables_ptr
 *
 *	Location reserved for internal tables (may not be aligned)
 *
 * @param unsigned int *constant_buffers_ptr
 *
 *	Location reserved for constant buffers (may not be aligned)
 *
 * @return int
 *
 *	The function always returns 0.
 */

int OpenCLABINDRangePassMemObjsImpl(x86::Context *ctx)
{
	// SIGpu *si_gpu = driver->si_gpu;

	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();

	// Arguments 
	int ndrange_id = (int)regs.getEcx();
	int kernel_id = (int)regs.getEdx();

	NDRange *ndrange = driver->getNDRangeById(ndrange_id);
	if (!ndrange)
		fatal("%s: invalid ndrange ID (%d)", __FUNCTION__, ndrange_id);

	Kernel *kernel = driver->getKernelById(kernel_id);
	if (!kernel)
		fatal("%s: invalid kernel ID (%d)", __FUNCTION__, kernel_id);

	 // When a fused device is present, the driver needs to set up
	 // the addresses of the internal buffers for the GPU as well
	 // as the MMU for both the internal buffers and kernel arguments 
	if (driver->isFused())
	{
		// 16 extra bytes allocated for alignment 
		unsigned tables_ptr = (regs.getEsi() + 15) & 0xFFFFFFF0;

		ndrange->setConstBufferTable(tables_ptr);

		// The successive tables must be aligned 
		ndrange->setResourceTable((ndrange->getConstBufferTableAddr() + 
			EmuConstBufTableSize + 16) & 0xFFFFFFF0);
		
		ndrange->setUAVTable((ndrange->getResourceTableAddr() +
			EmuResourceTableSize + 16) & 0xFFFFFFF0);
		
		unsigned constant_buffers_ptr = (regs.getEdi() + 15) & 0xFFFFFFF0;
		ndrange->setCB0(constant_buffers_ptr);
		ndrange->setCB1(constant_buffers_ptr + EmuConstBuf0Size);

		// Initialize the GPU MMU with the pages required for
		// ndrange execution using the same translations as the
		// CPU MMU 
		// opencl_si_ndrange_setup_mmu(ndrange, driver->x86_cpu->mmu, 
		// 	ctx->address_space_index, si_gpu->mmu, tables_ptr, 
		// 	constant_buffers_ptr);
	}
	else
	{
		// if (si_gpu)
		// {
		// 	opencl_si_kernel_create_ndrange_tables(ndrange, 
		// 		si_gpu->mmu); 
		// 	opencl_si_kernel_create_ndrange_constant_buffers(
		// 		ndrange, si_gpu->mmu); 
		// }
		// else
		{
			kernel->CreateNDRangeTables(ndrange);
			kernel->SetupNDRangeConstantBuffers(ndrange); 
		}
	}
	kernel->SetupNDRangeConstantBuffers(ndrange);
	kernel->SetupNDRangeArgs(ndrange);
	kernel->DebugNDRangeState(ndrange);

	// Return
	return 0;
}

/*
 * OpenCL ABI call #19 - si_ndrange_set_fused
 *
 * @param unsigned int fused
 *
 *	Whether to enable or disable the fused device
 *
 * @return int
 *
 *	The function always returns 0.
 */

int OpenCLABINDRangeSetFusedImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();

	SI::Gpu *si_gpu = driver->getGpu();

	driver->setFused((bool)regs.getEcx());

	// With a fused device, the GPU MMU will be initialized by
	// the CPU 
	if (driver->isFused())
	{
		x86::Emu::opencl_debug << misc::fmt("\tfused\n");
		assert(si_gpu);
		// si_gpu->mmu->read_only = 1;
	}
	else
	{
		x86::Emu::opencl_debug << misc::fmt("\tnot fused\n");
	}

	// Return
	return 0;
}

/*
 * OpenCL ABI call #20 - si_ndrange_flush
 *
 * @param int ndrange-id
 *
 *	ID of ND-Range to flush
 *
 * @return int
 *
 *	The function always returns 0.
 */

// bool OpenCLABINDRangeFlushCanWakeup()
// {
// 	// FIXME, x86 suspend mechanism
// 	return true;
// }

// int OpenCLABINDNDRangeFlushWakeup()
// {
// 	// FIXME, x86 suspend mechanism
// 	return 0;
// }

int OpenCLABINDRangeFlushImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();

	SI::Gpu *si_gpu = driver->getGpu();

	// If there's not a timing simulator, no need to flush 
	if (!si_gpu)
		return 0;

	int ndrange_id = (int)regs.getEcx();

	NDRange *ndrange = driver->getNDRangeById(ndrange_id);
	if (!ndrange)
		fatal("%s: invalid ndrange ID (%d)", __FUNCTION__, ndrange_id);

	x86::Emu::opencl_debug << misc::fmt("\tndrange %d\n", ndrange->getId());

	// Flush RW or WO buffers from this ND-Range 
	// SI::Kernel::FlushNDRangeBuffers(ndrange/*, si_gpu, x86_emu*/);

	// X86ContextSuspend(ctx, opencl_abi_si_ndrange_flush_can_wakeup, 
	// 	&(ndrange->flushing), opencl_abi_si_ndrange_flush_wakeup, 
	// 	&(ndrange->flushing));

	// Return
	return 0;
}

/*
 * OpenCL ABI call #21 - si_ndrange_free
 *
 * @param int ndrange-id
 *
 *	ID of ND-Range to free
 *
 * @return int
 *
 *	The function always returns 0.
 */

int OpenCLABINDRangeFreeImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();
	x86::Regs &regs = ctx->getRegs();

	int ndrange_id = (int)regs.getEcx();

	// Free 
	driver->RemoveNDRangeById(ndrange_id);

	// Return
	return 0;
}

/*
 * OpenCL ABI call #22 - ndrange_start
 *
 * Tell the driver that an nd-range (for any device) has
 * started executing. 
 *
 * @return int
 *
 *	The function always returns 0.
 */

int OpenCLABINDRangeStartImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();

	driver->incNDRangeRunning()	;

	// Return
	return 0;
}

/*
 * OpenCL ABI call #23 - ndrange_end
 *
 * Tell the driver that an nd-range (for any device) has
 * finished executing. 
 *
 * @return int
 *
 *	The function always returns 0.
 */

int OpenCLABINDRangeEndImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();

	driver->decNDRangeRunning()	;

	// Return
	return 0;
}

}  // namespace SIDriver

