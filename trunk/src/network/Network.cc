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

	// Parse the commands for manual(input trace) injection and testing.
	ParseConfigurationForCommands(config);

	// Debug information
	System::debug << misc::fmt("Network found: %s\n",name.c_str());
	Dump(std::cout);
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
			if (input_buffer_size <= System::message_size)
			{
				if(packet_size != 0)
					end_node_input_buffer_size =
							((System::message_size
							- 1)/
							packet_size + 1) *
							packet_size;
				else
					end_node_input_buffer_size =
							System::message_size;
			}

			if (output_buffer_size <= System::message_size)
			{
				if(packet_size != 0)
					end_node_output_buffer_size =
							((System::message_size
							- 1)/
							packet_size + 1) *
							packet_size;
				else
					end_node_output_buffer_size =
							System::message_size;
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

			// Get destination node
			std::string dst_name = ini_file->ReadString(section,
					"Dest");
			Node *destination = getNodeByName(dst_name);

			// Get number of virtual channels
			int num_virtual_channel = ini_file->ReadInt(section,
					"VC");

			// Bandwidth
			int bandwidth = ini_file->ReadInt(section, "Bandwidth",
					default_bandwidth);

			// Add link
			this->addBidirectionalLink(tokens[3],
					source, destination,
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


void Network::ParseConfigurationForCommands(misc::IniFile *ini_file)
{

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


Message *Network::ProduceMessage(Node *source_node, Node *destination_node,
			int size)
{
	auto message = misc::new_unique<Message>(message_id_counter,
			source_node, destination_node, size);
	Message *message_ptr = message.get();

	// Increase message id counter
	message_id_counter++;

	// Insert the created message into the hashtable
	this->message_table.emplace(message->getId(), std::move(message));

	// Return the pointer
	return message_ptr;
}


bool Network::CanSend(Node *source_node, Node *destination_node, int size)
{
	// Get output buffer
	RoutingTable::Entry *entry = routing_table.Lookup(source_node, 
			destination_node);
	Buffer *output_buffer = entry->getBuffer();

	// Check if route exist
	if (!output_buffer)
		return false;

	// Get current cycle
	long long cycle = esim::Engine::getInstance()->getCycle();

	// Check if output buffer is busy
	if (output_buffer->getWriteBusy() >= cycle)
		return false;

	// Check if output buffer is large enough
	int required_size = size;
	if (packet_size != 0)
		required_size = ((size - 1) / packet_size + 1) * packet_size;
	if (output_buffer->getCount() + required_size >
			output_buffer->getSize())
			return 0;

	// All criterion met, return true
	return true;
	
}


void Network::Send(Node *source_node, Node *destination_node, int size,
			esim::EventType *receive,
			esim::EventFrame *receive_frame)
{
	// Make sure both source node and destination node are end nodes
	if (dynamic_cast<EndNode *>(source_node) != nullptr)
		throw misc::Panic("Source node is not an end node");
	if (dynamic_cast<EndNode *>(destination_node) != nullptr)
			throw misc::Panic("Source node is not an end node");

	// Create message
	Message *message = ProduceMessage(source_node, destination_node, size);

	// TODO: generate trace here

	// Packetize message
	if (packet_size == 0)
		message->Packetize(size);
	else
	{
		message->Packetize(packet_size);
		// TODO: generate trace here
	}

	// Send the message out
	message->Send();
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
