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

#ifndef NETWORK_ROUTINGTABLE_H
#define NETWORK_ROUTINGTABLE_H

#include <vector>
#include <memory>

namespace net
{

class Network;
class Node;
  
class RoutingTable
{
public:

	class Entry
	{

	private:

		// Cost in hops
		int cost;

		// Next node to destination
		Node *next_node = nullptr;

		// Output buffer
		Buffer *buffer = nullptr;

	public:

		/// Constructor.
		Entry(int cost) : cost(cost) { };

		/// Set the routing cost.
		void setCost(int cost) { this->cost = cost; }

		/// Get the routing cost.
		int getCost() const { return cost; }

		/// Set entry's next node.
		void setNextNode(Node *next_node) { this->next_node = next_node; }

		/// Get entry's next node.
		Node *getNextNode() const { return next_node; }

		/// Set entry's next buffer.
		void setBuffer(Buffer *buffer) { this->buffer = buffer; }

		/// Get entry's next buffer.
		Buffer *getBuffer() const { return buffer; }
	};

private:

	// Associated network
	Network *network;

	// Dimension
	int dimension;

	// Entries
	std::vector<std::unique_ptr<Entry>> entries;

	// Flag set when a cycle was detected
	bool has_cycle;

public:

	/// Constructor
	RoutingTable(Network *network) : network(network) { }

	/// Get Dimension
	int getDimension() const { return dimension; }

	/// Initialize the routing table based on the nodes and links present
	/// in the network. This does not set up the routes, it just initializes
	/// the table structures.
	void Initialize();

	/// Perform a Floyd-Warshall to find the best routes
	void FloydWarshall();

	/// Look up the entry from a certain node to a certain node
	virtual Entry *Lookup(Node *source, Node *destination);

	/// Cycle detection mechanism in the routing table.
	void DetectCycle();

	/// Dump Routing table information.
	void Dump(std::ostream &os);

};

}

#endif
