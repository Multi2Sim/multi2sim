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
			entries.emplace_back(misc::new_unique<Entry>
				(i == j ? 0 : INT_MAX));
		}
	}

	// Set 1-hop connections
	for (int i = 0; i < dimension; i++)
	{
		Network *network = this->network;

		Node *node = network->getNodeByIndex(i);
		for (auto &source_buffer : node->getOutputBuffers())
		{
			Connection* connection = source_buffer->getConnection();
			for (auto &dst_buffer : connection->getDestinationBuffers())
			{
				Node* dst_node = dst_buffer->getNode();

				if (node != dst_node)
				{
					Entry* entry = Lookup(node, dst_node);
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
	int i,j,k;
	// The entry->next_node values do not necessarily point
	// to the immediate next hop after this.
	for (i = 0; i < dimension; i++)
	{
		for (j = 0; j < dimension; j++)
		{
			for (k = 0; k < dimension; k++)
			{
				Node *node_i = network->getNodeByIndex(i);
				Node *node_j = network->getNodeByIndex(j);
				Node *node_k = network->getNodeByIndex(k);

				auto entry_i_k = Lookup(node_i,node_k);
				auto entry_k_j = Lookup(node_k,node_j);
				auto entry_i_j = Lookup(node_i,node_j);

				int temp_cost = entry_i_k->getCost() +
						entry_k_j->getCost();
				if (entry_i_j->getCost() > temp_cost)
				{
					entry_i_j->setCost(temp_cost);
					entry_i_j->setNextNode(node_k);
				}
			}
		}
	}

	// Calculating the output buffer for the entry.
	Node *next_node = nullptr;
	Entry *entry = nullptr;

	for (i = 0; i < dimension; i++)
	{
		for (j = 0; j < dimension; j++)
		{

			Node *node_i = network->getNodeByIndex(i);
			Node *node_j = network->getNodeByIndex(j);

			auto entry_i_j = Lookup(node_i, node_j);
			next_node = entry_i_j->getNextNode();

			if (!next_node)
			{
				entry_i_j->setBuffer(nullptr);
				continue;
			}

			// Follow the buffers path
			for (;;)
			{
				entry = Lookup(node_i, next_node);
				if (entry->getCost() <= 1)
					break;
				next_node = entry->getNextNode();
			}

			// Get the output buffer based on the next node
			for (auto &buffer : node_i->getOutputBuffers())
			{
				Connection *connection = buffer->getConnection();
				for (auto &destination_buffer :
						connection->getDestinationBuffers())
				{
					Node *receive_node = destination_buffer->getNode();
					if (next_node != receive_node)
						continue;
					entry_i_j->setNextNode(receive_node);
					entry_i_j->setBuffer(buffer.get());
					goto found;
				}
			}
		}
	}
	found:
	// Look for cycle
	DetectCycle();
}

void RoutingTable::DetectCycle()
{
}


RoutingTable::Entry *RoutingTable::Lookup(Node *source,
		Node *destination)
{
	int dimension = network->getNumberNodes();
	int i = source->getID();
	int j = destination->getID();
	assert((dimension > 0) && (i < dimension) && (j < dimension));

	int location = i * dimension + j;
	return entries.at(location).get();
}


void RoutingTable::Dump()
{
	// The dump will be eddited to delicately print the table to a file
	for (auto i = 0; i < dimension; i++)
	{
		for (auto j = 0; j < dimension; j++)
		{
			Node *node_i = network->getNodeByIndex(i);
			Node *node_j = network->getNodeByIndex(j);
			Entry *entry_i_j = Lookup(node_i,node_j);
			if (entry_i_j->getBuffer())
			{
				fprintf(stderr,"%s --> %s = (%s:%s)\n", 
					node_i->getName().c_str(),
					node_j->getName().c_str(),
					entry_i_j->getNextNode()->
					getName().c_str(),
					entry_i_j->getBuffer()->
					getName().c_str());
			}
		}
	}
}

}
