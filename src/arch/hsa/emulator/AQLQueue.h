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

#ifndef ARCH_HSA_EMULATOR_AQLQUEUE_H
#define ARCH_HSA_EMULATOR_AQLQUEUE_H

#include <memory>

#include "AQLPacket.h"


namespace HSA
{

class AQLPacket;
class Component;

class AQLQueue
{
	// A struct that represents information of a aql queue
	struct AqlQueueFields {
		uint32_t queue_type;
		uint32_t queue_features;
		uint64_t base_address;
		uint64_t doorbell_signal;
		uint32_t size;
		uint32_t id;
		uint64_t service_queue;

		// Position where to write next
		uint64_t write_index;

		// Position to read next
		uint64_t read_index;
	};

	// The queue information is stored in a struct, this data structure is
	// stored in guest memeory
	struct AqlQueueFields *fields;

	// The address of the fields in guest memory
	uint32_t fields_address;

	// Device it associated with
	Component *associated_component = nullptr;

	// Convert the linear write/ read index to real position
	uint64_t IndexToAddress(unsigned long long index)
	{
		uint64_t address = fields->base_address +
				(index % fields->size) * sizeof(AQLPacket);
		return address;
	}

	/// Return the packet starts at a certain linear index
	AQLDispatchPacket *getPacket(uint64_t linear_index);

public:

	/// Constructor
	AQLQueue(uint32_t size, uint32_t type);

	/// Destructor
	~AQLQueue();

	/// Associate the queue with HSA component. Raise error if the queue
	/// has already been associated with another device
	void Associate(Component *component);

	/// Determine if the queue is empty
	bool isEmpty() const { return getReadIndex() == getWriteIndex(); }

	/// Allocate an AQL packet slot by incrementing the writeIndex by
	/// size of AQLPacket
	void allocatesPacketSlot() { fields->write_index += sizeof(AQLPacket); }

	/// Read next packet, increase read_index, mark the packet format as
	/// Invalid
	AQLDispatchPacket *ReadPacket();




	//
	// Getters and setters
	//

	/// Set queue type
	void setQueueType(uint32_t queue_type)
	{ 
		fields->queue_type = queue_type;
	}

	/// Get queue type
	uint32_t getQueueType() const
	{
		return fields->queue_type;
	}

	/// Set queue feature
	void setQueueFeature(uint32_t queue_feature)
	{ 
		fields->queue_features = queue_feature;
	}

	/// Get queue feature
	uint32_t getQueueFeature()
	{
		return fields->queue_features;
	}

	/// Get base address
	uint64_t getBaseAddress() const
	{
		return fields->base_address;
	}

	/// Set base address
	void setBaseAddress(uint64_t base_address)
	{
		fields->base_address = base_address;
	}

	/// Get bell signal
	uint64_t getBellSignal() const
	{
		return fields->doorbell_signal;
	}

	/// Set bell signal
	void setBellSignal(uint64_t bell_signal)
	{
		fields->doorbell_signal = bell_signal;
	}

	/// Get queue id
	uint32_t getQueueId() const{ return fields->id; }

	/// Set queue id
	void setQueueId(uint32_t queue_id) { fields->id = queue_id; }

	/// Get read index
	uint64_t getReadIndex() const { return fields->read_index; }

	/// Get service queue
	uint64_t getServiceQueue() const
	{
		return fields->service_queue;
	}

	/// Set service queue
	void setServiceQueue(uint64_t service_queue)
	{
		fields->service_queue = service_queue;
	}

	/// Get size
	uint32_t getSize() const { return fields->size; }

	/// Get write index
	uint64_t getWriteIndex() const { return fields->write_index; }

	/// Get the address of the queue struct in the guest memory
	uint32_t getFieldsAddress() const { return fields_address; }

};

}  // namespace HSA

#endif 

