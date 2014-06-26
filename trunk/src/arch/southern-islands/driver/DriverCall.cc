/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#include <arch/southern-islands/asm/Arg.h>
#include <arch/southern-islands/emu/Emu.h>
#include <lib/cpp/String.h>
#include <memory/Memory.h>

#include "Driver.h"


namespace SI
{


// ABI Call 'Init'
//
// ...
int Driver::CallInit(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}


/// ABI Call 'MemAlloc'
///
/// \param unsigned int size
///	Number of bytes to allocate
///
/// \return
///	The function returns a pointer in the device memory space. This pointer
///	should not be dereferenced in the runtime, but instead passed to other
///	ABI calls taking device pointers as input arguments.
int Driver::CallMemAlloc(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments
	unsigned size;

	// Read arguments
	memory->Read(args_ptr, sizeof(unsigned), (char *) &size);

	// Debug
	debug << misc::fmt("\tsize = %u\n", size);

	// Map new pages 
	SI::Emu *si_emu = SI::Emu::getInstance();	
	mem::Memory &video_mem = si_emu->getVideoMem();
	video_mem.Map(si_emu->getVideoMemTop(), size,
		mem::Memory::AccessRead | mem::Memory::AccessWrite);

	// Virtual address of memory object 
	unsigned device_ptr = si_emu->getVideoMemTop();
	debug << misc::fmt("\t%d bytes of device memory allocated at 0x%x\n",
		size, device_ptr);

	// For now, memory allocation in device memory is done by just 
	// incrementing a pointer to the top of the global memory space. 
	// Since memory deallocation is not implemented, "holes" in the 
	// memory space are not considered. 
	si_emu->incVideoMemTop(size);

	// Return device pointer 
	return device_ptr;
}

// ABI Call 'MemRead'
//
// ...
int Driver::CallMemRead(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}


/// ABI Call 'MemWrite'
///
/// Write memory from host into Southern Islands device.
///
/// \param void *device_ptr
///	Destination pointer in device memory.
///
/// \param void *host_ptr
///	Source pointer in host memory.
///
/// \param unsigned int size
///	Number of bytes to read.
///
/// \return
///	The function does not have any return value.
int Driver::CallMemWrite(mem::Memory *memory, unsigned args_ptr)
{
	SI::Emu *si_emu = SI::Emu::getInstance();
	mem::Memory &video_mem = si_emu->getVideoMem();

	// Arguments 
	unsigned device_ptr;
	unsigned host_ptr;
	unsigned size;

	// Read Arguments	
	memory->Read(args_ptr, sizeof(unsigned), (char *) &device_ptr);
	memory->Read(args_ptr + 4, sizeof(unsigned), (char *) &host_ptr);
	memory->Read(args_ptr + 8, sizeof(unsigned), (char *) &size);
	debug << misc::fmt("\tdevice_ptr = 0x%x, host_ptr = 0x%x, size = %d bytes\n",
			device_ptr, host_ptr, size);

	/* Check memory range */
	if (device_ptr + size > si_emu->getVideoMemTop())
		throw Error("Device not allocated");

	// Read memory from host to device
	std::unique_ptr<char> buffer(new char[size]);
	video_mem.Read(device_ptr, size, buffer.get());
	memory->Write(host_ptr, size, buffer.get());

	// Return
	return 0;
}


// ABI Call 'MemCopy'
//
// ...
int Driver::CallMemCopy(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}


// ABI Call 'MemFree'
//
// ...
int Driver::CallMemFree(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}


/// ABI Call 'ProgramCreate'
///
/// \return
///	Return unique program id
int Driver::CallProgramCreate(mem::Memory *memory, unsigned args_ptr)
{
	// Create program
	int program_count = getProgramCount();
	AddProgram(program_count);

	// Return
	return getProgramById(program_count)->getId();
}

/// ABI Call 'ProgramSetBinary'
///
/// Associate a binary to a Southern Islands program.
///
/// \param program_id
///	Program ID, as returned by a previous ABI call to 'Program Create'.
///
/// \param bin_ptr
///	Pointer to the memory space where the program binary can be found.
///
/// \param bin_size
///	Size of the program binary.
///
/// \return
///	No return value.
int Driver::CallProgramSetBinary(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments 
	int program_id;
	unsigned bin_ptr;
	unsigned bin_size;

	// Read arguments
	memory->Read(args_ptr, sizeof(int), (char *) &program_id);
	memory->Read(args_ptr + 4, sizeof(unsigned int), (char *) &bin_ptr);
	memory->Read(args_ptr + 8, sizeof(unsigned int), (char *) &bin_size);

	// Debug
	debug << misc::fmt("\tprogram_id = %d\n", program_id);
	debug << misc::fmt("\tbin_ptr = 0x%x\n", bin_ptr);
	debug << misc::fmt("\tbin_size = %u\n", bin_size);

	// Get program 
	Program *program = getProgramById(program_id);
	if (!program)
		throw Error(misc::fmt("Invalid program ID (%d)", program_id));

	// Set the binary
	std::unique_ptr<char> buffer(new char[bin_size]);
	memory->Read(bin_ptr, bin_size, buffer.get());
	program->setBinary(buffer.get(), bin_size);

	// No return value 
	return 0;
}


/// ABI Call 'KernelCreate'
///
/// \param int program_id
///	Program ID, as returned by ABI call 'Program Create'.
///
/// \param char *func_name
///	Kernel function name in the program.
///
/// \return int
///	Unique kernel ID.
int Driver::CallKernelCreate(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments
	int program_id;
	unsigned func_name_ptr;

	// Read arguments
	memory->Read(args_ptr, sizeof(int), (char *) &program_id);
	memory->Read(args_ptr + 4, sizeof(unsigned), (char *) &func_name_ptr);

	// Read function name
	std::string func_name = memory->ReadString(func_name_ptr);

	// Debug
	debug << misc::fmt("\tprogram_id = %d\n", program_id);
	debug << misc::fmt("\tfunc_name = '%s'\n", func_name.c_str());

	// Get program object 
	Program *program = getProgramById(program_id);
	if (!program)
		throw Error(misc::fmt("Invalid program ID (%d)", program_id));

	// Add kernel object
	int kernel_count = getKernelCount();
	AddKernel(kernel_count, func_name, program);

	// Return
	return getKernelById(kernel_count)->getId();
}


// ABI Call 'KernelSetArgValue'
//

/// OpenCL ABI call #10 - si_kernel_set_arg_value
///
/// Set a kernel argument with a basic type (cl_char, cl_int, cl_float, ...).
///
/// \param int kernel_id
/// 	Kernel ID, as returned by ABI call 'si_kernel_create'
///
/// \param unsigned index
/// 	Argument index to set.
///
/// \param void *host_ptr
///	Address in host memory containing the value of the argument. The memory
/// 	pointed to by this variable will be copied internally, keeping a copy of
/// 	the argument for future use.
///
/// \param int size
/// 	Argument size. This size must match the size encoded in the kernel
/// 	metadata for this particular argument.
///
/// @return int
///
///	No return value.

int Driver::CallKernelSetArgValue(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments
	int kernel_id;
	int index;
	unsigned host_ptr;
	int size;

	// Read arguments
	memory->Read(args_ptr, sizeof(int), (char *) &kernel_id);
	memory->Read(args_ptr + 4, sizeof(int), (char *) &index);
	memory->Read(args_ptr + 8, sizeof(unsigned), (char *) &host_ptr);
	memory->Read(args_ptr + 12, sizeof(int), (char *) &size);

	// Debug
	debug << misc::fmt("\tkernel_id=%d, index=%d\n", kernel_id, index);
	debug << misc::fmt("\thost_ptr=0x%x, size=%u\n", host_ptr, size);

	// Get kernel 
	Kernel *kernel = getKernelById(kernel_id);
	if (!kernel)
		throw Error(misc::fmt("Invalid kernel ID (%d)", kernel_id));

	// Get argument 
	ArgValue *arg = dynamic_cast<ArgValue *>(kernel->getArgByIndex(index));
	if (!arg || arg->getType() != ArgTypeValue)
		throw Error(misc::fmt("Invalid type for argument %d", index));

	// Check valid size 
	if (size != arg->getSize())
		throw Error(misc::fmt("Argument %d: Size %d expected, but "
				"%d found", index, arg->getSize(), size));

	// Save value 
	std::unique_ptr<void> value_ptr(operator new(size));
	memory->Read(host_ptr, size, (char *) value_ptr.get());
	arg->setValue(value_ptr.get());

	// No return value 
	return 0;
}


/// ABI Call 'KernelSet Arg Pointer'
///
/// Set a kernel argument of type 'cl_mem', or local memory. In general, any
/// argument that uses the 'pointer' name as first token in the metadata entry of
/// the kernel binary.
///
/// \param int kernel_id
///	Kernel ID, as returned by ABI call 'Kernel Create'
///
/// \param int index
///	Argument index to set.
///
/// \param void *device_ptr
///	If the argument represents a 'cl_mem' object in global memory, pointer
///	to device memory containing the data, as returned by a previous call to
///	'Mem Alloc'.
///	If the argument is a variable in local memory, the purpose of the call
///	is just allocating space for it, so this value should be NULL.
///
/// \param unsigned int size
///	If the argument represents a 'cl_mem' object, size allocated in global
///	memory for the object.
///	If the argument is a variable in local memory, number of bytes to be
///	allocated in the device local memory.
///
/// \return
///	No return value.
int Driver::CallKernelSetArgPointer(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments
	int kernel_id;
	int index;
	unsigned device_ptr;
	int size;

	// Read arguments
	memory->Read(args_ptr, sizeof(int), (char *) &kernel_id);
	memory->Read(args_ptr + 4, sizeof(int), (char *) &index);
	memory->Read(args_ptr + 8, sizeof(unsigned), (char *) &device_ptr);
	memory->Read(args_ptr + 12, sizeof(int), (char *) &size);

	// Debug
	debug << misc::fmt("\tkernel_id=%d, index=%d\n", kernel_id, index);
	debug << misc::fmt("\tdevice_ptr=0x%x, size=%u\n", device_ptr, size);

	// Get kernel 
	Kernel *kernel = getKernelById(kernel_id);
	if (!kernel)
		throw Error(misc::fmt("Invalid kernel ID (%d)", kernel_id));

	// Get argument 
	ArgPointer *arg = dynamic_cast<ArgPointer *> (kernel->getArgByIndex(index));
	if (!arg || arg->getType() != ArgTypePointer)
		throw Error(misc::fmt("Invalid type for argument %d", index));

	// Check valid size 
	if (size != arg->getSize())
		throw Error(misc::fmt("Argument %d: size %d expected, but "
				"%d found", index, arg->getSize(), size));

	// Save value 
	arg->setSetFlag(true);
	arg->setSize(size);
	arg->setDevicePtr(device_ptr);

	// No return value 
	return 0;
}


// ABI Call 'KernelSetArgSampler'
//
// ...
int Driver::CallKernelSetArgSampler(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}


// ABI Call 'KernelSetArgImage'
//
// ...
int Driver::CallKernelSetArgImage(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}


// ABI Call 'NDRangeCreate'
//
// ...
int Driver::CallNDRangeCreate(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}


// ABI Call 'NDRangeGetNumBufferEntries'
//
// ...
int Driver::CallNDRangeGetNumBufferEntries(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}

// ABI Call 'NDRangeSendWorkGroups'
//
// ...
int Driver::CallNDRangeSendWorkGroups(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}

// ABI Call 'NDRangeFinish'
//
// ...
int Driver::CallNDRangeFinish(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}

// ABI Call 'NDRangePassMemObjs'
//
// ...
int Driver::CallNDRangePassMemObjs(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}

// ABI Call 'NDRangeSetFused'
//
// ...
int Driver::CallNDRangeSetFused(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}

// ABI Call 'NDRangeFlush'
//
// ...
int Driver::CallNDRangeFlush(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}

// ABI Call 'NDRangeFree'
//
// ...
int Driver::CallNDRangeFree(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}


// ABI Call 'NDRangeStart'
//
// ...
int Driver::CallNDRangeStart(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}


// ABI Call 'NDRangeEnd'
//
// ...
int Driver::CallNDRangeEnd(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}


// ABI Call 'RuntimeDebug'
//
// ...
int Driver::CallRuntimeDebug(mem::Memory *memory, unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}

}  // namepsace SI

