/* 
 *  Multi2Sim
 *  Copyright (C) 2016 Amir Kavyan Ziabari (aziabari@ece.neu.edu)
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

#ifndef NETWORK_MESSAGE_H
#define NETWORK_MESSAGE_H

#include <vector>
#include <memory>

#include "Packet.h"

namespace net 
{
class Node;
class Network;

class Message
{

	// Id of the message
	long long id;

	// Network that this message belongs to 
	Network *network;

	// Source node
	Node *source_node;

	// Destination node
	Node *destination_node;

	// Size of the message
	int size;

	// A list of packet
	std::vector<std::unique_ptr<Packet>> packets;

	// A list of received packets
	std::vector<Packet *> received_packets;

	// Cycle when the message was sent
	long long send_cycle;

public:

	/// Constructor
	Message(long long id, Network *network, 
			Node *source_node, Node *destination_node,
			int size, long long cycle);

	/// Packetize
	void Packetize(int packet_size);

	/// Collect a packet that has arrived at its destination. If the 
	/// packet are the last packet to receive, return true. Otherwise, 
	/// return false.
	///
	/// This function can only be called in the received event handler. 
	/// This function would check if the packet belongs to the message, 
	/// and the packet has not been received before. This function would
	/// not pop the packet from the buffer.
	bool Assemble(Packet *packet);

	/// Get id of the message
	long long getId() const { return id; }

	/// Get the network
	Network *getNetwork() const { return network; }

	/// Get source node
	Node *getSourceNode() const { return source_node; }

	/// Get destination node
	Node *getDestinationNode() const { return destination_node; }

	/// Get message size
	int getSize() const { return size; }

	/// Get the cycle that message was sent
	long long getSendCycle() const { return send_cycle; }

	/// Get number of packets belongs to the message
	int getNumPackets() const { return packets.size(); }

	/// Get packet by index
	Packet *getPacket(int index) const { return packets[index].get(); }
};

}  // namespace net

#endif
