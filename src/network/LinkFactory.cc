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

#include "Network.h"
#include "LinkFactory.h"

namespace net
{

std::unique_ptr<Link> LinkFactory::ProduceLinkByIniSection(Network *network,
		const std::string &section, misc::IniFile &config)
{
	// Verify section name
	std::vector<std::string> tokens;
	misc::StringTokenize(section, tokens, ".");
	if (tokens.size() != 4 &&
			tokens[0] != "Network" &&
			tokens[1] != network->getName() &&
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
	Node *source_node = network->getNodeByName(source_name);

	// Get destination node
	std::string destination_name = config.ReadString(section, "Dest");
	if (destination_name == "")
		throw misc::Panic(misc::fmt("Destination not set in section %s", 
					section.c_str()));
	Node *destination_node = network->getNodeByName(destination_name);

	// Get the node
	return ProduceLink(network, name, source_node, destination_node);
}


std::unique_ptr<Link> LinkFactory::ProduceLink(Network *network,
		const std::string &name, Node *source_node, 
		Node *destination_node)
{
	std::unique_ptr<Link> link = std::unique_ptr<Link>(new Link());
	link->setName(name);	
	link->setSourceNode(source_node);
	link->setDestinationNode(destination_node);
	return link;
}

}
