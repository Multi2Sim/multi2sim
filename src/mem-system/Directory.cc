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

}  // namespace mem

