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


// Forward declarations
namespace network
{
class Message;
}


namespace mem
{

// Forward declarations
class Module;


/// Event frame for memory system events.
class Frame : public esim::Frame
{
	// Counter for identifiers
	static long long id_counter;

	// Reference magic number that all live frames should have. When a
	// frame is freed, its magic number is reset.
	static const unsigned Magic = 0x36fc9da7;

	// Frame's magic number
	unsigned magic = Magic;

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

	/// Reply message type
	enum ReplyType
	{
		ReplyNone = 0,
		ReplyAck,
		ReplyAckData,
		ReplyAckError
	};

	/// Message type
	enum MessageType
	{
		MessageNone = 0,
		MessageClearOwner
	};



	
	//
	// Public fields
	//

	/// Pointer to integer variable to be incremented when the access is
	/// over.
	int *witness = nullptr;

	/// Iterator to the current position of this frame in
	/// Module::accesses.
	std::list<Frame *>::iterator accesses_iterator;
	
	/// Iterator to the current position of this frame in
	/// Module::write_accesses
	std::list<Frame *>::iterator write_accesses_iterator;

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

	/// Flag indicating whether this access is a read.
	bool read = false;

	/// Flag indicating whether this access is a write.
	bool write = false;

	/// Flag indicating whether this access is a non-coherent write.
	bool nc_write = false;

	/// Flag indicating whether there is a block eviction in the current
	/// access.
	bool eviction = false;

	/// If true, this is a retried access.
	bool retry = false;

	/// Return error code from a child event chain.
	bool error = false;
	
	/// Flag indicating whether there was a hit in the cache
	bool hit = false;

	/// Flag activated when a block was not found in a find-and-lock
	/// event for a down-up request.
	bool block_not_found = false;

	/// Return value of a read request, indicating whether the transfered
	/// block is shared among other caches.
	bool shared = false;

	/// Tag associated with the access	
	int tag = -1;

	/// Set associated with the access
	int set = -1;

	/// Way associated with the access
	int way = -1;

	/// Tag of an evicted block
	int src_tag = -1;

	/// Set of an evicted block
	int src_set = -1;

	/// Way of an evicted block
	int src_way = -1;

	/// Block state
	Cache::BlockState state = Cache::BlockInvalid;

	/// Target module for transfers
	Module *target_module = nullptr;

	/// Exception module to send invalidations
	Module *except_module = nullptr;

	/// Number of pending replies
	int pending = 0;

	/// Message sent through the network for this access
	net::Message *message = nullptr;

	/// Type of reply
	ReplyType reply = ReplyNone;

	/// Type of message
	MessageType message_type = MessageNone;

	/// Size in bytes of a reply
	int reply_size = 0;

	/// If true, notify the lower-level directory that it should retain
	/// the owner.
	bool retain_owner = false;




	//
	// Functions
	//

	/// Return a new unique value for a frame identifier.
	static long long getNewId() { return ++id_counter; }

	/// Constructor
	Frame(long long id, Module *module, unsigned address);

	/// Destructor
	~Frame()
	{
		// Reset magic number, useful to detect memory corruption.
		magic = 0;
	}

	/// Return a unique identifier for this event frame, assigned
	/// internally when created.
	long long getId() const { return id; }

	/// Return the module associated with this event frame.
	Module *getModule() const { return module; }

	/// Return the memory address associated with this event frame.
	unsigned getAddress() const { return address; }

	/// Set the reply type to the given value only if it is a higher reply
	/// than the one set to far. This is useful to select a reply type from
	/// several sub-blocks.
	void setReplyIfHigher(ReplyType reply)
	{
		if (reply > this->reply)
			this->reply = reply;
	}

	/// Check for valid magic number. This function can be used for debug
	/// purposes to detect memory corruption in frame handling.
	void CheckMagic()
	{
		if (magic != Magic)
			throw misc::Panic("Corrupt memory frame");
	}
};


}  // namespace mem

#endif

