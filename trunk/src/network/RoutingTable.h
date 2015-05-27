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

#include "RoutingTableEntry.h"

namespace net
{
class Network;
  
class RoutingTable
{
protected:

	// Associated network
	Network *network;

	// Dimension
	int dimension;

	// Entries
	std::vector<std::unique_ptr<RoutingTableEntry>> entries;

	// Flag set when a cycle was detected
	bool has_cycle;

public:

	/// Constructor
	RoutingTable() {};

	/// Virtual destructor
	virtual ~RoutingTable() {};

	/// Set the associated network
	void setNetwork(Network *network) { this->network = network; }

	/// Get Dimension
	int getDimension() const { return dimension; }

	/// Initialize the routing table
	void InitRoutingTable();

	/// Perforam an Floyd-Warshall to find the best routes
	void FloydWarshall();

	/// Look up the entry from a certain node to a certain node
	virtual RoutingTableEntry *Lookup(Node *source, Node *destination);

};

}

#endif
