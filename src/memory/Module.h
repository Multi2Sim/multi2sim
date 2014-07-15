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

#include <memory>

#include <lib/esim/Engine.h>
#include <lib/esim/Queue.h>

#include "Cache.h"
#include "Directory.h"


// Forward declarations
namespace net { class Network; }
namespace net { class Node; }


namespace mem
{


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
	enum Range
	{
		RangeInvalid = 0,
		RangeBounds,
		RangeInterleaved
	};

	/// Module access types
	enum Access
	{
		AccessInvalid = 0,
		AccessLoad,
		AccessStore,
		AccessNCStore,
		AccessPrefetch
	};

private:

	class Frame : public esim::EventFrame
	{
	};
	
	/// Port in a memory module
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

	// Module type
	Type type;

	// Name of the module
	std::string name;

	// Block size for caches
	int block_size;

	// Log base 2 of block size
	int log_block_size;

	// Latency for data access in cycles
	int data_latency;

	// Directory access latency
	int dir_latency;

	// Number of entries in the MSHR register
	int mshr_size;

	// Cache level, where 1 is closest to processors
	int level = 0;
	
	// Smallest block size among higher modules (modules closer to the
	// processor). If there is no higher module, the sub-block size is
	// equal to the block size.
	int sub_block_size;

	// Number of sub-blocks, calculated as block_size / sub_block_size.
	int num_sub_blocks;



	//
	// Memory addresses
	//
	
	// Memory address range served by module
	Range range;

	// Range attributes
	union
	{
		// If range = RangeBounds
		struct
		{
			unsigned int low;
			unsigned int high;
		} bounds;

		// If range = RangeInterleaved
		struct
		{
			unsigned int mod;
			unsigned int div;
			unsigned int eq;
		} interleaved;
	};



	//
	// Ports
	//

	// Array of ports
	std::vector<std::unique_ptr<Port>> ports;

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
	net::Network *high_network;

	// Low network, closer to main memory
	net::Network *low_network;

	// Node in the high network that the module is associated with
	net::Node *high_node;

	// Node in the low network that the module is associated with
	net::Node *low_node;



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
	
	
	
	
// Continue here in C version of module.h once intrusive linked list is
// completely implemented.
// -----------struct mod_stack_t *access_list_head;

public:

};


}  // namespace mem

#endif

