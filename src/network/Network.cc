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
	// Get event-driven simulator engine, for convenience
	esim_engine = esim::Engine::getInstance();
}


void Network::ParseConfiguration(const std::string &section,
		misc::IniFile &config)
{
	// Non-empty values
	default_output_buffer_size = config.ReadInt(section,
			"DefaultOutputBufferSize", 0);
	default_input_buffer_size = config.ReadInt(section,
			"DefaultInputBufferSize",0);
	default_bandwidth = config.ReadInt(section,
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
	packet_size = config.ReadInt(section, "DefaultPacketSize", 0);

	// FIXME
	// This has to change. We have a general section for
	// frequency. Each Network should be able to have a different frequency
	// or their frequency should be set to the default frequency.
	net_frequency = config.ReadInt(section, "Frequency", 0);

	// Parse the configure file for nodes
	ParseConfigurationForNodes(config);

	// Parse the configure file for links
	ParseConfigurationForLinks(config);

	// Parse the configuration file for Buses
	ParseConfigurationForBuses(config);

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
}


void Network::ParseConfigurationForNodes(misc::IniFile &config)
{
	for (int i = 0; i < config.getNumSections(); i++)
	{
		std::string section = config.getSection(i);
		
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

		// Create string
		std::unique_ptr<Node> node = ProduceNodeByIniSection(
						section, config);
		AddNode(std::move(node));
	}
}


std::unique_ptr<Node> Network::ProduceNodeByIniSection(
			const std::string &section, 
			misc::IniFile &ini_file)
{
	// Verify section name
	std::vector<std::string> tokens;
	misc::StringTokenize(section, tokens, ".");
	if (tokens.size() != 4 &&
			tokens[0] != "Network" &&
			tokens[1] != getName() &&
			tokens[2] != "Node")
		throw misc::Error(misc::fmt("Section %s is not a node", 
					section.c_str()));

	// Get name of the node
	std::string name = tokens[3];

	// Get type string
	std::string type = ini_file.ReadString(section, "Type");

	// Get the node
	return ProduceNode(type, name);
}


std::unique_ptr<Node> Network::ProduceNode(
			const std::string &type, 
			const std::string &name)
{
	std::unique_ptr<Node> node;
	if (!strcmp(type.c_str(), "EndNode"))
	{
		// Produce end node
		node = std::unique_ptr<EndNode>(new EndNode());
	}
	else if (!strcmp(type.c_str(), "Switch"))
	{
		// Produce switch
		node = std::unique_ptr<Switch>(new Switch());
	}
	else
	{
		// Unsupported node type
		throw misc::Error(misc::fmt("Unsupported node type %s\n", 
					type.c_str()));
	};
	assert(node);
	node->setName(name);
	node->setID(getNumNodes());

	return node;
}


void Network::ParseConfigurationForBuses(misc::IniFile &config)
{
	for (int i = 0; i < config.getNumSections(); i++)
	{
		std::string section = config.getSection(i);

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
		if (strcasecmp(tokens[2].c_str(), "Bus"))
			continue;

		// Create string
		ProduceBusByIniSection(section, config);
	}
}


void Network::ProduceBusByIniSection(
			const std::string &section,
			misc::IniFile &ini_file)
{
	// Verify section name
	std::vector<std::string> tokens;
	misc::StringTokenize(section, tokens, ".");
	if (tokens.size() != 4 &&
			tokens[0] != "Network" &&
			tokens[1] != getName() &&
			tokens[2] != "Bus")
		throw misc::Error(misc::fmt("Section %s is not a bus",
					section.c_str()));

	// Get name string
	std::string name = tokens[3];

	// Get the Bus bandwidth
	int bandwidth = ini_file.ReadInt(section, "Bandwidth", default_bandwidth);

	if (bandwidth < 1)
	{
		throw misc::Panic(misc::fmt("Bus %s: Bandwidth cannot "
				"be less than 1",name.c_str()));
	}

	int lanes = ini_file.ReadInt(section,"Lanes", 1);

	if (lanes < 1)
	{
		throw misc::Error(misc::fmt("Bus %s: Bus-lanes cannot be "
				"less than 1",name.c_str()));
	}

	// Produce the bus
	std::unique_ptr<Bus> bus = ProduceBus(name, bandwidth, lanes);

	// Add the bus to the network
	AddBus(std::move(bus));
}


void Network::ParseConfigurationForBusPorts(misc::IniFile &ini_file)
{
	for (int i = 0; i < ini_file.getNumSections(); i++)
	{
		std::string section = ini_file.getSection(i);

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
		ProduceBusPortByIniSection(section, ini_file);
	}
}


void Network::ProduceBusPortByIniSection(
			const std::string &section,
			misc::IniFile &ini_file)
{
	// Verify section name
	std::vector<std::string> tokens;
	misc::StringTokenize(section, tokens, ".");
	if (tokens.size() != 4 &&
			tokens[0] != "Network" &&
			tokens[1] != getName() &&
			tokens[2] != "BusPort")
		throw misc::Error(misc::fmt("Section %s is not a bus",
					section.c_str()));

	// Get the Type
	// Get type string
	std::string type = ini_file.ReadString(section, "Type", "Bidirectional");

	if (((strcasecmp(type.c_str(), "Bidirectional")) &&
			(strcasecmp(type.c_str(), "Sender")) &&
			(strcasecmp(type.c_str(),"Receiver"))))
	{
		throw misc::Panic(misc::fmt("Port: Type %s not recognized",
				type.c_str()));
	}

	// Get The Bus
	std::string bus_name = ini_file.ReadString(section, "Bus");
	if (bus_name == "")
		throw misc::Panic(misc::fmt("Bus name not set in section %s",
					section.c_str()));

	Bus* bus = dynamic_cast<Bus *> (getConnectionByName(bus_name));
	if (!bus)
		throw misc::Panic(misc::fmt("Bus %s not in network",
					bus_name.c_str()));

	// Get the Node
	std::string node_name = ini_file.ReadString(section, "Node");
	if (node_name == "")
		throw misc::Panic(misc::fmt("Node not set in section %s",
					section.c_str()));

	Node *node = getNodeByName(node_name);
	if (!node)
		throw misc::Panic(misc::fmt("Node %s not in network",
					node_name.c_str()));

	// Get the port size
	int buffer_size = ini_file.ReadInt(section, "BufferSize", 0);


	Buffer * buffer;
	if (strcasecmp(type.c_str(), "Sender"))
	{
		buffer_size = (buffer_size == 0 ?
				default_output_buffer_size : buffer_size);
		buffer = node->AddOutputBuffer(buffer_size);
		bus->addSourceBuffer(buffer);
	}
	else if (strcasecmp(type.c_str(), "Receiver"))
	{
		buffer_size = (buffer_size == 0 ?
				default_input_buffer_size : buffer_size);
		buffer = node->AddInputBuffer(buffer_size);
		bus->addDestinationBuffer(buffer);
	}
	else if (strcasecmp(type.c_str(), "Bidirectional"))
	{
		int source_buffer_size = (buffer_size == 0 ?
				default_output_buffer_size : buffer_size);
		buffer = node->AddOutputBuffer(source_buffer_size);
		bus->addSourceBuffer(buffer);

		int destination_buffer_size = (buffer_size == 0 ?
				default_input_buffer_size : buffer_size);
		buffer = node->AddInputBuffer(destination_buffer_size);
		bus->addDestinationBuffer(buffer);
	}
}


std::unique_ptr<Bus> Network::ProduceBus(const std::string &name,
		int bandwidth, int lanes)
{
	std::unique_ptr<Bus> bus = std::unique_ptr<Bus>(new Bus(lanes));
	bus->setName(name);
	bus->setBandwidth(bandwidth);

	return bus;
}


void Network::AddBus(std::unique_ptr<Bus> bus)
{
	connections.push_back(std::move(bus));
}


void Network::ParseConfigurationForLinks(misc::IniFile &ini_file)
{
	for (int i = 0; i < ini_file.getNumSections(); i++)
	{
		std::string section = ini_file.getSection(i);
		
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

		// Create string
		ProduceLinkByIniSection(section, ini_file);
	}
}


bool Network::ParseConfigurationForRoutes(misc::IniFile &ini_file)
{
	return false;
}


void Network::ParseConfigurationForCommands(misc::IniFile &ini_file)
{

}


void Network::addLink(
		const std::string &name,
		Node *source_node,
		Node *dest_node,
		int bandwidth,
		int source_buffer_size,
		int dest_buffer_size,
		int num_virtual_channels)
{
	throw misc::Panic("Not implemented");
}
	
	
void Network::addBidirectionalLink(
		const std::string &name,
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


void Network::ProduceLinkByIniSection(
			const std::string &section, 
			misc::IniFile &ini_file)
{
	// Verify section name
	std::vector<std::string> tokens;
	misc::StringTokenize(section, tokens, ".");
	if (tokens.size() != 4 &&
			tokens[0] != "Network" &&
			tokens[1] != getName() &&
			tokens[2] != "Link")
		throw misc::Error(misc::fmt("Section %s is not a link", 
					section.c_str()));

	// Get name string
	std::string name = tokens[3];

	// Get type string
	std::string type = ini_file.ReadString(section, "Type", "Unidirectional");
	if (((strcasecmp(type.c_str(), "Unidirectional")) &&
			(strcasecmp(type.c_str(),"Bidirectional"))))
	{
		throw misc::Panic(misc::fmt("Link %s: Type %s not recognized",
				name.c_str(), type.c_str()));
	}

	// Get source node
	std::string source_name = ini_file.ReadString(section, "Source");
	if (source_name == "")
		throw misc::Panic(misc::fmt("Source not set in section %s", 
					section.c_str()));
	Node *source_node = getNodeByName(source_name);
	if (!source_node)
		throw misc::Panic(misc::fmt("Source %s not in network", 
					source_name.c_str()));

	// Get destination node
	std::string destination_name = ini_file.ReadString(section, "Dest");
	if (destination_name == "")
		throw misc::Panic(misc::fmt("Destination not set in section %s", 
					section.c_str()));
	Node *destination_node = getNodeByName(destination_name);
	if (!destination_node)
		throw misc::Panic(misc::fmt("Destination %s not in network", 
					destination_name.c_str()));

	// Get the link bandwidth
	int bandwidth = ini_file.ReadInt(section, "Bandwidth", default_bandwidth);
	if (bandwidth < 1)
	{
		throw misc::Panic(misc::fmt("Link %s: Bandwidth cannot "
				"be less than 1",name.c_str()));
	}

	// Get input buffer size -- allowing manual buffer sizes
	int input_buffer_size = ini_file.ReadInt(section, "InputBufferSize",
			default_input_buffer_size);

	// Get output buffer size
	int output_buffer_size = ini_file.ReadInt(section, "OutputBufferSize",
			default_output_buffer_size);

	// Check the buffer sizes
	if ((input_buffer_size < 1) || (output_buffer_size < 1))
	{
		throw misc::Panic(misc::fmt("Link %s: buffer size cannot "
				"be less than 1",name.c_str()));
	}

	// Get the number of virtual channels
	int virtual_channels = ini_file.ReadInt(section,"VC", 1);
	if (virtual_channels < 1)
	{
		throw misc::Error(misc::fmt("Link %s: Virtual Channel cannot be "
				"less than 1",name.c_str()));
	}

	// Add link
	addLink(name,
			source_node,
			destination_node,
			bandwidth, 
			input_buffer_size,
			output_buffer_size,
			virtual_channels);

	// If link is identified as bidirectional add another link with reverse
	// direction.
	if (!misc::StringCaseCompare(type, "Bidirectional"))
		addLink(name,
				destination_node,
				source_node,
				bandwidth, 
				input_buffer_size,
				output_buffer_size,
				virtual_channels);
}


std::unique_ptr<Link> Network::ProduceLink(
			const std::string &name, 
			Node *source_node, 
			Node *destination_node,
			int bandwidth,
			int input_buffer_size,
			int output_buffer_size,
			int virtual_channels)
{
	std::unique_ptr<Link> link = std::unique_ptr<Link>(new Link());
	link->setUserAssignedName(name);
	link->setSourceNode(source_node);
	link->setDestinationNode(destination_node);
	link->setBandwidth(bandwidth);
	link->setVirtualChannels(virtual_channels);

	std::string descriptive_name;
	for (int i= 0; i < virtual_channels; i++)
	{
		auto source_buffer = source_node->AddOutputBuffer(
				output_buffer_size);
		link->addSourceBuffer(source_buffer);
		auto destination_buffer = destination_node->AddInputBuffer(
				input_buffer_size);
		link->addDestinationBuffer(destination_buffer);

		if (i == 0)
			descriptive_name = misc::fmt("link_<%s.%s>_<%s.%s>",
					source_node->getName().c_str(),
					source_buffer->getName().c_str(),
					destination_node->getName().c_str(),
					destination_buffer->getName().c_str());

	}
	link->setName(descriptive_name);

	return link;
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


void Network::Dump(std::ostream &os = std::cout) const
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
}


Message *Network::ProduceMessage(Node *source_node, Node *destination_node,
			int size)
{
	auto message = misc::new_unique<Message>(msg_id_counter,
			source_node, destination_node, size);
	Message *message_ptr = message.get();

	// Increase message id counter
	msg_id_counter++;

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
	long long cycle = esim_engine->getCycle();

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
	if (source_node->getType() != "EndNode")
		throw misc::Panic("Source node is not an end node");
	if (destination_node->getType() != "EndNode")
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
	throw misc::Panic("Not implemented");
}


Switch *Network::addSwitch(int input_buffer_size,
		int output_buffer_size,
		int bandwidth,
		const std::string &name)
{
	throw misc::Panic("Not implemented");
}

}
