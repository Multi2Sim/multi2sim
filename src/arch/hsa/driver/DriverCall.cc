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
#include <mem-system/Memory.h>
#include <assert.h>

#include "Driver.h"


namespace HSA
{


// ABI Call 'Init'
//
// ...
int Driver::CallInit(mem::Memory *memory, unsigned args_ptr)
{
	assert(*memory);
	return 0;
}


// ABI Call 'PrintArgs'
//
// arg1
//	First argument to print
//
// arg2
//	Second
//
// arg3
//	Third
//
// Return value
//	This function returns 3.
//
int Driver::CallPrintArgs(mem::Memory *memory, unsigned args_ptr)
{
	assert(*memory);
	// Read arguments
	int arg1;
	int arg2;
	int arg3;
	memory->Read(args_ptr, 4, (char *) &arg1);
	memory->Read(args_ptr + 4, 4, (char *) &arg2);
	memory->Read(args_ptr + 8, 4, (char *) &arg3);

	// Debug
	debug << misc::fmt("  arg1 = %d\n", arg1);
	debug << misc::fmt("  arg2 = %d\n", arg2);
	debug << misc::fmt("  arg3 = %d\n", arg3);

	// Print arguments
	std::cout << misc::fmt("Driver ABI call says: %d, %d, %d\n",
			arg1, arg2, arg3);
	
	// Done
	return 3;
}

// ABI Call 'MemAlloc'
// Allocate memory in device
// size: the amount of memory to allocate in byte.
// return: address where the piece of memory is allocated on device
int Driver::CallMemAlloc(mem::Memory *memory, unsigned args_ptr)
{
	assert(*memory);
	unsigned size;
	memory->Read(args_ptr, 4, (char *) &size);
	debug << misc::fmt("	MemAlloc size = %d\n", size);
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
// device_ptr: a device pointer to be deallocate
// return: 0 if success (more error code)
int Driver::CallMemFree(mem::Memory *memory, unsigned args_ptr)
{
	assert(*memory);
	unsigned device_ptr;
	memory->Read(args_ptr, 4, (char *)&device_ptr);
	debug << misc::fmt("	MemFree device_ptr = 0x%08x\n", device_ptr);
	/* TODO:
	 * call a function in the emulator to deallocate the memory
	 * at device_ptr
	 * int ret = hsa_emu_mem_free(device_ptr);
	 */
	int ret = 0;
	return ret;
}

// ABI Call 'MemWrite'
// Copy buffer from the host to the device. The length of the buffer is defined in argument size
// device_ptr: address of device buffer as destination
// host_ptr: address of host buffer as source
// size: the size of buffer to write from host to device.
// return: 0 if success (more error code)
int Driver::CallMemWrite(mem::Memory *memory, unsigned args_ptr)
{
	assert(*memory);
	unsigned device_ptr;
	unsigned host_ptr;
	unsigned size;
	memory->Read(args_ptr, 4, (char *) &device_ptr);
	memory->Read(args_ptr + 4, 4, (char *) &host_ptr);
	memory->Read(args_ptr + 8, 4, (char *) &size);
	debug << misc::fmt("	MemWrite device_ptr = 0x%08x; host_ptr = 0x%08x; size = 0x%08x",
			device_ptr, host_ptr, size);
	char * buf = (char *)malloc(size);
	memory->Read(host_ptr, size, buf);
	/* TODO:
	 * write buffer to device memory
	 * device_memory->Write(device_ptr, size, buf);
	 */
	free(buf);
	return 0;
}

// ABI Call 'MemRead'
// Copy buffer from the device to the host. The length of the buffer is defined in size
// host_ptr: address of host buffer as destination
// device_ptr: address of device buffer as source
// size: size of buffer to write from device to host
// return: 0 if success (more error code)
int Driver::CallMemRead(mem::Memory *memory, unsigned args_ptr)
{
	assert(*memory);
	unsigned host_ptr;
	unsigned device_ptr;
	unsigned size;
	memory->Read(args_ptr, 4, (char *) &host_ptr);
	memory->Read(args_ptr + 4, 4, (char *) &device_ptr);
	memory->Read(args_ptr + 8, 4, (char *) &size);
	debug << misc::fmt("	MemWrite host_ptr = 0x%08x; device_ptr = 0x%08x; size = 0x%08x",
				host_ptr, device_ptr, size);
	char * buf = (char *)malloc(size);
	/* TODO:
	 * read buffer from device memory
	 * device_memory->Read(device_ptr, size, buf);
	 */
	memory->Write(host_ptr, size, buf);
	return 0;
}




}  // namepsace HSA

