/*
 *  Multi2Sim
 *  Copyright (C) 2014  Amir Kavyan Ziabari (aziabari@ece.neu.edu)
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

#include <algorithm>

#include "Network.h"
#include "Buffer.h"

namespace net
{

Buffer::Buffer(const std::string &name,
		int size, int index,
		Node *node, Connection *connection) :
		node(node),
		name(name),
		index(index),
		size(size),
		connection(connection)
{
}


void Buffer::InsertPacket(Packet *packet)
{
	// Check if buffer large enough to hold the packet
	if (count + packet->getSize() > size)
		throw misc::Panic("No enough space in buffer.");

	// Update count
	count += packet->getSize();

	// Insert the packet into buffer
	packets.push_back(packet);
}


void Buffer::RemovePacket(Packet *packet)
{
	// Check if the packet is in the buffer
	if (std::find(packets.begin(), packets.end(), packet) == packets.end())
		throw misc::Panic("Trying to remove a packet that is not in"
				" current buffer");

	// Reduce the occupied size of the buffer
	count -= packet->getSize();

	// Remove the packet
	packets.remove(packet);

	// Wake up the buffer event queue
	if (!event_queue.isEmpty())
		event_queue.WakeupAll();
}


void Buffer::PopPacket()
{
	// Check if there is a packet to be poped
	if (packets.size() == 0)
		throw misc::Panic("No packets to pop");

	// Get the reference of packet that is going to be poped
	Packet *packet = packets.front();

	// Remove the packet from the queue
	packets.pop_front();

	// Reduce the count of the packet
	count -= packet->getSize();

	// Wake up the buffer event queue
	if (!event_queue.isEmpty())
		event_queue.WakeupOne();
}

}  // namespace net
