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
		const std::string &descriptive_name,
		Node *src_node,
		Node *dst_node,
		int bandwidth,
		int source_buffer_size,
		int destination_buffer_size,
		int num_virtual_channels) :
						Connection(descriptive_name, network),
						source_node(src_node),
						destination_node(dst_node),
						num_virtual_channels(num_virtual_channels),
						name(name),
						bandwidth(bandwidth)
{
	for (int i = 0; i < num_virtual_channels ; i++)
	{
		// Create and add source buffer
		Buffer *source_buffer = src_node->addOutputBuffer(
				source_buffer_size, this);
		addSourceBuffer(source_buffer);

		// Create and add destination buffer
		Buffer *destination_buffer = dst_node->addInputBuffer(
				destination_buffer_size, this);
		addDestinationBuffer(destination_buffer);
	}
}


void Link::Dump(std::ostream &os) const
{
	// Dumping the simulator assigned name
	os << misc::fmt("[ Network.%s.Link.%s ]\n", network->getName().c_str(),
			getName().c_str());

	// Dumping the user assigned name
	os << misc::fmt("Name = %s\n", name.c_str());

	// Dump source buffers
	os << misc::fmt("SourceBuffers = ");
	for (auto buffer : source_buffers)
		os << misc::fmt("%s:%s ",
				buffer->getNode()->getName().c_str(),
				buffer->getName().c_str());
	os << "\n" ;

	// Dump destination buffers
	os << misc::fmt("DestinationBuffers = ");
	for (auto buffer : destination_buffers)
		os << misc::fmt("%s:%s ",
				buffer->getNode()->getName().c_str(),
				buffer->getName().c_str());
	os << "\n" ;

	// Dump statistics
	os << misc::fmt("Bandwidth = %d\n", bandwidth);
	os << misc::fmt("TransferredPackets = %lld\n", transferred_packets);
	os << misc::fmt("TransferredBytes = %lld\n", transferred_bytes);
	os << misc::fmt("BusyCycles = %lld\n", busy_cycles);

	// Statistics that depends on the cycle
	System *system = System::getInstance();
	long long cycle = system->getCycle();
	os << misc::fmt("BytesPerCycle = %0.4f\n", cycle ?
			(double) transferred_bytes / cycle : 0.0);
	os << misc::fmt("Utilization = %0.4f\n", cycle ?
			(double) transferred_bytes / (cycle * bandwidth) : 0.0);

	// Creating and empty line in dump
	os << "\n";
}


void Link::TransferPacket(Packet *packet)
{
	// Get current cycle
	esim::Engine *esim_engine = esim::Engine::getInstance();
	esim::Event *current_event = esim_engine->getCurrentEvent();
	System *system = System::getInstance();
	long long cycle = system->getCycle();

	// Retrieve related information
	Message *message = packet->getMessage();
	Node *node = packet->getNode();

	// Check if the packet is in an output buffer that connects to 
	// this link
	Buffer *source_buffer = packet->getBuffer();
	if (std::find(source_buffers.begin(), source_buffers.end(), 
			source_buffer) == source_buffers.end())
		throw misc::Panic("Packet is not ready to be send over the "
				"link");

	// Check if the packet is at the head of the buffer
	if (source_buffer->getBufferHead() != packet)
	{
		// Update debug information
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl_not_buf_head: %s:%s\n",
				network->getName().c_str(),
				message->getId(), packet->getId(),
				node->getName().c_str(), 
				source_buffer->getName().c_str());

		// Wait for the head to change
		source_buffer->Wait(current_event);
		return;
	}

	// Check if the link is busy
	if (busy >= cycle)
	{
		// Update debug information
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl at %s:%s busy_link: %s\n",
				network->getName().c_str(),
				message->getId(), packet->getId(),
				node->getName().c_str(), 
				source_buffer->getName().c_str(),
				getName().c_str());

		// Trace information
		System::trace << misc::fmt("net.packet "
				"net=\"%s\" name=\"P-%lld:%d\" "
				"state=\"%s:%s:link_busy\" "
				"stg=\"LB\"\n",
				network->getName().c_str(), message->getId(),
				packet->getId(),
				node->getName().c_str(),
				source_buffer->getName().c_str());

		esim_engine->Next(current_event, busy - cycle + 1);
		return;
	}

	// If buffer contains a packet but doesn't have the shared link
	// in control, wait
	Buffer *next_buffer = VirtualChannelArbitration();
	if (next_buffer != source_buffer)
	{
		// Update debug information
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl at %s:%s vc_arb: %s\n",
				network->getName().c_str(),
				message->getId(), packet->getId(),
				node->getName().c_str(), 
				source_buffer->getName().c_str(),
				name.c_str());

		// Trace information
		System::trace << misc::fmt("net.packet "
				"net=\"%s\" "
				"name=\"P-%lld:%d\" "
				"state=\"%s:%s:VC_arbitration_fail\" "
				"stg=\"VCA\"\n",
				network->getName().c_str(), message->getId(),
				packet->getId(),
				node->getName().c_str(),
				source_buffer->getName().c_str());

		// Next cycle to check again
		esim_engine->Next(current_event, 1);
		return;
	}

	// getting the destination buffer
	Buffer *destination_buffer = getDestinationBufferfromSource(source_buffer);
	if (!destination_buffer)
		throw misc::Panic(misc::fmt("Unsuccessful Virtual channel pairing"));

	// Check if the destination buffer is busy
	long long write_busy = destination_buffer->write_busy;
	if (write_busy >= cycle)
	{
		// Update debug information
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl_busy_dst_buf: %s:%s\n",
				network->getName().c_str(),
				message->getId(), packet->getId(),
				destination_buffer->getNode()->getName().c_str(),
				destination_buffer->getName().c_str());

		// Trace information
		System::trace << misc::fmt("net.packet "
				"net=\"%s\" "
				"name=\"P-%lld:%d\" "
				"state=\"%s:%s:Dest_buffer_busy\" "
				"stg=\"DBB\"\n",
				network->getName().c_str(), message->getId(),
				packet->getId(),
				node->getName().c_str(),
				source_buffer->getName().c_str());

		esim_engine->Next(current_event, write_busy - cycle + 1);
		return;
	}

	// Check if the destination buffer is full
	int packet_size = packet->getSize();
	if (destination_buffer->getCount() + packet_size >
			destination_buffer->getSize())
	{
		// Update debug information
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl_full_dst_buf: %s:%s\n",
				network->getName().c_str(),
				message->getId(), packet->getId(),
				destination_buffer->getNode()->getName().c_str(),
				destination_buffer->getName().c_str());

		// Trace information
		System::trace << misc::fmt("net.packet "
                		"net=\"%s\" "
		                "name=\"P-%lld:%d\" "
		                "state=\"%s:%s:Dest_buffer_full\" "
		                "stg=\"DBF\"\n",
		                network->getName().c_str(), message->getId(),
		                packet->getId(),
		                node->getName().c_str(),
		                source_buffer->getName().c_str());

		// Wait for a change in the buffer
		destination_buffer->Wait(current_event);
		return;
	}

	// Calculate latency and occupied resources
	int latency = (packet->getSize() - 1) / bandwidth + 1;
	source_buffer->read_busy = cycle + latency - 1;
	busy = cycle + latency - 1;
	destination_buffer->write_busy = cycle + latency - 1;

	// Transfer message to next input buffer
	source_buffer->ExtractPacket();
	destination_buffer->InsertPacket(packet);
	packet->setNode(destination_buffer->getNode());
	packet->setBuffer(destination_buffer);
	packet->setBusy(cycle + latency - 1);

	// Buffer's trace information
	System::trace << misc::fmt("net.packet_extract net=\"%s\" node=\"%s\" "
			"buffer=\"%s\" name=\"P-%lld:%d\" occpncy=%d\n",
			network->getName().c_str(),
			source_buffer->getNode()->getName().c_str(),
			source_buffer->getName().c_str(),
			message->getId(), packet->getId(),
			source_buffer->getOccupancyInBytes());
	System::trace << misc::fmt("net.packet_insert net=\"%s\" node=\"%s\" "
			"buffer=\"%s\" name=\"P-%lld:%d\" occpncy=%d\n",
			network->getName().c_str(),
			destination_buffer->getNode()->getName().c_str(),
			destination_buffer->getName().c_str(),
			message->getId(), packet->getId(),
			destination_buffer->getOccupancyInBytes());

	// Statistics
	busy_cycles += latency;
	transferred_bytes += packet_size;
	transferred_packets ++;
	source_node->incSentBytes(packet_size);
	source_node->incSentPackets();
	destination_node->incReceivedBytes(packet_size);
	destination_node->incReceivedPackets();

	System::trace << misc::fmt("net.link_transfer net=\"%s\" link=\"%s\" "
			"transB=%lld last_size=%d busy=%lld\n",
			network->getName().c_str(), getName().c_str(),
			transferred_bytes,
			packet->getSize(), busy);

	// Schedule input buffer event	
	esim_engine->Next(System::event_input_buffer, latency);
}

Buffer *Link::VirtualChannelArbitration()
{
	// Get the current cycle and current event
	System *system = System::getInstance();
	long long cycle = system->getCycle();

	// If last decision was made in the current cycle,
	// return the scheduled buffer
	if (scheduled_when == cycle)
		return scheduled_buffer;

	// Make a new decision
	scheduled_when = cycle;

	// Find output buffer to fetch from
	for (unsigned i = 0; i < source_buffers.size(); i++)
	{
		// Index of the next buffer we check to see if it has a packet
		int next_index = (last_scheduled_buffer_index + i + 1) %
				num_virtual_channels;

		// Next buffer we check
		Buffer *buffer = source_buffers[next_index];

		// Get the head packet
		Packet *packet = buffer->getBufferHead();

		// If there is no packet in the buffer, continue
		if (!packet)
			continue;

		// If there is a packet, it should be ready
		if (packet->getBusy() >= cycle)
			continue;

		// See if the source buffer is ready
		if (buffer->read_busy >= cycle)
			continue;

		// All conditioned satisfied
		scheduled_buffer = buffer;
		last_scheduled_buffer_index = next_index;
		return scheduled_buffer;
	}

	// No buffer is scheduled
	scheduled_buffer = nullptr;
	return nullptr;
}

Buffer *Link::getDestinationBufferfromSource(Buffer *buffer)
{
	// Search for the buffer in the link's list of source buffers
	for (unsigned i = 0; i < source_buffers.size(); i++)
		if (buffer == source_buffers[i])
		{
			// If the buffer is found, return the same buffer index in the
			// list of Link's destination buffers
			return destination_buffers[i];
		}

	// Buffer is not found
	return nullptr;
}
}
