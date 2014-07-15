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

AQLQueue::AQLQueue(unsigned int size)
{
	// Global queue id to assign
	static unsigned int process_queue_id = 0;

	// size must be a power of two
	if (log2(size) != floor(log2(size)))
		throw Error("Queue size must be a power of 2!");
	this->size = size;

	// Set default type and feature
	queue_type = HSAQueueMulti;
	queue_feature = 1;
	bell_signal = 0;
	service_queue = 0;
	queue_id = process_queue_id++;

	// Allocate buffer space for the queue
	buffer.reset(new AQLPacket[size]);
	base_address = (unsigned long long)buffer.get();
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


void AQLQueue::Enqueue(AQLPacket *packet)
{
	// 1. Allocating an AQL packet slot
	unsigned long long packet_id = write_index;
	allocatesPacketSlot();

	// 2. update the AQL packet with the task particulars
	memcpy((char *)toRecursiveIndex(packet_id), packet, sizeof(AQLPacket));

	// 3. Assigning the packet to the Packet Processor
	getPacket(packet_id)->Assign();

	// 4. Notifying the Packet Processor of the packet
	bell_signal = packet_id;

}


AQLPacket *AQLQueue::getPacket(unsigned long long linear_index)
{
	unsigned long long recursive_index = toRecursiveIndex(linear_index);
	return (AQLPacket *)recursive_index;
}

}  // namespace HSA
