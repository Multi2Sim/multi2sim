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

#include <arch/x86/emulator/Context.h>
#include <arch/x86/emulator/Emulator.h>
#include <arch/hsa/emulator/Emulator.h>
#include <arch/hsa/emulator/Component.h>
#include <arch/hsa/emulator/WorkItem.h>
#include <arch/hsa/emulator/StackFrame.h>

#include "HsaProgram.h"
#include "HsaExecutable.h"
#include "HsaExecutableSymbol.h"
#include "Driver.h"


namespace HSA
{

int Driver::CallInit(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	debug << misc::fmt("Executing driver function %s.\n", __FUNCTION__);
	debug << misc::fmt("Finished executing driver function %s, "
			"returning %d.\n", __FUNCTION__, 0);

	return 0;
}


int Driver::CallShutDown(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	debug << misc::fmt("Executing driver function %s.\n", __FUNCTION__);
	debug << misc::fmt("Finished executing driver function %s, "
				"returning %d.\n", __FUNCTION__, 0);
	return 0;
}


int Driver::CallSystemGetInfo(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint32_t attribute;
		uint32_t value;
	} data;

	memory->Read(args_ptr, sizeof(data), (char *)&data);

	switch (data.attribute)
	{
	case HSA_SYSTEM_INFO_VERSION_MAJOR:

	{
		uint16_t major = 1;
		memory->Write(data.value, 2, (char *)&major);
		break;
	}

	case HSA_SYSTEM_INFO_VERSION_MINOR:

	{
		uint16_t minor = 0;
		memory->Write(data.value, 2, (char *)&minor);
		break;
	}

	case HSA_SYSTEM_INFO_TIMESTAMP:

	{
		uint64_t timestamp = time(NULL);
		memory->Write(data.value, 8, (char *)&timestamp);
		break;
	}

	default:

		throw misc::Panic(misc::fmt("Unsupported attribute %d in "
				"get_system_info", data.attribute));
		return 0;
	}

	// Return success
	data.status = HSA_STATUS_SUCCESS;
	memory->Write(args_ptr, sizeof(data), (char *)&data);

	return 0;
}


int Driver::CallIterateAgents(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 1;
}


int Driver::CallAgentGetInfo(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint64_t agent;
		uint32_t attribute;
		uint32_t value;
	} data;
	memory->Read(args_ptr, sizeof(data), (char *)&data);

	// Try to find the agent
	Component *component = Emulator::getInstance()->
			getComponent(data.agent);
	if (component == nullptr)
	{
		data.status = HSA_STATUS_ERROR_INVALID_AGENT;
		memory->Write(args_ptr, sizeof(data), (char *)&data);
		return 0;
	}

	switch(data.attribute)
	{
	case HSA_AGENT_INFO_NAME:

	{
		memory->WriteString(data.value, component->getName());
		break;
	}

	case HSA_AGENT_INFO_VENDOR_NAME:

	{
		memory->WriteString(data.value, component->getVendorName());
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

	{
		uint32_t queue_max_size = 16384;
		memory->Write(data.value, 4, (char *)&queue_max_size);
		break;
	}

	case HSA_AGENT_INFO_QUEUE_TYPE:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_QUEUE_TYPE\n");
		break;

	case HSA_AGENT_INFO_NODE:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_NODE\n");
		break;

	case HSA_AGENT_INFO_DEVICE:

	{
		uint32_t type = component->getDeivceType();
		memory->Write(data.value, 4, (char *)&type);
		break;
	}

	case HSA_AGENT_INFO_CACHE_SIZE:

		throw misc::Panic("Unsupported agent_get_info attribute HSA_AGENT_INFO_CACHE_SIZE\n");
		break;

	case HSA_AGENT_INFO_ISA:

	{
		uint64_t isa = 0;
		memory->Write(data.value, 8, (char *)&isa);
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

		data.status = HSA_STATUS_ERROR_INVALID_ARGUMENT;
		break;
	}

	data.status = HSA_STATUS_SUCCESS;
	memory->Write(args_ptr, sizeof(data), (char *)&data);
	return 0;
}


int Driver::CallQueueCreate(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint64_t agent;
		uint32_t size;
		uint32_t type;
		uint32_t callback;
		uint32_t data;
		uint32_t private_segment_size;
		uint32_t group_segment_size;
		uint32_t queue;
	} data;
	memory->Read(args_ptr, sizeof(data), (char *)&data);

	// Retrieve component
	Emulator *emulator = Emulator::getInstance();
	Component *component = emulator->getComponent(data.agent);
	if (component == nullptr)
	{
		data.agent = HSA_STATUS_ERROR_INVALID_AGENT;
		memory->Write(args_ptr, sizeof(data), (char *)&data);
		return 0;
	}

	// Init queue
	uint32_t size = data.size;
	uint32_t type = data.type;
	auto new_queue = misc::new_unique<AQLQueue>(size, type);

	// Create and assign doorbell signal
	uint64_t doorbell_signal = signal_manager->CreateSignal(0);
	new_queue->setBellSignal(doorbell_signal);

	// Set the address to the queue struct
	uint32_t queue_addr = new_queue->getFieldsAddress();
	memory->Write(data.queue, 4, (char *)&queue_addr);

	// Move queue to the component
	component->addQueue(std::move(new_queue));

	// Write the result
	data.status = HSA_STATUS_SUCCESS;
	memory->Write(args_ptr, sizeof(data), (char *)&data);


	return 0;
}


int Driver::CallQueueDestroy(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueInactivate(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueLoadReadIndexAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	return CallQueueLoadReadIndexRelaxed(context, memory, args_ptr);
}


int Driver::CallQueueLoadReadIndexRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__((packed))
	{
		uint64_t value;
		uint32_t queue;
	} data;
	memory->Read(args_ptr, sizeof(data), (char *)&data);

	// Retrieve the read index
	uint64_t read_index;
	memory->Read(data.queue + 48, 8, (char *)&read_index);
	data.value = read_index;

	// Write read index back to argument
	memory->Write(args_ptr, sizeof(data), (char *)&data);

	return 0;
}


int Driver::CallQueueLoadWriteIndexAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	return CallQueueLoadWriteIndexRelaxed(context, memory, args_ptr);
}


int Driver::CallQueueLoadWriteIndexRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__((packed))
	{
		uint64_t value;
		uint32_t queue;
	} data;
	memory->Read(args_ptr, sizeof(data), (char *)&data);

	// Retrieve the read index
	uint64_t read_index;
	memory->Read(data.queue + 40, 8, (char *)&read_index);
	data.value = read_index;

	// Write read index back to argument
	memory->Write(args_ptr, sizeof(data), (char *)&data);

	return 0;
}


int Driver::CallQueueStoreWriteIndexRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// queue		| 0		| 8
	// value		| 8		| 8

	// Get arguments
	unsigned long long queue_ptr = getArgumentValue<unsigned long long>
			(0, memory, args_ptr);
	unsigned long long value = getArgumentValue<unsigned long long>
			(8, memory, args_ptr);

	// Dump debug information
	debug << misc::fmt("\tqueue: 0x%016llx, \n", queue_ptr);
	debug << misc::fmt("\tvalue: %lld, \n", value);

	// Set write index address
	memory->Write(queue_ptr + 40, 8, (char *)&value);

	return 0;
}


int Driver::CallQueueStoreWriteIndexReleased(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueCasWriteIndexAcqRel(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueCasWriteIndexAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueCasWriteIndexRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueCasWriteIndexReleased(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueAddWriteIndexAcqRel(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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


int Driver::CallQueueAddWriteIndexAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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


int Driver::CallQueueAddWriteIndexRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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


int Driver::CallQueueAddWriteIndexRelease(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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


int Driver::CallQueueStoreReadIndexRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallQueueStoreReadIndexRelease(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallAgentIterateRegions(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallRegionGetInfo(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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


int Driver::CallMemoryRegister(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallMemoryDeregister(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallMemoryAllocate(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// Arguments		| Offset	| Size
	// hsa_status_t		| 0		| 4
	// region		| 4		| 8
	// size			| 12		| 8
	// ptr			| 20		| 8

	unsigned long long region = getArgumentValue<unsigned long long>
			(4, memory, args_ptr);
	unsigned long long size = getArgumentValue<unsigned long long>
			(12, memory, args_ptr);
	unsigned long long ptr = getArgumentValue<unsigned long long>
			(20, memory, args_ptr);

	// Only recognize region 1 currently
	if (region != 1)
		return 0;

	// Allocate memory
	mem::Manager *manager = Emulator::getInstance()->getMemoryManager();
	unsigned address = manager->Allocate(size);

	// Write back
	memory->Write(ptr, 4, (char *)&address);

	return 0;
}


int Driver::CallMemoryFree(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalCreate(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__ ((packed))
	{
		uint32_t status;
		int64_t initial_value;
		uint32_t num_consumers;
		uint32_t consumers;
		uint32_t signal;
	} data;

	// Retrieve data
	memory->Read(args_ptr, sizeof(data), (char *)&data);

	// Create signal
	uint64_t signal_handler = signal_manager->CreateSignal(
			data.initial_value);

	// Write back
	data.status = HSA_STATUS_SUCCESS;
	memory->Write(data.signal, 8, (char *)&signal_handler);
	memory->Write(args_ptr, sizeof(data), (char *)&data);

	return 0;
}


int Driver::CallSignalDestroy(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalLoadRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__ ((packed))
	{
		int64_t value;
		uint64_t signal;
	} data;

	// Retrieve data
	memory->Read(args_ptr, sizeof(data), (char *)&data);

	// Set signal value
	data.value = signal_manager->GetValue(data.signal);

	// Return
	memory->Write(args_ptr, sizeof(data), (char *)&data);
	return 0;
}


int Driver::CallSignalLoadAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalStoreRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__ ((packed))
	{
		uint64_t signal;
		int64_t value;
	} data;

	// Retrieve data
	memory->Read(args_ptr, sizeof(data), (char *)&data);

	// Set signal value
	signal_manager->ChangeValue(data.signal, data.value);

	// Return
	return 0;
}


int Driver::CallSignalStoreRelease(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalWaitRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalWaitAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAndRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAndAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAndRelease(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAndAcqRel(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalOrRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalOrAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalOrRelease(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalOrAcqRel(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalXorRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalXorAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalXorRelease(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalXorAcqRel(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAddRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAddAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAddRelease(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalAddAcqRel(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalSubtractRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalSubtractAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalSubtractRelease(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalSubtractAcqRel(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalExchangeRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalExchangeAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalExchangeRelease(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalExchangeAcqRel(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalCasRelaxed(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalCasAcquire(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalCasRelease(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallSignalCasAcqRel(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallStatusString(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	__UNIMPLEMENTED__
	return 0;
}


int Driver::CallProgramCreate(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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


int Driver::CallProgramAddModule(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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

int Driver::CallProgramFinalize(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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


int Driver::CallProgramDestroy(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	return 0;
}


int Driver::CallExecutableCreate(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint32_t profile;
		uint32_t executable_stat;
		uint32_t options;
		uint32_t executable;
	} data;

	// Retrieve data
	memory->Read(args_ptr, sizeof(data), (char *)&data);

	// Create executable
	uint64_t executable_handler = (uint64_t)new HsaExecutable();
	memory->Write(data.executable, 8, (char *)&executable_handler);
	
	// Return status
	data.status = HSA_STATUS_SUCCESS;

	// Write back
	memory->Write(args_ptr, sizeof(data), (char *)&data);

	return 0;
}


int Driver::CallExecutableLoadCodeObject(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint64_t executable;
		uint64_t agent;
		uint64_t code_object;
		uint32_t options;
	} data;

	// Retrieve data
	memory->Read(args_ptr, sizeof(data), (char *)&data);

	// Create executable
	((HsaExecutable *)data.executable)->LoadCodeObject(
			(HsaCodeObject *)data.code_object);

	// Return success
	data.status = HSA_STATUS_SUCCESS;
	memory->Write(args_ptr, sizeof(data), (char *)&data);

	return 0;
}


int Driver::CallExecutableGetSymbol(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint64_t executable;
		uint32_t module_name;
		uint32_t symbol_name;
		uint64_t agent;
		int32_t call_convention;
		uint32_t symbol;
	} data;
	memory->Read(args_ptr, sizeof(data), (char *)&data);

	// Get the symbol object
	HsaExecutable *exe = (HsaExecutable *)data.executable;
	std::string symbol_name_str = memory->ReadString(data.symbol_name);
	HsaExecutableSymbol *exe_sym = exe->getSymbol(symbol_name_str.c_str());

	// Write back
	memory->Write(data.symbol, 8, (char *)&exe_sym);

	// Return success
	data.status = HSA_STATUS_SUCCESS;
	memory->Write(args_ptr, sizeof(data), (char *)&data);

	return 0;
}


int Driver::CallExecutableSymbolGetInfo(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint64_t executable_symbol;
		uint32_t attribute;
		uint32_t value;
	} data;
	memory->Read(args_ptr, sizeof(data), (char *)&data);
	HsaExecutableSymbol *symbol =
			(HsaExecutableSymbol *)data.executable_symbol;

	// Dispatch by attribute
	switch (data.attribute)
	{
	case HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT:

	{
		memory->Write(data.value, 8, (char *)&symbol);
		break;
	}

	case HSA_CODE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE:

	{
		unsigned int arg_size = symbol
				->getKernelArgumentSize();
		memory->Write(data.value, 4, (char *)&arg_size);
		break;
	}

	case HSA_CODE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE:

	{
		unsigned int group_size = 0;
		memory->Write(data.value, 4, (char *)&group_size);
		break;
	}

	case HSA_CODE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE:

	{
		unsigned int private_size = 0;
		memory->Write(data.value, 4, (char *)&private_size);
		break;
	}

	default:

		throw misc::Panic(misc::fmt("Unsupported kernel object "
				"attribute %d.\n", data.attribute));
	}

	// Return success
	data.status = HSA_STATUS_SUCCESS;
	memory->Write(args_ptr, sizeof(data), (char *)&data);

	return 0;
}


int Driver::CallInitFromX86(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	// This function is designed only to be called from the host.
	// args_ptr is the process id of the context running the host
	// program.
	x86::Emulator *x86_emu = x86::Emulator::getInstance();
	x86::Context *host_context = x86_emu->getContext(args_ptr);
	Emulator::getInstance()->setMemory(host_context->getMemory());

	return 0;
}


int Driver::CallNextAgent(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
{
	struct __attribute__ ((packed))
	{
		uint64_t current_agent_id;
		uint64_t next_agent_id;
		bool has_next;
	} agents;
	memory->Read(args_ptr, sizeof(agents), (char *)&agents);

	// Get next component
	Emulator *emulator = Emulator::getInstance();
	Component *component = emulator->getNextComponent(
			agents.current_agent_id);

	// If no more component,
	if (!component)
	{
		agents.has_next = false;
		agents.next_agent_id = 0;
	}
	else
	{
		agents.has_next = true;
		agents.next_agent_id = component->getHandler();
	}

	// Write result back
	memory->Write(args_ptr, sizeof(agents), (char *)&agents);

	return 0;
}


int Driver::CallNextRegion(comm::Context *context,
		mem::Memory *memory,
		unsigned args_ptr)
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

}

