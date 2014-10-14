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


int IterateAgentNext(mem::Memory *memory, unsigned args_ptr)
{
	std::cout << "In IterateAgentNext\n";
	
	std::cout << misc::fmt("In function %s ", __FUNCTION__);
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
	
	Driver *driver = Driver::getInstance();
	WorkItem *work_item = (WorkItem *)
			driver->getArgumentValue<unsigned long long>(16, 
					memory, args_ptr);
		
	StackFrame *stack_top = work_item->getStackTop();
	driver->ExitInterceptedEnvironment(args_ptr, stack_top);
	
	return 0;
}


int Driver::CallIterateAgents(mem::Memory *memory, unsigned args_ptr)
{	
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// callback		| 4		| 4
	// data			| 8		| 4
	// host_language	| 12		| 4
	// workitem_prt		| 16		| 8

	// Dump the argument information
	
	std::cout << misc::fmt("In function %s ", __FUNCTION__);
	std::cout << "ret: "<< 
		getArgumentValue<unsigned int>(0, memory, args_ptr);
	std::cout << ", callback: " << 
		getArgumentValue<unsigned int>(4, memory, args_ptr);
	std::cout << ", data: " <<
		getArgumentValue<unsigned>(8, memory, args_ptr);
	std::cout << ", host_lang: " <<
		getArgumentValue<unsigned int>(12, memory, args_ptr);
	std::cout << ", workitem_ptr: " << 
		getArgumentValue<unsigned long long>(16, memory, args_ptr)
		<< "\n";
	

	// Retrieve argument buffer
	char *arg_buffer = memory->getBuffer(args_ptr, 24,
			mem::Memory::AccessRead);

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

	// Get call back function name
	BrigFile *binary = ProgramLoader::getInstance()->getBinary();
	WorkItem *work_item = (WorkItem *)getArgumentValue<unsigned long long>(
			16, memory, args_ptr);
	unsigned callback_address = getArgumentValue<unsigned>(4, 
			memory, args_ptr);
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
	char *callee_buffer = function_args->getBuffer(arg1_name);
	if (!callee_buffer)
	{
		throw misc::Panic(misc::fmt("Creating argument %s failed!\n", arg1_name.c_str()));
	}
	unsigned long long *buf = (unsigned long long *)callee_buffer;
	*buf = component->getHandler();

	// Pass argument 2 (Address to the data field) to the callback
	BrigDirectiveSymbol *arg2 = (BrigDirectiveSymbol *)arg1_entry.next();
	std::string arg2_name = BrigStrEntry::GetStringByOffset(
			binary, arg2->name);
	function_args->DeclearVariable(arg2_name, 16, BRIG_TYPE_U32);
	callee_buffer = function_args->getBuffer(arg2_name);
	if (!callee_buffer)
	{
		throw misc::Panic(misc::fmt("Creating argument %s failed!\n", arg2_name.c_str()));
	}
	memcpy(callee_buffer, arg_buffer + 8, 8);

	// Set the stack frame to be an agent_iterate_callback
	stack_frame->setReturnCallback(&IterateAgentNext, args_ptr);

	// Add stack frame to the work item;
	work_item->PushStackFrame(stack_frame);

	return 1;
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
	std::cout << "Agent_handler: " << agent_handler << '\n';

	// Try to find the agent
	Component *component = Emu::getInstance()->getComponent(agent_handler);
	if (component == nullptr)
	{
		unsigned int *return_val = (unsigned int *)arg_buffer;
		*return_val = HSA_STATUS_ERROR_INVALID_AGENT; // HSA_STATUS_ERROR_INVALID_AGENT
		return 0;
	}

	// If the device is found, get the attribute queried
	unsigned int attribute = getArgumentValue<unsigned int>(12, memory, 
			args_ptr);
	std::cout << "Attribute: " << attribute << '\n';

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

