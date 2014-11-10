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

enum QueueType
{
	HSAQueueMulti = 0,
	HSAQueueSingle
};

class AQLQueue
{
	// MULTI or SINGLE
	unsigned int queue_type;

	// Features supported by the queue
	unsigned int queue_feature;

	// Base address to the buffer of packets
	unsigned long long base_address;

	// Bell signal
	unsigned long long bell_signal;

	// Maximum number of packet the queue can hold, must be a power of 2
	unsigned int size;

	// Queue id, unique in this process
	unsigned int queue_id;

	// A pointer to another User Mode Queue that can be used by the HSAIL
	// kernel to request system services. The serviceQueue property is
	// provided by the application to the runtime API call when the queue
	// is created, and may be NULL, the system provided serviceQueue or an
	// application managed queue.
	unsigned long long service_queue;

	// Position where to write next
	unsigned long long write_index;

	// Position to read next
	unsigned long long read_index;

	// Device it associated with
	Component *associated_component = nullptr;

	// The address of this queue in guest memory. If the queue is defined
	// in the guest memory, it should not be simply deleted, but should 
	// be removed from the guest memmory with the Manager::Free() function
	unsigned address_in_guest_memory;

	// Convert the linear write/ read index to real position
	unsigned long long toRecursiveIndex(unsigned long long index)
	{
		unsigned long long recurve_index =
				(index - base_address)
				% (size * sizeof(AQLPacket))
				+ base_address;
		//std::cout << misc::fmt("Mapping linear index %lld to "
		//		"recursive index %lld\n",
		//		index, recurve_index);
		return recurve_index;
	}

	/// Return the packet starts at a certain linear index
	AQLDispatchPacket *getPacket(unsigned long long linear_index);

public:

	/// Constructor
	AQLQueue(unsigned int size, QueueType type);

	/// Destructor
	~AQLQueue();

	/// Enqueue a packet
	void Enqueue(AQLDispatchPacket *packet);

	/// Associate the queue with HSA component. Raise error if the queue
	/// has already been associated with another device
	void Associate(Component *component);

	/// Determine if the queue is empty
	bool isEmpty() const { return read_index == write_index; }

	/// Allocate an AQL packet slot by incrementing the writeIndex by
	/// size of AQLPacket
	void allocatesPacketSlot() { write_index += sizeof(AQLPacket); }

	/// Read next packet, increase read_index, mark the packet format as
	/// Invalid
	AQLDispatchPacket *ReadPacket();

	/// If the queue is a unique_ptr defined on guest memory, this 
	/// function serves as a custom deleter. It avoid freeing the memory 
	/// from the operating system, but it frees the memory from the 
	/// Multi2Sim memory mananger.
	static void Deleter(AQLQueue *queue);




	//
	// Getters and setters
	//

	/// Set queue type
	void setQueueType(unsigned int queue_type)
	{ 
		this->queue_type = queue_type; 
	}

	/// Get queue type
	unsigned int getQueueType() const
	{
		return queue_type; 
	}

	/// Set queue feature
	void setQueueFeature(unsigned int queue_feature)
	{ 
		this->queue_feature = queue_feature; 
	}

	/// Get queue feature
	unsigned int getQueueFeature() 
	{
		return queue_feature;
	}

	/// Get base address
	unsigned long long getBaseAddress() const{ return base_address; }

	/// Set base address
	void setBaseAddress(unsigned long long base_address){ this->base_address = base_address; }

	/// Get bell signal
	unsigned long long getBellSignal() const{ return bell_signal; }

	/// Set bell signal
	void setBellSignal(unsigned long long bell_signal){ this->bell_signal = bell_signal; }

	/// Get queue id
	unsigned int getQueueId() const{ return queue_id; }

	/// Set queue id
	void setQueueId(unsigned int queue_id) { this->queue_id = queue_id; }

	/// Get read index
	unsigned long long getReadIndex() const{ return read_index; }

	/// Set read index
	//void setReadIndex(unsigned long long read_index) { this->read_index = read_index; }

	/// Get service queue
	unsigned long long getServiceQueue() const{ return service_queue; }

	/// Set service queue
	void setServiceQueue(unsigned long long service_queue){ this->service_queue = service_queue; }

	/// Get size
	unsigned int getSize() const{ return size; }

	/// Get write index
	unsigned long long getWriteIndex() const{ return write_index; }

	/// Get the address in guest memory
	void setAddressInGuestMemory(unsigned address)
	{
		address_in_guest_memory = address;
	}

};

}  // namespace HSA

#endif 

