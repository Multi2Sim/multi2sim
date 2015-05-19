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
  
class RoutingTable
{
	class RoutingTableEntry
	{
	private:
		// Cost in hops
		int cost;

		// Next node to destination
		Node *next_node = nullptr;

		// Output buffer
		Buffer *output_buffer = nullptr;

	public:
		/// Constructor
		RoutingTableEntry(int cost) :cost {cost} {};

		/// Set the cost
		void setCost(int cost) { this->cost = cost; }

		/// Get the cost
		int getCost() const { return this->cost; }

		/// Set next node
		void setNextNode(Node *node) { this->next_node = next_node; }

		/// Get next node
		Node *getNextNode() const { return this->next_node; }

	};
protected:

	// Associated network
	Network *network;

	// Dimention
	int dim;

	// Entries
	std::vector<std::unique_ptr<RoutingTableEntry>> entries;

	// Flag set when a cycle was detected
	bool has_cycle;

public:

	/// Constructor
	// RoutingTable();

	/// Set the associated network
	void setNetwork(Network *network) { this->network = network; }

	/// Initialize the routing table
	void InitRoutingTable();

	/// Perforam an Floyd-Warshall to find the best routes
	void FloydWarshall();

	/// Look up the entry from a certain node to a certain node
	RoutingTableEntry *Lookup(Node *source, Node *destination);

};

}

#endif
