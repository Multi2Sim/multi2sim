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
	// Dumping the swtich name
	os << misc::fmt("[ Network.%s.Node.%s ]\n", network->getName().c_str(),
			getName().c_str());

	// Dumping the switch bandwidth
	os << misc::fmt("Bandwidth = %d\n", bandwidth);

	// Getting the current cycle
	long long cycle = System::getInstance()->getCycle();

	// Dumping the statistics information
	os << misc::fmt("SentBytes = %lld\n", sent_bytes);
	os << misc::fmt("SentPackets = %lld\n", sent_packets);
	os << misc::fmt("SendRate = %0.4f\n", cycle ?
			(double) sent_bytes / cycle : 0.0 );
	os << misc::fmt("ReceivedBytes = %lld\n", received_bytes);
	os << misc::fmt("ReceivedPackets = %lld\n", received_packets);
	os << misc::fmt("ReceiveRate = %0.4f\n", cycle ?
			(double) received_bytes / cycle : 0.0 );

	// Dumping input buffers' information
	for (auto &buffer : input_buffers)
		buffer->Dump(os);

	// Dumping output buffers' information
	for (auto &buffer : output_buffers)
		buffer->Dump(os);

	// Creating an empty line in the dump
	os << "\n";
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
		throw misc::Panic(misc::fmt("Trying to forward "
				"a packet that is not in current "
				"switch").c_str());

	// Check if the input buffer is in read busy
	long long cycle = System::getInstance()->getCycle();
	Message *message = packet->getMessage();
	Network *network = message->getNetwork();
	if (input_buffer->read_busy >= cycle)
	{
		// Update debug information
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl_busy_sw_src_buf: %s:%s\n",
				network->getName().c_str(),
				message->getId(),
				packet->getId(),
				node->getName().c_str(),
				input_buffer->getName().c_str());

		// Coming back to this event when buffer is not busy
		esim_engine->Next(current_event, 
				input_buffer->read_busy - cycle + 1);
		return;
	}

	// Look up the routing table for next output buffer
	RoutingTable *routing_table = network->getRoutingTable();
	Node *destination_node = message->getDestinationNode();
	RoutingTable::Entry *entry = 
			routing_table->Lookup(node, destination_node);
	if (!entry) 
		throw misc::Panic(misc::fmt("%s: no route from %s "
				"to %s.",
				network->getName().c_str(), 
				node->getName().c_str(), 
				destination_node->getName().c_str()));
	Buffer *output_buffer = entry->getBuffer();

	// Check if the output buffer is busy
	if (output_buffer->write_busy >= cycle)
	{
		// Update debug information
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl_busy_sw_dst_buf: %s:%s\n",
				network->getName().c_str(),
				message->getId(),
				packet->getId(),
				output_buffer->getNode()->
				getName().c_str(),
				output_buffer->getName().c_str());

		// Update trace information
		System::trace << misc::fmt("net.packet "
				"net=\"%s\" "
				"name=\"P-%lld:%d\" "
				"state=\"%s:%s:Dest_buffer_busy\" "
				"stg=\"DBB\"\n",
				network->getName().c_str(),
				message->getId(),
				packet->getId(),
				node->getName().c_str(),
				input_buffer->getName().c_str());


		esim_engine->Next(current_event, 
				output_buffer->write_busy - cycle + 1);
		return;
	}

	// Check if the destination buffer has enough storage
	// to fit the packet
	if (packet->getSize() > output_buffer->getSize())
		throw misc::Panic(misc::fmt("%s: packet does not "
				"fit in buffer.\n",
				network->getName().c_str()));

	// If destination output buffer is full, wait
	if (output_buffer->getCount() + packet->getSize() > 
			output_buffer->getSize())
	{
		// Update debug information
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl_full_sw_dst_buf: %s:%s\n",
				network->getName().c_str(),
				message->getId(),
				packet->getId(),
				output_buffer->getNode()->
				getName().c_str(),
				output_buffer->getName().c_str());

		// Update trace information
		System::trace << misc::fmt("net.packet "
				"net=\"%s\" "
				"name=\"P-%lld:%d\" "
				"state=\"%s:%s:Dest_buffer_full\" "
				"stg=\"DBF\"\n",
				network->getName().c_str(),
				message->getId(),
				packet->getId(),
				node->getName().c_str(),
				input_buffer->getName().c_str());

		// Come back when buffer is not busy
		output_buffer->Wait(current_event);
		return;
	}

	// If scheduler says that it is not our turn, try later
	if (Schedule(output_buffer) != input_buffer)
	{
		// Update debug information
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl_sw_arb: %s\n",
				network->getName().c_str(),
				message->getId(),
				packet->getId(),
				name.c_str());

		esim_engine->Next(current_event, 1);
		return;
	}

	// Calculate latency and occupy resources
	int latency = (packet->getSize() - 1) / bandwidth + 1;
	input_buffer->read_busy = cycle + latency - 1;
	output_buffer->write_busy = cycle + latency - 1;

	// Transfer message to next output buffer
	input_buffer->ExtractPacket();
	output_buffer->InsertPacket(packet);
	packet->setBuffer(output_buffer);
	packet->setBusy(cycle + latency - 1);

	// Buffer's trace information
	System::trace << misc::fmt("net.packet_extract "
			"net=\"%s\" node=\"%s\" buffer=\"%s\" "
			"name=\"P-%lld:%d\" occpncy=%d\n",
			network->getName().c_str(),
			input_buffer->getNode()->getName().c_str(),
			input_buffer->getName().c_str(),
			message->getId(), packet->getId(),
			input_buffer->getOccupancyInBytes());

	System::trace << misc::fmt("net.packet_insert net=\"%s\" "
			"node=\"%s\" buffer=\"%s\" "
			"name=\"P-%lld:%d\" occpncy=%d\n",
			network->getName().c_str(),
			output_buffer->getNode()->getName().c_str(),
			output_buffer->getName().c_str(),
			message->getId(), packet->getId(),
			output_buffer->getOccupancyInBytes());

	// Schedule next event
	esim_engine->Next(System::event_output_buffer, latency);
}


Buffer *Switch::Schedule(Buffer *output_buffer) 
{
	// Checks if the scheduler is an output buffer
	// of current switch
	bool is_output_buffer = false;
	for (auto &buffer : output_buffers) 
		if (output_buffer == buffer.get())
		{
			is_output_buffer = true;
			break;
		}
	if (!is_output_buffer) 
		throw misc::Panic(misc::fmt("Buffer %s is not and "
				"output buffer of switch %s.",
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
	// previous decision is made, use 0
	int previous_input_buffer_index = output_buffer->getScheduledBuffer() ?
		output_buffer->getScheduledBuffer()->getIndex() : 0;
	
	// Checks if the output buffer is in write cycle
	if (output_buffer->write_busy >= cycle)
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
		if (input_buffer->read_busy >= cycle)
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
