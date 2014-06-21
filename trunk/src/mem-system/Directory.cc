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

#include <lib/cpp/String.h>

#include "Directory.h"
#include "System.h"


namespace mem
{

const int Directory::NoOwner;


Directory::Directory(const std::string &name,
		int num_sets,
		int num_ways,
		int num_subblocks,
		int num_nodes)
		:
		name(name),
		num_sets(num_sets),
		num_ways(num_ways),
		num_subblocks(num_subblocks),
		num_nodes(num_nodes),
		sharers(num_sets * num_ways * num_subblocks * num_nodes)
{
	// Initialize entries
	entries.reset(new Entry[num_sets * num_ways * num_subblocks]);
}
	

void Directory::setOwner(int set_id, int way_id, int subblock_id, int owner)
{
	// Set owner
	assert(owner == NoOwner || misc::inRange(owner, 0, num_nodes - 1));
	Entry *entry = getEntry(set_id, way_id, subblock_id);
	entry->setOwner(owner);

	// Trace
	System::trace << misc::fmt("mem.set_owner dir=\"%s\" "
			"x=%d y=%d z=%d owner=%d\n",
			name.c_str(),
			set_id,
			way_id,
			subblock_id,
			owner);
}
	

void Directory::setSharer(int set_id, int way_id, int subblock_id, int node_id)
{
	// Sanity
	assert(misc::inRange(set_id, 0, num_sets - 1));
	assert(misc::inRange(way_id, 0, num_ways - 1));
	assert(misc::inRange(subblock_id, 0, num_subblocks - 1));
	assert(misc::inRange(node_id, 0, num_nodes - 1));

	// Get position in bitmap
	int bit_id = set_id * num_ways * num_subblocks * num_nodes +
			way_id * num_subblocks * num_nodes +
			subblock_id * num_nodes +
			node_id;
	
	// Check if already set
	if (sharers[bit_id])
		return;
	
	// Set sharer
	Entry *entry = getEntry(set_id, way_id, subblock_id);
	assert(entry->getNumSharers() < num_nodes);
	entry->incNumSharers();
	sharers.Set(bit_id);
	
	// Trace
	System::trace << misc::fmt("mem.set_sharer dir=\"%s\" "
			"x=%d y=%d z=%d sharer=%d\n",
			name.c_str(),
			set_id,
			way_id,
			subblock_id,
			node_id);
}


void Directory::clearSharer(int set_id, int way_id, int subblock_id, int node_id)
{
	// Sanity
	assert(misc::inRange(set_id, 0, num_sets - 1));
	assert(misc::inRange(way_id, 0, num_ways - 1));
	assert(misc::inRange(subblock_id, 0, num_subblocks - 1));
	assert(misc::inRange(node_id, 0, num_nodes - 1));

	// Get position in bitmap
	int bit_id = set_id * num_ways * num_subblocks * num_nodes +
			way_id * num_subblocks * num_nodes +
			subblock_id * num_nodes +
			node_id;
	
	// Check if already clear
	if (!sharers[bit_id])
		return;
	
	// Clear sharer
	Entry *entry = getEntry(set_id, way_id, subblock_id);
	assert(entry->getNumSharers() > 0);
	entry->decNumSharers();
	sharers.Set(bit_id, false);
	
	// Trace
	System::trace << misc::fmt("mem.clear_sharer dir=\"%s\" "
			"x=%d y=%d z=%d sharer=%d\n",
			name.c_str(),
			set_id,
			way_id,
			subblock_id,
			node_id);
}


void Directory::clearAllSharers(int set_id, int way_id, int subblock_id)
{
	// Skip if no sharer is present
	Entry *entry = getEntry(set_id, way_id, subblock_id);
	if (entry->getNumSharers() == 0)
		return;
	
	// Get position in bitmap
	int bit_id = set_id * num_ways * num_subblocks * num_nodes +
			way_id * num_subblocks * num_nodes +
			subblock_id * num_nodes;
	
	// Clear all sharers
	entry->setNumSharers(0);
	for (int i = 0; i < num_nodes; i++)
		sharers.Set(bit_id + i, false);
	
	// Trace
	System::trace << misc::fmt("mem.clear_all_sharers dir=\"%s\" "
			"x=%d y=%d z=%d\n",
			name.c_str(),
			set_id,
			way_id,
			subblock_id);
}


bool Directory::isSharer(int set_id, int way_id, int subblock_id, int node_id)
{
	// Sanity
	assert(misc::inRange(set_id, 0, num_sets - 1));
	assert(misc::inRange(way_id, 0, num_ways - 1));
	assert(misc::inRange(subblock_id, 0, num_subblocks - 1));
	assert(misc::inRange(node_id, 0, num_nodes - 1));

	// Get position in bitmap
	int bit_id = set_id * num_ways * num_subblocks * num_nodes +
			way_id * num_subblocks * num_nodes +
			subblock_id * num_nodes +
			node_id;
	
	// Return whether sharer is present
	return sharers[bit_id];
}


bool Directory::isBlockSharedOrOwned(int set_id, int way_id)
{
	// Look for an owner or sharer
	for (int subblock_id = 0; subblock_id < num_subblocks; subblock_id++)
	{
		Entry *entry = getEntry(set_id, way_id, subblock_id);
		if (entry->getNumSharers() > 0 || entry->getOwner() != NoOwner)
			return true;
	}

	// No owner or sharer found
	return false;
}


void Directory::DumpSharers(int set_id, int way_id, int subblock_id,
		std::ostream &os)
{
	Entry *entry = getEntry(set_id, way_id, subblock_id);
	os << misc::fmt("  %d sharers: { ", entry->getNumSharers());
	for (int i = 0; i < num_nodes; i++)
		if (isSharer(set_id, way_id, subblock_id, i))
			os << misc::fmt("%d ", i);
	os << "}\n";
}


}  // namespace mem

