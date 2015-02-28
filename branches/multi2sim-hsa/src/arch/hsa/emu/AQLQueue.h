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

#ifndef ARCH_HSA_EMU_AQLQUEUE_H
#define ARCH_HSA_EMU_AQLQUEUE_H

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
		unsigned int queue_type;
		unsigned int queue_features;
		unsigned long long base_address;
		unsigned long long doorbell_signal;
		unsigned int size;
		unsigned int id;
		unsigned long long service_queue;

		// Position where to write next
		unsigned long long write_index;

		// Position to read next
		unsigned long long read_index;
	};

	// The queue information is stored in a struct, this data structure is
	// stored in guest memeory
	struct AqlQueueFields *fields;

	// The address of the fields in guest memory
	unsigned fields_address;

	// Device it associated with
	Component *associated_component = nullptr;

	// Convert the linear write/ read index to real position
	unsigned long long toRecursiveIndex(unsigned long long index)
	{
		unsigned long long recurve_index =
				(index - fields->base_address)
				% (fields->size * sizeof(AQLPacket))
				+ fields->base_address;
		//std::cout << misc::fmt("Mapping linear index %lld to "
		//		"recursive index %lld\n",
		//		index, recurve_index);
		return recurve_index;
	}

	/// Return the packet starts at a certain linear index
	AQLDispatchPacket *getPacket(unsigned long long linear_index);

public:

	/// Constructor
	AQLQueue(unsigned int size, unsigned int type);

	/// Destructor
	~AQLQueue();

	/// Enqueue a packet
	void Enqueue(AQLDispatchPacket *packet);

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
	void setQueueType(unsigned int queue_type)
	{ 
		fields->queue_type = queue_type;
	}

	/// Get queue type
	unsigned int getQueueType() const
	{
		return fields->queue_type;
	}

	/// Set queue feature
	void setQueueFeature(unsigned int queue_feature)
	{ 
		fields->queue_features = queue_feature;
	}

	/// Get queue feature
	unsigned int getQueueFeature() 
	{
		return fields->queue_features;
	}

	/// Get base address
	unsigned long long getBaseAddress() const
	{
		return fields->base_address;
	}

	/// Set base address
	void setBaseAddress(unsigned long long base_address)
	{
		fields->base_address = base_address;
	}

	/// Get bell signal
	unsigned long long getBellSignal() const
	{
		return fields->doorbell_signal;
	}

	/// Set bell signal
	void setBellSignal(unsigned long long bell_signal)
	{
		fields->doorbell_signal = bell_signal;
	}

	/// Get queue id
	unsigned int getQueueId() const{ return fields->id; }

	/// Set queue id
	void setQueueId(unsigned int queue_id) { fields->id = queue_id; }

	/// Get read index
	unsigned long long getReadIndex() const { return fields->read_index; }

	/// Set read index
	//void setReadIndex(unsigned long long read_index) { this->read_index = read_index; }

	/// Get service queue
	unsigned long long getServiceQueue() const
	{
		return fields->service_queue;
	}

	/// Set service queue
	void setServiceQueue(unsigned long long service_queue)
	{
		fields->service_queue = service_queue;
	}

	/// Get size
	unsigned int getSize() const { return fields->size; }

	/// Get write index
	unsigned long long getWriteIndex() const { return fields->write_index; }

	/// Get the address of the queue struct in the guest memory
	unsigned getFieldsAddress() const { return fields_address; }

};

}  // namespace HSA

#endif 

