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

#include <arch/kepler/emu/Emu.h>
#include <arch/kepler/asm/Asm.h>
#include <arch/kepler/emu/Grid.h>
#include <lib/cpp/String.h>
#include <lib/util/string.h>
#include <memory/Memory.h>

#include "Driver.h"
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
int Driver::CallInit(mem::Memory *memory, unsigned args_ptr)
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

	// Allocate memory
	Kepler::Emu *kpl_emu = Kepler::Emu::getInstance();
	mem::Memory *global_mem = kpl_emu->getGlobalMem();
	global_mem->Map(kpl_emu->getGlobalMemTop(), size,
			mem::Memory::AccessRead | mem::Memory::AccessWrite);

	// Virtual address of memory
	unsigned device_ptr = kpl_emu->getGlobalMemTop();
	debug << misc::fmt("\t%d bytes of device memory allocated at 0x%x\n",
			size, device_ptr);

	// Increase global memory top
	kpl_emu->incGloablMemTop(size);

	// Return device pointer
	return device_ptr;
}


/// ABI Call 'MemRead'
///
/// ...
int Driver::CallMemRead(mem::Memory *memory, unsigned args_ptr)
{
	Kepler::Emu *kpl_emu = Kepler::Emu::getInstance();
	mem::Memory* global_mem = kpl_emu->getGlobalMem();

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

	// Check memory range
	if (device_ptr + size > kpl_emu->getGlobalMemTop())
		throw Error("Accessing device memory not allocated");

	// Read memory from to device host
	std::unique_ptr<char> buffer(new char[size]);
	memory->Read(device_ptr, size, buffer.get());
	global_mem->Write(host_ptr, size, buffer.get());

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
int Driver::CallMemWrite(mem::Memory *memory, unsigned args_ptr)
{
	Kepler::Emu *kpl_emu = Kepler::Emu::getInstance();
	mem::Memory *global_mem = kpl_emu->getGlobalMem();

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

	// Check memory range
	if (device_ptr + size > kpl_emu->getGlobalMemTop())
		throw Error("Accessing device memory not allocated");

	// Read memory from host to device
	std::unique_ptr<char> buffer(new char[size]);
	global_mem->Read(device_ptr, size, buffer.get());
	memory->Write(host_ptr, size, buffer.get());

	// Return
	return 0;
}


/// ABI Call 'LaunchKernel'
///
/// ...
int Driver::CallLaunchKernel(mem::Memory *memory, unsigned args_ptr)
{
	Kepler::Emu *kpl_emu = Kepler::Emu::getInstance();
	mem::Memory* const_mem = kpl_emu->getConstMem();

	// Arguments
	unsigned function_id;
	unsigned grid_dim[3];
	unsigned block_dim[3];
	unsigned shared_mem_size;
	unsigned stream;
	unsigned kernel_args;
	unsigned extra;

	// Read arguments
	memory->Read(args_ptr, sizeof(unsigned), (char *) &function_id);
	memory->Read(args_ptr + 4, sizeof(unsigned), (char *) &grid_dim[0]);
	memory->Read(args_ptr + 8, sizeof(unsigned), (char *) &grid_dim[1]);
	memory->Read(args_ptr + 12, sizeof(unsigned), (char *) &grid_dim[2]);
	memory->Read(args_ptr + 16, sizeof(unsigned), (char *) &block_dim[0]);
	memory->Read(args_ptr + 20, sizeof(unsigned), (char *) &block_dim[1]);
	memory->Read(args_ptr + 24, sizeof(unsigned), (char *) &block_dim[2]);
	memory->Read(args_ptr + 28, sizeof(unsigned), (char *) &shared_mem_size);
	memory->Read(args_ptr + 32, sizeof(unsigned), (char *) &stream);
	memory->Read(args_ptr + 36, sizeof(unsigned), (char *) &kernel_args);
	memory->Read(args_ptr + 40, sizeof(unsigned), (char *) &extra);

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

	// Get function
	Function *function = getFunction(function_id);
	if (function == nullptr)
		throw Driver::Error(misc::fmt("Invalid function ID (%d)",
				function_id));

	// Set up arguments
	int offset = 0x20;  // Start writing at this position
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
		offset += 0x4;
	}

	// Create grid
	Grid *grid;
	grid = new Grid(function);

	// Set up grid
	grid->SetupSize(grid_dim, block_dim);
	grid->GridSetupConstantMemory();

	// Add to pending list
	kpl_emu->PushPendingGrid(grid);

	// Return value
	return 0;
}


/// ABI Call 'MemGetInfo'  FIXME
///
/// param unsigned free;
/// Returned free global memory in bytes.
///
/// param unsigned total
/// Returned total global memory in bytes
///
/// return value
/// the return is always 0 on success
int Driver::CallMemGetInfo(mem::Memory *memory, unsigned args_ptr)
{
	// Get Emu instance and global memory
	Kepler::Emu *kpl_emu = Kepler::Emu::getInstance();

	// Arguments
	unsigned free;
	unsigned total;

	// Read Arguments
	memory->Read(args_ptr, sizeof(unsigned), (char *) &free);
	memory->Read(args_ptr + 4, sizeof(unsigned), (char *) &total);

	// Debug Info
	debug << misc::fmt("\tout: free=%u\n", free);
	debug << misc::fmt("\tout: total=%u\n", total);

	// Write Result to Device
	kpl_emu->setGlobalMemFreeSize(free);
	kpl_emu->setGlobalMemTotalSize(total);

	// Return
	return 0;
}


/// ABI Call 'ModuleLoad'
///
/// \param char *path
///	Path of a cubin binary to load the module from.
///
/// \return
///	[...] FIXME
int Driver::CallModuleLoad(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments
	unsigned path_ptr;
	memory->Read(args_ptr, sizeof(unsigned), (char *) &path_ptr);

	// Get path to cubin binary
	char path[MAX_STRING_SIZE];
	memory->Read(path_ptr, MAX_STRING_SIZE, path);

	// Create module
	Module *module = addModule(path);
	return module->getId();
}


int Driver::CallModuleGetFunction(mem::Memory *memory, unsigned args_ptr)
{	/*
	// Arguments
	unsigned module_id;
	char func_name[MAX_STRING_SIZE];;

	// Read Arguments
	memory->Read(args_ptr, sizeof (unsigned), (char*) &module_id);
	memory->Read(args_ptr, sizeof func_name, func_name);

	// Debug Info
	debug << misc::fmt("\tout: module_id=%u\n", module_id);

	CUmodule *module;
	// Get module
	module = module_list(module_id);

	// Create function
	CUfunction function(module, func_name);  */

	return 0;

}

}  // namespace Kepler
