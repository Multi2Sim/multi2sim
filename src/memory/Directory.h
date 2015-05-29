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

#ifndef MEMORY_DIRECTORY_H
#define MEMORY_DIRECTORY_H

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

		/// Increment the number of sharers
		void incNumSharers() { num_sharers++; }

		/// Decrement the number of sharers
		void decNumSharers() { num_sharers--; }

		/// Set the number of sharers
		void setNumSharers(int num_sharers)
		{
			this->num_sharers = num_sharers;
		}
	};

private:

	// Name of directory
	std::string name;

	// Directory dimensions
	int num_sets;
	int num_ways;
	int num_sub_blocks;
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
	/// \param num_sub_blocks
	///	Number of sub-blocks per way
	///
	/// \param num_nodes
	///	Number of nodes that can be sharers of each sub-block
	Directory(const std::string &name,
			int num_sets,
			int num_ways,
			int num_sub_blocks,
			int num_nodes);
	
	/// Return the number of sets
	int getNumSets() { return num_sets; }

	/// Return the number of ways
	int getNumWays() { return num_ways; }

	/// Return number of sub-blocks
	int getNumSubBlocks() { return num_sub_blocks; }

	/// Return the number of nodes that can be sharers of each sub-block
	int getNumNodes() { return num_nodes; }

	/// Return a directory entry
	Entry *getEntry(int set_id, int way_id, int sub_block_id)
	{
		assert(misc::inRange(set_id, 0, num_sets - 1));
		assert(misc::inRange(way_id, 0, num_ways - 1));
		assert(misc::inRange(sub_block_id, 0, num_sub_blocks - 1));
		return &entries.get()[set_id * num_ways * num_sub_blocks +
				way_id * num_sub_blocks +
				sub_block_id];
	}

	/// Set new owner for the directory entry
	void setOwner(int set_id, int way_id, int sub_block_id, int owner);

	/// Activate one sharer for a directory entry
	void setSharer(int set_id, int way_id, int sub_block_id, int node);

	/// Disable one sharer for a directory entry
	void clearSharer(int set_id, int way_id, int sub_block_id, int node);

	/// Clear all sharers of a directory entry
	void clearAllSharers(int set_id, int way_id, int sub_block_id);

	/// Return whether a sharer is present in a directory entry
	bool isSharer(int set_id, int way_id, int sub_block_id, int node_id);

	/// Return whether part of a block is shared or owned
	bool isBlockSharedOrOwned(int set_id, int way_id);

	/// Dump array of sharers of a sub-block into an output stream
	void DumpSharers(int set_id, int way_id, int sub_block_id,
			std::ostream &os = std::cout);
};


}  // namespace mem

#endif

