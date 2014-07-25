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

#include "Emu.h"
#include "AQLPacket.h"
#include "AQLQueue.h"

namespace HSA
{

AQLQueue::AQLQueue(unsigned int size, QueueType type)
{
	// Global queue id to assign
	static unsigned int process_queue_id = 0;

	// size must be a power of two
	if (log2(size) != floor(log2(size)))
		throw Error("Queue size must be a power of 2!");
	this->size = size;

	// Set default type and feature
	queue_type = type;
	queue_feature = 1;
	bell_signal = 0;
	service_queue = 0;
	queue_id = process_queue_id++;

	// Allocate buffer space for the queue
	Emu *emu = Emu::getInstance();
	mem::Manager *manager = emu->getManager();
	assert(sizeof(AQLDispatchPacket) == 64);
	base_address = manager->Allocate(size * sizeof(AQLDispatchPacket),
			sizeof(AQLDispatchPacket));

	// Set initial write and read index to base address
	write_index = base_address;
	read_index = base_address;
}


AQLQueue::~AQLQueue()
{}


void AQLQueue::Associate(Component *component)
{
	if (associated_component)
		throw Error("Re-associated a queue to a device!");
	associated_component = component;
}


void AQLQueue::Enqueue(AQLDispatchPacket *packet)
{
	// 1. Allocating an AQL packet slot
	unsigned long long packet_id = write_index;
	allocatesPacketSlot();

	// 2. update the AQL packet with the task particulars
	Emu *emu = Emu::getInstance();
	mem::Memory *memory = emu->getMemory();
	memory->Write(packet_id, sizeof(AQLDispatchPacket), (char *)packet);

	// 3. Assigning the packet to the Packet Processor
	AQLDispatchPacket *saved_packet = getPacket(packet_id);
	//std::cout << misc::fmt("saved_packet address: 0x%llx\n",
	//		(unsigned long long)saved_packet);
	saved_packet->Assign();

	// 4. Notifying the Packet Processor of the packet
	bell_signal = packet_id;

}


AQLDispatchPacket *AQLQueue::getPacket(unsigned long long linear_index)
{
	// Convert the linear index to real recursive index
	unsigned long long recursive_index = toRecursiveIndex(linear_index);

	// Get the memory object
	Emu *emu = Emu::getInstance();
	mem::Memory *memory = emu->getMemory();

	// Returns the buffer in real memory space
	//std::cout << misc::fmt("Getting packet at 0x%llx\n", recursive_index);
	AQLDispatchPacket *packet = (AQLDispatchPacket *)memory->getBuffer(
			recursive_index,
			sizeof(AQLDispatchPacket),
			mem::Memory::AccessWrite);

	// Return the packet buffer
	return packet;
}


AQLDispatchPacket *AQLQueue::ReadPacket()
{
	// If the queue is empty, return a nullptr
	if (isEmpty())
		return nullptr;

	// Get the pointer to the packet
	AQLDispatchPacket *packet = getPacket(read_index);

	// Set packet format to invalid
	packet->setFormat(AQLFormatInvalid);

	// Increase read index
	read_index += 64;

	// Return the packet
	return packet;
}

}  // namespace HSA
