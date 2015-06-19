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

	// Cycle until a read operation on buffer lasts
	long long read_busy = -1;

	// Cycle until a write operation on buffer lasts
	long long write_busy = -1;

	// A list of packets in the buffer
	std::list<Packet *> packets;

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

	/// Get the cycle when write operation will finish
	long long getWriteBusy() const { return write_busy; }

	/// Set the cycle when write operation will finish
	void setWriteBusy(long long write_busy) 
	{ 
		this->write_busy = write_busy;
	}

	/// Get the cycle when read operation will finish
	long long getReadBusy() const { return read_busy; }

	/// Set the cycle when read operation will finish
	void setReadBusy(long long read_busy)
	{
		this->read_busy = read_busy;
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
	/// destoried. The message still keeps the ownership of the packet. 
	/// When the message is destoried, the packet is destoried together.
	void PopPacket();

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

};

}  // namespace net

#endif

