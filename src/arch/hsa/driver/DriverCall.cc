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

#include <lib/cpp/String.h>
#include <memory/Memory.h>

#include "Driver.h"


namespace HSA
{

/// ABI Call 'Init'
/// Version check between runtime and driver
///
/// \param runtime_major
///	Major version from the runtime
///
/// \param runtime_minor
///	Minor version from the runtime
///
/// \return
///	0 if version check passed
int Driver::CallInit(mem::Memory *memory, unsigned args_ptr)
{
	// Read arguments
	int runtime_major;
	int runtime_minor;
	memory->Read(args_ptr, 4, (char *)&runtime_major);
	memory->Read(args_ptr + 4, 4, (char *)&runtime_minor);

	// Debug
	debug << misc::fmt("\tMulti2Sim implementation in host: v.%d.%d.",
			runtime_major, runtime_minor);
	debug << misc::fmt("\tMulti2Sim implementation in guest: v.%d.%d.",
				driver_major, driver_minor);

	// Version check
	if(driver_major != runtime_major)
		misc::fatal("Host and device major versions do not match!");
	if(driver_minor < runtime_minor)
		misc::fatal("Device minor version is lower than host minor version!");
	return 0;
}


// ABI Call 'MemAlloc'
// Allocate memory in device
//
// size
// 	The amount of memory to allocate in byte.
//
// Return Value
//  Address where the piece of memory is allocated on device
//
int Driver::CallMemAlloc(mem::Memory *memory, unsigned args_ptr)
{
	// Read arguments
	unsigned size;
	memory->Read(args_ptr, 4, (char *)&size);

	// Debug
	debug << misc::fmt("\tMemAlloc size = %d\n", size);

	// Allocate memory
	/* TODO:
	 * call a function in the emulator to get a piece of memory
	 * and return it as an int.
	 * int device_ptr = hsa_emu_mem_alloc(size);
	 */
	int device_ptr = 0;
	return device_ptr;
}


// ABI Call 'MemFree'
// Deallocate memory from device
//
// device_ptr
//  a device pointer to be deallocate
//
// Return Value
//  0 if success (more error code)
//
int Driver::CallMemFree(mem::Memory *memory, unsigned args_ptr)
{
	// Read arguments
	unsigned device_ptr;
	memory->Read(args_ptr, 4, (char *)&device_ptr);

	// Debug
	debug << misc::fmt("\tMemFree device_ptr = 0x%08x\n", device_ptr);

	// Free memory
	/* TODO:
	 * call a function in the emulator to deallocate the memory
	 * at device_ptr
	 * int ret = hsa_emu_mem_free(device_ptr);
	 */
	int ret = 0;
	return ret;
}


// ABI Call 'MemWrite'
// Copy buffer from the host to the device.
// The length of the buffer is defined in argument size
//
// device_ptr
//  address of device buffer as destination
//
// host_ptr
//  address of host buffer as source
//
// size
//  the size of buffer to write from host to device.
//
// Return Value
//  0 if success (more error code)
//
int Driver::CallMemWrite(mem::Memory *memory, unsigned args_ptr)
{
	// Read arguments
	unsigned device_ptr;
	unsigned host_ptr;
	unsigned size;
	memory->Read(args_ptr, 4, (char *) &device_ptr);
	memory->Read(args_ptr + 4, 4, (char *) &host_ptr);
	memory->Read(args_ptr + 8, 4, (char *) &size);

	// Debug
	debug << misc::fmt("\tMemWrite device_ptr = 0x%08x", device_ptr);
	debug << misc::fmt("\tMemWrite host_ptr = 0x%08x;", host_ptr);
	debug << misc::fmt("\tMemWrite size = 0x%08x", size);

	// Read From host, write to device1
	std::unique_ptr<char> buf(new char[size]);
	memory->Read(host_ptr, size, buf.get());
	/* TODO:
	 * write buffer to device memory
	 * device_memory->Write(device_ptr, size, buf);
	 */
	return 0;
}


// ABI Call 'MemRead'
// Copy buffer from the device to the host. The length of the buffer is defined in size
//
// host_ptr
//  Address of host buffer as destination
//
// device_ptr
//  Address of device buffer as source
//
// size
//  Size of buffer to write from device to host
//
// Return Value
//  0 if success (more error code)
//
int Driver::CallMemRead(mem::Memory *memory, unsigned args_ptr)
{
	// Read arguments
	unsigned host_ptr;
	unsigned device_ptr;
	unsigned size;
	memory->Read(args_ptr, 4, (char *) &host_ptr);
	memory->Read(args_ptr + 4, 4, (char *) &device_ptr);
	memory->Read(args_ptr + 8, 4, (char *) &size);

	// Debug
	debug << misc::fmt("\tMemRead host_ptr = 0x%08x;", host_ptr);
	debug << misc::fmt("\tMemRead device_ptr = 0x%08x;", device_ptr);
	debug << misc::fmt("\tMemRead size = 0x%08x", size);

	// Read from device, write to host
	std::unique_ptr<char> buf(new char[size]);
	/* TODO:
	 * read buffer from device memory
	 * device_memory->Read(device_ptr, size, buf.get());
	 */
	memory->Write(host_ptr, size, buf.get());
	return 0;
}


// ABI Call 'MemCopyDevice'
// Copy from one device buffer to another device buffer
//
// device_dest_ptr
//  Address of device buffer as destination
//
// device_src_ptr
//  Address of device buffer as source
//
// size
//  Size of buffer to copy
//
// Return Value
//  0 if success (more error code)
//
int Driver::CallMemCopyDevice(mem::Memory *memory, unsigned args_ptr)
{
	// Read arguments
	unsigned device_dest_ptr;
	unsigned device_src_ptr;
	unsigned size;
	memory->Read(args_ptr, 4, (char *) &device_dest_ptr);
	memory->Read(args_ptr + 4, 4, (char *) &device_src_ptr);
	memory->Read(args_ptr + 8, 4, (char *) &size);

	// Debug
	debug << misc::fmt("\tMemCopyDevice device_dest_ptr = 0x%08x",
			device_dest_ptr);
	debug << misc::fmt("\tMemCopyDevice device_src_ptr = 0x%08x",
			device_src_ptr);
	debug << misc::fmt("\tMemCopyDevice size = 0x%08x", size);

	// Read from device, write to device
	std::unique_ptr<char> buf(new char[size]);
	/* TODO:
	 * read from source buffer in device memory
	 * device_memory->Read(device_src_ptr, size, buf.get());
	 * write to destination buffer in device memory
	 * device_memory->Write(device_dest_ptr, size, buf.get());
	 */
	return 0;
}


// ABI Call 'ProgramCreateWithBinary'
// Create a program with binary and return an unique ID to the program
//
// file
//  Pointer to an ELF file in the host memory
//
// size
//  Size of the ELF file
//
// Return Value
//  Unique program ID
//
// FIXME: Figure out the mechanism for the binary file
// 1. load the binary to device memory or only pass the pointer
// 2. ELFReader only support loading binary from the multi2sim host machine
int Driver::CallProgramCreateWithBinary(mem::Memory *memory, unsigned args_ptr)
{
	// Read arguments
	void *file;
	unsigned size;
	memory->Read(args_ptr, 4, (char *) &file);
	memory->Read(args_ptr, 4, (char *) &size);

	// Create Program object
	unsigned id = getNewProgramID();
	Program * program = new Program(id);
	program_list.push_back(program);

	// Load binary
	program->setBinary((char *)file, size);

	// debug
	debug << misc::fmt("\tProgram with ID %d created", id);
	debug << misc::fmt("\tBinary location = %p", file);
	debug << misc::fmt("\tBinary size = %d", size);

	// Return unique program ID
	return (int)id;
}


// ABI Call 'KernelCreate'
// Create a kernel for a given program
//
// program_id
//  ID of a program to for which the kernel is created
//
// kernel_name
//  Name of the kernel in program
//
// Return Value
//  Unique kernel ID
int Driver::CallKernelCreate(mem::Memory *memory, unsigned args_ptr)
{
	// Read arguments
	unsigned program_id;
	std::string *kernel_name;
	memory->Read(args_ptr, 4, (char *) &program_id);
	memory->Read(args_ptr, 4, (char *) &kernel_name);

	// Create new Kernel
	unsigned kernel_id = getNewKernelID();
	Kernel * kernel = new Kernel(kernel_id, kernel_name, program_id);
	kernel_list.push_back(kernel);

	// Debug
	debug << misc::fmt("\tKernel created for Program %d", program_id);
	debug << misc::fmt("\tKernel name is %s", kernel_name->c_str());
	return kernel_id;
}



}  // namepsace HSA

