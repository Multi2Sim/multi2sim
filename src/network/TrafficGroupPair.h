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

#ifndef NETWORK_TRAFFICGROUPPAIR_H
#define NETWORK_TRAFFICGROUPPAIR_H

namespace net
{

class TrafficGroupPair
{

	// Group of source node
	std::vector<Node *> source_nodes;

	// Group of destination node
	std::vector<Node *> destination_nodes;

public:

	// Add source node
	void AddSourceNode(Node *node) 
	{
		source_nodes.push_back(node);
	}

	// Add destination node
	void AddDestinationNode(Node *node)
	{
		destination_nodes.push_back(node);
	}

	// Get number of source nodes
	unsigned int getNumberSourceNode() const
	{
		return source_nodes.size();
	}

	// Get number of destination nodes
	unsigned int getNumberDestinationNode() const
	{
		return destination_nodes.size();
	}

	// Get source by index
	Node *getSourceNodeByIndex(unsigned int index) const
	{
		return source_nodes.at(index);
	}

	// Get destination by index
	Node *getDestinationNodeByIndex(unsigned int index) const
	{
		return destination_nodes.at(index);
	}
};

}

#endif
