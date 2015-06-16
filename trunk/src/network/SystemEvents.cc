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
#include "Frame.h"
#include "RoutingTable.h"
#include "System.h"

namespace net
{

esim::EventType *System::event_type_send;
esim::EventType *System::event_type_output_buffer;
esim::EventType *System::event_type_input_buffer;
esim::EventType *System::event_type_receive;


void System::EventTypeSendHandler(esim::EventType *type, 
		esim::EventFrame *frame)
{
	// Useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *network_frame = misc::cast<Frame *>(frame);

	// Lookup route from routing table
	Packet *packet = network_frame->getPacket();
	Message *message = packet->getMessage();
	Network *network = message->getNetwork();
	Node *source_node = message->getSourceNode();
	Node *destination_node = message->getDestinationNode();
	RoutingTable *routing_table = network->getRoutingTable();
	RoutingTable::Entry *entry = routing_table->Lookup(source_node, 
			destination_node);
	Buffer *output_buffer = entry->getBuffer();
	if (!output_buffer)
		throw misc::Panic(misc::fmt("%s: no route from %s to %s.", 
				network->getName().c_str(),
				source_node->getName().c_str(),
				destination_node->getName().c_str()));

	// Check if buffer fits the message
	if (message->getSize() > output_buffer->getSize())
		throw misc::Panic(misc::fmt("%s: message does not fit in "
				"buffer.\n", __FUNCTION__));

	// Check if the buffer is too full to fit the new message
	if (output_buffer->getCount() + packet->getSize() > 
			output_buffer->getSize())
		throw misc::Panic(misc::fmt("%s: output buffer full.", 
					__FUNCTION__));

	// Insert in output buffer (1 cycle latency)
	System *system = getInstance();
	long long cycle = system->getCycle();
	output_buffer->InsertPacket(packet);
	output_buffer->setWriteBusy(cycle);
	packet->setNode(source_node);
	packet->setBuffer(output_buffer);
	packet->setBusy(cycle);

	// Schedule next event
	esim_engine->Next(event_type_output_buffer, 1);
}
	

void System::EventTypeOutputBufferHandler(esim::EventType *type, 
		esim::EventFrame *frame)
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


void System::EventTypeInputBufferHandler(esim::EventType *type, 
		esim::EventFrame *frame)
{
	// Get esim engine
	esim::Engine *esim_engine = esim::Engine::getInstance();
	
	// Cast event frame type
	Frame *network_frame = misc::cast<Frame *>(frame);

	// Lookup route from routing table
	Packet *packet = network_frame->getPacket();
	Buffer *buffer = packet->getBuffer();
	Node *node = packet->getNode();

	// If the message is not at buffer head, process later
	if (buffer->getBufferHead() == packet)
	{
		buffer->Wait(type);
		return;
	}

	// If this is the destination node, schedule receive event
	if (node == packet->getMessage()->getDestinationNode())
	{
		esim_engine->Next(event_type_receive);
		return;
	}

	// If this not destination, current node must be a switch 
	Switch *switch_node = dynamic_cast<Switch *>(node);
	if (switch_node == nullptr)
		throw Error("Message can only pass through switch nodes");

	// Switch forward the packet
	switch_node->Forward(packet);
}


void System::EventTypeReceiveHandler(esim::EventType *type, 
		esim::EventFrame *frame)
{
	// Cast event frame type
	Frame *network_frame = misc::cast<Frame *>(frame);

	// Lookup route from routing table
	Packet *packet = network_frame->getPacket();
	Message *message = packet->getMessage();
	Node *node = packet->getNode();
	Network *network = message->getNetwork();

	// Check if the packet can be assembled 
	if (message->Assemble(packet))
	{
		network->Receive(node, message);
	}
}

}

