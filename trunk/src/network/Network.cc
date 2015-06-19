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

#include <cstring>
#include <csignal>

#include <lib/esim/Engine.h>

#include "Buffer.h"
#include "Bus.h"
#include "Connection.h"
#include "EndNode.h"
#include "Network.h"
#include "RoutingTable.h"
#include "Switch.h"
#include "Frame.h"

namespace net
{


Network::Network(const std::string &name) :
		name(name),
		routing_table(this)
{
}


void Network::ParseConfiguration(misc::IniFile *config,
		const std::string &section)
{
	// Non-empty values
	default_output_buffer_size = config->ReadInt(section,
			"DefaultOutputBufferSize", 0);
	default_input_buffer_size = config->ReadInt(section,
			"DefaultInputBufferSize",0);
	default_bandwidth = config->ReadInt(section,
			"DefaultBandwidth",0);
	if (!default_output_buffer_size || !default_input_buffer_size ||
			!default_bandwidth)
	{
		throw misc::Error(misc::fmt(
				"%s:%s:\nDefault values can not be "
				"zero/non-existent.\n", __FILE__, 
				__FUNCTION__));
	}

	// Packet size and frequency
	packet_size = config->ReadInt(section, "DefaultPacketSize", 0);

	// FIXME
	// This has to change. We have a general section for
	// frequency. Each Network should be able to have a different frequency
	// or their frequency should be set to the default frequency.
	net_frequency = config->ReadInt(section, "Frequency", 0);

	// Parse the configure file for nodes
	ParseConfigurationForNodes(config);

	// Parse the configure file for links
	ParseConfigurationForLinks(config);

	// Parse the configuration file for Buses
	// ParseConfigurationForBuses(config);

	// Parse the configuration file for Bus ports
	ParseConfigurationForBusPorts(config);

	// Time to create the initial routing table
	routing_table.Initialize();

	// Parse the routing elements, for manual routing.
	if (!ParseConfigurationForRoutes(config))
		routing_table.FloydWarshall();
}


void Network::ParseConfigurationForNodes(misc::IniFile *config)
{
	for (int i = 0; i < config->getNumSections(); i++)
	{
		std::string section = config->getSection(i);
		
		// Tokenize section name
		std::vector<std::string> tokens;
		misc::StringTokenize(section, tokens, ".");

		// Check section name
		if (tokens.size() != 4)
			continue;
		if (strcasecmp(tokens[0].c_str(), "Network"))
			continue;
		if (strcasecmp(tokens[1].c_str(), name.c_str()))
			continue;
		if (strcasecmp(tokens[2].c_str(), "Node"))
			continue;

		// Get name
		std::string node_name = tokens[3];

		// Get properties
		std::string type = config->ReadString(section, "Type");
		int input_buffer_size = config->ReadInt(section,
				"InputBufferSize", default_input_buffer_size);
		int output_buffer_size = config->ReadInt(section,
				"OutputBufferSize", default_output_buffer_size);
		int bandwidth = config->ReadInt(section, "BandWidth",
				default_bandwidth);

		// Create node
		if (!strcasecmp(type.c_str(), "EndNode"))
		{
			int end_node_input_buffer_size = input_buffer_size;
			int end_node_output_buffer_size = output_buffer_size;

			// End-node should be able to contain an entire msg or
			// or equivalent number of packets for that message.
			if (input_buffer_size <= System::getMessageSize())
			{
				if(packet_size != 0)
					end_node_input_buffer_size =
							((System::getMessageSize()
							- 1)/
							packet_size + 1) *
							packet_size;
				else
					end_node_input_buffer_size =
							System::getMessageSize();
			}

			if (output_buffer_size <= System::getMessageSize())
			{
				if(packet_size != 0)
					end_node_output_buffer_size =
							((System::getMessageSize()
							- 1)/
							packet_size + 1) *
							packet_size;
				else
					end_node_output_buffer_size =
							System::getMessageSize();
			}
			addEndNode(end_node_input_buffer_size,
					end_node_output_buffer_size,
					node_name, NULL);
		}
		else if (!strcasecmp(type.c_str(), "Switch"))
		{
			addSwitch(input_buffer_size, output_buffer_size,
					bandwidth, node_name);
		}
		else
		{
			throw misc::Panic(misc::fmt("Node type %s not "
					"supported.", type.c_str()));
		}
	}
}


void Network::ParseConfigurationForBusPorts(misc::IniFile *ini_file)
{
	for (int i = 0; i < ini_file->getNumSections(); i++)
	{
		std::string section = ini_file->getSection(i);

		// Tokenize section name
		std::vector<std::string> tokens;
		misc::StringTokenize(section, tokens, ".");

		// Check section name
		if (tokens.size() != 4)
			continue;
		if (strcasecmp(tokens[0].c_str(), "Network"))
			continue;
		if (strcasecmp(tokens[1].c_str(), name.c_str()))
			continue;
		if (strcasecmp(tokens[2].c_str(), "BusPort"))
			continue;

		// Create string
		// ProduceBusPortByIniSection(section, ini_file);
	}
}


void Network::ParseConfigurationForLinks(misc::IniFile *ini_file)
{
	for (int i = 0; i < ini_file->getNumSections(); i++)
	{
		// Tokenize section name
		std::string section = ini_file->getSection(i);
		std::vector<std::string> tokens;
		misc::StringTokenize(section, tokens, ".");

		// Check section name
		if (tokens.size() != 4)
			continue;
		if (strcasecmp(tokens[0].c_str(), "Network"))
			continue;
		if (strcasecmp(tokens[1].c_str(), name.c_str()))
			continue;
		if (strcasecmp(tokens[2].c_str(), "Link"))
			continue;

		// Get type and create by type
		std::string type = ini_file->ReadString(section, "Type");
		if (!strcasecmp(type.c_str(), "Bidirectional"))
		{
			// Get source node
			std::string src_name = ini_file->ReadString(section,
					"Source");
			Node *source = getNodeByName(src_name);
			if (!source)
				throw Error(misc::fmt("%s: %s: invalid node name",
						section.c_str(),
						src_name.c_str()));

			// Get destination node
			std::string dst_name = ini_file->ReadString(section,
					"Dest");
			Node *destination = getNodeByName(dst_name);
			if (!destination)
				throw Error(misc::fmt("%s: %s: invalid node name",
						section.c_str(),
						dst_name.c_str()));

			// Get number of virtual channels
			int num_virtual_channel = ini_file->ReadInt(section,
					"VC");

			// Bandwidth
			int bandwidth = ini_file->ReadInt(section, "Bandwidth",
					default_bandwidth);

			// Add link
			addBidirectionalLink(tokens[3],
					source,
					destination,
					bandwidth,
					default_input_buffer_size,
					default_output_buffer_size,
					num_virtual_channel);
		}
		else
		{
			throw misc::Panic(misc::fmt("Node type %s not "
					"supported.", type.c_str()));
		}

	}
}


bool Network::ParseConfigurationForRoutes(misc::IniFile *ini_file)
{
	return false;
}


void Network::addBidirectionalLink(const std::string name,
		Node *source_node,
		Node *dest_node,
		int bandwidth,
		int source_buffer_size,
		int dest_buffer_size,
		int num_virtual_channels)
{
	addLink(name,
			source_node,
			dest_node,
			bandwidth,
			source_buffer_size,
			dest_buffer_size,
			num_virtual_channels);
	addLink(name,
			dest_node,
			source_node,
			bandwidth,
			source_buffer_size,
			dest_buffer_size,
			num_virtual_channels);
}


Node *Network::getNodeByName(const std::string &name) const
{
	for (auto &node : nodes)
	{
		if (node->getName() == name)
		{
			return node.get();
		}
	}
	return nullptr;
}


Connection *Network::getConnectionByName(const std::string &name) const
{
	for (auto &connection : connections)
	{
		if (connection->getName() == name)
		{
			return connection.get();
		}
	}
	return nullptr;
}


Node *Network::getNodeByUserData(void *user_data) const
{
	// Nothing for null user data
	if (!user_data)
		return nullptr;
	
	// Search node
	for (auto &node : nodes)
		if (node->getUserData() == user_data)
			return node.get();
	
	// Not found
	return nullptr;
}


void Network::Dump(std::ostream &os) const
{
	// Dump network information
	os << misc::fmt("\n***** Network %s *****\n", name.c_str());
	os << misc::fmt("\tDefault input buffer size: %d\n", 
			default_input_buffer_size);
	os << misc::fmt("\tDefault output buffer size: %d\n", 
			default_output_buffer_size);
	os << misc::fmt("\tDefault bandwidth: %d\n", 
			default_bandwidth);

	// Print node information
	for (auto &node : nodes)
		node->Dump(os);

	// Print links
	for (auto &link : connections)
		link->Dump(os);

	// Print routing table
	this->routing_table.Dump(os);	
}


Message *Network::newMessage(EndNode *source_node, EndNode *destination_node,
			int size)
{
	// Insert the created message into the hashtable
	message_table.emplace(message_id_counter, misc::new_unique<Message>(
				message_id_counter, this, source_node, 
				destination_node, size));

	// Get the pointer to the newly created messag
	Message *message = message_table[message_id_counter].get();

	// Increase message id counter
	message_id_counter++;

	// Return the pointer
	return message;
}


bool Network::CanSend(EndNode *source_node,
		EndNode *destination_node,
		int size,
		esim::Event *retry_event)
{
	// If 'retry_event' was specified, we must be in an event handler
	esim::Engine *esim_engine = esim::Engine::getInstance();
	assert(!retry_event || esim_engine->getCurrentEvent());

	// Get output buffer
	RoutingTable::Entry *entry = routing_table.Lookup(source_node, 
			destination_node);
	Buffer *output_buffer = entry->getBuffer();

	// Check if route exist
	if (!output_buffer)
		throw Error(misc::fmt("No route from '%s' to '%s'",
				source_node->getName().c_str(),
				destination_node->getName().c_str()));

	// Get current cycle
	System *system = System::getInstance();
	long long cycle = system->getCycle();

	// Check if output buffer is busy
	if (output_buffer->getWriteBusy() >= cycle)
	{
		if (retry_event)
			esim_engine->Next(retry_event,
					output_buffer->getWriteBusy()
					- cycle + 1);
		return false;
	}

	// Check if output buffer is large enough
	int required_size = size;
	if (packet_size != 0)
		required_size = ((size - 1) / packet_size + 1) * packet_size;
	if (output_buffer->getCount() + required_size >
			output_buffer->getSize())
	{
		if (retry_event)
			output_buffer->Wait(retry_event);
		return false;
	}

	// All criteria met, return true
	return true;
}


Message *Network::Send(EndNode *source_node,
		EndNode *destination_node,
		int size,
		esim::Event *receive_event)
{
	System::debug << misc::fmt("[Network] Send %d bytes from "
			"'%s' to '%s'\n",
			size,
			source_node->getName().c_str(),
			destination_node->getName().c_str());

	// Get esim engine
	esim::Engine *esim_engine = esim::Engine::getInstance();
	esim_engine->Next(receive_event, 1);

	// Create message
	Message *message = newMessage(source_node, destination_node, size);

	// Packetize message
	if (packet_size == 0)
		message->Packetize(size);
	else 
	{
		// TODO generate trace here
		message->Packetize(packet_size);
	}

	// Send the message out
	for (int i = 0; i < message->getNumPacket(); i++)
	{
		Packet *packet = message->getPacket(i);
		
		// Create event frame
		auto frame = misc::new_shared<Frame>(packet, receive_event);

		// Schedule send event
		esim_engine->Call(System::event_send, frame);
	}

	// Return message
	return message;
}


Message *Network::TrySend(EndNode *source_node,
		EndNode *destination_node,
		int size,
		esim::Event *receive_event,
		esim::Event *retry_event)
{
	// Check if message can be sent
	if (!CanSend(source_node, destination_node, size, retry_event))
		return nullptr;

	// Send message
	return Send(source_node, destination_node, size, receive_event);
}


void Network::Receive(EndNode *node, Message *message)
{
	// Assert that the location of the packets are in the receive node
	for (int i = 0; i < message->getSize(); i++)
	{
		Packet *packet = message->getPacket(i);
		Node *packet_node = packet->getNode();
		if (packet_node != node)
			throw misc::Panic(misc::fmt(
					"Packet %d of the message %lld has "
					"not arrived.", packet->getSessionId(), 
					message->getId()));
	}

	// Remove packets from their buffer
	for (int i = 0; i < message->getSize(); i++)
	{
		Packet *packet = message->getPacket(i);
		Buffer *buffer = packet->getBuffer();
		buffer->RemovePacket(packet);
	}

	// Destory the message
	message_table.erase(message->getId());
}


EndNode *Network::addEndNode(int input_buffer_size,
		int output_buffer_size,
		const std::string &name,
		void *user_data)
{
	// Create end node
	nodes.emplace_back(misc::new_unique<EndNode>(this,
			nodes.size(),
			input_buffer_size,
			output_buffer_size,
			name,
			user_data));
	EndNode *node = misc::cast<EndNode *>(nodes.back().get());
	num_end_nodes++;
	return node;
}


Switch *Network::addSwitch(int input_buffer_size,
		int output_buffer_size,
		int bandwidth,
		const std::string &name)
{
	// Create switch
	nodes.emplace_back(misc::new_unique<Switch>(this,
			nodes.size(),
			input_buffer_size,
			output_buffer_size,
			bandwidth,
			name,
			nullptr));
	Switch *node = misc::cast<Switch *>(nodes.back().get());
	return node;
}


Link *Network::addLink(
		const std::string name,
		Node *source_node,
		Node *dest_node,
		int bandwidth,
		int source_buffer_size,
		int dest_buffer_size,
		int num_virtual_channels)
{
	// Create link
	connections.emplace_back(misc::new_unique<Link>(this,
			name,
			source_node,
			dest_node,
			bandwidth,
			source_buffer_size,
			dest_buffer_size,
			num_virtual_channels));
	Link *link = misc::cast<Link *>(connections.back().get());

	// Create buffers
	// source_node->addInputBuffer(source_buffer_size, link);
	// dest_node->addOutputBuffer(dest_buffer_size, link);

	// Return
	return link;
}

}
