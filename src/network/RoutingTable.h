/*
 *  Multi2Sim
 *  Copyright (C) 2016 Amir Kavyan Ziabari (aziabari@ece.neu.edu)
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

#ifndef NETWORK_ROUTINGTABLE_H
#define NETWORK_ROUTINGTABLE_H

#include <vector>
#include <memory>

namespace net
{

class Network;
class Node;
class Buffer;
  
class RoutingTable
{
public:

	class Entry
	{
	public:

		// Cost in hops
		int cost;

	private:

		// Next node to destination
		Node *next_node = nullptr;

		// Output buffer
		Buffer *buffer = nullptr;

	public:

		/// Constructor.
		Entry(int cost, Node *next_node, Buffer *buffer) :
			cost(cost),
			next_node(next_node),
			buffer(buffer)
		{};

		/// Get entry's next node.
		Node *getNextNode() const { return next_node; }

		/// Set next node
		void setNextNode(Node *next_node) 
		{ 
			this->next_node = next_node; 
		}

		/// Get entry's next buffer.
		Buffer *getBuffer() const { return buffer; }

		/// Set the buffer to next node
		void setBuffer(Buffer *buffer) { this->buffer = buffer; }
	};

private:

	// Associated network
	Network *network;

	// Dimension
	int dimension = 0;

	// Entries
	std::vector<std::unique_ptr<Entry>> entries;

public:

	/// Constructor
	RoutingTable(Network *network);

	/// Get Dimension
	int getDimension() const { return dimension; }

	/// Initialize the routing table based on the nodes and links present
	/// in the network. This does not set up the routes, it just initializes
	/// the table structures.
	void Initialize();

	/// Perform a Floyd-Warshall to find the best routes
	void FloydWarshall();

	/// Look up the entry from a certain node to a certain node
	Entry *Lookup(Node *source, Node *destination) const;

	/// Generating the route file
	void DumpRoutes(const std::string &path);

	/// Dump Routing table information.
	void Dump(std::ostream &os = std::cout) const;

	/// Check if the routing table has cycle
	bool hasCycle();

	/// Update a route manually. This function is used for adding route-steps.
	/// Route-step is an element in the list of connections that provide the
	/// path between two nodes in manual routing. Each routes requires the
	/// source and destination of the route, as well as the next node which
	/// lies within the path.
	///
	/// \param source
	///			Identifies the source node of the route-step
	///
	/// \param destination
	///			Identifies the destination end-node of the route-step.
	///
	/// \param next
	///			Identifies the next node within the path of the route-step.
	///
	/// \param virtual_channel
	///			Identifies the virtual_channel specific for the route-step,
	///			if the next connection within the path is a link.
	///
	void UpdateRoute(Node *source,
			Node *destination,
			Node *next,
			int virtual_channel);

	/// Update the cost of the paths in the routing table based on 
	/// all the routes that are created
	void UpdateManualRoutingCost();

};

}

#endif
