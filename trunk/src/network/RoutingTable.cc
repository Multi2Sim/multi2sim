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

RoutingTable::RoutingTable(Network *network) :
		network(network)
{
	this->dimension = network->getNumNodes();
	Initialize();
}


void RoutingTable::Initialize()
{
	// Check if the routing table is already initialized
	if (!entries.empty())
		throw misc::Panic("Routing table already initialized.");

	// Set dimension
	dimension = network->getNumNodes();

	// Initiate table with infinite costs
	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{
			entries.emplace_back(misc::new_unique<Entry>(
					i == j ? 0 : dimension, 
					nullptr, nullptr));
		}
	}

	// Set 1-hop connections
	for (int i = 0; i < dimension; i++)
	{
		Node *node = network->getNode(i);
		for (int j = 0; j < node->getNumOutputBuffer(); j++)
		{
			Buffer *source_buffer = node->getOutputBuffer(j);
			Connection* connection = source_buffer->getConnection();
			for (int k = 0; k < 
					connection->getNumDestinationBuffers();
					k++)
			{
				Buffer *dst_buffer = connection->
					getDestinationBuffer(k);
				Node* dst_node = dst_buffer->getNode();
				if (node != dst_node)
				{
					Entry* entry = Lookup(node, dst_node);
					entry->setCost(1);
					entry->setNextNode(dst_node);
					entry->setBuffer(source_buffer);
				}
			}
		}
	}
}


void RoutingTable::FloydWarshall()
{
	// The entry->next_node values do not necessarily point
	// to the immediate next hop after this.
	for (int k = 0; k < dimension; k++)
	{
		for (int i = 0; i < dimension; i++)
		{
			for (int j = 0; j < dimension; j++)
			{
				Node *node_i = network->getNode(i);
				Node *node_j = network->getNode(j);
				Node *node_k = network->getNode(k);

				Entry *entry_i_k = Lookup(node_i,node_k);
				Entry *entry_k_j = Lookup(node_k,node_j);
				Entry *entry_i_j = Lookup(node_i,node_j);

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
	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{

			Node *node_i = network->getNode(i);
			Node *node_j = network->getNode(j);

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
			for (int k = 0; k < node_i->getNumOutputBuffer(); k++)
			{
				Buffer *buffer = node_i->getOutputBuffer(k);
				bool found = false;
				Connection *connection = buffer->getConnection();
				for (int m = 0; m < connection->
						getNumDestinationBuffers(); 
						m++)
				{
					Buffer *destination_buffer = 
						connection->
						getDestinationBuffer(m);
					Node *receive_node = 
							destination_buffer->
							getNode();
					if (next_node != receive_node)
						continue;
					entry_i_j->setNextNode(receive_node);
					entry_i_j->setBuffer(buffer);
					found = true;
					break;
				}
				if (found)
					break;
			}
		}
	}

	// Look for cycle
	DetectCycle();
}


void RoutingTable::DetectCycle()
{
}


RoutingTable::Entry *RoutingTable::Lookup(Node *source,
		Node *destination) const
{
	int i = source->getIndex();
	int j = destination->getIndex();
	assert((dimension > 0) && (i < dimension) && (j < dimension));

	int location = i * dimension + j;
	return entries.at(location).get();
}


void RoutingTable::Dump(std::ostream &os) const
{
	os << "\t";
	for (int i = 0; i < dimension; i++)
	{
		Node *node = network->getNode(i);
		os << "\t" << node->getName().c_str() << " \t\t";
	}
	os << "\n";

	for (int i = 0; i < dimension; i++)
	{
		Node *node_i = network->getNode(i);
		os << node_i->getName().c_str() << "\t\t";
		for (int j = 0; j < dimension; j++)
		{
			Node *node_j = network->getNode(j);
			Entry *entry = Lookup(node_i,node_j);

			os << entry->getCost() << ":";
			if (entry->getNextNode())
				os << entry->getNextNode()->getName().c_str();
			else
				os << "-" ;
			os << ":\t";
			if (entry->getBuffer())
				os << entry->getBuffer()->getName().c_str();
			else
				os << "-\t";
			os << "\t";
		}
		os << "\n";
	}
	os << "\n";
}

}