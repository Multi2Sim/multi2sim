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
	throw misc::Panic(misc::fmt("Unimplemented driver function %s", \
	__FUNCTION__));

#include <arch/hsa/asm/BrigDef.h>
#include <arch/hsa/emu/Emu.h>
#include <arch/hsa/emu/Component.h>

#include "DriverCallbackInfo.h"
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


int IterateAgentNext(DriverCallbackInfo *args)
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

	/*	
	std::cout << misc::fmt("In function %s ", __FUNCTION__);
	std::cout << misc::fmt("last_handler: %lld, ", last_handler);
	std::cout << "ret: "<< 
		Driver::getInstance()->
		getArgumentValue<unsigned int>(0, memory, args_ptr);
	std::cout << ", callback: " << 
		Driver::getInstance()->
		getArgumentValue<unsigned int>(4, memory, args_ptr);
	std::cout << ", data: " <<
		Driver::getInstance()->
		getArgumentValue<unsigned>(8, memory, args_ptr);
	std::cout << ", host_lang: " <<
		Driver::getInstance()->
		getArgumentValue<unsigned int>(12, memory, args_ptr);
	std::cout << ", workitem_ptr: " << 
		Driver::getInstance()->
		getArgumentValue<unsigned long long>(16, memory, args_ptr)
		<< "\n";
	*/

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
	unsigned callback_address = driver->getArgumentValue<unsigned>
			(4, memory,args_ptr);
	unsigned long long data_address = driver->getArgumentValue
			<unsigned long long>(8, memory, args_ptr);
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
	debug << ", \n\tdata: " <<
		getArgumentValue<unsigned long long>(12, memory, args_ptr);
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
	BrigDirectiveFunction *function_directory =
			(BrigDirectiveFunction *)
			(unsigned long long)callback_address;
	std::string callback_name = BrigStrEntry::GetStringByOffset(binary,
			function_directory->name);

	// Create new stack frame
	Function *callback = ProgramLoader::getInstance()->getFunction(
			callback_name);
	StackFrame *stack_frame = new StackFrame(
			callback, work_item);

	// Pass argument into stack frame
	VariableScope *function_args = stack_frame->getFunctionArguments();

	// Declare return argument
	BrigDirEntry function_dir_entry((char *)function_directory, binary);
	BrigDirectiveSymbol *out_arg_directory =
			(BrigDirectiveSymbol *)function_dir_entry.next();
	std::string arg_name = BrigStrEntry::GetStringByOffset(binary,
			out_arg_directory->name);
	function_args->DeclearVariable(arg_name, 4, out_arg_directory->type);

	// Pass argument 1 (Address handler) to the callback
	BrigDirectiveSymbol *arg1 = (BrigDirectiveSymbol *)
			BrigDirEntry::GetDirByOffset(binary,
					function_directory->firstInArg);
	BrigDirEntry arg1_entry((char *)arg1, binary);
	std::string arg1_name = BrigStrEntry::GetStringByOffset(
			binary, arg1->name);
	function_args->DeclearVariable(arg1_name, 8, BRIG_TYPE_U64);
	unsigned long long *callee_buffer = 
			(unsigned long long *)function_args->
			getBuffer(arg1_name);
	*callee_buffer = componentHandler;

	// Pass argument 2 (Address to the data field) to the callback
	BrigDirectiveSymbol *arg2 = (BrigDirectiveSymbol *)arg1_entry.next();
	std::string arg2_name = BrigStrEntry::GetStringByOffset(
			binary, arg2->name);
	function_args->DeclearVariable(arg2_name, 16, BRIG_TYPE_U64);
	callee_buffer = (unsigned long long *)
			function_args->getBuffer(arg2_name);
	*callee_buffer = data_address;

	// Setup info for return callback function
	mem::Memory *memory = Emu::getInstance()->getMemory();
	std::unique_ptr<AgentIterateNextInfo> callback_info(
			new AgentIterateNextInfo(work_item, memory,
					args_ptr, componentHandler));

	// Set the stack frame to be an agent_iterate_callback
	stack_frame->setReturnCallback(&IterateAgentNext,
			std::move(callback_info));
	
	// Add stack frame to the work item;
	work_item->PushStackFrame(stack_frame);
}


int Driver::CallAgentGetInfo(mem::Memory *memory, unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// agent		| 4		| 8
	// attribute		| 12		| 4
	// value		| 16		| 4
	// host_language	| 20		| 4
	// workitem_ptr		| 24		| 8

	// Retrieve argument buffer
	char *arg_buffer = memory->getBuffer(args_ptr, 16,
			mem::Memory::AccessRead);

	// Retrieve agent handler
	unsigned long long agent_handler = 
			getArgumentValue<unsigned long long>(4, memory, 
					args_ptr);

	// Try to find the agent
	Component *component = Emu::getInstance()->getComponent(agent_handler);
	if (component == nullptr)
	{
		unsigned int *return_val = (unsigned int *)arg_buffer;
		*return_val = HSA_STATUS_ERROR_INVALID_AGENT;
		return 0;
	}

	// If the device is found, get the attribute queried
	unsigned int attribute = getArgumentValue<unsigned int>(12, memory, 
			args_ptr);

	// Retrieve the pointer to the value
	unsigned value_address = getArgumentValue<unsigned int>(16, memory, 
			args_ptr);
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
		if (component->IsGPU())
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
		unsigned int *return_val = (unsigned int *)arg_buffer;
		*return_val = HSA_STATUS_ERROR_INVALID_ARGUMENT;
		return 0;
		break;
	}

	unsigned int *return_val = (unsigned int *)arg_buffer;
	*return_val = HSA_STATUS_SUCCESS;
	return 0;
}


int Driver::CallQueueCreate(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
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
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueLoadReadIndexRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueLoadWriteIndexAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueLoadWriteIndexRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
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
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueAddWriteIndexAcquire(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueAddWriteIndexRelaxed(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueAddWriteIndexRelease(mem::Memory *memory, unsigned args_ptr)
{
	__UNIMPLEMENTED__
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
int AgentIterateNext();

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

}

