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


void System::EventTypeSendHandler(esim::Event *type, 
		esim::Frame *frame)
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

	// Dump debug information
	debug << misc::fmt("[Network] Send Handler net=\"%s\", "
			"msg-->pkt=%lld-->%d, "
			"source_node=\"%s\", destination_node=\"%s\", "
			"output_buffer=\"%s\"\n",
			network->getName().c_str(), 
			message->getId(), 
			packet->getSessionId(), 
			source_node->getName().c_str(), 
			destination_node->getName().c_str(), 
			output_buffer->getName().c_str());

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
	esim_engine->Next(event_output_buffer, 1);
}
	

void System::EventTypeOutputBufferHandler(esim::Event *type, 
		esim::Frame *frame)
{
	// Cast event frame type
	Frame *network_frame = misc::cast<Frame *>(frame);

	// Lookup route from routing table
	Packet *packet = network_frame->getPacket();
	Message *message = packet->getMessage();
	Buffer *buffer = packet->getBuffer();
	Node *node = packet->getNode();
	Network *network = message->getNetwork();

	// Dump debug information
	debug << misc::fmt("[Network] Output Buffer Event Handler, net=\"%s\", "
			"msg-->pkt=%lld-->%d, node=\"%s\", buf=\"%s\"\n",
			network->getName().c_str(), message->getId(), 
			packet->getSessionId(), node->getName().c_str(), 
			buffer->getName().c_str());
	
	// Let the connection to pass the packet to input buffer
	Connection *connection = buffer->getConnection();
	connection->TransferPacket(packet);
}


void System::EventTypeInputBufferHandler(esim::Event *type, 
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

	// Dump debug information
	debug << misc::fmt("[Network] Input Buffer Event Handler, net=\"%s\", "
			"msg-->pkt=%lld-->%d, node=\"%s\", buf=\"%s\"\n",
			network->getName().c_str(), message->getId(),
			packet->getSessionId(), node->getName().c_str(),
			buffer->getName().c_str());

	// If the message is not at buffer head, process later
	if (buffer->getBufferHead() != packet)
	{
		buffer->Wait(type);
		return;
	}

	// If this is the destination node, schedule receive event
	if (node == packet->getMessage()->getDestinationNode())
	{
		esim_engine->Next(event_receive);
		return;
	}

	// If this not destination, current node must be a switch 
	Switch *switch_node = dynamic_cast<Switch *>(node);
	if (switch_node == nullptr)
		throw Error("Message can only pass through switch nodes");

	// Switch forward the packet
	switch_node->Forward(packet);
}


void System::EventTypeReceiveHandler(esim::Event *type, 
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

	// Dump debug information
	debug << misc::fmt("[Network] Receive Event Handler, net=\"%s\", "
			"msg-->pkt=%lld-->%d, node=\"%s\"\n",
			network->getName().c_str(), message->getId(),
			packet->getSessionId(), node->getName().c_str());

	// Check if the message arrived at an end node
	if (!node)
		throw misc::Panic("The message is not arriving at an end node");

	// Check if the packet can be assembled 
	if (message->Assemble(packet))
	{
		if (network_frame->automatic_receive)
			network->Receive(node, message);
		else 
			esim_engine->Return();
	}
}

}

