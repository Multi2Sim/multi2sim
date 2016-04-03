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
#include <fstream>

#include <lib/esim/Engine.h>

#include "Buffer.h"
#include "Bus.h"
#include "Connection.h"
#include "EndNode.h"
#include "Frame.h"
#include "Network.h"
#include "RoutingTable.h"
#include "Switch.h"

namespace net
{


static const char *err_cycle_detected =
	"\tA cycle is detected in the graph representing the routing table"
	"for the network. Routing cycles can cause deadlocks in simulations,"
	"that can in turn make the simulation stall with no output.";

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
	bool ideal = config->ReadBool(section, "Ideal", false);
	fix_latency = config->ReadInt(section, "FixLatency", 0);

	// In case both ideal and fix latency variables are set
	if (ideal && fix_latency)
	{
		throw Error(misc::fmt("%s: An ideal network is a network"
				"with a fix latency of 1. Both "
				"variables cannot be set at the same "
				"time for the network %s\n%s",
				config->getPath().c_str(),
				name.c_str(),
				System::err_config_note));
	}
	else if (ideal && (!fix_latency || fix_latency == 1))
	{
		fix_latency = 1;
	}

	// Throw an error if fix latency is not correct
	// otherwise throw a warning saying a lot of components are
	// ineffective
	if (fix_latency < 0)
	{
		throw Error(misc::fmt("%s: Network %s cannot have a "
				"negative fix latency",
				config->getPath().c_str(),
				name.c_str()));
	}

	// Print a warning in case constant network is used
	if (fix_latency > 0)
	{
		misc::Warning("Network %s: Simulator is using "
				"a network with a constant latency. Many of "
				"the network components (such as links and buses) "
				"and the topology they form, are now "
				"ineffective.", name.c_str());
	}

	// Throw an error if default values are not set
	if (!default_output_buffer_size || !default_input_buffer_size ||
			!default_bandwidth)
		throw misc::Error(misc::fmt(
				"%s: %s:\nDefault values can not be "
				"zero/non-existent.\n%s", 
				config->getPath().c_str(),
				name.c_str(), System::err_config_note));

	// Packet size
	packet_size = config->ReadInt(section, "DefaultPacketSize", 0);

	if ((default_output_buffer_size < 0) || 
			(default_input_buffer_size < 0) ||
			(default_bandwidth < 0) || 
			(packet_size < 0))
		throw misc::Error(misc::fmt(
				"%s: %s:\nDefault values can not be "
				"negative.\n%s", config->getPath().c_str(),
				name.c_str(), System::err_config_note));

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

	// If the network with current routing contains a cycle, warn
	if (routing_table.hasCycle())
		misc::Warning("Network %s: Cycle found in the "
				"routing table.\n%s", name.c_str(),
				err_cycle_detected);
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

		// Make sure node is not duplicate
		if (getNodeByName(node_name) != nullptr)
			throw Error(misc::fmt("%s: Node '%s' already "
					"exists in the network\n", 
					config->getPath().c_str(),
					node_name.c_str()));

		// Get properties
		std::string type = config->ReadString(section, "Type");
		int input_buffer_size = config->ReadInt(section,
				"InputBufferSize", default_input_buffer_size);
		int output_buffer_size = config->ReadInt(section,
				"OutputBufferSize", default_output_buffer_size);
		int bandwidth = config->ReadInt(section, "BandWidth",
				default_bandwidth);

		// Error if the buffer sizes are wrong
		if ((input_buffer_size < 1) || (output_buffer_size < 1) ||
				(bandwidth < 1))
			throw Error(misc::fmt("%s: Invalid argument for Node "
					"'%s'\n%s", config->getPath().c_str(),
					node_name.c_str(), 
					System::err_config_note));

		// Enforcing type
		config->Enforce(section, "Type");

		// Create node
		if (!strcasecmp(type.c_str(), "EndNode"))
		{
			// End-node should be able to contain an entire msg
			// or equivalent number of packets for that message.
			int required_buffer_size = 0;

			if (packet_size != 0)
				required_buffer_size =
						(((System::getMessageSize() - 1) 
						/ packet_size + 1) *
						packet_size);
			else
				required_buffer_size =
						System::getMessageSize();

			if (input_buffer_size < required_buffer_size ||
					output_buffer_size < required_buffer_size)
				throw Error(misc::fmt("%s: Buffer size on the " 
						"end node should be able to " 
						"fit at least a whole message, "
						"or all the packets of the "
						"message",
						config->getPath().c_str()));

			addEndNode(input_buffer_size,
					output_buffer_size,
					node_name, NULL);
		}
		else if (!strcasecmp(type.c_str(), "Switch"))
		{
			addSwitch(input_buffer_size, output_buffer_size,
					bandwidth, node_name);
		}
		else
		{
			throw Error(misc::fmt("%s: Node type '%s' is not "
					"supported.\n%s",
					config->getPath().c_str(), type.c_str(),
					System::err_config_note));
		}
	}
}


void Network::ParseConfigurationForBuses(misc::IniFile *ini_file)
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
		if (strcasecmp(tokens[2].c_str(), "Bus"))
			continue;

		// Get the bus name
		std::string name = tokens[3];

		// Get the bandwidth
		int bandwidth = ini_file->ReadInt(section, "Bandwidth",
				default_bandwidth);
		if (bandwidth < 1)
			throw Error(misc::fmt("%s: Bus '%s', bandwidth cannot "
					"be zero/negative.\n", 
					ini_file->getPath().c_str(),
					name.c_str()));

		// Get the number of lanes
		int lanes = ini_file->ReadInt(section, "Lanes", 1);
		if (lanes < 1)
			throw Error(misc::fmt("%s: Bus '%s', number of lanes "
					"cannot be zero/negative.\n",
					ini_file->getPath().c_str(),
					name.c_str()));

		// Create a new Bus
		addBus(name, bandwidth, lanes);
	}
}


Bus *Network::addBus(const std::string name, int bandwidth, int lanes)
{
	// Create bus
	connections.emplace_back(misc::new_unique<Bus>(this,
			name,
			bandwidth,
			lanes));
	Bus *bus = misc::cast<Bus *>(connections.back().get());

	// Return
	return bus;
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

		// Get the bus name
		std::string bus_name = ini_file->ReadString(section, "Bus");
		if (bus_name == "")
			throw Error(misc::fmt("%s: Bus name should be "
					"included for the port.",
					ini_file->getPath().c_str()));

		// Find the bus
		Bus *bus = dynamic_cast<Bus *>(getConnectionByName(bus_name));
		if (!bus)
			throw Error(misc::fmt("%s: Bus '%s' does not exist "
					"in the network.", 
					ini_file->getPath().c_str(),
					bus_name.c_str()));

		// Get the node's name
		std::string node_name = ini_file->ReadString(section, "Node");
		if (node_name == "")
			throw Error(misc::fmt("%s: Section [%s]: Node is "
					"not set for the busport.", 
					ini_file->getPath().c_str(),
					section.c_str()));
	
		// Get the node by the name
		Node *node = getNodeByName(node_name);
		if (!node)
			throw Error(misc::fmt("%s: Section [%s]: Node '%s' "
					"does not exist in the network.",
					ini_file->getPath().c_str(),
					section.c_str(), node_name.c_str()));

		// Get port type and adding the buffer(s)
		std::string type = ini_file->ReadString(section, "Type",
				"Bidirectional");
		int buffer_size;
		Buffer *buffer;
		Connection *connection = misc::cast<Connection *>(bus);

		// Setting up the buffer
		// In case the node sends through the port
		if (!strcasecmp(type.c_str(), "Sender"))
		{
			buffer_size = ini_file->ReadInt(section, "BufferSize",
					default_output_buffer_size);
			if (buffer_size < 1)
				throw Error(misc::fmt("%s: Section [%s]: Buffer"
						" size cannot be less than 1",
						ini_file->getPath().c_str(),
						section.c_str()));
			buffer = node->addOutputBuffer(buffer_size, connection);
			bus->addSourceBuffer(buffer);
		}

		// The node only receives through the port
		else if (!strcasecmp(type.c_str(), "Receiver"))
		{
			buffer_size = ini_file->ReadInt(section, "BufferSize",
					default_input_buffer_size);
			if (buffer_size < 1)
				throw Error(misc::fmt("%s: Section [%s]: Buffer"
						" size cannot be less than 1",
						ini_file->getPath().c_str(),
						section.c_str()));
			buffer = node->addInputBuffer(buffer_size, connection);
			bus->addDestinationBuffer(buffer);
		}

		// The node can both send and receive through the port
		else if (!strcasecmp(type.c_str(), "Bidirectional"))
		{
			// Setting the size of the buffer
			buffer_size = ini_file->ReadInt(section, "BufferSize");
			if (buffer_size == 0)
			{
				buffer = node->addOutputBuffer(
						default_output_buffer_size,
						connection);
				bus->addSourceBuffer(buffer);
				buffer = node->addInputBuffer(
						default_input_buffer_size,
						connection);
				bus->addDestinationBuffer(buffer);
			}
			else if (buffer_size >= 1)
			{
				buffer = node->addOutputBuffer(buffer_size,
						connection);
				bus->addSourceBuffer(buffer);
				buffer = node->addInputBuffer(buffer_size,
						connection);
				bus->addDestinationBuffer(buffer);
			}
			else if (buffer_size < 0)
			{
				throw Error(misc::fmt("%s: Section [%s]: "
						"Buffer size cannot be less "
						"than 1.", 
						ini_file->getPath().c_str(),
						section.c_str()));
			}
		}
		else
		{
			throw misc::Error(misc::fmt("%s: Section [%s]: Type "
					"'%s' is not recognized",
					ini_file->getPath().c_str(), 
					section.c_str(),
					type.c_str()));
		}
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

		std::string link_name = tokens[3];

		// Get type and create by type
		std::string type = ini_file->ReadString(section, "Type");

		if (!strcasecmp(type.c_str(), "Bidirectional") ||
				!strcasecmp(type.c_str(), "Unidirectional"))
		{
			// Get source node name
			std::string src_name = ini_file->ReadString(
					section, "Source");
			if (src_name.empty())
				throw Error(misc::fmt("%s: Source node is not "
						"provided for link '%s'.\n",	
						ini_file->getPath().c_str(),
						link_name.c_str()));

			// Get source node using name
			Node *source = getNodeByName(src_name);
			if (!source)
				throw Error(misc::fmt("%s: Source node '%s' is "
						"invalid for link '%s'.\n",
						ini_file->getPath().c_str(),
						src_name.c_str(), 
						link_name.c_str()));

			// Get destination node name
			std::string dst_name = ini_file->ReadString(
						section, "Dest");

			if (dst_name.empty())
				throw Error(misc::fmt("%s: Destination node is "
						"not provided for link '%s'.\n", 
						ini_file->getPath().c_str(),
						link_name.c_str()));

			// Get destination node using name
			Node *destination = getNodeByName(dst_name);
			if (!destination)
				throw Error(misc::fmt("%s: Destination node "
						"'%s' is invalid for link "
						"'%s'.\n",
						ini_file->getPath().c_str(), 
						dst_name.c_str(),
						link_name.c_str()));

			// Make sure link has different source and destination
			if (source == destination)
				throw Error(misc::fmt("%s: Link '%s', source "
						"and destination cannot "
						"be the same.\n", 
						ini_file->getPath().c_str(),
						link_name.c_str()));

			// Bandwidth
			int bandwidth = ini_file->ReadInt(section, "Bandwidth",
					default_bandwidth);
			if (bandwidth < 1)
				throw Error(misc::fmt("%s: Link '%s', "
						"bandwidth cannot be "
						"zero/negative.\n", 
						ini_file->getPath().c_str(),
						link_name.c_str()));

			// Get number of virtual channels
			int num_virtual_channel = ini_file->ReadInt(section,
					"VC", 1);
			if (num_virtual_channel < 1)
				throw Error(misc::fmt("%s: Link '%s', virtual "
						"channel cannot be "
						"zero/negative.\n", 
						ini_file->getPath().c_str(),
						link_name.c_str()));

			// Get the input buffer size for link
			int input_buffer_size = ini_file->ReadInt(section,
					"InputBufferSize", 
					default_input_buffer_size);
			if (input_buffer_size < 1)
				throw Error(misc::fmt("%s: Link '%s', input "
						"buffer size cannot be "
						"zero/negative.\n",
						ini_file->getPath().c_str(),
						link_name.c_str()));

			// Get the output buffer size for link
			int output_buffer_size = ini_file->ReadInt(section,
					"OutputBufferSize",
					default_output_buffer_size);
			if (output_buffer_size < 1)
				throw Error(misc::fmt("%s: Link '%s', output "
						"buffer size cannot be "
						"zero/negative.\n",
						ini_file->getPath().c_str(),
						link_name.c_str()));

			// Add link
			if (!strcasecmp(type.c_str(), "Bidirectional"))
				addBidirectionalLink(link_name,
						source,
						destination,
						bandwidth,
						input_buffer_size,
						output_buffer_size,
						num_virtual_channel);
			else if (!strcasecmp(type.c_str(), "Unidirectional"))
				addLink(link_name,
						source,
						destination,
						bandwidth,
						input_buffer_size,
						output_buffer_size,
						num_virtual_channel);
		}
		else
		{
			throw Error(misc::fmt("%s: Link type '%s' is not "
					"supported.\n%s", 
					ini_file->getPath().c_str(),
					type.c_str(), 
					System::err_config_note));
		}

	}
}


bool Network::ParseConfigurationForRoutes(misc::IniFile *ini_file)
{
	bool routing = false;
	for (int i = 0; i < ini_file->getNumSections(); i++)
	{
		// Tokenize section name
		std::string section = ini_file->getSection(i);
		std::vector<std::string> tokens;
		misc::StringTokenize(section, tokens, ".");

		// Check section name
		if (tokens.size() != 3)
			continue;
		if (strcasecmp(tokens[0].c_str(), "Network"))
			continue;
		if (strcasecmp(tokens[1].c_str(), name.c_str()))
			continue;
		if (strcasecmp(tokens[2].c_str(), "Routes"))
			continue;

		// Set routing to true
		routing = true;

		// Find and update routes
		for (auto &source : nodes)
		{
			for (auto &destination : nodes)
			{
				if (dynamic_cast<EndNode *>(destination.get()))
				{
					// Create a string in the 
					// format of src.to.dst
					std::string route = source->getName() + 
							".to." +
							destination->getName();

					// Look for the new string as a variable 
					// in the section
					std::string destination_string = 
							ini_file->ReadString(
							section, route);

					// If the string in the format exists 
					// in the section
					if (destination_string != "")
					{
						// Tokenize result to 
						// destination:virtual_channel
						std::vector<std::string>
								 string_tokens;
						misc::StringTokenize(
								destination_string,
								string_tokens, ":");
						if (string_tokens.size() > 2)
							throw Error(misc::fmt(
									"Network %s: route "
									"%s.to.%s: wrong format "
									"for next node\n",
									name.c_str(),
									source->getName().c_str(),
									destination->
									getName().c_str()));

						// Get destination node
						Node *next = getNodeByName(
								string_tokens[0]);
						if (!next)
							throw Error(misc::fmt(
									"Network %s: route %s.to.%s: "
									"invalid node name '%s'\n",
									name.c_str(),
									source->getName().c_str(),
									destination->
									getName().c_str(),
									string_tokens[0].c_str()));

						// Get the VC, if any
						int virtual_channel = 0;
						if (string_tokens.size() == 2)
							virtual_channel = atoi(
									string_tokens[1].c_str());
						if (virtual_channel < 0)
							throw Error(misc::fmt("Network %s: route "
									"%s.to.%s: virtual channel cannot "
									"be negative\n",
									name.c_str(),
									source->getName().c_str(),
									destination->getName().c_str()));

						routing_table.UpdateRoute(source.get(),
								destination.get(),
								next, virtual_channel);
					}
				}
			}
		}
		routing_table.UpdateManualRoutingCost();
	}
	return routing;
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


void Network::DumpReport(const std::string &path)
{
	// Open file
	std::string net_path = name + "_" + path;
	std::ofstream f(net_path);
	if (!f)
		throw Error(misc::fmt("%s: cannot open file for write",
				path.c_str()));

	// Dump into file
	DumpReport(f);
}


void Network::StaticGraph(const std::string &path)
{
	// Open file
	std::string graph_path = name + "_" + path;
	std::ofstream f(graph_path);
	if (!f)
		throw Error(misc::fmt("%s: cannot open file for write",
				path.c_str()));

	// Create a graph
	this->graph = misc::new_unique<net::Graph>(this);

	// Get the graph pointer
	Graph *graph = this->graph.get();

	// Calculate the coordinations of vertices for the graph
	graph->LayeredDrawing();

	// Dump the required data into the output file
	graph->DumpGraph(f);
}


void Network::DumpReport(std::ostream &os) const
{
	// Dump network information
	os << misc::fmt("[ Network.%s ]\n", name.c_str());
	os << misc::fmt("Transfers = %lld\n", transfers);
	os << misc::fmt("AverageMessageSize = %0.2f\n", transfers ?
			(double) accumulated_bytes / transfers : 0.0);
	os << misc::fmt("TransferredBytes = %lld\n", accumulated_bytes);
	os << misc::fmt("AverageLatency = %.4f\n", transfers ?
			(double) accumulated_latency / transfers : 0.0);

	// Cycle related information
	System *system = System::getInstance();
	long long cycle = system->getCycle();
	os << misc::fmt("Cycles = %llu\n", cycle);

	// Creating an empty link before starting the links
	os << "\n";

	// Print links
	for (auto &link : connections)
		link->Dump(os);

	// Print node information
	for (auto &node : nodes)
		node->Dump(os);

	// Creating an empty line in the dump before starting the next network
	os << "\n";
}


Message *Network::newMessage(EndNode *source_node, EndNode *destination_node,
		int size)
{
	// Get the current cycle
	System *system = System::getInstance();
	long long cycle = system->getCycle();

	// Insert the created message into the hashtable
	message_table.emplace(message_id_counter, misc::new_unique<Message>(
			message_id_counter, this, source_node,
			destination_node, size, cycle));

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

	// If there is no route, return
	if (!output_buffer)
		return false;

	// Get current cycle
	System *system = System::getInstance();
	long long cycle = system->getCycle();

	// Check if output buffer is busy
	if (output_buffer->write_busy >= cycle)
	{
		if (retry_event)
			esim_engine->Next(retry_event,
					output_buffer->write_busy - cycle + 1);
		return false;
	}

	// Get the least required size in the buffer
	int required_size = size;
	if (packet_size != 0)
		required_size = ((size - 1) / packet_size + 1) * packet_size;

	// Check if the buffer can fit one message
	if (required_size > output_buffer->getSize())
		throw Error("Buffer too small for the "
			"message size.");
	
	// Check if the buffer has enough space for the current message
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
	// Get esim engine
	esim::Engine *esim_engine = esim::Engine::getInstance();

	// Create message
	Message *message = newMessage(source_node, destination_node, size);

	// Updating trace with new message creation
	net::System::trace << misc::fmt("net.new_msg net=\"%s\" "
			"name=\"M-%lld\" size=%d state=\"%s:create\"\n",
			name.c_str(), message->getId(),
			message->getSize(), source_node->getName().c_str());

	// Packetize message
	if (packet_size == 0)
		message->Packetize(size);
	else 
		message->Packetize(packet_size);

	// Updating the trace with the message's packetization information
	net::System::trace << misc::fmt("net.msg net=\"%s\" name=\"M-%lld\" "
			"state=\"%s:packetize\"\n",
			name.c_str(), message->getId(),
			source_node->getName().c_str());

	// Debug information
	System::debug << misc::fmt("net: %s - send M-%lld "
			"'%s'-->'%s'\n",
			name.c_str(),
			message->getId(),
			source_node->getName().c_str(),
			destination_node->getName().c_str());

	// Send the message out
	for (int i = 0; i < message->getNumPackets(); i++)
	{
		Packet *packet = message->getPacket(i);

		// Update the trace with the new packet and its state
		net::System::trace << misc::fmt("net.new_packet net=\"%s\" "
				"name=\"P-%lld:%d\" size=%d state=\"%s:packetizer\"\n",
				name.c_str(), message->getId(),
				packet->getId(), packet->getSize(),
				source_node->getName().c_str());

		// Update the trace with the new packet association
		net::System::trace << misc::fmt("net.packet_msg net=\"%s\" "
				"name=\"P-%lld:%d\" message=\"M-%lld\"\n",
				name.c_str(), message->getId(),
				packet->getId(), message->getId());
		
		// Create event frame
		auto frame = misc::new_shared<Frame>(packet);

		// The packet will be received automatically if the user didn't
		// pass any receive event
		frame->automatic_receive = !receive_event;

		// Schedule send event. The "Execute" function is used instead of 
		// the "Call" function, since the "Call" function schedules the
		// event for later, while "Execute" instantly start the event
		// handling. With "Execute", the packet acquires the space in
		// the buffer, and if the buffer is full, the next packet
		// cannot traverse forward, until enough space is available.
		// However with "Call", the buffer entry is not reserved, so
		// a race condition would occur.
		esim_engine->Execute(System::event_send, frame, receive_event);
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
	// Get output buffer
	RoutingTable::Entry *entry = routing_table.Lookup(source_node, 
			destination_node);
	Buffer *output_buffer = entry->getBuffer();
	
	// Check if route exist
	if (!output_buffer)
		throw Error(misc::fmt("No route from '%s' to '%s'.",
				source_node->getName().c_str(),
				destination_node->getName().c_str()));

	// Check if message can be sent
	if (!CanSend(source_node, destination_node, size, retry_event))
		return nullptr;

	// Send message
	return Send(source_node, destination_node, size, receive_event);
}


void Network::Receive(EndNode *node, Message *message)
{
	// Assert that the location of the packets are in the receive node
	for (int i = 0; i < message->getNumPackets(); i++)
	{
		Packet *packet = message->getPacket(i);
		Node *packet_node = packet->getNode();
		if (packet_node != node)
			throw Error(misc::fmt(
					"Packet %d of the message %lld has "
					"not arrived.", packet->getId(),
					message->getId()));
	}

	// Update network Statistics
	long long cycle = System::getInstance()->getCycle();
	transfers++;
	accumulated_bytes += message->getSize();
	accumulated_latency += cycle - message->getSendCycle();

	// Remove packets from their buffer
	for (int i = 0; i < message->getNumPackets(); i++)
	{
		Packet *packet = message->getPacket(i);
		Buffer *buffer = packet->getBuffer();

		// In the case the network is fixed, there are no
		// buffer insertion and extraction. Otherwise, extract
		// from buffer and report in trace
		if (!hasConstantLatency())
		{
			// Remove the packet from buffer
			buffer->RemovePacket(packet);

			// Updating the trace with extraction of the packet
			// from the buffer
			System::trace << misc::fmt("net.packet_extract "
					"net=\"%s\" node=\"%s\" buffer=\"%s\" "
					"name=\"P-%lld:%d\" occpncy=%d\n",
					name.c_str(),
					buffer->getNode()->getName().c_str(),
					buffer->getName().c_str(),
					message->getId(), packet->getId(),
					buffer->getOccupancyInBytes());
		}

		// Updating the trace with end of packet
		// transmission information
		System::trace << misc::fmt("net.end_packet net=\"%s\" "
				"name=\"P-%lld:%d\"\n",
				name.c_str(), message->getId(),
				packet->getId());
	}

	// Dump debug information
	System::debug << misc::fmt("net: %s - M-%lld rcv'd at %s\n",
			name.c_str(),
			message->getId(),
			node->getName().c_str());

	// Updating the trace with the end of the message
	System::trace << misc::fmt("net.end_msg net=\"%s\" name=\"M-%lld\"\n",
			name.c_str(), message->getId());

	// Destroy the message
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
	// Check if the nodes are not both end-nodes
	if ((dynamic_cast<EndNode *>(source_node))
			&& (dynamic_cast<EndNode *>(dest_node)))
				throw Error(misc::fmt("Network '%s': Link "
				"'%s' cannot connect two end-nodes.", 
				this->name.c_str(),
				name.c_str()));

	// Creating a unique name for unidirectional link
	std::string descriptive_name = "link_" + source_node->getName() +
			"_" + dest_node->getName();

	// Create link
	connections.emplace_back(misc::new_unique<Link>(this,
			name,
			descriptive_name,
			source_node,
			dest_node,
			bandwidth,
			source_buffer_size,
			dest_buffer_size,
			num_virtual_channels));
	Link *link = misc::cast<Link *>(connections.back().get());

	// Return
	return link;
}


void Network::TraceHeader()
{
	// Get instance of the system
	System *net_system = System::getInstance();

	// Dump general network information in trace file
	net_system->trace.Header(misc::fmt("net.create "
			"name=\"%s\" "
			"num_nodes=\"%d\" "
			"packet_size=\"%d\"\n",
			name.c_str(),
			(int) nodes.size(),
			packet_size));

	// Dump information about nodes in trace file
	for (auto &node : nodes)
	{
		if (dynamic_cast<EndNode *>(node.get()))
			net_system->trace.Header(misc::fmt("net.node net_name=\"%s\" "
					"node_index=\"%d\" node_name=\"%s\" node_type=1\n",
					name.c_str(),
					node->getIndex(),
					node->getName().c_str()));
		else if (dynamic_cast<Switch *>(node.get()))
			net_system->trace.Header(misc::fmt("net.node net_name=\"%s\" "
					"node_index=\"%d\" node_name=\"%s\" node_type=2\n",
					name.c_str(),
					node->getIndex(),
					node->getName().c_str()));
		else
			throw misc::Panic("Unknown node type in trace header\n");
	}

	// Dump information about links in trace file
	for (auto &connection : connections)
	{
		Link *link = dynamic_cast<Link *>(connection.get());
		if (link)
			net_system->trace.Header(misc::fmt("net.link net_name=\"%s\" "
					"link_name=\"%s\" src_node=\"%s\" "
					"dst_node=\"%s\" vc_num=%d bw=%d\n",
					name.c_str(), link->getName().c_str(),
					link->getSourceNode()->getName().c_str(),
					link->getDestinationNode()->getName().c_str(),
					link->getNumVirtualChannels(),
					link->getBandwidth()));
		else
			throw misc::Panic("Buses are not supported in the current "
					"version of visualization tool\n");
	}

	// Dump information about buffers in the trace file's header
	for (auto &node : nodes)
	{
		// Update the trace header with information on node's input buffers
		Buffer *buffer = nullptr;
		for (int i = 0; i < node->getNumInputBuffers(); i++)
		{
			buffer = node->getInputBuffer(i);
			net_system->trace.Header(misc::fmt("net.input_buffer "
					"net_name=\"%s\" node_name=\"%s\" buffer_name=\"%s\" "
					"buffer_size=%d buffer_type=1 connection=\"%s\"\n",
					name.c_str(),
					node->getName().c_str(),
					buffer->getName().c_str(),
					buffer->getSize(),
					buffer->getConnection()->getName().c_str()));
		}

		// Update the trace header with information on node's output buffers
		for (int i = 0; i < node->getNumOutputBuffers(); i++)
		{
			buffer = node->getOutputBuffer(i);
			net_system->trace.Header(misc::fmt("net.output_buffer "
					"net_name=\"%s\" node_name=\"%s\" buffer_name=\"%s\" "
					"buffer_size=%d buffer_type=1 connection=\"%s\"\n",
					name.c_str(),
					node->getName().c_str(),
					buffer->getName().c_str(),
					buffer->getSize(),
					buffer->getConnection()->getName().c_str()));
		}
	}
}

}
