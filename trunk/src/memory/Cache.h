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

#ifndef MEMORY_CACHE_H
#define MEMORY_CACHE_H

#include <memory>

#include <lib/cpp/List.h>
#include <lib/cpp/String.h>


namespace mem
{

class Cache
{
public:

	/// Possible values for block replacement policy
	enum ReplacementPolicy
	{
		ReplacementInvalid,
		ReplacementLRU,
		ReplacementFIFO,
		ReplacementRandom
	};

	/// String map for ReplacementPolicy
	static misc::StringMap replacement_policy_map;

	/// Possible values for write policy
	enum WritePolicy
	{
		WriteInvalid,
		WriteBack,
		WriteThrough
	};

	/// String map for WritePolicy
	static misc::StringMap write_policy_map;

	/// Possible values for a cache block state
	enum BlockState
	{
		BlockInvalid,
		BlockNonCoherent,
		BlockModified,
		BlockOwned,
		BlockExclusive,
		BlockShared
	};

	/// String map for BlockState
	static misc::StringMap block_state_map;

	/// Cache block. This class is a child of misc::List::Node because one
	/// block will belong to one set's LRU list. See documentation of
	/// misc::List::Node for details.
	class Block : public misc::List::Node
	{
		unsigned tag;
		unsigned transient_tag;
		unsigned way;
		bool prefetched;
		BlockState state;
	};

	/// Cache set
	class Set
	{
		// List of blocks in LRU order
		misc::List lru;

		// Position in Cache::blocks where the blocks start for this set
		Block *blocks;
	};

private:

	// Name of the cache, used for debugging purposes
	std::string name;

	// Cache geometry
	unsigned num_sets;
	unsigned num_ways;
	unsigned block_size;
	
	// Mask used to get the block address
	unsigned block_mask;

	// Log base 2 of the block size
	int log_block_size;

	// Block replacement policy
	ReplacementPolicy replacement_policy;

	// Write policy (write-back, write-through)
	WritePolicy write_policy;

	// Array of sets
	std::unique_ptr<Set> sets;

	// Array of blocks
	std::unique_ptr<Block> blocks;

public:


};

}  // namespace mem

#endif

