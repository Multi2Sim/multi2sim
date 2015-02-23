/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#define __UNIMPLEMENTED__ \
	throw misc::Panic(misc::fmt("Unimplemented driver function %s\n", \
	__FUNCTION__));

#include <arch/hsa/emu/Emu.h>
#include <arch/hsa/emu/Component.h>
#include <arch/hsa/emu/WorkItem.h>
#include <arch/hsa/emu/StackFrame.h>
#include <arch/hsa/emu/ProgramLoader.h>

#include "Driver.h"

namespace HSA
{

int Driver::CallInit(mem::Memory *memory, unsigned args_ptr)
{
	debug << misc::fmt("Executing driver function %s.\n", __FUNCTION__);
	debug << misc::fmt("Finished executing driver function %s, "
			"returning %d.\n", __FUNCTION__, 0);
	return 0;
}


int Driver::CallShutDown(mem::Memory *memory, unsigned args_ptr)
{
	debug << misc::fmt("Executing driver function %s.\n", __FUNCTION__);
	debug << misc::fmt("Finished executing driver function %s, "
				"returning %d.\n", __FUNCTION__, 0);
	return 0;
}


int Driver::CallSystemGetInfo(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::IterateAgentNext(DriverCallbackInfo *args)
{
	// Convert the info object to be this function specific
	AgentIterateNextInfo *info = (AgentIterateNextInfo *)args;
	mem::Memory *memory = info->getMemory();
	WorkItem *work_item = info->getWorkItem();
	unsigned args_ptr = info->getArgsPtr();
	unsigned long long last_handler =
			info->getLastComponentHandler();

	// All information retrieved, then we can pop the stack
	work_item->PopStack();


	debug << misc::fmt("In function %s ", __FUNCTION__);
	debug << misc::fmt("last_handler: %lld, ", last_handler);
	debug << "ret: "<<
		getArgumentValue<unsigned int>(0, memory, args_ptr);
	debug << ", callback: " <<
		getArgumentValue<unsigned int>(4, memory, args_ptr);
	debug << ", data: " <<
		getArgumentValue<unsigned long long>(12, memory, args_ptr);
	debug << ", host_lang: " <<
		getArgumentValue<unsigned int>(20, memory, args_ptr);
	debug << ", workitem_ptr: " <<
		getArgumentValue<unsigned long long>(24, memory, args_ptr)
		<< "\n";

	// Get virtual machine setup
	Driver *driver = Driver::getInstance();
	Emu *emu = Emu::getInstance();
	Component *component = emu->getNextComponent(last_handler);

	// No component to iterate anymore
	if (!component){
		// Set return argument to HSA_STATUS_SUCCESS
		driver->setArgumentValue<unsigned int>(HSA_STATUS_SUCCESS, 0,
				memory, args_ptr);

		// Exit intercepted environment
		StackFrame *stack_top = work_item->getStackTop();
		driver->ExitInterceptedEnvironment(args_ptr, stack_top);

		// Return 0 to tell the function finised its execution
		return 0;
	}

	// Construct a stack frame and implant it the the caller stack
	unsigned callback_address = driver->getArgumentValue<unsigned long long>
			(4, memory,args_ptr);
	unsigned long long data_address = driver->getArgumentValue
			<unsigned long long>(12, memory, args_ptr);
	driver->StartAgentIterateCallback(work_item, callback_address,
			component->getHandler(), data_address, args_ptr);

	return 1;
}


int Driver::CallIterateAgents(mem::Memory *memory, unsigned args_ptr)
{	
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// callback		| 4		| 8
	// data			| 12		| 8
	// host_language	| 20		| 4
	// workitem_prt		| 24		| 8

	// Dump the argument information
	debug << misc::fmt("In function %s", __FUNCTION__);
	debug << "\n\thsa_status_t: "<< 
		getArgumentValue<unsigned int>(0, memory, args_ptr);
	debug << ", \n\tcallback: " << 
		getArgumentValue<unsigned long long>(4, memory, args_ptr);
	debug << misc::fmt(", \n\tdata: 0x%llx", getArgumentValue
			<unsigned long long>(12, memory, args_ptr));
	debug << ", \n\thost_lang: " <<
		getArgumentValue<unsigned int>(20, memory, args_ptr);
	debug << ", \n\tworkitem_ptr: " << 
		getArgumentValue<unsigned long long>(24, memory, args_ptr)
		<< "\n";

	// Get virtual machine setup
	Emu *emu = Emu::getInstance();
	Component *component = emu->getNextComponent(0);

	// No component to iterate anymore
	if (!component){
		// Set return argument to HSA_STATUS_SUCCESS
		setArgumentValue<unsigned int>(HSA_STATUS_SUCCESS, 0, 
				memory, args_ptr);

		// Return 0 to tell the function finised its execution
		return 0;
	}

	// Construct a stack frame and implant it the the caller stack
	WorkItem *workitem = (WorkItem *)getArgumentValue<unsigned long long>(
			24, memory, args_ptr);
	unsigned callback_address = getArgumentValue<unsigned long long>(
			4, memory, args_ptr);
	unsigned long long data_address = getArgumentValue<unsigned long long>
		(12, memory, args_ptr);	
	StartAgentIterateCallback(workitem, callback_address, 
			component->getHandler(), data_address,
			args_ptr);

	return 1;
}


void Driver::StartAgentIterateCallback(WorkItem *work_item,
		unsigned callback_address, 
		unsigned long long componentHandler, 
		unsigned long long data_address,
		unsigned args_ptr)
{
	// Get call back function name
	BrigFile *binary = ProgramLoader::getInstance()->getBinary();
	auto function_dir = binary->getCodeEntryByOffset(callback_address);
	std::string callback_name = function_dir->getName();

	// Create new stack frame
	Function *callback = ProgramLoader::getInstance()->getFunction(
			callback_name);
	auto stack_frame = misc::new_unique<StackFrame>(callback, work_item);

	// Pass argument into stack frame
	VariableScope *function_args = stack_frame->getFunctionArguments();

	// Declare return argument
	auto out_arg_directory = function_dir->Next();
	std::string arg_name = out_arg_directory->getName();
	SegmentManager *func_arg_seg = stack_frame->getFuncArgSegment();
	function_args->DeclearVariable(arg_name, out_arg_directory->getType(),
			out_arg_directory->getDim(), func_arg_seg);

	// Pass argument 1 (Address handler) to the callback
	auto arg1 = function_dir->getFirstInArg();
	std::string arg1_name = arg1->getName();
	function_args->DeclearVariable(arg1_name, BRIG_TYPE_U64,
			arg1->getDim(), func_arg_seg);
	unsigned long long *callee_buffer = 
			(unsigned long long *)function_args->
			getBuffer(arg1_name);
	*callee_buffer = componentHandler;

	// Pass argument 2 (Address to the data field) to the callback
	auto arg2 = arg1->Next();
	std::string arg2_name = arg2->getName();
	function_args->DeclearVariable(arg2_name, BRIG_TYPE_U64,
			arg2->getDim(), func_arg_seg);
	callee_buffer = (unsigned long long *)
			function_args->getBuffer(arg2_name);
	*callee_buffer = data_address;

	// Setup info for return callback function
	mem::Memory *memory = Emu::getInstance()->getMemory();
	auto callback_info = misc::new_unique<AgentIterateNextInfo>(
			work_item, memory, args_ptr, componentHandler);

	// Set the stack frame to be an agent_iterate_callback
	stack_frame->setReturnCallback(&Driver::IterateAgentNext,
			std::move(callback_info));

	// Add stack frame to the work item;
	work_item->PushStackFrame(std::move(stack_frame));
}


int Driver::CallAgentGetInfo(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// agent		| 4		| 8
	// attribute		| 12		| 4
	// value		| 16		| 8
	// host_language	| 24		| 4
	// workitem_ptr		| 28		| 8

	// Retrieve agent handler
	unsigned long long agent_handler = 
			getArgumentValue<unsigned long long>(4, memory, 
					args_ptr);

	// Try to find the agent
	Component *component = Emu::getInstance()->getComponent(agent_handler);
	if (component == nullptr)
	{
		setArgumentValue<unsigned int>(HSA_STATUS_ERROR_INVALID_AGENT,
				0, memory, args_ptr);
		return 0;
	}

	// If the device is found, get the attribute queried
	unsigned int attribute = getArgumentValue<unsigned int>(12, memory, 
			args_ptr);

	// Retrieve the pointer to the value
	unsigned long long value_address = getArgumentValue<unsigned long long>
			(16, memory, args_ptr);
	char *value_ptr = memory->getBuffer(value_address, 8, 
			mem::Memory::AccessWrite);

	switch(attribute)
	{
	case HSA_AGENT_INFO_NAME:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_NAME\n");
		break;
	case HSA_AGENT_INFO_VENDOR_NAME:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INF_VENDOR_NAME\n");
		break;
	case HSA_AGENT_INFO_FEATURE:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_FEATURE\n");
		break;
	case HSA_AGENT_INFO_WAVEFRONT_SIZE:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_WAVEFRONT_SIZE\n");
		break;
	case HSA_AGENT_INFO_WORKGROUP_MAX_DIM:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_WORKGROUP_MAX_DIM\n");
		break;
	case HSA_AGENT_INFO_WORKGROUP_MAX_SIZE:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_WORKGROUP_MAX_SIZE\n");
		break;
	case HSA_AGENT_INFO_GRID_MAX_DIM:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_GRID_MAX_DIM\n");
		break;
	case HSA_AGENT_INFO_GRID_MAX_SIZE:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_GRID_MAX_SIZE\n");
		break;
	case HSA_AGENT_INFO_FBARRIER_MAX_SIZE:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_FBARRIER_MAX_SIZE\n");
		break;
	case HSA_AGENT_INFO_QUEUES_MAX:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_QUEUES_MAX\n");
		break;
	case HSA_AGENT_INFO_QUEUE_MAX_SIZE:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_QUEUE_MAX_SIZE\n");
		break;
	case HSA_AGENT_INFO_QUEUE_TYPE:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_QUEUE_TYPE\n");
		break;
	case HSA_AGENT_INFO_NODE:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_NODE\n");
		break;
	case HSA_AGENT_INFO_DEVICE:
		if (component->getDeivceType() == HSA_DEVICE_TYPE_GPU)
		{
			*value_ptr = 1;
		}
		else
		{
			*value_ptr = 0;
		}
		break;
	case HSA_AGENT_INFO_CACHE_SIZE:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_CACHE_SIZE\n");
		break;
	case HSA_EXT_AGENT_INFO_IMAGE1D_MAX_DIM:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_IMAGE1D_MAX_DIM\n");
		break;
	case HSA_EXT_AGENT_INFO_IMAGE2D_MAX_DIM:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_IMAGE2D_MAX_DIM\n");
		break;
	case HSA_EXT_AGENT_INFO_IMAGE3D_MAX_DIM:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_IMAGE3D_MAX_DIM\n");
		break;
	case HSA_EXT_AGENT_INFO_IMAGE_ARRAY_MAX_SIZE:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_IMAGE_ARRAY_MAX_SIZE\n");
		break;
	case HSA_EXT_AGENT_INFO_IMAGE_RD_MAX:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_IMAGE_RD_MAX\n");
		break;
	case HSA_EXT_AGENT_INFO_IMAGE_RDWR_MAX:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_IMAGE_RDWR_MAX\n");
		break;
	case HSA_EXT_AGENT_INFO_SAMPLER_MAX:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_SAMPLER_MAX\n");
		break;
	case HSA_AGENT_INFO_COUNT:
		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_COUNT\n");
		break;
	default:
		setArgumentValue<unsigned int>(
				HSA_STATUS_ERROR_INVALID_ARGUMENT, 
				0, memory, args_ptr);
		return 0;
		break;
	}
	setArgumentValue<unsigned int>(
			HSA_STATUS_ERROR_INVALID_ARGUMENT, 
			0, memory, args_ptr);
	return 0;
}


int Driver::CallQueueCreate(mem::Memory *memory, unsigned args_ptr)
{
	// Name			| Offset		| Size
	// ---------------------------------------------------
	// status		| 0			| 4
	// agent		| 4			| 8
	// size			| 12			| 4
	// type			| 16			| 4
	// callback		| 20			| 8
	// service_queue	| 28			| 8
	// queue		| 36			| 8

	// Declare arguments as variables
	unsigned long long agent = getArgumentValue<unsigned long long>
			(4, memory, args_ptr);
	unsigned int size = getArgumentValue<unsigned int>
			(12, memory, args_ptr);
	unsigned int type = getArgumentValue<unsigned int>
			(16, memory, args_ptr);
	unsigned long long callback = getArgumentValue<unsigned long long>
			(20, memory, args_ptr);
	unsigned long long service_queue = getArgumentValue<unsigned long long>
			(28, memory, args_ptr);
	unsigned long long queue = getArgumentValue<unsigned long long>
			(36, memory, args_ptr);
	unsigned int host_lang = getArgumentValue<unsigned int>
			(44, memory, args_ptr);
	unsigned long long work_item = getArgumentValue<unsigned long long>
			(48, memory, args_ptr);
	
	// Dump argument for debug purpose
	debug << misc::fmt("\tStatus: %d, \n", getArgumentValue
			<unsigned int>(0, memory, args_ptr));
	debug << misc::fmt("\tAgent: %lld, \n", agent);
	debug << misc::fmt("\tSize: %d, \n", size);
	debug << misc::fmt("\tType: %d, \n", type);
	debug << misc::fmt("\tCallback: 0x%016llx, \n", callback);
	debug << misc::fmt("\tService queue: 0x%016llx, \n", service_queue);
	debug << misc::fmt("\tQueue: 0x%016llx, \n", queue);
	debug << misc::fmt("\tHost language: %d, \n", host_lang);
	debug << misc::fmt("\tWork item address: 0x%016llx\n", work_item);

	// No support for callback and service queue yet
	if (callback != 0)
		throw misc::Panic("Call back in runtime function create \
				queue is not supported");
	if (service_queue != 0)
		throw misc::Panic("Service queue in runtime function create \
				queue is not supported");

	// Retrieve component
	Component *component = Emu::getInstance()->getComponent(agent);
	if (component == nullptr)
	{
		setArgumentValue<unsigned int>(HSA_STATUS_ERROR_INVALID_AGENT,
				0, memory, args_ptr);
	}

	// Init queue
	auto new_queue = misc::new_unique<AQLQueue>(size, type);

	// Set the address to the queue struct
	unsigned long long *queue_addr =
			(unsigned long long *)memory->getBuffer(queue, 16,
					mem::Memory::AccessWrite);
	*queue_addr = new_queue->getFieldsAddress();

	// Move queue to the component
	component->addQueue(std::move(new_queue));

	return 0;
}


int Driver::CallQueueDestroy(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueInactivate(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueLoadReadIndexAcquire(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// value		| 0		| 8
	// queue		| 8		| 8

	// Get arguments
	unsigned long long queue_ptr = getArgumentValue<unsigned long long>
			(8, memory, args_ptr);

	// Dump debug information
	debug << misc::fmt("\tqueue: 0x%016llx, \n", queue_ptr);

	// Retrieve the read index
	unsigned long long *read_index = (unsigned long long *)memory->
			getBuffer(queue_ptr + 48, 8, mem::Memory::AccessRead);

	// Write read index back to argument
	this->setArgumentValue<unsigned long long>(*read_index, 0, memory,
			args_ptr);

	return 0;
}


int Driver::CallQueueLoadReadIndexRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// value		| 0		| 8
	// queue		| 8		| 8

	// Get arguments
	unsigned long long queue_ptr = getArgumentValue<unsigned long long>
			(8, memory, args_ptr);

	// Dump debug information
	debug << misc::fmt("\tqueue: 0x%016llx, \n", queue_ptr);

	// Retrieve the read index
	unsigned long long *read_index = (unsigned long long *)memory->
			getBuffer(queue_ptr + 48, 8, mem::Memory::AccessRead);

	// Write read index back to argument
	this->setArgumentValue<unsigned long long>(*read_index, 0, memory,
			args_ptr);

	return 0;
}


int Driver::CallQueueLoadWriteIndexAcquire(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// value		| 0		| 8
	// queue		| 8		| 8

	// Get arguments
	unsigned long long queue_ptr = getArgumentValue<unsigned long long>
			(8, memory, args_ptr);

	// Dump debug information
	debug << misc::fmt("\tqueue: 0x%016llx, \n", queue_ptr);

	// Retrieve the read index
	unsigned long long *write_index = (unsigned long long *)memory->
			getBuffer(queue_ptr + 40, 8, mem::Memory::AccessRead);

	// Write read index back to argument
	this->setArgumentValue<unsigned long long>(*write_index, 0, memory,
			args_ptr);

	return 0;
}


int Driver::CallQueueLoadWriteIndexRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// value		| 0		| 8
	// queue		| 8		| 8

	// Get arguments
	unsigned long long queue_ptr = getArgumentValue<unsigned long long>
			(8, memory, args_ptr);

	// Dump debug information
	debug << misc::fmt("\tqueue: 0x%016llx, \n", queue_ptr);

	// Retrieve the read index
	unsigned long long *write_index = (unsigned long long *)memory->
			getBuffer(queue_ptr + 40, 8, mem::Memory::AccessRead);

	// Write read index back to argument
	this->setArgumentValue<unsigned long long>(*write_index, 0, memory,
			args_ptr);

	return 0;
}


int Driver::CallQueueStoreWriteIndexRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueStoreWriteIndexReleased(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueCasWriteIndexAcqRel(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueCasWriteIndexAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueCasWriteIndexRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueCasWriteIndexReleased(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueAddWriteIndexAcqRel(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// prev_value		| 0		| 8
	// queue		| 8		| 8
	// value		| 16		| 8

	// Get arguments
	unsigned long long queue_ptr = getArgumentValue<unsigned long long>
			(8, memory, args_ptr);
	unsigned long long value = getArgumentValue<unsigned long long>
				(16, memory, args_ptr);

	// Dump debug information
	debug << misc::fmt("\tqueue: 0x%016llx, \n", queue_ptr);
	debug << misc::fmt("\tvalue: %lld, \n", value);

	// Retrieve the read index
	unsigned long long *write_index = (unsigned long long *)memory->
			getBuffer(queue_ptr + 40, 8, mem::Memory::AccessWrite);

	// Write write index back to argument
	this->setArgumentValue<unsigned long long>(*write_index, 0, memory,
			args_ptr);

	// Update the write index
	*write_index = *write_index + value;

	return 0;
}


int Driver::CallQueueAddWriteIndexAcquire(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// prev_value		| 0		| 8
	// queue		| 8		| 8
	// value		| 16		| 8

	// Get arguments
	unsigned long long queue_ptr = getArgumentValue<unsigned long long>
			(8, memory, args_ptr);
	unsigned long long value = getArgumentValue<unsigned long long>
				(16, memory, args_ptr);

	// Dump debug information
	debug << misc::fmt("\tqueue: 0x%016llx, \n", queue_ptr);
	debug << misc::fmt("\tvalue: %lld, \n", value);

	// Retrieve the read index
	unsigned long long *write_index = (unsigned long long *)memory->
			getBuffer(queue_ptr + 40, 8, mem::Memory::AccessWrite);

	// Write write index back to argument
	this->setArgumentValue<unsigned long long>(*write_index, 0, memory,
			args_ptr);

	// Update the write index
	*write_index = *write_index + value;

	return 0;
}


int Driver::CallQueueAddWriteIndexRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// prev_value		| 0		| 8
	// queue		| 8		| 8
	// value		| 16		| 8

	// Get arguments
	unsigned long long queue_ptr = getArgumentValue<unsigned long long>
			(8, memory, args_ptr);
	unsigned long long value = getArgumentValue<unsigned long long>
				(16, memory, args_ptr);

	// Dump debug information
	debug << misc::fmt("\tqueue: 0x%016llx, \n", queue_ptr);
	debug << misc::fmt("\tvalue: %lld, \n", value);

	// Retrieve the read index
	unsigned long long *write_index = (unsigned long long *)memory->
			getBuffer(queue_ptr + 40, 8, mem::Memory::AccessWrite);

	// Write write index back to argument
	this->setArgumentValue<unsigned long long>(*write_index, 0, memory,
			args_ptr);

	// Update the write index
	*write_index = *write_index + value;

	return 0;
	return 0;
}


int Driver::CallQueueAddWriteIndexRelease(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// prev_value		| 0		| 8
	// queue		| 8		| 8
	// value		| 16		| 8

	// Get arguments
	unsigned long long queue_ptr = getArgumentValue<unsigned long long>
			(8, memory, args_ptr);
	unsigned long long value = getArgumentValue<unsigned long long>
				(16, memory, args_ptr);

	// Dump debug information
	debug << misc::fmt("\tqueue: 0x%016llx, \n", queue_ptr);
	debug << misc::fmt("\tvalue: %lld, \n", value);

	// Retrieve the read index
	unsigned long long *write_index = (unsigned long long *)memory->
			getBuffer(queue_ptr + 40, 8, mem::Memory::AccessWrite);

	// Write write index back to argument
	this->setArgumentValue<unsigned long long>(*write_index, 0, memory,
			args_ptr);

	// Update the write index
	*write_index = *write_index + value;

	return 0;
	return 0;
}


int Driver::CallQueueStoreReadIndexRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueStoreReadIndexRelease(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallAgentIterateRegions(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallRegionGetInfo(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallMemoryRegister(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallMemoryDeregister(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallMemoryAllocate(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallMemoryFree(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalCreate(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalDestory(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalLoadRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalLoadAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalStoreRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalStoreRelease(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalWaitRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalWaitAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAndRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAndAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAndRelease(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAndAcqRel(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalOrRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalOrAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalOrRelease(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalOrAcqRel(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalXorRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalXorAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalXorRelease(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalXorAcqRel(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAddRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAddAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAddRelease(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAddAcqRel(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalSubtractRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalSubtractAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalSubtractRelease(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalSubtractAcqRel(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalExchangeRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalExchangeAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalExchangeRelease(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalExchangeAcqRel(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalCasRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalCasAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalCasRelease(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalCasAcqRel(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallStatusString(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallPrintU32(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// integer		| 0		| 4);

	unsigned int integer = getArgumentValue<unsigned int>
			(0, memory, args_ptr);
	std::cout << integer;

	return 0;
}


int Driver::CallPrintU64(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// integer		| 0		| 8

	unsigned int integer = getArgumentValue<unsigned int>
			(0, memory, args_ptr);
	std::cout << integer;

	return 0;
}

}

