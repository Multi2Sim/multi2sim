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
		misc::fatal("%s: accessing device memory not allocated",
				__FUNCTION__);

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
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'MemFree'
//
// ...
int Driver::CallMemFree(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
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
		misc::fatal("%s: invalid program ID (%d)",
				__FUNCTION__, program_id);

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
		misc::fatal("%s: invalid program ID (%d)",
				__FUNCTION__, program_id);

	// Add kernel object
	int kernel_count = getKernelCount();
	AddKernel(kernel_count, func_name, program);

	// Return
	return getKernelById(kernel_count)->getId();
}


// ABI Call 'KernelSetArgValue'
//
// ...
int Driver::CallKernelSetArgValue(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
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
	// No return value 
	return 0;
}

// ABI Call 'KernelSetArgSampler'
//
// ...
int Driver::CallKernelSetArgSampler(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'KernelSetArgImage'
//
// ...
int Driver::CallKernelSetArgImage(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}


// ABI Call 'NdrangeCreate'
//
// ...
int Driver::CallNdrangeCreate(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}


// ABI Call 'NdrangeGetNumBufferEntries'
//
// ...
int Driver::CallNdrangeGetNumBufferEntries(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}

// ABI Call 'NdrangeSendWorkGroups'
//
// ...
int Driver::CallNdrangeSendWorkGroups(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}

// ABI Call 'NdrangeFinish'
//
// ...
int Driver::CallNdrangeFinish(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}

// ABI Call 'NdrangePassMemObjs'
//
// ...
int Driver::CallNdrangePassMemObjs(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}

// ABI Call 'NdrangeSetFused'
//
// ...
int Driver::CallNdrangeSetFused(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}

// ABI Call 'NdrangeFlush'
//
// ...
int Driver::CallNdrangeFlush(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}

// ABI Call 'NdrangeFree'
//
// ...
int Driver::CallNdrangeFree(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}

// ABI Call 'NdrangeStart'
//
// ...
int Driver::CallNdrangeStart(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}

// ABI Call 'NdrangeEnd'
//
// ...
int Driver::CallNdrangeEnd(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'RuntimeDebug'
//
// ...
int Driver::CallRuntimeDebug(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

}  // namepsace SI

