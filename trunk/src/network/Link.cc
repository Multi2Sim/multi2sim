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

#include <lib/esim/Event.h>

#include "Node.h"
#include "Link.h"

namespace net
{

Link::Link(Network *network,
		const std::string &name,
		Node *src_node,
		Node *dst_node,
		int bandwidth,
		int source_buffer_size,
		int destination_buffer_size,
		int num_virtual_channels) :
		Connection(name, network, bandwidth),
		source_node(src_node),
		destination_node(dst_node),
		num_virtual_channels(num_virtual_channels)
{
	// Create and add source buffer
	Buffer *source_buffer = src_node->addOutputBuffer(source_buffer_size, 
			this);
	addSourceBuffer(source_buffer);

	// Create and add destination buffer
	Buffer *destination_buffer = dst_node->addInputBuffer(
			destination_buffer_size, this);
	addDestinationBuffer(destination_buffer);
}

void Link::Dump(std::ostream &os) const
{
	os << misc::fmt("\n***** Link %s *****\n", name.c_str());
	
	// Dump sources
	for (auto buffer : source_buffers)
	{
		os << misc::fmt("%s \t->", 
				buffer->getNode()->getName().c_str());
	}

	// Dump destinations
	for (auto buffer : destination_buffers)
	{
		os << misc::fmt(" \t %s", 
				buffer->getNode()->getName().c_str());
	}
}


void Link::TransferPacket(Packet *packet)
{
	// Get current cycle
	esim::Engine *esim_engine = esim::Engine::getInstance();
	esim::Event*current_event = esim_engine->getCurrentEvent();
	long long cycle = System::getInstance()->getCycle();

	// Check if the packet is in an output buffer that connects to 
	// this link
	Buffer *source_buffer = packet->getBuffer();
	if (std::find(source_buffers.begin(), source_buffers.end(), 
			source_buffer) == source_buffers.end())
	{
		throw misc::Panic("Packet is not ready to be send over the "
				"link");
	}

	// Check if the packet is at the head of the buffer
	if (source_buffer->getBufferHead() != packet)
	{
		source_buffer->Wait(current_event);
		return;
	}

	// Check if the link is busy
	if (busy >= cycle)
	{
		esim_engine->Next(current_event, busy - cycle + 1);
		return;
	}

	// Check if the destination buffer is busy
	Buffer *destination_buffer = destination_buffers[0];
	long long write_busy = destination_buffer->getWriteBusy();
	if (write_busy >= cycle)
	{
		esim_engine->Next(current_event, write_busy - cycle + 1);
		return;
	}

	// Check if the destination buffer is full
	if (destination_buffer->getCount() + packet->getSize() > 
			destination_buffer->getSize())
	{
		destination_buffer->Wait(current_event);
		return;
	}

	// Calculate latency and occypy resources
	int latency = (packet->getSize() - 1) / bandwidth + 1;
	source_buffer->setReadBusy(cycle + latency - 1);
	busy = cycle + latency - 1;
	destination_buffer->setWriteBusy(cycle + latency - 1);

	// Transfer message to next input buffer
	source_buffer->PopPacket();		
	destination_buffer->InsertPacket(packet);
	packet->setNode(destination_buffer->getNode());
	packet->setBuffer(destination_buffer);
	packet->setBusy(cycle + latency - 1);

	// Schedule input buffer event	
	esim_engine->Next(System::event_input_buffer);
}

}
