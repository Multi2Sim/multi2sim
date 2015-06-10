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

#ifndef MEMORY_FRAME_H
#define MEMORY_FRAME_H

#include <list>
#include <memory>

#include <lib/esim/Event.h>
#include <lib/esim/Queue.h>

#include "Cache.h"
#include "Module.h"


namespace mem
{

// Forward declarations
class Module;


/// Event frame for memory system events.
class Frame : public esim::EventFrame
{
	// Counter for identifiers
	static long long id_counter;

	// Unique identifier, initialized in constructor.
	long long id;

	// Current module, initialized in constructor.
	Module *module;

	// Physical address, initialized in constructor.
	unsigned address;

public:

	/// Direction of an access
	enum RequestDirection
	{
		RequestDirectionInvalid = 0,
		RequestDirectionUpDown,
		RequestDirectionDownUp
	};


	
	//
	// Public fields
	//

	/// Pointer to integer variable to be incremented when the access is
	/// over.
	int *witness = nullptr;

	/// Iterator to the current position of this frame in
	/// Module::access_list.
	std::list<Frame *>::iterator access_list_iterator;
	
	/// Iterator to the current position of this frame in
	/// Module::write_access_list.
	std::list<Frame *>::iterator write_access_list_iterator;

	/// Type of memory access
	Module::AccessType access_type = Module::AccessInvalid;

	/// If true, this access has been coalesced with another access.
	bool coalesced = false;

	/// If this access is coalesced, this field refers to the oldest
	/// access to the same block that this access is coalesced with.
	Frame *master_frame = nullptr;

	/// Queue of suspended dependent frames. When the access represented
	/// by this frame completes, it will wake up all accesses enqueued
	/// here.
	esim::Queue queue;

	/// Direction of an access. Up-down refers to processor-to-memory, while
	/// down-up refers to memory-to-processor direction.
	RequestDirection request_direction = RequestDirectionInvalid;

	/// Port currently being locked by this access, or nullptr if the
	/// access is not locking a port.
	Module::Port *port = nullptr;

	/// Flag indicating whether this access has locked a port for the first
	/// time. It is used to decide whether we're still on time to
	/// coalesce.
	bool port_locked = false;

	/// If true, this access waits in the queue of a port for it to be
	/// released. If false, it backtracks to give priority to other accesses
	/// and avoid deadlock.
	bool blocking = false;

	/// If true, this is a read request. If false, it is a write request.
	bool read = true;

	/// If true, this is a retried access.
	bool retry = false;

	/// Return error code from a child event chain.
	bool error = false;
	
	/// Tag associated with the access	
	int tag = -1;

	/// Set associated with the access
	int set = -1;

	/// Way associated with the access
	int way = -1;

	/// Block state
	Cache::BlockState state = Cache::BlockInvalid;



	//
	// Functions
	//

	/// Return a new unique value for a frame identifier.
	static long long getNewId() { return ++id_counter; }

	/// Constructor
	Frame(long long id, Module *module, unsigned address) :
			id(id),
			module(module),
			address(address)
	{
	}

	/// Return a unique identifier for this event frame, assigned
	/// internally when created.
	long long getId() const { return id; }

	/// Return the module associated with this event frame.
	Module *getModule() const { return module; }

	/// Return the memory address associated with this event frame.
	unsigned getAddress() const { return address; }
};


}  // namespace mem

#endif

