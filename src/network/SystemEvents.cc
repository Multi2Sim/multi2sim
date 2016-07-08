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

#include <lib/esim/Engine.h>

#include "Packet.h"
#include "Message.h"
#include "Network.h"
#include "Switch.h"
#include "EndNode.h"
#include "Frame.h"
#include "RoutingTable.h"
#include "System.h"

namespace net
{

esim::Event *System::event_send;
esim::Event *System::event_output_buffer;
esim::Event *System::event_input_buffer;
esim::Event *System::event_receive;


void System::EventTypeSendHandler(esim::Event *event,
		esim::Frame *frame)
{
	// Useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *network_frame = misc::cast<Frame *>(frame);

	// Get the network related field from the event
	Packet *packet = network_frame->getPacket();
	Message *message = packet->getMessage();
	Network *network = message->getNetwork();
	Node *source_node = message->getSourceNode();
	Node *destination_node = message->getDestinationNode();

	// For fix delay go around the network
	if (network->hasConstantLatency())
	{
		// Debug Information
		debug << misc::fmt("net: %s - M-%lld:%d - "
				"fix_lat=%d\n",
				network->getName().c_str(),
				message->getId(),
				packet->getId(),
				network->getFixLatency());

		// Update the network related statistics
		source_node->incSentBytes(packet->getSize());
		source_node->incSentPackets();
		destination_node->incReceivedBytes(packet->getSize());
		destination_node->incReceivedPackets();
		packet->setNode(destination_node);
		esim_engine->Next(event_receive,
				network->getFixLatency());
		return;
	}

	// Lookup route from routing table
	RoutingTable *routing_table = network->getRoutingTable();
	RoutingTable::Entry *entry = routing_table->Lookup(
			source_node,
			destination_node);
	Buffer *output_buffer = entry->getBuffer();
	if (!output_buffer)
		throw misc::Panic(misc::fmt("%s: no route from "
				"%s to %s.",
				network->getName().c_str(),
				source_node->getName().c_str(),
				destination_node->getName().c_str()));

	// Check if buffer fits the message
	if (message->getSize() > output_buffer->getSize())
		throw misc::Panic(misc::fmt("%s: message does not "
				"fit in buffer.\n", __FUNCTION__));

	// Check if the buffer is too full to fit the new message
	if (output_buffer->getCount() + packet->getSize() > 
			output_buffer->getSize())
		throw misc::Panic(misc::fmt("%s: output buffer full.", 
				__FUNCTION__));

	// Insert in output buffer (1 cycle latency)
	System *system = getInstance();
	long long cycle = system->getCycle();
	output_buffer->InsertPacket(packet);
	output_buffer->write_busy = cycle;
	packet->setNode(source_node);
	packet->setBuffer(output_buffer);
	packet->setBusy(cycle);

	// Update trace with buffer information
	System::trace << misc::fmt("net.packet_insert "
			"net=\"%s\" node=\"%s\" buffer=\"%s\" "
			"name=\"P-%lld:%d\" occpncy=%d\n",
			network->getName().c_str(),
			output_buffer->getNode()->getName().c_str(),
			output_buffer->getName().c_str(),
			message->getId(), packet->getId(),
			output_buffer->getOccupancyInBytes());

	// Schedule next event
	esim_engine->Next(event_output_buffer, 1);
}
	

void System::EventTypeOutputBufferHandler(esim::Event *event,
		esim::Frame *frame)
{
	// Cast event frame type
	Frame *network_frame = misc::cast<Frame *>(frame);

	// Lookup route from routing table
	Packet *packet = network_frame->getPacket();
	Buffer *buffer = packet->getBuffer();

	// Let the connection to pass the packet to input buffer
	Connection *connection = buffer->getConnection();
	connection->TransferPacket(packet);
}


void System::EventTypeInputBufferHandler(esim::Event *event,
		esim::Frame *frame)
{
	// Get esim engine
	esim::Engine *esim_engine = esim::Engine::getInstance();
	
	// Cast event frame type
	Frame *network_frame = misc::cast<Frame *>(frame);

	// Lookup route from routing table
	Packet *packet = network_frame->getPacket();
	Buffer *buffer = packet->getBuffer();
	Node *node = packet->getNode();
	Message *message = packet->getMessage();
	Network *network = message->getNetwork();

	// If this is the destination node, schedule receive event
	if (node == packet->getMessage()->getDestinationNode())
	{
		esim_engine->Next(event_receive);
		return;
	}

	// If the message is not at buffer head, process later
	if (buffer->getBufferHead() != packet)
	{
		// Debug info
		debug << misc::fmt("net: %s - M-%lld:%d -"
				"stl_not_buf_head: %s:%s\n",
				network->getName().c_str(),
				message->getId(),
				packet->getId(),
				node->getName().c_str(),
				buffer->getName().c_str());

		// Schedule event for later
		buffer->Wait(event);
		return;
	}

	// If this not destination, current node must be a switch 
	Switch *switch_node = dynamic_cast<Switch *>(node);
	if (switch_node == nullptr)
		throw Error("Message can only pass through "
				"switch nodes");

	// Switch forward the packet
	switch_node->Forward(packet);
}


void System::EventTypeReceiveHandler(esim::Event *event,
		esim::Frame *frame)
{
	// Cast event frame type
	Frame *network_frame = misc::cast<Frame *>(frame);

	// Get esim engine
	esim::Engine *esim_engine = esim::Engine::getInstance();

	// Lookup route from routing table
	Packet *packet = network_frame->getPacket();
	Message *message = packet->getMessage();
	EndNode *node = dynamic_cast<EndNode *>(packet->getNode());
	Network *network = message->getNetwork();

	// Check if the message arrived at an end node
	if (!node)
		throw misc::Panic("The message is not arriving at an end node");

	// Check if the packet can be assembled 
	if (message->Assemble(packet))
	{
		{
			// Produce the depacketize in the trace, if message
			// was packetized
			if (message->getNumPackets() > 1)
				System::trace << misc::fmt("net.msg net=\"%s\" "
						"name=\"M-%lld\" "
						"state=\"%s:depacketize\"\n",
						network->getName().c_str(),
						message->getId(),
						node->getName().c_str());

			// Receive the message just when there is
			// no return event
			if (network_frame->automatic_receive)
				network->Receive(node, message);
			else
				esim_engine->Return();
		}
	}
}

}

