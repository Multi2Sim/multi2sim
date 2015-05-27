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
		name(name)
{
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

	// Time to create the initial Routing Table
	AddRoutingTable();

	// Parse the routing elements, for manual routing.
	if(!ParseConfigurationForRoutes(config))
		this->routing_table->FloydWarshall();

	// Parse the commands for manual(input trace) injection and testing.
	ParseConfigurationForCommands(config);

	// Debug information
	System::debug << misc::fmt("Network found: %s\n",name.c_str());
}


void Network::AddRoutingTable()
{
	routing_table = misc::new_unique<RoutingTable> (this);
	routing_table->InitRoutingTable();
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
	if (!strcmp(type.c_str(), "EndNode"))
	{
		// Produce end node
		std::unique_ptr<Node> node = 
			std::unique_ptr<EndNode>(new EndNode());
		node->setName(name);
		return node;
	}
	else if (!strcmp(type.c_str(), "Switch"))
	{
		// Produce switch
		std::unique_ptr<Node> node = 
			std::unique_ptr<Switch>(new Switch());
		node->setName(name);
		return node;
	}
	else
	{
		// Unsupported node type
		throw misc::Error(misc::fmt("Unsupported node type %s\n", 
					type.c_str()));
	};

	// Should not get here
	return std::unique_ptr<Node>(nullptr);
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


void Network::AddLink(std::unique_ptr<Link> link)
{
	// Verify if the source and the destination is a node in the list
	std::string src_name = link->getSourceNode()->getName();
	std::string dst_name = link->getDestinationNode()->getName();
	if (!getNodeByName(src_name))
		throw misc::Error(misc::fmt("Source node %s not in network", 
					src_name.c_str()));
	if (!getNodeByName(dst_name))
		throw misc::Error(misc::fmt("Destination node %s not in network",
					dst_name.c_str()));

	// Insert the link
	connections.push_back(std::move(link));
			
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
		throw  misc::Panic(misc::fmt("Link %s: buffer size cannot "
				"be less than 1",name.c_str()));
	}
	// Get the number of virtual channels
	int virtual_channels = ini_file.ReadInt(section,"VC", 1);

	if (virtual_channels < 1)
	{
		throw misc::Error(misc::fmt("Link %s: Virtual Channel cannot be "
				"less than 1",name.c_str()));
	}

	// Get the Link
	std::unique_ptr<Link> link = ProduceLink(name, source_node,
			destination_node, bandwidth, input_buffer_size, output_buffer_size,
			virtual_channels);

	// Add the link to the network
	AddLink(std::move(link));

	// if link is identified as bidirectional add another link with reverse
	// direction.
	if (!(strcasecmp(type.c_str(), "Bidirectional")))
	{
		link = ProduceLink(name, destination_node, source_node, bandwidth,
				output_buffer_size, input_buffer_size, virtual_channels);

		AddLink(std::move(link));
	}
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
		auto source_buffer = source_node->AddOutputBuffer(output_buffer_size);
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
	{
		node->Dump(os);
	}

	// Print links
	for (auto &link : connections)
	{
		link->Dump(os);
	}
}


bool Network::CanSend(Node *source_node, Node *destination_node, int size)
{
	// Get output buffer
	RoutingTable::Entry *entry = routing_table->Lookup(source_node, 
			destination_node);
	Buffer *output_buffer = entry->getBuffer();

	// Check if route exist
	if (!output_buffer)
		return false;

	// All criterion met, return true
	return true;
	
}


EndNode *Network::addEndNode(int input_buffer_size,
		int output_buffer_size,
		const std::string &name,
		void *user_data)
{
	throw misc::Panic("Not implemented");
}

}
