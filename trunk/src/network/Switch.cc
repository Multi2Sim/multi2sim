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

#include "Packet.h"
#include "Switch.h"

namespace net
{

void Switch::Dump(std::ostream &os) const
{
	os << misc::fmt("\n***** %s: %s *****\n", 
			"Switch",
			getName().c_str());
}


void Switch::Forward(Packet *packet) 
{
	// Get current event
	esim::Engine *esim_engine = esim::Engine::getInstance();
	esim::Event *current_event = esim_engine->getCurrentEvent();

	// Get the input buffer that the packet is in
	Buffer *input_buffer = packet->getBuffer();

	// Assert the input buffer is an input buffer of current node
	Node *node  = input_buffer->getNode();
	if (node != this) 
		throw misc::Panic(misc::fmt("Trying to forward a packet "
				"that is not in current switch").c_str());

	// Check if the input buffer is in read busy
	long long cycle = System::getInstance()->getCycle();
	Message *message = packet->getMessage();
	Network *network = message->getNetwork();
	if (input_buffer->getReadBusy() >= cycle) 
	{
		System::debug << misc::fmt("pkt a=\"stall\" "
				"net=\"%s\" msg-->pkt=%lld:%d "
				"why=\"src-busy\"",
				network->getName().c_str(),
				message->getId(),
				packet->getSessionId());
		esim_engine->Next(current_event, 
				input_buffer->getReadBusy() - cycle + 1);
		return;
	}

	// Look up the routing table for next output buffer
	RoutingTable *routing_table = network->getRoutingTable();
	Node *destination_node = message->getDestinationNode();
	RoutingTable::Entry *entry = 
			routing_table->Lookup(node, destination_node);
	if (!entry) 
		throw misc::Panic(misc::fmt("%s: no route from %s to %s.",
				network->getName().c_str(), 
				node->getName().c_str(), 
				destination_node->getName().c_str()));
	Buffer *output_buffer = entry->getBuffer();

	// Check if the output buffer is busy
	if (output_buffer->getWriteBusy() >= cycle)
	{
		System::debug << misc::fmt("pkt "
					"a=\"stall\" "
					"net=\"%s\" "
					"msg-->pkt=%lld:%d "
					"why=\"dst-busy\"\n",
					network->getName().c_str(),
					message->getId(),
					packet->getSessionId());
		esim_engine->Next(current_event, 
				output_buffer->getWriteBusy() - cycle + 1);
		return;
	}

	// Check if the destination buffer larget enough to fit the packet
	if (packet->getSize() > output_buffer->getSize())
		throw misc::Panic(misc::fmt("%s: packet does not fit in "
				"buffer.\n", 
				network->getName().c_str()));

	// If destination output buffer is full, wait
	if (output_buffer->getCount() + packet->getSize() > 
			output_buffer->getSize())
	{
		System::debug << misc::fmt("pkt "
				"a=\"stall\" "
				"net=\"%s\" "
				"msg-->pkt=%lld:%d "
				"why=\"dst-full\"\n",
				network->getName().c_str(),
				message->getId(),
				packet->getSessionId());
		output_buffer->Wait(current_event);
		return;
	}

	// If scheduler says that it is not our turn, try later
	if (Schedule(output_buffer) != input_buffer)
	{
		System::debug << misc::fmt("pkt "
				"a=\"stall\" "
				"net=\"%s\" "
				"msg-->pkt=%lld:%d "
				"why=\"sched\"\n",
				network->getName().c_str(),
				message->getId(),
				packet->getSessionId());

		esim_engine->Next(current_event, 1);
		return;
	}

	// Calculate latency and occupy resources
	int latency = (packet->getSize() - 1) / bandwidth + 1;
	input_buffer->setReadBusy(cycle + latency - 1);
	output_buffer->setWriteBusy(cycle + latency - 1);

	// Transfer message to next output buffer
	input_buffer->PopPacket();
	output_buffer->InsertPacket(packet);
	packet->setBuffer(output_buffer);
	packet->setBusy(cycle + latency - 1);

	// Schedule next event
	esim_engine->Next(System::event_output_buffer, latency);
}


Buffer *Switch::Schedule(Buffer *output_buffer) 
{
	// Checks if the scheduler is an output buffer of current switch	
	bool is_output_buffer = false;
	for (auto &buffer : output_buffers) 
		if (output_buffer == buffer.get())
		{
			is_output_buffer = true;
			break;
		}
	if (!is_output_buffer) 
		throw misc::Panic(misc::fmt("Buffer %s is not and output "
				"buffer of switch %s.", 
				output_buffer->getName().c_str(),
				this->getName().c_str()));

	// Get current cycle
	long long cycle = System::getInstance()->getCycle();

	// Checks if the decision has already been made
	if (output_buffer->getScheduledCycle() == cycle) 
		return output_buffer->getScheduledBuffer();

	// Otherwise, make a new decision
	output_buffer->setScheduledCycle(cycle);

	// Get the index of the buffer of the previous devision. If no 
	// previous decision is make, use 0
	int previous_input_buffer_index = output_buffer->getScheduledBuffer() ?
		output_buffer->getScheduledBuffer()->getIndex() : 0;
	
	// Checks if the output buffer is in write cycle
	if (output_buffer->getWriteBusy() >= cycle)
		throw misc::Panic(misc::fmt("Cannot schedule a busy output "
					"buffer %s", 
					output_buffer->getName().c_str()));
	
	// Find an input buffer to be linked. This loop cannot be replaced
	// with the new C++11 iterate loop, because the item be be accessed
	// does not start with 0. The real index with be calculated based on
	// the value of i.
	for (unsigned int i = 0; i < input_buffers.size(); i++) 
	{
		// Calculate the input buffer to check
		int index = (previous_input_buffer_index + i + 1) % 
			input_buffers.size();

		// Get the input buffer
		Buffer *input_buffer = input_buffers[index].get();

		// Skip empty buffer
		if (input_buffer->getCount() == 0)
			continue;

		// Skip the buffer in read busy
		if (input_buffer->getReadBusy() >= cycle)
			continue;

		// Skip the buffer whose first packet is not to be forwarded
		// to the output buffer
		Packet *packet = input_buffer->getBufferHead();
		Message *message = packet->getMessage();
		Node *destination_node = message->getDestinationNode();
		Network *network = message->getNetwork();
		RoutingTable *routingTable = network->getRoutingTable();
		RoutingTable::Entry *entry = routingTable->Lookup(this, 
				destination_node);
		if (!entry) 
			throw misc::Panic(misc::fmt("No route found from "
					"node %s to node %s", 
					this->getName().c_str(),
					destination_node->getName().c_str()));
		Buffer *next_buffer = entry->getBuffer();
		if (next_buffer != output_buffer)
			continue;
	
		// There must be enough space left in the output buffer
		if (output_buffer->getCount() + packet->getSize() > 
			output_buffer->getSize())
			continue;

		// All conditions satisfied - schedule
		output_buffer->setScheduledBuffer(input_buffer);
		return input_buffer;
	}

	// For some reason, no input buffer is ready
	output_buffer->setScheduledBuffer(nullptr);
	return nullptr;
}

}