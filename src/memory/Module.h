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
#include <unordered_set>

#include <lib/cpp/Misc.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Queue.h>
#include <network/EndNode.h>

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

	// Port in a memory module
	struct Port
	{
		// Memory access locking port, or nullptr if the port is free
		Frame *frame = nullptr;

		// Memory accesses waiting in port
		esim::Queue queue;
	};

private:


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
	net::EndNode *high_network_node = nullptr;

	// Node in the low network that the module is associated with
	net::EndNode *low_network_node = nullptr;


	
	//
	// In-flight accesses
	//

	// List of all in-flight accesses
	std::list<Frame *> accesses;

	// List of all in-flight write accesses
	std::list<Frame *> write_accesses;
	
	// Number of in-flight coalesced accesses. This is a number
	// between 0 and access_list.size() at all times.
	int num_coalesced_accesses = 0;

	// Hash table of accesses, indexed by a block address (that is, a
	// memory address divided by the module's block size). There can be
	// multiple in-flight accesses for the same block.
	std::unordered_multimap<unsigned, Frame *> in_flight_block_addresses;

	// Set containing all in-flight access identifiers
	std::unordered_set<long long> in_flight_access_ids;




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

	long long num_accesses = 0;
	long long num_retry_accesses = 0;

	long long num_evictions = 0;

	long long num_directory_entry_conflicts = 0;
	long long num_retry_directory_entry_conflicts = 0;

	long long num_conflict_invalidations = 0;

public:
	
	// Statistics for up-down accesses
	long long num_reads = 0;
	long long num_read_hits = 0;
	long long num_read_misses = 0;
	long long num_coalesced_reads = 0;
	long long num_writes = 0;
	long long num_write_hits = 0;
	long long num_write_misses = 0;
	long long num_coalesced_writes = 0;
	long long num_nc_writes = 0;
	long long num_nc_write_hits = 0;
	long long num_nc_write_misses = 0;
	long long num_coalesced_nc_writes = 0;
	long long num_retry_reads = 0;
	long long num_retry_read_hits = 0;
	long long num_retry_read_misses = 0;
	long long num_retry_writes = 0;
	long long num_retry_write_hits = 0;
	long long num_retry_write_misses = 0;
	long long num_retry_nc_writes = 0;
	long long num_retry_nc_write_hits = 0;
	long long num_retry_nc_write_misses = 0;

	// Statistics for down-up accesses
	long long num_read_probes = 0;
	long long num_write_probes = 0;
	long long num_retry_read_probes = 0;
	long long num_retry_write_probes = 0;

	// Statistics for blocking/non-blocking accesses
	long long num_blocking_reads = 0;
	long long num_blocking_writes = 0;
	long long num_blocking_nc_writes = 0;
	long long num_non_blocking_reads = 0;
	long long num_non_blocking_writes = 0;
	long long num_non_blocking_nc_writes = 0;

	// Statistics for other coherence traffic
	long long num_hlc_evictions = 0;

	// Statistics that are possibly power related
	long long num_directory_accesses = 0;
	long long num_data_accesses = 0;

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
	bool canAccess(int address) const;

	/// Return module name
	const std::string &getName() const { return name; }

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
	int getDataLatency() const { return data_latency; }

	/// Set the high network and high network node that the module is
	/// connected to.
	void setHighNetwork(net::Network *high_network,
			net::EndNode *high_network_node)
	{
		this->high_network = high_network;
		this->high_network_node = high_network_node;
	}

	/// Set the low network and low network node that the module is
	/// connected to.
	void setLowNetwork(net::Network *low_network,
			net::EndNode *low_network_node)
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

	/// Given a memory address, return the low module (the one closer to
	/// main memory) that serves that address. The current module must also
	/// serve the given address.
	///
	/// If the current module is a cache memory, and no lower module serves
	/// the address, the function will cause a fatal error, assuming a bad
	/// memory configuration given by the user.
	///
	/// If the current module is main memory, the function returns
	/// `nullptr`.
	///
	Module *getLowModuleServingAddress(unsigned address) const;

	/// Add a low module (one that is closer to main memory)
	void addLowModule(Module *low_module)
	{
		low_modules.push_back(low_module);
	}

	/// Return `true` if the current module serves the address given in
	/// the argument.
	bool ServesAddress(unsigned address) const;

	/// Get the low network (the one closer to main memory)
	net::Network *getLowNetwork() const { return low_network; }

	/// Get the node representing this module in the network that is closer
	/// to main memory.
	net::EndNode *getLowNetworkNode() const { return low_network_node; }

	/// Get the network that is closer to the processor.
	net::Network *getHighNetwork() const { return high_network; }

	/// Get the node representing this module in the network that is closer
	/// to the processor.
	net::EndNode *getHighNetworkNode() const { return high_network_node; }

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

	/// Return the owner module associated with a directory entry. If the
	/// entry has no owner, the function returns `nullptr`. A directory
	/// must have been created for this module.
	Module *getOwner(int set_id, int way_id, int sub_block_id);

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

	/// Return whether the given module is a sharer for the given
	/// directory entry (set, way, and sub-block) in the current module. A
	/// directory must have been created for this module.
	///
	/// The given module's low network must be the same as the current
	/// module's high network.
	///
	/// Argument \a module cannot be \c nullptr.
	///
	bool isSharer(int set_id, int way_id, int sub_block_id, Module *module)
	{
		assert(directory.get());
		assert(module);
		int index = getSharerIndex(module);
		return directory->isSharer(set_id, way_id, sub_block_id, index);
	}

	/// Return the number of sharers for the given directory entry. A
	/// directory must have been created for this module.
	int getNumSharers(int set_id, int way_id, int sub_block_id)
	{
		assert(directory.get());
		Directory::Entry *entry = directory->getEntry(set_id, way_id,
				sub_block_id);
		return entry->getNumSharers();
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
	/// \param return_event
	///	Event to be scheduled when the access completes. If the
	///	Access() function is invoked within an event handler, the
	///	current frame will be available within the event handler of
	///	\a return_event. Use \c nullptr (default) for no return event.
	///
	/// \return frame_id
	///	The function returns a unique identifier of the new memory
	///	access.
	///
	long long Access(AccessType access_type,
			unsigned address,
			int *witness = nullptr,
			esim::Event *return_event = nullptr);
	
	/// Add the given frame to the list of in-flight accesses, and record
	/// its access type. This function is invoked internally by the event
	/// handlers of the first NMOESI event for an access.
	void StartAccess(Frame *frame, AccessType access_type);

	/// Remove the given frame from the list of in-flight accesses. This
	/// function is invoked internally by the event handler of the last
	/// NMOESI event for an access.
	void FinishAccess(Frame *frame);

	/// Return the youngest in-flight access older than \a older_than_frame
	/// to the block containing \a address. If \a older_than_frame is
	/// nullptr, return the youngest in-flight access containing \a address.
	/// The function returns nullptr if there is no in-flight access to
	/// block containing \a address.
	Frame *getInFlightAddress(unsigned address,
			Frame *older_than_frame = nullptr);

	/// Return the youngest in-flight write older than \a older_than_frame.
	/// If \a older_than_frame is `nullptr`, return the youngest in-flight
	/// write. Return `nullptr` if there is no in-flight write.
	Frame *getInFlightWrite(Frame *older_than_frame = nullptr);

	/// Given a byte address, return whether there is an in-flight access
	/// to that same byte address or to any other byte address within the
	/// same block.
	bool isInFlightAddress(unsigned address);

	/// Return whether an access with the given identifier is still in
	/// flight. The access identifier is that returned by Access()
	bool isInFlightAccess(long long id);

	/// Dump information about all event-driven simulation frames associated
	/// with in-flight accesses in the module.
	void DumpInFlightAddresses(std::ostream &os = std::cout);

	/// Dump the module information.
	void Dump(std::ostream &os = std::cout) const;

	/// Check if an access to a module can be coalesced with another access
	/// older than 'older_than_frame'. If 'older_than_frame' is nullptr,
	/// check if it can be coalesced with any in-flight access. If it can,
	/// return the access that it would be coalesced with. Otherwise, return
	/// nullptr.
	Frame *canCoalesce(AccessType access_type,
			unsigned address,
			Frame *older_than_frame = nullptr);

	/// Coalesce access \a frame with access \a master_frame. The master
	/// frame must represent the oldest access in a coalesced chain.
	void Coalesce(Frame *master_frame, Frame *frame);

	// Lock a port, and schedule event when done. If there is no free port,
	// the access is enqueued in the port waiting list, and it will retry
	// once a port becomes available with a call to UnlockPort().
	void LockPort(Frame *frame, esim::Event *event);

	// Unlock the given port, currently locked by the given frame. If there
	// is any access waiting in the port queue to lock it, the port will
	// be locked right away, and the waiting event will be scheduled for
	// time 0.
	void UnlockPort(Port *port, Frame *frame);

	/// Look for the given address in the cache associated with this
	/// module. Three cases are possible:
	///
	/// - There is a block with a state other than I (invalid) and a tag
	///   matching the given address. This is considered a hit, and the
	///   function returns true. The set, way, tag, and state of the found
	///   block are populated in the arguments passed by reference.
	///
	/// - There is a block with a transient tag matching the given address
	///   and a locked directory entry. This is considered a hit as well,
	///   and the function returns true, regardless of the current state
	///   of the block. The set, way, tag, and state of the found block
	///   are populated in the arguments passed by reference.
	///
	/// - None of the two previous cases apply, which is considered a miss,
	///   and the function returns false. The `set` and `tag` arguments
	///   are populated according to the set where the block would be
	///   found, and the tag corresponding to the address, respectively.
	///   The `state` argument is set to `Cache::BlockInvalid`, and the
	///   `way` argument is set to 0.
	///
	bool FindBlock(unsigned address,
			int &set,
			int &way,
			int &tag,
			Cache::BlockState &state);

	/// Flush the module.
	///
	/// \param witness
	///	Pointer to an integer variable that will be incremented once
	///	the access has completed.
	///
	void Flush(int *witness);

	/// Recursively flush the cache utilizing FlushCache.
	/// This function must be invoked internally by a event handler.
	void RecursiveFlush();

	/// Flush every block in the cache.
	/// This function is invoked internally by RecursiveFlush().
	void FlushCache();

	/// Return an iterator to the first element of the access list.
	std::list<Frame *>::iterator getAccessListBegin()
	{
		return accesses.begin();
	}

	/// Return a past-the-end iterator for the list of in-flight accesses.
	std::list<Frame *>::iterator getAccessListEnd()
	{
		return accesses.end();
	}

	/// Return an iterator to the first element of the list of in-flight
	/// write accesses.
	std::list<Frame *>::iterator getWriteAccessListBegin()
	{
		return write_accesses.begin();
	}

	/// Return a past-the-end iterator to the list of in-flight write
	/// accesses.
	std::list<Frame *>::iterator getWriteAccessListEnd()
	{
		return write_accesses.end();
	}




	//
	// Statistics
	//

	/// Increment number of accesses
	void incAccesses() { num_accesses++; }

	/// Increment number of retried accesses
	void incRetryAccesses() { num_retry_accesses++; }

	/// Increment the number of evictions
	void incEvictions() { num_evictions++; }

	/// Increment number of coalesced reads
	void incCoalescedReads() { num_coalesced_reads++; }

	/// Increment number of coalesced writes
	void incCoalescedWrites() { num_coalesced_writes++; }

	/// Increment number of coealesced non-coherent writes
	void incCoalescedNCWrites() { num_coalesced_nc_writes++; }

	/// Increment the number of conflicts while trying to lock directory
	/// entries.
	void incDirectoryEntryConflicts() { num_directory_entry_conflicts++; }

	/// Increment the number of invalidations due to conflicts.
	void incConflictInvalidations() { num_conflict_invalidations++; }

	/// Increment the number of conflicts found when trying to lock a
	/// directory entry in a retried access.
	void incRetryDirectoryEntryConflicts() { num_retry_directory_entry_conflicts++; }

	/// Increment the number of accesses to the directory.
	void incDirectoryAccesses() { num_directory_accesses++; }

	/// Increment the number of accesses to the data.
	void incDataAccesses() { num_data_accesses++; }

	/// Update the following statistics based on the information collected
	/// from the given frame:
	///
	/// - Number of reads
	/// - Number of retried reads
	/// - Number of read hits
	/// - Number of retried read hits
	/// - Number of read misses
	/// - Number of retried read misses
	/// - Number of non-coherent writes
	/// - Number of retried non-coherent writes
	/// - Number of non-coherent write hits
	/// - Number of retried non-coherent write hits
	/// - Number of non-coherent write misses
	/// - Number of retried non-coherent write misses
	/// - Number of write probes
	/// - Number of retried write probes
	/// - Number of read probes
	/// - Number of retried read probes
	/// - Number of evictions
	///
	void UpdateStats(Frame *frame);

	/// Return a random latency calculated in proportion to the standard
	/// data latency, used for access retries.
	int getRetryLatency() const;
};


}  // namespace mem

#endif

