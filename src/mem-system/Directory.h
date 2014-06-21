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

#ifndef MEM_SYSTEM_DIRECTORY_H
#define MEM_SYSTEM_DIRECTORY_H

#include <cassert>

#include <lib/cpp/Bitmap.h>
#include <lib/cpp/Misc.h>


namespace mem
{

/// A cache directory in the memory system
class Directory
{
public:

	/// Value set to an owner identifier to represent no owner
	static const int NoOwner = -1;

	/// Directory entry
	class Entry
	{
		// Owner identifier
		int owner = NoOwner;

		// Number of sharers
		int num_sharers = 0;

	public:

		/// Return owner identifier
		int getOwner() const { return owner; }

		/// Return number of sharers
		int getNumSharers() const { return num_sharers; }

		/// Set new owner
		void setOwner(int owner) { this->owner = owner; }
	};

private:

	// Name of directory
	std::string name;

	// Directory dimensions
	int num_sets;
	int num_ways;
	int num_subblocks;
	int num_nodes;

	// Bitmap of sharers for the entire directory
	misc::Bitmap sharers;

	// Directory entries
	std::unique_ptr<Entry> entries;

public:

	/// Constructor
	///
	/// \param name
	///	Name of directory, used for debug purposes
	///
	/// \param num_sets
	///	Number of sets in the directory
	///
	/// \param num_ways
	///	Number of ways per set
	///
	/// \param num_subblocks
	///	Number of sub-blocks per way
	///
	/// \param num_nodes
	///	Number of nodes that can be sharers of each sub-block
	Directory(const std::string &name,
			int num_sets,
			int num_ways,
			int num_subblocks,
			int num_nodes);

	/// Return a directory entry
	Entry *getEntry(int set_id, int way_id, int subblock_id)
	{
		assert(misc::inRange(set_id, 0, num_sets - 1));
		assert(misc::inRange(way_id, 0, num_ways - 1));
		assert(misc::inRange(subblock_id, 0, num_subblocks - 1));
		return &entries.get()[set_id * num_ways * num_subblocks +
				way_id * num_subblocks +
				subblock_id];
	}

	/// Set new owner for the directory entry
	void setOwner(int set_id, int way_id, int subblock_id, int owner);
};


}  // namespace mem

#endif

