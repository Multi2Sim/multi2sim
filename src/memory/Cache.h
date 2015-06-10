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
	static const misc::StringMap ReplacementPolicyMap;

	/// Possible values for write policy
	enum WritePolicy
	{
		WriteInvalid,
		WriteBack,
		WriteThrough
	};

	/// String map for WritePolicy
	static const misc::StringMap WritePolicyMap;

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
	static const misc::StringMap BlockStateMap;

	/// Cache block. This class is a child of misc::List::Node because one
	/// block will belong to one set's LRU list. See documentation of
	/// misc::List::Node for details.
	class Block
	{
		// Only Cache needs to initialize fields
		friend class Cache;

		// Block tag
		unsigned tag = 0;

		// Transient tag assigned by NMOESI protocol
		unsigned transient_tag = 0;

		// Way identifier
		unsigned way_id = 0;

		// Block state
		BlockState state = BlockInvalid;

		// The block belongs to an LRU list
		misc::List<Block>::Node lru_node;
	
	public:

		/// Constructor
		Block() : lru_node(this)
		{
		}

		/// Get the block tag
		unsigned getTag() const { return tag; }

		/// Get the way index of this block
		unsigned getWayId() const { return way_id; }

		/// Get the transient trag set in this block
		unsigned getTransientTag() const { return transient_tag; }

		/// Get the block state
		BlockState getState() const { return state; }

		/// Set new state and tag
		void setStateTag(BlockState state, unsigned tag)
		{
			this->state = state;
			this->tag = tag;
		}
	};

private:

	// Cache set
	class Set
	{
		// Only Cache needs to initialize fields
		friend class Cache;

		// List of blocks in LRU order
		misc::List<Block> lru_list;

		// Position in Cache::blocks where the blocks start for this set
		Block *blocks;
	};

	// Name of the cache, used for debugging purposes
	std::string name;

	// Cache geometry
	unsigned num_sets;
	unsigned num_ways;
	unsigned block_size;

	// Number of sets times number of ways
	unsigned num_blocks;
	
	// Mask used to get the block address
	unsigned block_mask;

	// Log base 2 of the block size
	int log_block_size;

	// Block replacement policy
	ReplacementPolicy replacement_policy;

	// Write policy (write-back, write-through)
	WritePolicy write_policy;

	// Array of sets
	std::unique_ptr<Set[]> sets;

	// Array of blocks
	std::unique_ptr<Block[]> blocks;

	/// Return a pointer to a cache set
	Set *getSet(unsigned set_id)
	{
		assert(misc::inRange(set_id, 0, num_sets - 1));
		return &sets[set_id];
	}

public:

	/// Constructor
	Cache(const std::string &name,
			unsigned num_sets,
			unsigned num_ways,
			unsigned block_size,
			ReplacementPolicy replacement_policy,
			WritePolicy write_policy);
	
	/// Return a pointer to a cache block
	Block *getBlock(unsigned set_id, unsigned way_id) const
	{
		assert(misc::inRange(set_id, 0, num_sets - 1));
		assert(misc::inRange(way_id, 0, num_ways - 1));
		return &blocks[set_id * num_ways + way_id];
	}

	/// Decode a physical address.
	///
	/// \param address
	///	Address to be decoded
	///
	/// \param set_id
	///	Return here the set index for the address.
	///
	/// \param tag
	///	Return here the tag for the address
	///
	/// \param block_offset
	///	Return here the block offset for the address
	///
	void DecodeAddress(unsigned address,
			unsigned &set_id,
			unsigned &tag,
			unsigned &block_offset) const;

	/// Check whether an address is present in the cache.
	///
	/// \param address
	///	Physical address to search for.
	///
	/// \param set_id
	///	If the address was found in the cache, return here the set
	///	index.
	///
	/// \param way_id
	///	If the address was found in the cache, return here the way
	///	index.
	///
	/// \param state
	///	If the address was found in the cache, return here the state of
	///	the block containing the address.
	///
	/// \return
	///	The function returns true if the address was found in the cache
	///	in a block with a valid state.
	bool FindBlock(unsigned address,
			unsigned &set_id,
			unsigned &way_id,
			BlockState &state) const;

	/// Set a new tag and state for a cache block. If a new tag is set to
	/// the block, this function also updates the FIFO counters to indicate
	/// that a new block was brought to the cache.
	///
	/// \param set_id
	///	Set of the block to modify.
	///
	/// \param way_id
	///	Way of the block to modify.
	///
	/// \param tag
	///	New tag for the block
	///
	/// \param state
	///	New state for the block
	void setBlock(unsigned set_id,
			unsigned way_id,
			unsigned tag,
			BlockState state);

	/// Return the tag and the state of a cache block.
	///
	/// \param set_id
	///	Set of the block to query.
	///
	/// \param way_id
	///	Way of the block to query.
	///
	/// \param tag
	///	The tag of the queried block is returned here.
	///
	/// \param state
	///	The state of the queried block is returned here.
	///
	void getBlock(unsigned set_id,
			unsigned way_id,
			unsigned &tag,
			BlockState &state) const;

	/// Mark a block as last accessed as per the LRU policy. This function
	/// internally updates the linked list that keeps track of the LRU order
	/// of the blocks in a set.
	void AccessBlock(unsigned set_id, unsigned way_id);

	/// Return the way index of the block to be replaced in the given set,
	/// as per the current block replacement policy.
	unsigned ReplaceBlock(unsigned set_id);

	/// Set the transient tag of a block.
	void setTransientTag(unsigned set_id, unsigned way_id, unsigned tag)
	{
		Block *block = getBlock(set_id, way_id);
		block->transient_tag = tag;
	}



	//
	// Getters
	//

	/// Return the name of the cache
	const std::string &getName() const { return name; }

	/// Return the number of sets
	unsigned getNumSets() const { return num_sets; }

	/// Return the number of ways
	unsigned getNumWays() const { return num_ways; }

	/// Return the block size
	unsigned getBlockSize() const { return block_size; }

	/// Return the replacement policy
	ReplacementPolicy getReplacementPolicy() const { return replacement_policy; }

	/// Return the write policy
	WritePolicy getWritePolicy() const { return write_policy; }

	/// Return a mask used to extract the bits corresponding to the block
	/// offset of an address.
	unsigned getBlockMask() const { return block_mask; }

	/// Return the log2 of the block size
	int getLogBlockSize() const { return log_block_size; }
};


}  // namespace mem

#endif

