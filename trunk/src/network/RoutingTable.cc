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

void RoutingTable::InitRoutingTable()
{
	// Check if the routing table is already initiated
	if (!entries.empty())
		throw misc::Panic("Routing table already initialized.");

	// Set dimension
	dim = network->getNumberNodes();

	// Initiate table with infinite costs
	for (int i = 0; i < dim; i++)
	{
		for (int j = 0; j < dim; j++)
		{

			entries.emplace_back(std::unique_ptr<RoutingTableEntry>
			(new RoutingTableEntry(i == j ? 0 : INT_MAX)));

		}
	}
}

RoutingTable::RoutingTableEntry *RoutingTable::Lookup(Node *source,
		Node *destination)
{
	int dimension = network->getNumberNodes();
	int i = source->getID();
	int j = destination->getID();
	assert((dimension > 0) && (i < dimension) && (j < dimension));

    int location = i * dimension + j;
    return entries[location].get();
}
}

