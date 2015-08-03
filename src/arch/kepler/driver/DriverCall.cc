/*
 *  Multi2Sim

 *  Copyright (C) 2014  Xun Gong (gong.xun@husky.neu.edu)
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

#include <memory>

#include <arch/kepler/disassembler/Disassembler.h>
#include <arch/kepler/emulator/Emulator.h>
#include <arch/kepler/emulator/Grid.h>
#include <lib/cpp/String.h>
#include <lib/util/string.h>
#include <memory/Memory.h>

#include "Driver.h"
#include "Module.h"
#include "Function.h"


namespace Kepler
{

// Driver version numbers
const int Driver::version_major = 2;
const int Driver::version_minor = 0;


/// ABI Call 'Init'
///
/// This function return the version information of the driver.
///
/// \param version_ptr
///	This argument is a pointer to a data structure in guest memory where
///	the driver will place version information. The data structure has the
///	following layout:
///
///	struct {
///		int major;
///		int minor;
///	};
///
/// \return
///	The function always returns 0.
int Driver::CallInit(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Read arguments
	unsigned version_ptr;
	memory->Read(args_ptr, 4, (char *) &version_ptr);

	// Debug
	debug << misc::fmt("\tversion_ptr = 0x%x\n", version_ptr);

	// Return version numbers
	memory->Write(version_ptr, 4, (char *) &version_major);
	memory->Write(version_ptr + 4, 4, (char *) &version_minor);

	// Success
	return 0;
}


///	ABI Call 'MemAlloc'
///
/// \param pointer device_ptr
///	The pointer points to allocated memory
///
/// \param unsigned size
///	Number of bytes to allocate
///
/// \return
///	The function returns a pointer in the device memory space. This pointer
///	should not be dereferenced in the runtime, but instead passed to other
///	ABI calls taking device pointers as input arguments.
int Driver::CallMemAlloc(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Arguments
	unsigned size;

	// Read arguments
	memory->Read(args_ptr, sizeof(unsigned), (char *) &size);

	// Debug
	debug << misc::fmt("\tsize = %u\n", size);

	// Allocate memory
	Kepler::Emulator *kpl_emu = Kepler::Emulator::getInstance();
	mem::Memory *global_mem = kpl_emu->getGlobalMemory();

	// Allocate 4 more bytes of memory
	global_mem->Map(kpl_emu->getGlobalMemoryTop(), size + 4,
			mem::Memory::AccessRead | mem::Memory::AccessWrite);
	// Use the first word to record the size of memory
	global_mem->Write(kpl_emu->getGlobalMemoryTop(), sizeof(unsigned),
					(char *) &size);
	// Make the pointer point to the second word of the allocated memory
	unsigned addr = kpl_emu->getGlobalMemoryTop() + sizeof(unsigned) ;

	// Debug information
	debug << misc::fmt("\t%u bytes of device memory allocated at 0x%x\n",
			size + 4, kpl_emu->getGlobalMemoryTop());
	debug << misc::fmt("\tmemory base address is = 0x%x\n",
					kpl_emu->getGlobalMemoryTop());

	// Increase global memory top FIXME
	kpl_emu->incGloablMemoryTop(size + 4);

	return addr;
}


/// ABI Call 'MemRead'
///
/// Read memory from device into host.
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
int Driver::CallMemRead(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	Kepler::Emulator *kpl_emu = Kepler::Emulator::getInstance();
	mem::Memory* global_mem = kpl_emu->getGlobalMemory();

	// Arguments
	unsigned device_ptr;
	unsigned host_ptr;
	unsigned size;

	// Read Arguments
	memory->Read(args_ptr, sizeof(unsigned), (char *) &host_ptr);
	memory->Read(args_ptr + 4, sizeof(unsigned), (char *) &device_ptr);
	memory->Read(args_ptr + 8, sizeof(unsigned), (char *) &size);
	debug << misc::fmt("\tdevice_ptr = 0x%x, host_ptr = 0x%x, size = %d bytes\n",
			device_ptr, host_ptr, size);

	// Check memory range
	if (device_ptr + size > kpl_emu->getGlobalMemoryTop())
		throw Error("Accessing device memory not allocated");

	// Read memory from device to host
	std::unique_ptr<char> buffer(new char[size]);
	global_mem->Read(device_ptr, size, buffer.get());
	memory->Write(host_ptr, size, buffer.get());

	// Return
	return 0;
}


/// ABI Call 'MemWrite'
///
/// Write memory from host into device.
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
int Driver::CallMemWrite(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	Kepler::Emulator *kpl_emu = Kepler::Emulator::getInstance();
	mem::Memory *global_mem = kpl_emu->getGlobalMemory();

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
	debug << misc::fmt("\tMemory top is = 0x%x\n",
					kpl_emu->getGlobalMemoryTop());

	// Check memory range
	//if (device_ptr + size > kpl_emu->getGlobalMemTop())
	//	throw Error("Accessing device memory not allocated");

	// Read memory from host to device
	std::unique_ptr<char> buffer(new char[size]);
	memory->Read(host_ptr, size, buffer.get());
	global_mem->Write(device_ptr, size, buffer.get());

	// Return
	return 0;
}


/// ABI Call 'LaunchKernel'
///
/// Invokes the kernel function whose id is function_id on a gridDimX * gridDimY
/// * gridDimZ grid of blocks. Each block contains blockDimX * blockDimY *
/// blockDimZ threads. SharedMemBytes sets the amount of dynamic shared memory
/// that will be available to each thread block
///
/// \param unsigned function_id
///	Function unique identifier
///
/// \param unsigned grid_dim[0]
///	Width of grid in blocks
///
/// \param unsigned grid_dim[1]
///	Height of grid in blocks
///
/// \param unsigned grid_dim[0]
///	Depth of grid in blocks
///
/// \param unsigned block_dim[0]
///	X dimension of each thread block
///
/// \param unsigned grid_dim[1]
///	Y dimension of each thread block
///
/// \param unsigned grid_dim[0]
///	Z dimension of each thread block
///
/// \param unsigned shared_mem_size
///	Dynamic shared-memory size per thread block in bytes
///
/// \param unsigned stream
///	Stream identifier
///
/// \param void* *kernel_args
///	Array of pointers to kernel parameters
///
/// \param extra
///	extra options (to be decided)
///
/// \return
///	The function does not have any return value.
int Driver::CallLaunchKernel(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	Kepler::Emulator *kpl_emu = Kepler::Emulator::getInstance();
	mem::Memory* const_mem = kpl_emu->getConstMemory();

	// Arguments
	unsigned function_id;
	unsigned function_name_ptr;
	unsigned grid_dim[3];
	unsigned block_dim[3];
	unsigned shared_mem_size;
	unsigned stream;
	unsigned kernel_args;
	unsigned extra;

	// Read arguments
	memory->Read(args_ptr, sizeof(unsigned), (char *) &function_id);
	memory->Read(args_ptr + 4, sizeof(unsigned), (char *) &function_name_ptr);
	memory->Read(args_ptr + 8, sizeof(unsigned), (char *) &grid_dim[0]);
	memory->Read(args_ptr + 12, sizeof(unsigned), (char *) &grid_dim[1]);
	memory->Read(args_ptr + 16, sizeof(unsigned), (char *) &grid_dim[2]);
	memory->Read(args_ptr + 20, sizeof(unsigned), (char *) &block_dim[0]);
	memory->Read(args_ptr + 24, sizeof(unsigned), (char *) &block_dim[1]);
	memory->Read(args_ptr + 28, sizeof(unsigned), (char *) &block_dim[2]);
	memory->Read(args_ptr + 32, sizeof(unsigned), (char *) &shared_mem_size);
	memory->Read(args_ptr + 36, sizeof(unsigned), (char *) &stream);
	memory->Read(args_ptr + 40, sizeof(unsigned), (char *) &kernel_args);
	memory->Read(args_ptr + 44, sizeof(unsigned), (char *) &extra);

	// Debug
	debug << misc::fmt("\tfunction_id = 0x%08x\n", function_id);
	debug << misc::fmt("\tgrid_dimX = %u\n", grid_dim[0]);
	debug << misc::fmt("\tgrid_dimY = %u\n", grid_dim[1]);
	debug << misc::fmt("\tgrid_dimZ = %u\n", grid_dim[2]);
	debug << misc::fmt("\tblock_dimX = %u\n", block_dim[0]);
	debug << misc::fmt("\tblock_dimY = %u\n", block_dim[1]);
	debug << misc::fmt("\tblock_dimZ = %u\n", block_dim[2]);
	debug << misc::fmt("\tshared_mem_usage = %u\n", shared_mem_size);
	debug << misc::fmt("\tstream_handle = 0x%08x\n", stream);
	debug << misc::fmt("\tkernel_args = 0x%08x\n", kernel_args);

	// Read function name
	std::string function_name;
	Function *function;
	function_name = memory->ReadString(function_name_ptr);

	// Get function in the module list
	for (unsigned i = 0; i < modules.size(); i++)
	{
		for (int j = 0; j < modules[i]->getNumFunctions(); j++)
		{
			if(function_name == modules[i]->getFunctionName(j))
				function = modules[i]->getFunction(j);
		}
	}

	// If function_name is not found
	if (function == nullptr)
		throw Driver::Error(misc::fmt("Invalid function ID (%d)",
				function_id));

	// Set up arguments
	int offset = 0x140;  // Start writing at this position
	for (int i = 0; i < function->getNumArguments(); ++i)
	{
		// Read argument value
		unsigned arg_ptr;
		unsigned temp;
		memory->Read(kernel_args + i * 4, sizeof(unsigned),
				(char *) &arg_ptr);
		memory->Read(arg_ptr, sizeof(unsigned), (char *) &temp);

		// Store argument value
		Argument *argument = function->getArgument(i);
		argument->setValue(temp);

		// Write value to constant memory
		const_mem->Write(offset, sizeof(unsigned),(char *) &temp);

		//std::cout<<"in function	"<<__FUNCTION__<<"the const is"<<temp<<std::endl;
		offset += 0x4;
	}

	// Create grid
	Grid *grid = kpl_emu->addGrid(function);

	// Set up grid
	grid->SetupSize(grid_dim, block_dim);
	grid->GridSetupConstantMemory();

	// Add to pending list
	kpl_emu->PushPendingGrid(grid);

	// Return value
	return 0;
}


/// ABI Call 'MemGetInfo'
///
/// \param unsigned *free;
///	Returned free global memory in bytes.
///
/// \param unsigned *total
///	Returned total global memory in bytes
///
/// \return
///	the return is always 0 on success
int Driver::CallMemGetInfo(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Get Emu instance and global memory
	Kepler::Emulator *kpl_emu = Kepler::Emulator::getInstance();

	// Arguments
	unsigned free;
	unsigned total;

	// Read value from device
	free = kpl_emu->getGlobalMemoryFreeSize();
	total = kpl_emu->getGlobalMemoryTotalSize();

	// Debug Info
	debug << misc::fmt("\tout: free=%u\n", free);
	debug << misc::fmt("\tout: total=%u\n", total);

	// Write results
	memory->Write(args_ptr, sizeof(unsigned), (char *) &free);
	memory->Write(args_ptr + 4, sizeof(unsigned), (char *) &total);

	// Return
	return 0;
}


/// ABI Call 'ModuleLoad'
///
/// The function loads a computer module
///
/// \param char *path
///	Path of a cubin binary to load the module from.
///
/// \return
///	The function always returns 0
int Driver::CallModuleLoad(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Arguments
	unsigned path_ptr;
	memory->Read(args_ptr, sizeof(unsigned), (char *) &path_ptr);

	// Get path to cubin binary
	char path[MAX_STRING_SIZE];
	memory->Read(path_ptr, MAX_STRING_SIZE, path);

	// Create module
	addModule(path);
	return 0;
}


/// ABI Call 'ModuleGetFunction'
///
/// Return a handle to a function within a module If no function of that name
/// exists, returns error
///
/// \param unsigned module_id
///	Module unique identifier
///
/// \param unsigned name_addr
///	Address of function name
///
/// \return
///	The return value is a valid function id on success otherwise an error will
///	be thrown
int Driver::CallModuleGetFunction(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Read module id
	unsigned module_id;
	memory->Read(args_ptr, sizeof (unsigned), (char*) &module_id);

	// Read address of function name
	unsigned name_addr;
	memory->Read(args_ptr + 4, sizeof (unsigned), (char*) &name_addr);

	//Read function name
	std::string function_name;
	function_name = memory->ReadString(name_addr);

	// Debug Info
	debug << misc::fmt("\tout: module_id=%u\n", module_id);

	// Find function name in function list and return function id
	modules[module_id]->addFunction(modules[module_id].get(),function_name);

	// If no function found, return error  TODO
	//throw Driver::Error(misc::fmt("Invalid function name (%s)",
	//			function_name.c_str()));

	return 0;
}


/// ABI Call 'MemFree'
///
/// Frees device memory
///
/// \param unsigned *device_ptr
/// Pointer to the Device memory will be freed
///
/// \return value
///	the return is always 0
int Driver::CallMemFree(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	/*
	// Arguments
	//unsigned device_ptr_addr;
	unsigned device_ptr;
	// Read Arguments
	memory->Read(args_ptr, sizeof(unsigned), (char *) &device_ptr);
	//memory->Read(device_ptr_addr, sizeof(unsigned), (char *) &device_ptr);

	// Debug Info
	debug << misc::fmt("\tDevice memory deallocated at 0x%08x\n", device_ptr);

	// Deallocate memory
	Kepler::Emu *kpl_emu = Kepler::Emu::getInstance();
	//mem::Memory *global_mem = kpl_emu->getGlobalMem();
	//global_mem->Unmap(kpl_emu->getGlobalMemTop(), sizeof( &device_ptr));
	//std::cout<<sizeof((unsigned*)device_ptr)<<std::endl;
	//global_mem->Unmap(device_ptr, sizeof(device_ptr));
	// Return  */
	return 0;
}

}  // namespace Kepler
