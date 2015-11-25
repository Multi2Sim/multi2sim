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

#ifndef NETWORK_BUFFER_H
#define NETWORK_BUFFER_H

#include <lib/esim/Engine.h>
#include <lib/esim/Event.h>
#include <lib/esim/Queue.h>

#include "System.h"

namespace net
{
class Connection;
class Node;
class Packet;

class Buffer
{

	// Node that the buffer belongs to
	Node *node;

	// Buffer Name
	std::string name;

	// Buffer index
	int index;

	// Buffer size
	int size;

	// Occupied buffer entries
	int count = 0;

	// A queue of events suspended due to the buffer 
	esim::Queue event_queue;

	// Connection that the buffer is connected to
	Connection *connection;

	// The scheduled cycle is for an arbiter to keep track of when 
	// a decision is made on this buffer. An init value of -1 means
	// no previous decision has been made.
	long long scheduled_cycle = -1;

	// The scheduled buffer is the buffer that is linked within a switch
	// or a bus.
	Buffer *scheduled_buffer = nullptr;

	// A list of packets in the buffer
	std::list<Packet *> packets;



	//
	// Statistics
	//

	// Current byte occupancy of the buffer
	int occupancy_in_bytes = 0;

	// current packet occupancy of the buffer
	int occupancy_in_packets = 0;

	// Last time buffer occupancy was measured
	long long occupancy_measured_cycle = 0;

	// Accumulated bytes that occupied the buffer
	long long accumulated_occupancy_in_bytes = 0;

	// Accumulated packets that occupied the buffer
	long long accumulated_occupancy_in_packets = 0;

public:

	/// Constructor
	Buffer(const std::string &name,
			int size, int index,
			Node *node, Connection *connection);

	/// Get the name of the link.
	std::string getName() const { return name; }

	/// Get buffer size
	int getSize() const { return size; }

	/// Get index.
	int getIndex() const { return index; }

	/// Get buffer's node.
	Node *getNode() const { return this->node; }

	/// Get count
	int getCount() const { return count; }

	/// Get buffer's connection.
	Connection *getConnection() const { return this->connection; }

	/// Get the scheduled cycle
	long long getScheduledCycle() const { return scheduled_cycle; }

	/// Set the scheduled cycle
	void setScheduledCycle(long long scheduled_cycle) 
	{
		this->scheduled_cycle = scheduled_cycle;
	}

	/// Get the scheduled buffer
	Buffer *getScheduledBuffer() const { return scheduled_buffer; }

	/// Get the buffer's occupancy
	int getOccupancyInBytes() const { return occupancy_in_bytes; }

	/// Set the scheduled buffer
	void setScheduledBuffer(Buffer *scheduled_buffer)
	{
		this->scheduled_buffer = scheduled_buffer;
	}

	/// Suspend the current event chain in the event queue associated with
	/// the buffer. This function must be invoked within an event handler.
	void Wait(esim::Event *event)
	{
		event_queue.Wait(event);
	}

	/// Wake up all events in the queue
	void Wakeup()
	{
		event_queue.WakeupAll();
	}

	/// Insert a packet to the buffer. A pointer is used because the 
	/// message keeps the ownership of the packet
	void InsertPacket(Packet *packet);

	/// Pop the packet at the head of the buffer. The packet is not 
	/// destoryed. The message still keeps the ownership of the packet.
	/// When the message is destroyed, the packet is destroyed together.
	void ExtractPacket();

	/// Get number of packets in the buffer
	int getNumPacket()
	{
		return packets.size();
	}

	/// Get the first packet in the buffer
	Packet *getBufferHead() 
	{
		if (packets.empty())
			return nullptr;
		return packets.front();
	}

	/// Remove a certain packet from the buffer
	void RemovePacket(Packet *packet);

	/// Updating the buffer statistics
	void UpdateOccupancyInformation();

	/// Dump the buffer information
	void Dump(std::ostream &os = std::cout);



	//
	// Public fields
	//

	// Cycle until a read operation on buffer lasts
	long long read_busy = -1;

	// Cycle until a write operation on buffer lasts
	long long write_busy = -1;

};

}  // namespace net

#endif

