/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <climits>

#include <lib/cpp/Error.h>

#include "Node.h"
#include "Network.h"
#include "RoutingTable.h"

namespace net
{
class Buffer;
class Connection;

void RoutingTable::InitRoutingTable()
{
	// Check if the routing table is already initiated
	if (!entries.empty())
		throw misc::Panic("Routing table already initialized.");

	// Set dimension
	dimension = network->getNumberNodes();

	// Initiate table with infinite costs
	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{

			entries.emplace_back(std::unique_ptr<RoutingTableEntry>
			(new RoutingTableEntry(i == j ? 0 : INT_MAX)));

		}
	}

	// Set 1-hop connections
	for (int i = 0; i < dimension; i++)
	{
		Network *network = this->network;

		Node *node = network->getNodeByIndex(i);

		for (auto &source_buffer : node->getOutputBufferList())
		{
			fprintf(stderr,"source buffer is %s\n",source_buffer->getName().c_str());
			Connection* connection = source_buffer->getConnection();

			for (auto &dst_buffer : connection->getDestinationBuffers())
			{
				Node* dst_node = dst_buffer->getNode();

				if (node != dst_node)
				{
					RoutingTableEntry* entry = Lookup(node, dst_node);
					entry->setCost(1);
					entry->setNextNode(dst_node);
					entry->setBuffer(source_buffer.get());
				}

			}
		}
	}
}

void RoutingTable::FloydWarshall()
{

}

RoutingTable::RoutingTableEntry *RoutingTable::Lookup(Node *source,
		Node *destination)
{
	int dimension = network->getNumberNodes();
	int i = source->getID();
	int j = destination->getID();
	assert((dimension > 0) && (i < dimension) && (j < dimension));

    int location = i * dimension + j;
    return entries.at(location).get();
}
}

