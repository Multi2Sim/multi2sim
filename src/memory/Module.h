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

#ifndef MEMORY_MODULE_H
#define MEMORY_MODULE_H

#include <list>
#include <memory>
#include <unordered_map>

#include <lib/cpp/Misc.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Queue.h>
#include <network/Node.h>

#include "Cache.h"
#include "Directory.h"


// Forward declarations
namespace net { class Network; }
namespace net { class Node; }


namespace mem
{

// Forward declarations
class Frame;


/// Memory module, representing a cache or a main memory module.
class Module
{
public:

	/// Module types
	enum Type
	{
		TypeInvalid = 0,
		TypeCache,
		TypeMainMemory,
		TypeLocalMemory
	};

	/// Address range
	enum RangeType
	{
		RangeInvalid = 0,
		RangeBounds,
		RangeInterleaved
	};

	/// Module access types
	enum AccessType
	{
		AccessInvalid = 0,
		AccessLoad,
		AccessStore,
		AccessNCStore
	};

private:

	// Port in a memory module
	class Port
	{
		// Port lock status
		bool locked = false;

		// Cycle when port was last locked
		long long lock_when = 0;

		// Memory access locking port
		std::shared_ptr<Frame> frame;

		// Memory accesses waiting in port
		esim::Queue queue;
	};



	//
	// Module properties
	//

	// Name of the module
	std::string name;

	// Module type
	Type type;

	// Block size for caches
	int block_size;

	// Log base 2 of block size
	int log_block_size = 0;

	// Latency for data access in cycles
	int data_latency = 1;

	// Directory access latency
	int directory_latency = 1;

	// Number of entries in the MSHR register
	int mshr_size = 1;

	// Cache level, where 1 is closest to processors
	int level = 0;
	
	// Smallest block size among higher modules (modules closer to the
	// processor). If there is no higher module, the sub-block size is
	// equal to the block size.
	int sub_block_size = 0;

	// Number of sub-blocks, calculated as block_size / sub_block_size.
	int num_sub_blocks = 0;



	//
	// Memory addresses
	//
	
	// Memory address range served by module
	RangeType range_type;

	// Range attributes
	union
	{
		// If range_type = RangeBounds
		struct
		{
			unsigned int low;
			unsigned int high;
		} bounds;

		// If range_type = RangeInterleaved
		struct
		{
			unsigned int mod;
			unsigned int div;
			unsigned int eq;
		} interleaved;
	} range;



	//
	// Ports
	//

	// Array of ports
	std::vector<Port> ports;

	// Number of ports
	int num_ports = 0;

	// Number of locked ports
	int num_locked_ports = 0;

	// Accesses waiting for an available port
	esim::Queue port_queue;



	//
	// Directory
	//

	// Associated directory
	std::unique_ptr<Directory> directory;

	// Total directory size in entries
	int directory_size = 0;

	// Number of sets in the directory
	int directory_num_sets = 0;

	// Directory associativity
	int directory_num_ways = 0;



	//
	// Networks
	//

	// High network, closer to the processor
	net::Network *high_network = nullptr;

	// Low network, closer to main memory
	net::Network *low_network = nullptr;

	// Node in the high network that the module is associated with
	net::Node *high_network_node = nullptr;

	// Node in the low network that the module is associated with
	net::Node *low_network_node = nullptr;


	
	//
	// In-flight accesses
	//

	// List of all in-flight accesses
	std::list<Frame *> access_list;

	// List of all in-flight write accesses
	std::list<Frame *> write_access_list;
	
	// Number of in-flight coalesced accesses. This is a number
	// between 0 and access_list.getSize() at all times.
	int access_list_coalesced_count = 0;

	// Hash table of accesses, indexed by an frame ID
	std::unordered_map<long long, Frame *> access_map;




	//
	// Other
	//

	// Waiting list of memory accesses
	esim::Queue queue;

	// Associated cache
	std::unique_ptr<Cache> cache;

	// List of previous-level modules, closer to the processor
	std::vector<Module *> high_modules;

	// List of next-level modules, closer to main memory
	std::vector<Module *> low_modules;
	


	//
	// Statistics
	//

	long long accesses = 0;
	long long retry_accesses = 0;

	long long evictions = 0;

	long long dir_entry_conflicts = 0;
	long long retry_dir_entry_conflicts = 0;

	long long conflict_invalidations = 0;

	// Statistics for up-down accesses
	long long reads = 0;
	long long read_hits = 0;
	long long read_misses = 0;
	long long coalesced_reads = 0;
	long long writes = 0;
	long long write_hits = 0;
	long long write_misses = 0;
	long long coalesced_writes = 0;
	long long nc_writes = 0;
	long long nc_write_hits = 0;
	long long nc_write_misses = 0;
	long long coalesced_nc_writes = 0;
	long long retry_reads = 0;
	long long retry_read_hits = 0;
	long long retry_read_misses = 0;
	long long retry_writes = 0;
	long long retry_write_hits = 0;
	long long retry_write_misses = 0;
	long long retry_nc_writes = 0;
	long long retry_nc_write_hits = 0;
	long long retry_nc_write_misses = 0;

	// Statistics for down-up accesses
	long long read_probes = 0;
	long long write_probes = 0;
	long long retry_read_probes = 0;
	long long retry_write_probes = 0;

	// Statistics for other coherence traffic
	long long hlc_evictions = 0;

	// FIXME Update the local memory protocol and remove these
	long long effective_reads = 0;
	long long effective_writes = 0;

	// Statistics that are possibly power related
	long long dir_accesses = 0;
	long long data_accesses = 0;

public:

	/// Constructor
	Module(const std::string &name,
			Type type,
			int num_ports,
			int block_size,
			int data_latency);
	
	/// Set the directory properties. This does not instantiate the
	/// directory, it just saves its properties internally.
	void setDirectoryProperties(int directory_num_sets,
			int directory_num_ways,
			int directory_latency)
	{
		this->directory_num_sets = directory_num_sets;
		this->directory_num_ways = directory_num_ways;
		this->directory_latency = directory_latency;
		directory_size = directory_num_sets * directory_num_ways;
	}

	/// Initialize the associated directory.
	void InitializeDirectory(
			int num_sets,
			int num_ways,
			int num_sub_blocks,
			int num_nodes)
	{
		assert(!directory.get());
		directory = misc::new_unique<Directory>(
				name,
				num_sets,
				num_ways,
				num_sub_blocks,
				num_nodes);
	}

	/// Return the directory associated with the module. If no directory
	/// was initialized, nullptr is returned. A directory should be
	/// initialized with a call to InitializeDirectory().
	Directory *getDirectory() { return directory.get(); }

	/// Return the number of sets of the directory associated with the
	/// module, as set by setDirectoryProperties().
	int getDirectoryNumSets() { return directory_num_sets; }

	/// Return the associativity of the directory associated with the
	/// module, as set by setDirectoryProperties().
	int getDirectoryNumWays() { return directory_num_ways; }

	/// Return the directory latency, as set by setDirectoryProperties().
	int getDirectoryLatency() { return directory_latency; }

	/// Return the size of the directory (sets * ways), as set by
	/// setDirectoryProperties().
	int getDirectorySize() { return directory_size; }

	/// Set the MSHR size in number of entries
	void setMSHRSize(int mshr_size) { this->mshr_size = mshr_size; }

	/// Return whether the module can be accessed. A module can be accessed
	/// if there are available ports and enough room in the MSHR register.
	bool canAccess();

	/// Return module name
	std::string getName() const { return name; }

	/// Return the module type
	Type getType() const { return type; }

	/// Return number of ports
	int getNumPorts() const { return num_ports; }

	/// Return block size
	int getBlockSize() const { return block_size; }

	/// Return the size of the smallest block in the higher modules (those
	/// closer to the processor).
	int getSubBlockSize() const { return sub_block_size; }

	/// Return the number of sub-blocks per block. The size of a sub-block is
	/// the smallest from the block sizes of the higher modules (those closer
	/// to the processor).
	int getNumSubBlocks() const { return num_sub_blocks; }

	/// Update the size of the smallest block among the modules connected
	/// to this module and closer to the processor. This setter automatically
	/// updates the number of sub-blocks in each block.
	void setSubBlockSize(int sub_block_size)
	{
		assert(sub_block_size > 0);
		this->sub_block_size = sub_block_size;
		num_sub_blocks = block_size / sub_block_size;
	}

	/// Return the module level, where level 1 is the closes to the
	/// processor.
	int getLevel() const { return level; }

	/// Set the level of the module. This function is invoked during
	/// the memory configuration parsing process.
	void setLevel(int level)
	{
		assert(level >= 1);
		this->level = level;
	}

	/// Return data access latency
	int getLatency() const { return data_latency; }

	/// Set the high network and high network node that the module is
	/// connected to.
	void setHighNetwork(net::Network *high_network,
			net::Node *high_network_node)
	{
		this->high_network = high_network;
		this->high_network_node = high_network_node;
	}

	/// Set the low network and low network node that the module is
	/// connected to.
	void setLowNetwork(net::Network *low_network,
			net::Node *low_network_node)
	{
		this->low_network = low_network;
		this->low_network_node = low_network_node;
	}

	/// Create a cache structure associated with the module.
	void setCache(unsigned num_sets,
			unsigned num_ways,
			unsigned block_size,
			Cache::ReplacementPolicy replacement_policy,
			Cache::WritePolicy write_policy)
	{
		assert(!cache.get());
		cache = misc::new_unique<Cache>(name,
				num_sets,
				num_ways,
				block_size,
				replacement_policy,
				write_policy);
	}

	/// Get the cache structure associated with the module, as previously
	/// created by a call to setCache(). If setCache() wasn't invoked
	/// before, return nullptr.
	Cache *getCache() const { return cache.get(); }
	
	/// Set the address range served by the module between \a low and
	/// \a high physical addresses.
	void setRangeBounds(unsigned low, unsigned high)
	{
		range_type = RangeBounds;
		range.bounds.low = low;
		range.bounds.high = high;
	}

	/// Make the module serve an interleaved address space. An address will
	/// be served by the module if module \a mod and divided by \a div is
	/// equal to \a eq.
	void setRangeInterleaved(unsigned mod, unsigned div, unsigned eq)
	{
		range_type = RangeInterleaved;
		range.interleaved.mod = mod;
		range.interleaved.div = div;
		range.interleaved.eq = eq;
	}

	/// Return the number of high modules connected to this module (closer
	/// to the processor).
	int getNumHighModules() const { return high_modules.size(); }

	/// Return the high module (module closer to the processor) connected to
	/// this module with the given index.
	Module *getHighModule(int index) const
	{
		assert(index >=0 && index < (int) high_modules.size());
		return high_modules[index];
	}

	/// Add a high module (one that is closer to the processor)
	void addHighModule(Module *high_module)
	{
		high_modules.push_back(high_module);
	}

	/// Return the number of low modules connected to this module (closer to
	/// main memory).
	int getNumLowModules() const { return low_modules.size(); }

	/// Return the low module (module closer to main memory) connected to
	/// this module with the given index.
	Module *getLowModule(int index) const
	{
		assert(index >= 0 && index < (int) low_modules.size());
		return low_modules[index];
	}

	/// Add a low module (one that is closer to main memory)
	void addLowModule(Module *low_module)
	{
		low_modules.push_back(low_module);
	}

	/// Get the low network (the one closer to main memory)
	net::Network *getLowNetwork() const { return low_network; }

	/// Get the node representing this module in the network that is closer
	/// to main memory.
	net::Node *getLowNetworkNode() const { return low_network_node; }

	/// Get the network that is closer to the processor.
	net::Network *getHighNetwork() const { return high_network; }

	/// Get the node representing this module in the network that is closer
	/// to the processor.
	net::Node *getHighNetworkNode() const { return high_network_node; }

	/// Given a module connected to the high network of the current module,
	/// and closer to the processor, return the index in the sharers bitmap
	/// that it occupies in the current module. The sharer module's low
	/// network must be the same as the current module's high network.
	int getSharerIndex(Module *module) const
	{
		assert(high_network == module->low_network);
		return module->low_network_node->getIndex();
	}

	/// Set the given module as the owner for the given set, way, and
	/// sub-block of the current module's directory. A directory must have
	/// been created for this module. Argument \a module can be nullptr,
	/// indicating that the owner should be set to no-owner.
	///
	/// The given module's low network must be the same as the current
	/// module's high network.
	///
	void setOwner(int set_id, int way_id, int sub_block_id, Module *module)
	{
		assert(directory.get());
		int index = module ? getSharerIndex(module) : Directory::NoOwner;
		directory->setOwner(set_id, way_id, sub_block_id, index);
	}

	/// Add the given module as a sharer for the given set, way, and
	/// sub-block of the current module's directory. A directory must
	/// have been created for this module.
	///
	/// The given module's low network must be the same as the current
	/// module's high network.
	///
	/// Argument \a module cannot be \c nullptr.
	///
	void setSharer(int set_id, int way_id, int sub_block_id, Module *module)
	{
		assert(directory.get());
		assert(module);
		int index = getSharerIndex(module);
		directory->setSharer(set_id, way_id, sub_block_id, index);
	}
	
	/// Access the module.
	///
	/// \param access_type
	///	Type of access: load, store, nc-store
	///
	/// \param address
	///	Physical address.
	///
	/// \param witness
	///	Pointer to an integer variable that will be incremented once
	///	the access has completed. This argument is optional, and can
	///	be set to \c nullptr.
	///
	long long Access(AccessType access_type,
			unsigned address,
			int *witness = nullptr);
	
	/// Add the given frame to the list of in-flight accesses, and record
	/// its access type. This function is invoked internally by the event
	/// handlers of certain NMOESI events.
	void StartAccess(Frame *frame, AccessType access_type);
};


}  // namespace mem

#endif

