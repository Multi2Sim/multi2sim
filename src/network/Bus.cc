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

#include "Bus.h"
#include "Network.h"

namespace net
{


Bus::Bus(Network *network, const std::string &name, 
		int bandwidth, int num_lanes) :
		Connection(name, network)
{
	for (int i = 0; i < num_lanes; i++)
	{
		lanes.emplace_back(misc::new_unique<Lane>(bandwidth));
		Lane *lane = lanes.back().get();
		lane->index = i;
	}
}


void Lane::Dump(std::ostream &os) const
{
	// Dumping lane statistic informations
	os << misc::fmt("BusLane.%d.Bandwidth = %d\n", index, bandwidth);
	os << misc::fmt("BusLane.%d.TransferredPackets = %lld\n", index, 
		transferred_packets);
	os << misc::fmt("BusLane.%d.TransferredBytes = %lld\n", index,
		transferred_bytes);
	os << misc::fmt("BuseLane.%d.BusyCycles = %lld\n", index,
		busy_cycles);

	// Statistics that depends on the cycle
	System *system = System::getInstance();
	long long cycle = system->getCycle();
	os << misc::fmt("BusLane.%d.BytesPerCycle = %0.4f\n", index, cycle ?
			(double) transferred_bytes / cycle : 0.0);
	os << misc::fmt("BusLane.%d.Utilization = %0.4f\n", index, cycle ?
			(double) transferred_bytes / (cycle * bandwidth) : 0.0);
}


void Bus::Dump(std::ostream &os = std::cout) const
{
	// Dumping user assigned name
	os << misc::fmt("[ Network.%s.Bus.%s ]\n", network->getName().c_str(),
			name.c_str());

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
	for (unsigned i = 0 ; i < lanes.size() ; i++)
		lanes[i]->Dump(os);

	// Creating and empty line in dump
	os << "\n";
}


void Bus::TransferPacket(Packet *packet)
{
	// Get the current event
	esim::Engine *esim_engine = esim::Engine::getInstance();
	esim::Event *current_event = esim_engine->getCurrentEvent();

	// Retrieve related information
	Message *message = packet->getMessage();
	Node *node = packet->getNode();
	int packet_size = packet->getSize();

	// Check the packet to make sure it is in an output buffer that connects
	// to this bus
	Buffer *source_buffer = packet->getBuffer();
	if (std::find(source_buffers.begin(), source_buffers.end(),
			source_buffer) == source_buffers.end())
		throw Error(misc::fmt("Packet %lld:%d is not in a source buffer."
				"of the bus",message->getId(), packet->getId()));

	// Check if the packet is on the head of its buffer
	if (source_buffer->getBufferHead() != packet)
	{
		// Update debug information
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl_not_buf_head: %s:%s\n",
				network->getName().c_str(),
				message->getId(), packet->getId(),
				node->getName().c_str(), 
				source_buffer->getName().c_str());

		// Schedule the event for next time buffer head has changed
		source_buffer->Wait(current_event);
		return;
	}

	// Get the next entry in the routing table
	RoutingTable *routing_table = network->getRoutingTable();
	Node *destination_node = message->getDestinationNode();
	RoutingTable::Entry *entry =
			routing_table->Lookup(node, destination_node);
	if (!entry)
		throw misc::Panic(misc::fmt("%s: no route from %s to %s.",
				network->getName().c_str(),
				node->getName().c_str(),
				destination_node->getName().c_str()));

	// Get the next buffer to transmit to, through bus
	Buffer *destination_buffer = nullptr;
	for (Buffer *buffer : destination_buffers)
	{
		if (entry->getNextNode() == buffer->getNode())
		{
			destination_buffer = buffer;
			break;
		}
	}
	if (destination_buffer == nullptr)
		throw misc::Panic(misc::fmt("%s: Next node is not connected "
				"to the bus '%s'.", network->getName().c_str(),
				name.c_str()));

	// Get the current cycle
	System *system = System::getInstance();
	long long cycle = system->getCycle();

	// Check if the destination buffer is not busy
	if (destination_buffer->write_busy >= cycle)
	{
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl_busy_dest_buf: %s:%s\n", 
				network->getName().c_str(),
				message->getId(), packet->getId(),
				destination_buffer->getNode()->getName().c_str(),
				destination_buffer->getName().c_str());
		esim_engine->Next(current_event,
				destination_buffer->write_busy - cycle + 1);
		return;
	}

	// Check if the destination buffer is not full
	if (packet_size > destination_buffer->getSize())
		throw misc::Panic(misc::fmt("%s: Packet does not in node %s of "
				"buffer %s",network->getName().c_str(),
				destination_buffer->getNode()->getName().c_str(),
				destination_buffer->getName().c_str()));
	if (destination_buffer->getCount() + packet_size >
			destination_buffer->getSize())
	{
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl_full_bus_dest_buf: %s - %s:%s\n", 
				network->getName().c_str(),
				message->getId(), packet->getId(),
				name.c_str(),
				destination_buffer->getNode()->getName().c_str(),
				destination_buffer->getName().c_str());
		destination_buffer->Wait(current_event);
		return;
	}

	// Return the next lane that is not busy for the current buffer
	Lane *lane = Arbitration(source_buffer);
	if (!lane)
	{
		System::debug << misc::fmt("net: %s - M-%lld:%d - "
				"stl_bus_arb: %s\n", 
				network->getName().c_str(),
				message->getId(), packet->getId(),
				this->name.c_str());
		esim_engine->Next(current_event, 1);
		return;
	}

	// Calculate latency and occupied resources
	int latency = ((packet_size - 1) / lane->getBandwidth() + 1);
	source_buffer->read_busy = cycle + latency - 1;
	lane->busy = cycle + latency - 1;
	destination_buffer->write_busy = cycle + latency - 1;

	// Transfer packet to the next buffer
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

	// Update the statistics
	lane->incBusyCycles(latency);
	lane->incTransferredBytes(packet_size);
	lane->incTransferredPackets();
	Node *source_node = source_buffer->getNode();
	destination_node = destination_buffer->getNode();
	source_node->incSentBytes(packet_size);
	source_node->incSentPackets();
	destination_node->incReceivedBytes(packet_size);
	destination_node->incReceivedPackets();

	// Schedule next input buffer event
	esim_engine->Next(System::event_input_buffer, latency);
}


Buffer *Bus::LaneArbitration(Lane *lane)
{
	// Get the current cycle
	System *system = System::getInstance();
	long long cycle = system->getCycle();

	// If a decision was made in the current cycle, return the decided buffer
	if (lane->scheduled_when >= cycle)
		return lane->scheduled_buffer;

	// Make a new decision for this lane at this cycle even if it is busy
	// at the moment
	lane->scheduled_when = cycle;
	if (lane->busy >= cycle)
	{
		lane->scheduled_buffer = nullptr;
		return nullptr;
	}

	// Get the last node that had the permission to transmit on the whole bus
	int input_buffer_count = getNumSourceBuffers();
	int last_input_node_index = last_node_index;

	// Find an input buffer to fetch from for this lane
	for (int i = 0; i < getNumSourceBuffers(); i++)
	{
		// Applying round-robin to inputs, starting from last scheduled node
		int input_buffer_index = (last_input_node_index + i + 1)
				% input_buffer_count;
		Buffer *buffer = source_buffers[input_buffer_index];

		// There must be a packet at the head of buffer
		Packet *packet = buffer->getBufferHead();
		if (!packet)
			continue;

		// The packet must be ready
		if (packet->getBusy() >= cycle)
			continue;

		// The buffer must be ready to be read
		if (buffer->read_busy >= cycle)
			continue;

		// Return the scheduled buffer if all conditions where satisfied
		last_node_index = input_buffer_index;
		lane->scheduled_buffer = buffer;
		return buffer;
	}

	// Return null if non of the input buffers are ready
	lane->scheduled_buffer = nullptr;
	return nullptr;

}


Lane *Bus::Arbitration(Buffer *current_buffer)
{
	// The arbitration returns first available lane of the bus
	for (auto &lane : lanes)
	{
		// Arbitrate between existing lanes and sources, and return a lane
		// that is available for the requesting buffer
		if (LaneArbitration(lane.get()) == current_buffer)
			return lane.get();
	}

	// Return null if there are no available lanes for the packet
	return nullptr;
}

}
