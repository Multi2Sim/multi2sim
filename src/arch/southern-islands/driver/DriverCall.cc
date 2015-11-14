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

#include <cassert>

#include <arch/southern-islands/disassembler/Argument.h>
#include <arch/southern-islands/emulator/Emulator.h>
#include <arch/southern-islands/timing/Gpu.h>
#include <arch/southern-islands/timing/Timing.h>
#include <arch/x86/emulator/Context.h>
#include <lib/cpp/String.h>
#include <memory/Memory.h>

#include "Driver.h"


namespace SI
{


// ABI Call 'Init'
//
// ...
int Driver::CallInit(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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

	// Map new pages 
	SI::Emulator *si_emu = SI::Emulator::getInstance();	
	mem::Memory *video_mem = si_emu->getVideoMemory();
	video_mem->Map(si_emu->getVideoMemoryTop(), size,
		mem::Memory::AccessRead | mem::Memory::AccessWrite);

	// Virtual address of memory object 
	unsigned device_ptr = si_emu->getVideoMemoryTop();

	debug << misc::fmt("\t%d bytes of device memory allocated at 0x%x\n",
		size, device_ptr);

	// For now, memory allocation in device memory is done by just 
	// incrementing a pointer to the top of the global memory space. 
	// Since memory deallocation is not implemented, "holes" in the 
	// memory space are not considered. 
	si_emu->incVideoMemoryTop(size);

	// Return device pointer 
	return device_ptr;
}


// ABI Call 'MemRead'
//
// ...
int Driver::CallMemRead(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	SI::Emulator *emulator = SI::Emulator::getInstance();
	mem::Memory *video_memory = emulator->getVideoMemory();

	// Arguments 
	unsigned host_ptr;
	unsigned device_ptr;
	unsigned size;
	
	// Check for fused memory
	if (fused)                                                       
		throw Error(misc::fmt("%s: GPU is set as a fused device, "
				"so the x86 memory operations should be used", 
				__FUNCTION__));       

	// Read Arguments	
	memory->Read(args_ptr, sizeof(unsigned), (char *) &host_ptr);
	memory->Read(args_ptr + 4, sizeof(unsigned), (char *) &device_ptr);
	memory->Read(args_ptr + 8, sizeof(unsigned), (char *) &size);

	// Debug                                                          
	debug << misc::fmt("\thost_ptr = 0x%x, device_ptr = 0x%x, "
			"size = %d bytes\n", host_ptr, device_ptr, size);                             

	// Check memory range
	if (device_ptr + size > emulator->getVideoMemoryTop())
		throw Error(misc::fmt("%s: accessing device memory not "
				"allocated", __FUNCTION__));                                   

	// Read memory from host to device
	auto buffer = misc::new_unique_array<char>(size);
	video_memory->Read(device_ptr, size, buffer.get());
	memory->Write(host_ptr, size, buffer.get());
	
	// Return                                                         
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
int Driver::CallMemWrite(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	SI::Emulator *emulator = SI::Emulator::getInstance();
	mem::Memory *video_memory = emulator->getVideoMemory();

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
	if (device_ptr + size > emulator->getVideoMemoryTop())
		throw Error(misc::fmt("Device not allocated"));

	// Read memory from host to device
	auto buffer = misc::new_unique_array<char>(size);
	memory->Read(host_ptr, size, buffer.get());
	video_memory->Write(device_ptr, size, buffer.get());

	// Return
	return 0;
}


// ABI Call 'MemCopy'
//
// ...
int Driver::CallMemCopy(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	SI::Emulator *emulator = SI::Emulator::getInstance();
	mem::Memory *video_memory = emulator->getVideoMemory();
	
	// Arguments
	unsigned int dest_ptr;                                                   
	unsigned int src_ptr;                                                    
	unsigned int size;                                                       

	// Check for fused memory
	if (fused)                                                                                                                            
		throw Error(misc::fmt("%s: GPU is set as a fused device, "
				"so the x86 memory operations should be used", 
				__FUNCTION__));

	// Read Arguments	
	memory->Read(args_ptr, sizeof(unsigned), (char *) &dest_ptr);
	memory->Read(args_ptr + 4, sizeof(unsigned), (char *) &src_ptr);
	memory->Read(args_ptr + 8, sizeof(unsigned), (char *) &size);

	// Debug                                                          
	debug << misc::fmt("\tdest_ptr = 0x%x, src_ptr = 0x%x, "
			"size = %d bytes\n", dest_ptr, src_ptr, size);                             

	// Check memory range
	if (src_ptr + size > emulator->getVideoMemoryTop() || 
			dest_ptr + size > emulator->getVideoMemoryTop())
		throw Error(misc::fmt("%s: accessing device memory not "
				"allocated", __FUNCTION__));                                   

	// Read memory from host to device
	auto buffer = misc::new_unique_array<char>(size);
	video_memory->Read(src_ptr, size, buffer.get());
	video_memory->Write(dest_ptr, size, buffer.get());

	// Return
	return 0;  
}


// ABI Call 'MemF// ...
int Driver::CallMemFree(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	//unsigned int device_ptr;                                                 

	// Read Arguments	
	//memory->Read(args_ptr, sizeof(unsigned), (char *) &device_ptr);
	
	// debug
	//debug << misc::fmt("\tdevice_ptr = %u\n", device_ptr);                         

	// For now, this call is ignored. No deallocation of global memory can   
	// happen.

	// Return device pointer                                           
	//return device_ptr; 
	return 0;
}


/// ABI Call 'ProgramCreate'
///
/// \return
///	Return unique program id
int Driver::CallProgramCreate(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Create program
	int program_id = getProgramCount();
	AddProgram(program_id);

	// Return
	Program *program = getProgramById(program_id);
	assert(program);
	return program->getId();
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
int Driver::CallProgramSetBinary(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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
	auto buffer = misc::new_unique_array<char>(bin_size);
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
int Driver::CallKernelCreate(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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

int Driver::CallKernelSetArgValue(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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
	ValueArgument *arg = dynamic_cast<ValueArgument *>(
			kernel->getArgByIndex(index));
	if (!arg || arg->getType() != Argument::TypeValue)
		throw Error(misc::fmt("Invalid type for argument %d", index));

	debug << misc::fmt("\tname=%s\n", (arg->name).c_str());
	
	// Dynamically allocate value_ptr and release it so ownership can be
	// taken by the unique pointer in class Arg
	
	auto value = misc::new_unique_array<char>(size);
	memory->Read(host_ptr, size, value.get());

	// Save value and size
	arg->setValue(std::move(value));
	arg->size = size;
	arg->set = true;

	// No return value 
	return 0;
}


/// ABI Call 'KernelSetArgPointer'
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
int Driver::CallKernelSetArgPointer(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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
	PointerArgument *arg = dynamic_cast<PointerArgument *>(
			kernel->getArgByIndex(index));
	if (!arg || arg->getType() != Argument::TypePointer)
		throw Error(misc::fmt("Invalid type for argument %d", index));

	debug << misc::fmt("\tname=%s\n", (arg->name).c_str());
	// Save value 
	arg->set = true;
	arg->size = size;
	arg->setDevicePtr(device_ptr);

	// No return value 
	return 0;
}


// ABI Call 'KernelSetArgSampler'
//
// ...
int Driver::CallKernelSetArgSampler(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}


// ABI Call 'KernelSetArgImage'
//
// ...
int Driver::CallKernelSetArgImage(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}


/// ABI Call 'NDRangeCreate'
///
/// Create and initialize an ND-Range for the supplied kernel.
///
/// \param int kernel_id
/// 	Kernel ID, as returned by ABI call 'KernelCreate'
///
/// \param int work_dim
///	Number of work dimensions. This is an integer number between 1 and 3,
///	which determines the number of elements of the following arrays.
///
/// \param unsigned int *global_offset
///	Array of 'work_dim' integers containing global offsets.
///
/// \param unsigned int *global_size
///	Array of 'work_dim' integers containing the ND-Range global size in each
///	dimension.
///
/// \param unsigned int *local_size
///	Array of 'work_dim' integers containing the local size in each
///	dimension.
///
/// \return int
///	ID of new nd-range

int Driver::CallNDRangeCreate(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Get emulator and timing instance
	Emulator *emulator = Emulator::getInstance();

	// Arguments
	int kernel_id;
	int work_dim;
	unsigned global_offset_ptr;
	unsigned global_size_ptr;
	unsigned local_size_ptr;

	unsigned int global_offset[3];
	unsigned int global_size[3];
	unsigned int local_size[3];

	// Read arguments
	memory->Read(args_ptr, sizeof(int), (char *) &kernel_id);
	memory->Read(args_ptr + 4, sizeof(int), (char *) &work_dim);
	memory->Read(args_ptr + 8, sizeof(int), (char *) &global_offset_ptr);
	memory->Read(args_ptr + 12, sizeof(int), (char *) &global_size_ptr);
	memory->Read(args_ptr + 16, sizeof(int), (char *) &local_size_ptr);

	// Debug
	debug << misc::fmt("\tkernel_id=%d, work_dim=%d\n", 
		kernel_id, work_dim);
	debug << misc::fmt("\tglobal_offset_ptr=0x%x, global_size_ptr=0x%x, "
		"local_size_ptr=0x%x\n", global_offset_ptr, global_size_ptr, local_size_ptr);
	
	// Debug 
	assert(work_dim >= 1 && work_dim <= 3);
	memory->Read(global_offset_ptr, work_dim * 4, (char *) global_offset);
	memory->Read(global_size_ptr, work_dim * 4, (char *) global_size);
	memory->Read(local_size_ptr, work_dim * 4, (char *) local_size);
	for (int i = 0; i < work_dim; i++)
		debug << misc::fmt("\tglobal_offset[%d] = %u\n", i, global_offset[i]);
	for (int i = 0; i < work_dim; i++)
		debug << misc::fmt("\tglobal_size[%d] = %u\n", i, global_size[i]);
	for (int i = 0; i < work_dim; i++)
		debug << misc::fmt("\tlocal_size[%d] = %u\n", i, local_size[i]);

	// Get kernel
	SI::Kernel *kernel = getKernelById(kernel_id);
	if (!kernel)
		throw Error(misc::fmt("%s: invalid kernel ID (%d)", 
			__FUNCTION__, kernel_id));

	// Create ND-Range
	NDRange *ndrange = emulator->addNDRange();
	debug << misc::fmt("\tcreated ndrange %d\n", ndrange->getId());

	// Initialize from kernel binary encoding dictionary
	ndrange->InitializeFromKernel(kernel);

	// Set the global and local sizes
	ndrange->SetupSize(global_size, local_size, work_dim);

	// Calculate the allowed work groups per wavefront pool and 
	// compute unit. Create the address space for the MMU.
	
	// Get gpu object from timing instance
	if (Timing::getSimKind() == comm::Arch::SimDetailed)
	{
		Gpu *gpu  = Timing::getInstance()->getGpu();
	 	gpu->MapNDRange(ndrange);
		ndrange->address_space = gpu->getMmu()
				->newSpace("Southern Islands");
	}
	
	// Return ID of new nd-range 
	return ndrange->getId();
}


// ABI Call 'NDRangeGetNumBufferEntries'
//
// ...
int Driver::CallNDRangeGetNumBufferEntries(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	unsigned host_ptr;                                                   
	int available_buffer_entries;                                            

	// Read in arguments                                                          
	memory->Read(args_ptr, sizeof(int), (char *) &host_ptr);
	
	// TODO - Implement this part for timing simulator
	// if (si_gpu)                                                              
	// {                                                                        
	//  	available_buffer_entries =                                       
	// 		SI_DRIVER_MAX_WORK_GROUP_BUFFER_SIZE -                   
	// 		list_count(si_gpu->waiting_work_groups);                 
	// }                                                                        
	//else                                                                     
	//{                                                                        
	
	// Set available buffer entries
	available_buffer_entries = MaxWorkGroupBufferSize;

	// Debug
	debug << misc::fmt("\tavailable buffer entries = %d\n", 
			available_buffer_entries);                                       

	// Write to memory
	memory->Write(host_ptr, sizeof available_buffer_entries, 
			(const char *) &available_buffer_entries);

	// Return
	return 0;
}


// ABI Call 'NDRangeSendWorkGroups'
//
// ...
int Driver::CallNDRangeSendWorkGroups(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Get emulator instance
	SI::Emulator *emulator = SI::Emulator::getInstance();
	
	// Read arguments
	int ndrange_id;                                                          
	unsigned work_group_start;                                           
	unsigned work_group_count;                                           
	memory->Read(args_ptr, sizeof(int), (char *) &ndrange_id);
	memory->Read(args_ptr + 4, sizeof(unsigned), (char *) &work_group_start);
	memory->Read(args_ptr + 8, sizeof(unsigned), (char *) &work_group_count);

	// Get ndrange
	NDRange *ndrange = emulator->getNDRangeById(ndrange_id);

	// Check for ndrange
	if (!ndrange)                                                            
		throw Error(misc::fmt("%s: invalid ndrange ID (%d)",
				__FUNCTION__, ndrange_id));  
	
	// Debug
	debug << misc::fmt("\tndrange %d\n", ndrange_id);                             

	assert(work_group_count <= MaxWorkGroupBufferSize - 
			ndrange->getNumWaitingWorkgroups());

	debug << misc::fmt("\treceiving %d work groups: (%d) through (%d)\n",          
			work_group_count, work_group_start,                              
			work_group_start + work_group_count - 1);                        

	// Receive work groups (add them to the waiting queue)               
	for (unsigned work_group_id = work_group_start;                                   
			work_group_id < work_group_start + work_group_count;             
			work_group_id++)                                                                                                                        
		ndrange->AddWorkgroupIdToWaitingList(work_group_id);                                          

	// Return
	return 0;
}


// ABI Call 'NDRangeFinish'
//
// ...
int Driver::CallNDRangeFinish(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Get emulator instance
	SI::Emulator *emulator = SI::Emulator::getInstance();
	
	int ndrange_id;

	// Read arguments
	memory->Read(args_ptr, sizeof(int), (char *) &ndrange_id);

	// Get ndrange
	NDRange *ndrange = emulator->getNDRangeById(ndrange_id);
	
	// Check for ndrange
	if (!ndrange)                                                            
		throw Error(misc::fmt("%s: invalid ndrange ID (%d)",
				__FUNCTION__, ndrange_id));  
	
	// Last work group has been sent
	ndrange->setLastWorkgroupSent(true);

	// If no work-groups are left in the queues, remove the nd-range         
	// from the driver list                                           
	if (!(ndrange->getNumWorkgroups()) &&                         
			!(ndrange->getNumWaitingWorkgroups()))                       
	{                                   
		// The NDRange has finished 
		debug << misc::fmt("\tnd-range %d finished\n", ndrange_id);            
	}                                                                        
	else                                                                     
	{
		// The NDRange has not finished. Suspend the context until it
		// completes
		debug << misc::fmt("\twaiting for nd-range %d to finish (blocking)\n", 
				ndrange_id);
		context->Suspend();
		ndrange->SetSuspendedContext(context);
	}                                                                 

	// Return
	return 0;
}


// ABI Call 'NDRangePassMemObjs'
//
// ...
int Driver::CallNDRangePassMemObjs(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Get emulator instance
	SI::Emulator *emulator = SI::Emulator::getInstance();
	
	int kernel_id;
	int ndrange_id;
	unsigned table_ptr;
	unsigned cb_ptr;
	
	// Read arguments
	memory->Read(args_ptr, sizeof(int), (char *) &ndrange_id);
	memory->Read(args_ptr + 4, sizeof(int), (char *) &kernel_id);
	memory->Read(args_ptr + 8, sizeof(int), (char *) &table_ptr);
	memory->Read(args_ptr + 12, sizeof(int), (char *) &cb_ptr);
	
	// Get NDRange
	NDRange *ndrange = emulator->getNDRangeById(ndrange_id);
	if (!ndrange)
		throw Error(misc::fmt("%s: invalid ndrange ID (%d)", 
			__FUNCTION__, ndrange_id));

	// Get kernel
	SI::Kernel *kernel = getKernelById(kernel_id);
	if (!kernel)
		throw Error(misc::fmt("%s: invalid kernel ID (%d)", 
			__FUNCTION__, kernel_id));

	// TODO - Add support for fused memory
	// TODO - Add support for timing simulator

	// Allocate tables and constant buffers
	kernel->CreateNDRangeTables(ndrange);
	kernel->CreateNDRangeConstantBuffers(ndrange);

	// Setup constant buffers and arguments
	kernel->SetupNDRangeConstantBuffers(ndrange);
	kernel->SetupNDRangeArgs(ndrange);
	kernel->DebugNDRangeState(ndrange);
	
	// Return
	return 0;
}


// ABI Call 'NDRangeSetFused'
//
// ...
int Driver::CallNDRangeSetFused(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Read arguments
	memory->Read(args_ptr, sizeof(bool), (char *) &fused);

	// With a fused device, the GPU MMU will be initialized by               
	// the CPU                                                         
	// if (driver->fused)                                                       
	// {                                                                        
	//	opencl_debug("\tfused\n");                                       
	//	assert(si_gpu);                                                  
	//	si_gpu->mmu->read_only = 1;                                      
	// }                                                                        
	// else                                                                     
	//{                                                                       
	
	debug << misc::fmt("\tnot fused\n");                                                                                                           

	// Return
	return 0;
}


// ABI Call 'NDRangeFlush'
//
// ...
int Driver::CallNDRangeFlush(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Get emulator instance
	SI::Emulator *emulator = SI::Emulator::getInstance();
	
	// TODO - add support for timing simulator
	// If there's not a timing simulator, no need to flush
	// if (!si_gpu)                                                             
	//	return 0;                                                        


	// Read arguments
	int ndrange_id;                                                          
	memory->Read(args_ptr, sizeof(int), (char *) &ndrange_id);
	
	// Get NDRange
	NDRange *ndrange = emulator->getNDRangeById(ndrange_id);
	if (!ndrange)
		throw Error(misc::fmt("%s: invalid ndrange ID (%d)", 
			__FUNCTION__, ndrange_id));

	debug << misc::fmt("\tndrange %d\n", ndrange_id);                             

	// TODO - more support for the timing simulator
	// Flush RW or WO buffers from this ND-Range                          
	//opencl_si_kernel_flush_ndrange_buffers(ndrange, si_gpu, x86_emu);        

	// Return                                      
	return 0;
}


// ABI Call 'NDRangeFree'
//
// ...
int Driver::CallNDRangeFree(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Get emulator instance
	SI::Emulator *emulator = SI::Emulator::getInstance();
	
	int ndrange_id;                                                          
	
	// Read arguments
	memory->Read(args_ptr, sizeof(int), (char *) &ndrange_id);
	
	// Get NDRange
	NDRange *ndrange = emulator->getNDRangeById(ndrange_id);
	if (!ndrange)
		throw Error(misc::fmt("%s: invalid ndrange ID (%d)", 
			__FUNCTION__, ndrange_id));

	// Free       
	emulator->RemoveNDRange(ndrange);

	// Return
	return 0;
}


// ABI Call 'NDRangeStart'
//
// ...
int Driver::CallNDRangeStart(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Get emulator instance
	SI::Emulator *emulator = SI::Emulator::getInstance();

	// Increment number of ndranges that are running
	emulator->incNDRangesRunning();

	// Return
	return 0;
}


// ABI Call 'NDRangeEnd'
//
// ...
int Driver::CallNDRangeEnd(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Get emulator instance
	SI::Emulator *emulator = SI::Emulator::getInstance();

	// Decrement number of ndranges that are running
	emulator->decNDRangesRunning();

	// Return
	return 0;
}


// ABI Call 'RuntimeDebug'
//
// ...
int Driver::CallRuntimeDebug(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	throw misc::Panic("ABI call not implemented");
	return 0;
}

}  // namepsace SI

