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

#include <cmath>

#include "Emulator.h"
#include "AQLPacket.h"
#include "AQLQueue.h"

namespace HSA
{

AQLQueue::AQLQueue(uint32_t size, uint32_t type)
{
	// Global queue id to assign
	static unsigned int process_queue_id = 0;

	// Allocate queue fields in quest memory
	Emulator *emulator = Emulator::getInstance();
	mem::Manager *manager = emulator->getMemoryManager();
	mem::Memory *mem = emulator->getMemory();
	fields_address = manager->Allocate(
			sizeof(AqlQueueFields));
	fields = (struct AqlQueueFields *)mem->getBuffer(
			fields_address, sizeof(AqlQueueFields),
			mem::Memory::AccessWrite);

	// size must be a power of two
	if (log2(size) != floor(log2(size)))
		throw Error("Queue size must be a power of 2!");
	fields->size = size;

	// Set default type and feature
	fields->queue_type = type;
	fields->queue_features = 1;
//	fields->doorbell_signal = (unsigned long long)Emulator::getInstance()->
//			CreateSignal(0);
	fields->service_queue = 0;
	fields->id = process_queue_id++;

	// Allocate buffer space for the queue
	assert(sizeof(AQLDispatchPacket) == 64);
	fields->base_address = manager->Allocate(
			size * sizeof(AQLDispatchPacket),
			sizeof(AQLDispatchPacket));

	// Set initial write and read index to 0
	fields->write_index = 0;
	fields->read_index = 0;
}


AQLQueue::~AQLQueue()
{
	// Two things to be done in destructor
	// 1. Free the memory allocated for the packets buffer
	// 2. Free the memory allocated for the queue fields
//	Emulator *emulator = Emulator::getInstance();
//	mem::Manager *manager = emulator->getMemoryManager();
//	manager->Free(fields->base_address);
//	manager->Free(fields_address);
}


void AQLQueue::Associate(Component *component)
{
	if (associated_component)
		throw Error("Re-associated a queue to a device!");
	associated_component = component;
}


AQLDispatchPacket *AQLQueue::getPacket(uint64_t index)
{
	// Convert the linear index to real recursive index
	uint64_t address = IndexToAddress(index);

	// Get the memory object
	Emulator *emulator = Emulator::getInstance();
	mem::Memory *memory = emulator->getMemory();

	// Returns the buffer in real memory space
	//std::cout << misc::fmt("Getting packet at 0x%llx\n", recursive_index);
	AQLDispatchPacket *packet = (AQLDispatchPacket *)memory->getBuffer(
			address,
			sizeof(AQLDispatchPacket),
			mem::Memory::AccessRead);

	// Return the packet buffer
	return packet;
}


AQLDispatchPacket *AQLQueue::ReadPacket()
{
	// If the queue is empty, return a nullptr
	if (isEmpty())
		return nullptr;

	// Get the pointer to the packet
	AQLDispatchPacket *packet = getPacket(getReadIndex());

	// Set packet format to invalid
	packet->setFormat(AQLFormatInvalid);

	// Increase read index
	fields->read_index += 1;

	// Return the packet
	return packet;
}

}  // namespace HSA
