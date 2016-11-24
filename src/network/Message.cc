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

#include <lib/cpp/Misc.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/String.h>
#include <lib/esim/Engine.h>

#include "System.h"
#include "Packet.h"
#include "Frame.h"
#include "Network.h"
#include "Message.h"

namespace net
{

Message::Message(long long id, 
		Network *network,
		Node *source_node, 
		Node *destination_node, 
		int size,
		long long cycle) :
		id(id),
		network(network),
		source_node(source_node),
		destination_node(destination_node),
		size(size),
		send_cycle(cycle)
{
}


void Message::Packetize(int packet_size)
{
	int packet_count = (size - 1) / packet_size + 1;
	for (int i = 0; i < packet_count; i++)
	{
		auto packet = misc::new_unique<Packet>(this, packet_size);
		this->packets.push_back(std::move(packet));
	}
}


bool Message::Assemble(Packet *packet)
{
	// Check if the packet belongs to this message
	bool is_packet_belongs_to_message = false;
	for (auto &message_packet : packets)
	{
		if (message_packet.get() == packet)
		{
			is_packet_belongs_to_message = true;
			break;
		}
	}
	if (!is_packet_belongs_to_message)
		throw misc::Panic("Cannot assemble the message from a packet"
				"that does not belongs this message.");

	// Check if the packet has been assembled before
	for (auto &received_packet : received_packets)
	{
		if (received_packet == packet)
			throw misc::Panic("Packets have been assembled twice");
	}

	// Mark the packet has been received
	received_packets.push_back(packet);

	// Update the trace with the position of the packet, the depacketizer
	net::System::trace << misc::fmt("net.packet net=\"%s\" "
			"name=\"P-%lld:%d\" state=\"%s:depacketizer\" stg=\"DC\"\n",
			network->getName().c_str(), id,
			packet->getId(),
			packet->getNode()->getName().c_str());

	// Check if all the packets of the message received
	if (received_packets.size() == packets.size())
	{
		return true;
	}
	return false;
}

}  // namespace net
