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
		for (int j = 0; j < node->getNumOutputBuffers(); j++)
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
			for (int k = 0; k < node_i->getNumOutputBuffers(); k++)
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
}


bool RoutingTable::hasCycle()
{
	// First create an empty graph
	std::unique_ptr<misc::Graph> graph = misc::new_unique<misc::Graph>();

	// Create an unordered_map for mapping each buffer to vertices of
	// the graph
	std::unordered_map<Buffer *, misc::Vertex *> buffer_to_vertex;

	// For every output buffer that plays a role in routing table
	for (int node_id = 0; node_id < dimension; node_id++)
	{
		// Get the node
		Node *node = network->getNode(node_id);

		// For each output buffer from the output buffer list,
		// figure out if the output buffer is in the routing table
		for (int buffer_id = 0; buffer_id < node->getNumOutputBuffers();
				buffer_id++)
		{
			// Get the output buffer from the list in the node 
			Buffer *output_buffer = node->getOutputBuffer(
					buffer_id);

			// Find the buffer in the routing table
			for (int destination_node_id = 0; 
					destination_node_id < dimension;
					destination_node_id++)
			{
				// Get the destination node
				Node *destination_node = network->getNode(
					destination_node_id);

				// Look up the routing table
				Entry *entry = Lookup(node, destination_node);

				// Check if the current output buffer is the 
				// path between the two nodes
				if (output_buffer == entry->getBuffer())
				{
					// This means the buffer is involved 
					// in the graph and might be part of 
					// a deadlock scenario. So we create
					// a vertex for it
					graph->addVertex(misc::new_unique<
							misc::Vertex>
							(output_buffer->getName().
							c_str()));

					// Get the vertex which is the last
					// vertex in the vertices list
					misc::Vertex *vertex = graph->getVertex(
							graph->getNumVertices() - 1);

					// Map the vertex with the buffer for
					// future easy reference
					buffer_to_vertex.insert(std::make_pair(
							output_buffer,
							vertex));
					
					// We break from the for look and move
					// on to the next buffer since we
					// already find the buffer in the table
					break;
				}
			}
		} 
	}

	// For every source node
	for (int source_id = 0; source_id < dimension; source_id++)
	{
		// Get the source node
		Node *source_node = network->getNode(source_id);

		//For every destination node
		for (int destination_id = 0; destination_id< dimension; 
				destination_id++)
		{
			// Get the destination node
			Node *destination_node = network->getNode(
					destination_id);

			// If the source and destination are the same, continue
			if (source_node == destination_node)
				continue;

			// Find the entry in the routing table
			Entry *entry = Lookup(source_node, destination_node);

			// Get the associated output buffer for the entry
			Buffer *source_vertex_buffer = entry->getBuffer();
			if (source_vertex_buffer)
			{
				// Get the next node of the entry
				Node *next_node = entry->getNextNode();

				// Based on the next node perform another 
				// lookup in the routing table to retrieve the 
				// next output buffer (next_node, destination)
				Entry *next_entry = Lookup(next_node, 
						destination_node);

				// Get the associated output buffer for the
				// next entry if any
				Buffer *destination_vertex_buffer = next_entry->
						getBuffer();
				if (destination_vertex_buffer)
				{

					// Find the source vertex based on the 
					// source output buffer in the unordered 
					// map
					auto source_vertex_it = 
							buffer_to_vertex.find(
							source_vertex_buffer);
					assert(source_vertex_it != 
							buffer_to_vertex.end());

					// Find the destination vertex based on 
					// the destination output buffer
					auto destination_vertex_it = 
							buffer_to_vertex.find(
							destination_vertex_buffer);
					assert(destination_vertex_it != 
							buffer_to_vertex.end());

					// First see if the edge exists
					if (!graph->findEdge(source_vertex_it->second,
						destination_vertex_it->second))
					{
						// Add an edge to the graph based 
						// on the source and the destination 
						//vertex
						graph->addEdge(misc::new_unique<
								misc::Edge>
								(source_vertex_it->
								second,
								destination_vertex_it->
								second),
								source_vertex_it->
								second,
								destination_vertex_it->
								second);
					}
				}
			}
		}
	}

	// Run the cycle-detection algorithm on the graph to find cycle
	if (graph->hasCycle())
		return true;

	// No cycle was detected
	return false;	
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


void RoutingTable::UpdateRoute(Node *source, Node *destination,
		Node *next, int virtual_channel)
{
	// Check if source and destination are not the same
	if (source == destination)
		throw Error(misc::fmt("Network %s: route %s.to.%s: "
				"source and destination cannot be the same\n",
				network->getName().c_str(),
				source->getName().c_str(),
				destination->getName().c_str()));

	// Getting the routing table entry
	Entry *entry = Lookup(source, destination);

	// Setting the next node
	entry->setNextNode(next);

	// Exit strategy from the nested loop
	bool route_updated = false;

	// Setting the next buffer
	Buffer *buffer = nullptr;
	for (int i = 0; i < source->getNumOutputBuffers(); i++)
	{
		// Exit the loop if route has already been updated
		if (route_updated == true)
			break;

		// Find a buffer that is connected to next node through a 
		// connection
		buffer = source->getOutputBuffer(i);
		Connection *connection = buffer->getConnection();

		// If the connection is a link, all its destination
		// buffers are connected to the same node.
		if (dynamic_cast<Link *>(connection))
		{
			// Compare link's destination to next node of the route
			Link *link = misc::cast<Link *>(connection);
			if (link->getDestinationNode() == next)
			{
				// Check the value of the virtual channel
				if (virtual_channel > link->getNumVirtualChannels() - 1)
					throw Error(misc::fmt("Network %s: route "
							"%s.to.%s: wrong virtual "
							"channel\n",
							network->getName().c_str(),
							source->getName().c_str(),
							destination->getName().c_str()));

				// The virtual channel acts as an offset in 
				// link's source list
				Buffer *entry_buffer = link->getSourceBuffer(
						virtual_channel);
				assert(entry_buffer->getNode() == source);

				// Update the entry with calculated buffer
				entry->setBuffer(entry_buffer);
				route_updated = true;
				break;
			}
		}
		else
		{
			// No other type of connection would have virtual channels
			if (virtual_channel != 0)
				throw Error(misc::fmt("Network %s: route %s.to.%s:"
						"existing connection does not support "
						"virtual channel",
						network->getName().c_str(),
						source->getName().c_str(),
						destination->getName().c_str()));

			// Find the buffer connected to next node
			for (int j = 0; j < connection->getNumDestinationBuffers(); j++)
			{
				Buffer *destination_buffer = connection->
						getDestinationBuffer(j);
				if (destination_buffer->getNode() == next)
				{
					// Update the entry
					entry->setBuffer(destination_buffer);

					// Exit the nested loop
					route_updated = true;
					break;
				}
			}
		}
	}

	// If the route was not found, the route-step is wrongly provided.
	if (!route_updated)
		throw Error(misc::fmt("Network %s: route %s.to.%s: "
				"missing connection\n",
				network->getName().c_str(),
				source->getName().c_str(),
				destination->getName().c_str()));
}

}
