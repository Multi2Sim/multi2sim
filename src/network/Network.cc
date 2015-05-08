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

#include "Switch.h"
#include "EndNode.h"
#include "Link.h"
#include "Network.h"

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
	net_frequency = config.ReadInt(section, "Frequency", 0);

	// Parse the configure file for nodes
	ParseConfigurationForNodes(config);

	// Parse the configure file for links
	ParseConfigurationForLinks(config);

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
			misc::IniFile &config)
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
	std::string type = config.ReadString(section, "Type");

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


void Network::ParseConfigurationForLinks(misc::IniFile &config)
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
		if (strcasecmp(tokens[2].c_str(), "Link"))
			continue;

		// Create string
		std::unique_ptr<Link> link = ProduceLinkByIniSection(
						section, config);
		AddLink(std::move(link));
	}
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
		throw misc::Error(misc::fmt("Destinatio node %s not in network", 
					dst_name.c_str()));

	// Insert the link
	links.push_back(std::move(link));
			
}


std::unique_ptr<Link> Network::ProduceLinkByIniSection(
			const std::string &section, 
			misc::IniFile &config)
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
	std::string type = config.ReadString(section, "Type");

	// Get source node
	std::string source_name = config.ReadString(section, "Source");
	if (source_name == "")
		throw misc::Panic(misc::fmt("Source not set in section %s", 
					section.c_str()));
	Node *source_node = getNodeByName(source_name);
	if (!source_node)
		throw misc::Panic(misc::fmt("Source %s not in network", 
					source_name.c_str()));

	// Get destination node
	std::string destination_name = config.ReadString(section, "Dest");
	if (destination_name == "")
		throw misc::Panic(misc::fmt("Destination not set in section %s", 
					section.c_str()));
	Node *destination_node = getNodeByName(destination_name);
	if (!destination_node)
		throw misc::Panic(misc::fmt("Destination %s not in network", 
					destination_name.c_str()));

	// Get the Link
	return ProduceLink(name, source_node, destination_node);
}


std::unique_ptr<Link> Network::ProduceLink(
			const std::string &name, 
			Node *source_node, 
			Node *destination_node)
{
	std::unique_ptr<Link> link = std::unique_ptr<Link>(new Link());
	link->setName(name);	
	link->setSourceNode(source_node);
	link->setDestinationNode(destination_node);
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


Link *Network::getLinkByName(const std::string &name) const
{
	for (auto &link : links)
	{
		if (link->getName() == name)
		{
			return link.get();
		}
	}
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
	for (auto &link : links)
	{
		link->Dump(os);
	}
}

}
