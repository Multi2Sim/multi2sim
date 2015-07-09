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

#ifndef NETWORK_LINK_H
#define NETWORK_LINK_H

#include <lib/cpp/String.h>

#include "Connection.h"
namespace net
{

class Node;
class Buffer;

class Link : public Connection
{

	// Link source node
	Node *source_node;

	// Link destination node
	Node *destination_node;

	// Number of virtual channels on link
	int num_virtual_channels;

	// user assigned name
	std::string name;




	//
	// Scheduling and arbitration
	//

	// Link busy cycle for event scheduling
	long long busy = -1;

	// Last cycle a buffer was assigned to a link in virtual channel
	// arbitration
	long long scheduled_when = -1;

	// Last buffer that has the ownership of physical link in virtual
	// channel arbitration
	Buffer *scheduled_buffer = nullptr;




	//
	// Statistics
	//

	// Number of bytes that was transfered through the links
	long long transferred_bytes = 0;

	// Number of cycles that the link was busy
	long long busy_cycles = 0;

	// Number of packets that traversed the link
	long long transferred_packets = 0;

public:

	/// Constructor
	Link(Network *network,
			const std::string &name,
			const std::string &link_name,
			Node *src_node,
			Node *dst_node,
			int bandwidth,
			int source_buffer_size,
			int destination_buffer_size,
			int num_virtual_channel);

	/// Set virtual channels
	void setNumVirtualChannels(const int num_virtual_channel)
	{
		this->num_virtual_channels = num_virtual_channel;
	}

	/// Set source node
	void setSourceNode(Node* node) { this->source_node = node; }

	/// Get source node
	Node *getSourceNode() const { return source_node; }

	/// Set destination node
	void setDestinationNode(Node* node) { this->destination_node = node; }

	/// Get the number of busy cycles
	long long getBusyCycle() { return busy_cycles; }

	/// Get th amount of transfered bytes
	long long getTransferredBytes() { return transferred_bytes; }

	/// Get destination node
	Node *getDestinationNode() const { return destination_node; }

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const Link &link)
	{
		link.Dump(os);
		return os;
	}

	/// Dump the node information.
	void Dump(std::ostream &os = std::cout) const;

	/// Transfer the packet from an output buffer 
	void TransferPacket(Packet *packet);
};


}  // namespace net

#endif

