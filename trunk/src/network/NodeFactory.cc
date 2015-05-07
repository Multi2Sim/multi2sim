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

#include <memory>

#include "Node.h"
#include "EndNode.h"

#include "NodeFactory.h"

namespace net 
{

std::unique_ptr<Node> NodeFactory::ProduceNodeByIniSection(Network *network,
		const std::string &section, misc::IniFile &config)
{
	// Verify network
	std::string section_copy = section + ".";
	char *token;
	token = strtok((char *)section_copy.c_str(), ".");
	assert(!strcmp("Network", token));

	// Verify network name matches
	token = strtok(NULL, ".");
	assert(!strcmp(network->getName().c_str(), token));

	// Verity node
	token = strtok(NULL, ".");
	assert(!strcmp("Node", token));

	// Get name of the node
	token = strtok(NULL, ".");
	std::string name = token;

	// Get type string
	std::string type = config.ReadString(section, "Type");

	// Get the node
	return ProduceNode(network, type, name);
}


std::unique_ptr<Node> NodeFactory::ProduceNode(Network *network, 
		const std::string &type, const std::string &name)
{
	if (!strcmp(type.c_str(), "EndNode"))
	{
		std::unique_ptr<Node> node = 
			std::unique_ptr<EndNode>(new EndNode());
		node->setName(name);
		return node;
	}
	else
	{
		throw misc::Error(misc::fmt("Unsupported node type %s\n", 
					type.c_str()));
	};

	// Should not get here
	return std::unique_ptr<Node>(nullptr);
}

}  // namespace net
