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

namespace SI
{


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
	x86::Emu::opencl_debug << misc::fmt << misc::fmt << misc::fmt << misc::fmt << misc::fmt(
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
	x86::Emu::opencl_debug << misc::fmt << misc::fmt << misc::fmt << misc::fmt(
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

int OpenCLABIKernalCreateImpl(x86::Context *ctx)
{
	Driver::OpenCLSIDriver *driver = Driver::OpenCLSIDriver::getInstance();

	x86::Regs &regs  = ctx->getRegs();
	mem::Memory &mem = ctx->getMem();

	// Arguments 
	int program_id         = (int)regs.getEcx();
	unsigned func_name_ptr = regs.getEdx();
	x86::Emu::opencl_debug << misc::fmt << misc::fmt << misc::fmt(
		"\tprogram_id=%d, func_name_ptr=0x%x\n", program_id, func_name_ptr);

	// Read function name 
	std::string func_name = mem.ReadString(func_name_ptr);
	x86::Emu::opencl_debug << misc::fmt << misc::fmt << misc::fmt(
		"\tfunc_name='%s'\n", func_name.c_str());

	// Get program object 
	Program *program = driver->getProgramById(program_id);
	if (!program)
		fatal("%s: invalid program ID (%d)",
				__FUNCTION__, program_id);

	// Create kernel 
	std::unique_ptr<Kernel> kernel(new Kernel(driver->getKernelCount(),
			func_name, program));
	x86::Emu::opencl_debug << misc::fmt << misc::fmt << misc::fmt(
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
	x86::Emu::opencl_debug << misc::fmt << misc::fmt(
		"\tkernel_id=%d, index=%d\n", kernel_id, index);
	x86::Emu::opencl_debug << misc::fmt << misc::fmt(
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
	void *value_ptr = (void *)calloc(1, size);
	mem.Read(host_ptr, size, (char *)value_ptr);
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
	x86::Emu::opencl_debug << misc::fmt << misc::fmt(
		"\tkernel_id=%d, index=%d\n", kernel_id, index);
	x86::Emu::opencl_debug << misc::fmt << misc::fmt(
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
	x86::Emu::opencl_debug << misc::fmt << misc::fmt("\tkernel_id=%d, index=%d\n", kernel_id, index);

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
	x86::Emu::opencl_debug << misc::fmt << misc::fmt("\tkernel_id=%d, index=%d\n", kernel_id, index);

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

int OpenCLABINDRangeGetBufferEntriesImpl(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenCLABINDRangeSendWorkGoupsImpl(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenCLABINDRangeFinishImpl(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenCLABINDRangePassMemObjsImpl(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenCLABINDRangeSetFusedImpl(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenCLABINDRangeFlushImpl(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenCLABINDRangeFreeImpl(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenCLABINDRangeStartImpl(x86::Context *ctx)
{
	// Return
	return 0;
}

int OpenCLABINDRangeEndImpl(x86::Context *ctx)
{
	// Return
	return 0;
}

}  // namespace SIDriver

