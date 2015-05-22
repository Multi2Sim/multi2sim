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

#include <arch/x86/emu/Context.h>
#include <arch/x86/emu/Emu.h>
#include <arch/hsa/emu/Emu.h>
#include <arch/hsa/emu/Component.h>
#include <arch/hsa/emu/WorkItem.h>
#include <arch/hsa/emu/StackFrame.h>
#include <arch/hsa/emu/ProgramLoader.h>

#include "HsaProgram.h"
#include "HsaExecutable.h"
#include "HsaExecutableSymbol.h"
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
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// attribute		| 4		| 4
	// data			| 8		| 12

	unsigned int attribute = getArgumentValue<unsigned int>(4, memory,
			args_ptr);
	unsigned int data_ptr = getArgumentValue<unsigned int>(8, memory,
			args_ptr);

	switch (attribute)
	{
	case HSA_SYSTEM_INFO_VERSION_MAJOR:

	{
		unsigned *data = (unsigned *)memory->getBuffer(
				data_ptr, 4, mem::Memory::AccessWrite);
		*data = 0;
		break;
	}

	case HSA_SYSTEM_INFO_VERSION_MINOR:

	{
		unsigned *data = (unsigned *)memory->getBuffer(
				data_ptr, 4, mem::Memory::AccessWrite);
		*data = 99;
		break;
	}

	case HSA_SYSTEM_INFO_TIMESTAMP:

	{
		unsigned long long *data = (unsigned long long *)
				memory->getBuffer(data_ptr, 4,
						mem::Memory::AccessWrite);
		*data = time(NULL);
		break;
	}

	case HSA_SYSTEM_INFO_TIMESTAMP_FREQUENCY:

	{
		unsigned long long *data = (unsigned long long *)
				memory->getBuffer(data_ptr, 4,
						mem::Memory::AccessWrite);
		*data = 1;
		break;
	}

	case HSA_SYSTEM_INFO_SIGNAL_MAX_WAIT:

	{
		unsigned long long *data = (unsigned long long *)
				memory->getBuffer(data_ptr, 4,
						mem::Memory::AccessWrite);
		*data = 100;
		break;
	}

	default:

		// Return error
		setArgumentValue<unsigned int>(
				HSA_STATUS_ERROR_INVALID_ARGUMENT, 0,
				memory, args_ptr);
		return 0;
	}

	// Return success
	setArgumentValue<unsigned int>(
				HSA_STATUS_SUCCESS, 0,
				memory, args_ptr);

	return 0;
}

/*
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

		// Return 0 to tell the function finished its execution
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
*/


int Driver::CallIterateAgents(mem::Memory *memory, unsigned args_ptr)
{	
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// callback		| 4		| 8
	// data			| 12		| 8

	// Dump the argument information
	debug << misc::fmt("In function %s", __FUNCTION__);
	debug << "\n\thsa_status_t: "<< 
		getArgumentValue<unsigned int>(0, memory, args_ptr);
	debug << ", \n\tcallback: " << 
		getArgumentValue<unsigned long long>(4, memory, args_ptr);
	debug << misc::fmt(", \n\tdata: 0x%llx", getArgumentValue
			<unsigned long long>(12, memory, args_ptr));

	// Get virtual machine setup
	Emu *emu = Emu::getInstance();
	Component *component = emu->getNextComponent(0);

	// No component to iterate anymore
	if (!component){
		// Set return argument to HSA_STATUS_SUCCESS
		setArgumentValue<unsigned int>(HSA_STATUS_SUCCESS, 0, 
				memory, args_ptr);

		// Return 0 to tell the function finished its execution
		return 0;
	}
/*
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
			*/

	return 1;
}

/*
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
*/


int Driver::CallAgentGetInfo(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// agent		| 4		| 8
	// attribute		| 12		| 4
	// value		| 16		| 8

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

	{
		std::string name =component->getName();
		strcpy(value_ptr, name.c_str());
		break;
	}

	case HSA_AGENT_INFO_VENDOR_NAME:

	{
		std::string vendor_name =component->getVendorName();
		strcpy(value_ptr, vendor_name.c_str());
		break;
	}

	case HSA_AGENT_INFO_FEATURE:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_FEATURE\n");
		break;

	case HSA_AGENT_INFO_MACHINE_MODEL:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_FEATURE\n");
		break;

	case HSA_AGENT_INFO_PROFILE:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_PROFILE\n");
		break;

	case HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE\n");
		break;

	case HSA_AGENT_INFO_BASE_PROFILE_DEFAULT_FLOAT_ROUNDING_MODES:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_BASE_PROFILE_DEFAULT_FLOAT_ROUNDING_MODES\n");
		break;

	case HSA_AGENT_INFO_FAST_F16_OPERATION:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_FAST_F16_OPERATION\n");
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

	case HSA_AGENT_INFO_QUEUE_MIN_SIZE:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_QUEUE_MIN_SIZE\n");
		break;

	case HSA_AGENT_INFO_QUEUE_MAX_SIZE:

		*(unsigned int *)value_ptr = 16384;
		break;

	case HSA_AGENT_INFO_QUEUE_TYPE:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_QUEUE_TYPE\n");
		break;

	case HSA_AGENT_INFO_NODE:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_NODE\n");
		break;

	case HSA_AGENT_INFO_DEVICE:

	{
		unsigned int type = component->getDeivceType();
		memcpy(value_ptr, &type, 4);
		break;
	}

	case HSA_AGENT_INFO_CACHE_SIZE:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_CACHE_SIZE\n");
		break;

	case HSA_AGENT_INFO_ISA:

	{
		void *isa = nullptr;
		memcpy(value_ptr, &isa, 8);
		break;
	}

	case HSA_AGENT_INFO_EXTENSIONS:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_EXTENSIONS\n");
		break;

	case HSA_AGENT_INFO_VERSION_MAJOR:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_VERSION_MAJOR\n");
		break;

	case HSA_AGENT_INFO_VERSION_MINOR:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_VERSION_MINOR\n");
		break;

	default:
		setArgumentValue<unsigned int>(
				HSA_STATUS_ERROR_INVALID_ARGUMENT, 
				0, memory, args_ptr);
		return 0;
		break;
	}
	setArgumentValue<unsigned int>(
			HSA_STATUS_SUCCESS, 
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
	// data			| 28			| 8
	// private_segment_size | 36			| 4
	// group_segment_size   | 40			| 4
	// queue		| 44			| 8
	// host_lang		| 52			| 4
	// work_item_ptr	| 56			| 8

	// Declare arguments as variables
	unsigned long long agent = getArgumentValue<unsigned long long>
			(4, memory, args_ptr);
	unsigned int size = getArgumentValue<unsigned int>
			(12, memory, args_ptr);
	unsigned int type = getArgumentValue<unsigned int>
			(16, memory, args_ptr);
	unsigned long long callback = getArgumentValue<unsigned long long>
			(20, memory, args_ptr);
	unsigned long long data = getArgumentValue<unsigned long long>
			(28, memory, args_ptr);
	unsigned long long queue = getArgumentValue<unsigned long long>
			(44, memory, args_ptr);
	unsigned int host_lang = getArgumentValue<unsigned int>
			(52, memory, args_ptr);
	unsigned long long work_item = getArgumentValue<unsigned long long>
			(56, memory, args_ptr);
	
	// Dump argument for debug purpose
	debug << misc::fmt("\tStatus: %d, \n", getArgumentValue
			<unsigned int>(0, memory, args_ptr));
	debug << misc::fmt("\tAgent: %lld, \n", agent);
	debug << misc::fmt("\tSize: %d, \n", size);
	debug << misc::fmt("\tType: %d, \n", type);
	debug << misc::fmt("\tCallback: 0x%016llx, \n", callback);
	debug << misc::fmt("\tData: 0x%016llx, \n", data);
	debug << misc::fmt("\tQueue: 0x%016llx, \n", queue);
	debug << misc::fmt("\tHost language: %d, \n", host_lang);
	debug << misc::fmt("\tWork item address: 0x%016llx\n", work_item);

	// No support for callback and service queue yet
	if (callback != 0)
		throw misc::Panic("Call back in runtime function create \
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
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// region		| 4		| 8
	// attribute		| 12 		| 4
	// data 		| 16 		| 8
	unsigned long long region = getArgumentValue<unsigned long long>(
			4, memory, args_ptr);
	unsigned int attribute = getArgumentValue<unsigned int>(
			12, memory, args_ptr);
	unsigned long long data = getArgumentValue<unsigned long long>(
			16, memory, args_ptr);

	// Only recognize region 1
	if (region != 1)
		return 0;

	//
	switch(attribute)
	{
	case HSA_REGION_INFO_SEGMENT:

	{
		hsa_region_segment_t segment = HSA_REGION_SEGMENT_GLOBAL;
		memory->Write(data, 4, (char *)&segment);
		break;
	}

	case HSA_REGION_INFO_GLOBAL_FLAGS:

	{
		unsigned int flag = 0;
		flag |= HSA_REGION_GLOBAL_FLAG_KERNARG;
		flag |= HSA_REGION_GLOBAL_FLAG_FINE_GRAINED;
		memory->Write(data, 4, (char *)&flag);
		break;
	}

	default:

		throw misc::Panic(misc::fmt("Unsupported region info attribute "
				"%d", attribute));

	}

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
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// initial_value	| 4		| 8
	// num_consumers	| 12		| 4
	// consumers		| 16		| 8
	// signal		| 24 		| 8
	hsa_status_t status = HSA_STATUS_SUCCESS;
	long long initial_value = getArgumentValue<long long>(4, memory,
			args_ptr);
	unsigned long long signal = getArgumentValue<unsigned long long>(24,
			memory, args_ptr);

	// Create signal
	long long *new_signal = new long long;

	// Assign init value
	*new_signal = initial_value;

	// Write back
	memory->Write(signal, 8, (char *)&new_signal);
	memory->Write(args_ptr, 4, (char *)&status);

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


int Driver::CallProgramCreate(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// machine_model	| 4		| 4
	// profile		| 8		| 4
	// default_float_rounding_mode | 12 	| 4
	// options		| 16 		| 8
	// program		| 24 		| 8

	unsigned int machine_model = getArgumentValue<unsigned int>
			(4, memory, args_ptr);
	unsigned int profile = getArgumentValue<unsigned int>
			(8, memory, args_ptr);
	unsigned int rounding = getArgumentValue<unsigned int>
			(12, memory, args_ptr);
	unsigned long long options = getArgumentValue<unsigned long long>
			(16, memory, args_ptr);
	unsigned long long program = getArgumentValue<unsigned long long>
			(24, memory, args_ptr);

	debug << misc::fmt("machine_model: %d,\n", machine_model);
	debug << misc::fmt("profile: %d,\n", profile);
	debug << misc::fmt("rounding: %d,\n", rounding);
	debug << misc::fmt("options: 0x%016llx,\n", options);
	debug << misc::fmt("program: 0x%016llx,\n", program);

	// Create new program and write the address to the handle
	HsaProgram *new_program = new HsaProgram();
	memory->Write(program, 8, (char *)&new_program);

	// Return success
	setArgumentValue<unsigned int>(
				HSA_STATUS_SUCCESS, 0,
				memory, args_ptr);

	return 0;
}


int Driver::CallProgramAddModule(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// program		| 4		| 8
	// module		| 12		| 8
	unsigned long long program = getArgumentValue<unsigned long long>
			(4, memory, args_ptr);
	unsigned long long module = getArgumentValue<unsigned long long>
				(12, memory, args_ptr);

	// Print debug information
	debug << misc::fmt("program: 0x%016llx,\n", program);
	debug << misc::fmt("module: 0x%016llx,\n", module);

	// Get module header
	BrigModuleHeader header;
	memory->Read(module, sizeof(BrigModuleHeader), (char *)&header);
	unsigned long long module_size = header.byteCount;

	// Add module to program
	auto module_buffer = misc::new_unique_array<char>(module_size);
	memory->Read((unsigned)module, module_size, module_buffer.get());

	((HsaProgram *)program)->AddModule(module_buffer.get());

	// Return success
	setArgumentValue<unsigned int>(
				HSA_STATUS_SUCCESS, 0,
				memory, args_ptr);
	return 0;
}

int Driver::CallProgramFinalize(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// program		| 4		| 8
	// isa			| 12		| 8
	// call_convention	| 20		| 4
	// control_directives	| 24 		| 144
	// options		| 168		| 8
	// code_object_type	| 176		| 4
	// code_object 		| 180		| 8

	// Retrieve data
	unsigned long long program = getArgumentValue<unsigned long long>
			(4, memory, args_ptr);
	unsigned long long code_object = getArgumentValue<unsigned long long>
			(180, memory, args_ptr);

	// Print debug information
	debug << misc::fmt("program: 0x%016llx,\n", program);
	debug << misc::fmt("code_object: 0x%016llx,\n", code_object);

	// Create an new code object from the program
	HsaProgram *new_code_object = new HsaProgram(*(HsaProgram *)program);

	// Write back
	memory->Write(code_object, 8, (char *)&new_code_object);

	// Return success
	setArgumentValue<unsigned int>(
				HSA_STATUS_SUCCESS, 0,
				memory, args_ptr);

	return 0;
}


int Driver::CallProgramDestroy(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}


int Driver::CallExecutableCreate(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments 		| Offset	| Size
	// hsa_status_t		| 0 		| 4
	// profile		| 4		| 8
	// executable_stat	| 12		| 4
	// options		| 16		| 8
	// executable 		| 24		| 8

	// Retrieve data
	unsigned long long executable = getArgumentValue<unsigned long long>
			(24, memory, args_ptr);

	// Print debug information
	debug << misc::fmt("executable: 0x%016llx, \n", executable);

	// Create executable
	HsaExecutable *new_executable = new HsaExecutable();
	
	// Write back
	memory->Write(executable, 8, (char *)&new_executable);

	// Return success
	setArgumentValue<unsigned int>(
				HSA_STATUS_SUCCESS, 0,
				memory, args_ptr);

	return 0;
}


int Driver::CallExecutableLoadCodeObject(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments 		| Offset	| Size
	// hsa_status_t		| 0 		| 4
	// executable		| 4		| 8
	// agent		| 12		| 8
	// code_object		| 20		| 8
	// options 		| 28		| 8

	// Retrieve data
	unsigned long long executable = getArgumentValue<unsigned long long>
			(4, memory, args_ptr);
	unsigned long long code_object = getArgumentValue<unsigned long long>
			(20, memory, args_ptr);

	// Print debug information
	debug << misc::fmt("executable: 0x%016llx, \n", executable);
	debug << misc::fmt("code_object: 0x%016llx, \n", code_object);

	// Create executable
	((HsaExecutable *)executable)->LoadCodeObject((HsaCodeObject *)code_object);

	// Return success
	setArgumentValue<unsigned int>(
				HSA_STATUS_SUCCESS, 0,
				memory, args_ptr);

	return 0;
}


int Driver::CallExecutableGetSymbol(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments 		| Offset	| Size
	// hsa_status_t		| 0 		| 4
	// executable		| 4		| 8
	// module_name		| 12		| 8
	// symbol_name		| 20		| 8
	// agent 		| 28		| 8
	// call_convention	| 36		| 4
	// symbol 		| 40		| 8

	// Retrieve data
	unsigned long long executable = getArgumentValue<unsigned long long>
			(4, memory, args_ptr);
	unsigned long long symbol_name = getArgumentValue<unsigned long long>
			(20, memory, args_ptr);
	unsigned long long symbol = getArgumentValue<unsigned long long>
			(40, memory, args_ptr);

	// Print debug information
	debug << misc::fmt("executable: 0x%016llx, \n", executable);
	debug << misc::fmt("symbol_name: 0x%016llx, \n", symbol_name);
	debug << misc::fmt("symbol: 0x%016llx, \n", symbol);

	// Get the symbol object
	HsaExecutable *exe = (HsaExecutable *)executable;
	std::string symbol_name_str = memory->ReadString(symbol_name);
	HsaExecutableSymbol *exe_sym = exe->getSymbol(symbol_name_str.c_str());

	// Write back
	memory->Write(symbol, 8, (char *)&exe_sym);

	// Return success
	setArgumentValue<unsigned int>(
				HSA_STATUS_SUCCESS, 0,
				memory, args_ptr);

	return 0;
}


int Driver::CallExecutableSymbolGetInfo(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments 		| Offset	| Size
	// hsa_status_t		| 0 		| 4
	// executable_symbol	| 4		| 8
	// attribute		| 12		| 4
	// value		| 16		| 8

	// Retrieve data
	HsaExecutableSymbol *executable_symbol =
			(HsaExecutableSymbol *)getArgumentValue
			<unsigned long long>(4, memory, args_ptr);
	unsigned int attribute = getArgumentValue<unsigned int>
			(12, memory, args_ptr);
	unsigned long long value = getArgumentValue<unsigned long long>
			(16, memory, args_ptr);

	// Print debug information
	debug << misc::fmt("executable_symbol: %p, \n", executable_symbol);
	debug << misc::fmt("attribute: %d, \n", attribute);
	debug << misc::fmt("value: 0x%016llx, \n", value);

	// Dispatch by attribute
	switch (attribute)
	{
	case HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT:

	{
		memory->Write(value, 8, (char *)&executable_symbol);
		break;
	}

	case HSA_CODE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE:

	{
		unsigned int arg_size = executable_symbol
				->getKernelArgumentSize();
		memory->Write(value, 4, (char *)&arg_size);
		break;
	}

	case HSA_CODE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE:

	{
		unsigned int group_size = 0;
		memory->Write(value, 4, (char *)&group_size);
		break;
	}

	case HSA_CODE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE:

	{
		unsigned int private_size = 0;
		memory->Write(value, 4, (char *)&private_size);
		break;
	}

	default:

		throw misc::Panic(misc::fmt("Unsupported kernel object "
				"attribute %d.\n", attribute));
	}

	// Return success
	setArgumentValue<unsigned int>(HSA_STATUS_SUCCESS, 0, memory, args_ptr);

	return 0;
}


int Driver::CallInitFromX86(mem::Memory *memory, unsigned args_ptr)
{
	// This function is designed only to be called from the host.
	// args_ptr is the process id of the context running the host
	// program.
	x86::Emu *x86_emu = x86::Emu::getInstance();
	x86::Context *host_context = x86_emu->getContext(args_ptr);
	Emu::getInstance()->setMemory(host_context->__getMemSharedPtr());

	return 0;
}


int Driver::CallNextAgent(mem::Memory *memory, unsigned args_ptr)
{
	// This function is designed only to be called from the host.
	// Returns the next agent that has a greater agent_id
	// Returns 0 if there is no more agent

	// Arguments		| Offset	| Size
	// next_agent_id	| 0		| 8
	// present_agnet_id	| 8		| 16

	// Set the first next agent id to 0
	int next_agent_id = getArgumentValue<unsigned long long>
		(8, memory, args_ptr);

	// Get next component
	Emu *emu = Emu::getInstance();
	Component *component = emu->getNextComponent(next_agent_id);

	// If no more component,
	if (!component)
	{
		setArgumentValue<unsigned long long>(0, 0, memory, args_ptr);
	}
	else
	{
		// If component exist return the handler
		setArgumentValue<unsigned long long>(component->getHandler(), 0,
				memory, args_ptr);
	}

	return 0;
}


int Driver::CallNextRegion(mem::Memory *memory, unsigned args_ptr)
{
	// This function is designed only to be called from the host.
	// Returns the next region that has a greater agent_id
	// Returns 0 if there is no more region

	// Arguments		| Offset	| Size
	// next_region_id	| 0		| 8
	// agent_id		| 8		| 16
	// curr_region_id	| 16		| 24

	// Set the first next agent id to 0
	unsigned long long curr_region_id = getArgumentValue<unsigned long long>
			(16, memory, args_ptr);

	// Only return current id
	if (curr_region_id == 0)
		setArgumentValue<unsigned long long>(1, 0, memory, args_ptr);
	else
		setArgumentValue<unsigned long long>(0, 0, memory, args_ptr);

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

	unsigned long long integer = getArgumentValue<unsigned long long>
			(0, memory, args_ptr);
	std::cout << integer;

	return 0;
}


int Driver::CallPrintF32(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// number		| 0		| 8

	float number = getArgumentValue<float>
			(0, memory, args_ptr);
	std::cout << number << "\n";

	return 0;
}


int Driver::CallPrintString(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// str_ptr		| 0		| 8

	unsigned long long addr = getArgumentValue<unsigned long long>
			(0, memory, args_ptr);
	char *string = Emu::getInstance()->getMemory()->getBuffer(addr, 
			2, mem::Memory::AccessWrite);
					
	std::cout << misc::fmt("%s", string);

	return 0;
}

int Driver::CallStringToU32(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// integer		| 0		| 4
	// str_ptr		| 4		| 8

	unsigned long long addr = getArgumentValue<unsigned long long>
			(4, memory, args_ptr);
	char *buf = Emu::getInstance()->getMemory()->getBuffer(addr, 
			2, mem::Memory::AccessWrite);
	std::string str(buf);
	
	unsigned int integer;
	try
	{
		integer = stoi(str);
	}
	catch (int e)
	{
		throw Error("Failed to convert string to integer");
	}
	setArgumentValue<unsigned int>(integer, 0, memory, args_ptr);
	return 0;
}

}

