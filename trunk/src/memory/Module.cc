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

#include "Frame.h"
#include "Module.h"
#include "System.h"


namespace mem
{


Module::Module(const std::string &name,
		Type type,
		int num_ports,
		int block_size,
		int data_latency)
		:
		name(name),
		type(type),
		block_size(block_size),
		data_latency(data_latency),
		num_ports(num_ports)
{
	// Create 'num_ports' in vector of ports
	ports.resize(num_ports);

	// Block size
	assert(!(block_size & (block_size - 1)) && block_size >= 4);
	log_block_size = misc::LogBase2(block_size);
}


bool Module::ServesAddress(unsigned address) const
{
	// Address bounds
	if (range_type == RangeBounds)
		return address >= range.bounds.low &&
				address <= range.bounds.high;

	// Interleaved addresses
	if (range_type == RangeInterleaved)
		return (address / range.interleaved.div) %
				range.interleaved.mod ==
				range.interleaved.eq;

	// Invalid
	throw misc::Panic("Invalid range type");
}


Module *Module::getLowModuleServingAddress(unsigned address) const
{
	// The address must be served by the current module
	assert(ServesAddress(address));

	// Main memory does not have a low module
	if (type == TypeMainMemory)
	{
		assert(!low_modules.size());
		return nullptr;
	}

	// Check which low module serves address
	Module *server_module = nullptr;
	for (Module *low_module : low_modules)
	{
		// Skip if this low module doesn't serve address
		if (!low_module->ServesAddress(address))
			continue;

		// Address served by more than one module
		if (server_module)
			throw Error(misc::fmt("%s: low modules '%s' "
					"and '%s' both serve address 0x%x",
					name.c_str(),
					server_module->getName().c_str(),
					low_module->getName().c_str(),
					address));

		// Assign serving module
		server_module = low_module;
	}

	// Error if no low module serves address
	if (!server_module)
		throw Error(misc::fmt("Module %s: no lower module "
				"serves address 0x%x",
				name.c_str(),
				address));

	// Return server module
	return server_module;
}


Module *Module::getOwner(int set_id, int way_id, int sub_block_id)
{
	// Get directory entry
	assert(directory.get());
	Directory::Entry *entry = directory->getEntry(set_id,
			way_id,
			sub_block_id);

	// Get owner ID
	assert(entry);
	int owner = entry->getOwner();

	// No owner
	if (owner == Directory::NoOwner)
		return nullptr;
	
	// Get node
	net::Node *node = high_network->getNode(owner);
	assert(node);

	// Return owner module
	return (Module *) node->getUserData();
}


bool Module::canAccess(int address) const
{
	// There must be a free port
	assert(num_locked_ports <= num_ports);
	if (num_locked_ports == num_ports)
		return false;

	// If no MSHR is given, module can be accessed
	if (!mshr_size)
		return true;

	// Module can be accessed if number of non-coalesced in-flight accesses
	// is smaller than the MSHR size.
	int num_non_coalesced_accesses = access_list.size() -
			access_list_coalesced_count;
	return num_non_coalesced_accesses < mshr_size;
}


long long Module::Access(AccessType access_type,
		unsigned address,
		int *witness)
{
	// Create a new event frame
	auto frame = misc::new_shared<Frame>(
			Frame::getNewId(),
			this,
			address);
	frame->witness = witness;

	// Select initial event type
	esim::Event *event;
	switch (type)
	{

	case TypeCache:
	case TypeMainMemory:

		switch (access_type)
		{

		case AccessLoad:

			event = System::event_load;
			break;

		case AccessStore:

			event = System::event_store;
			break;

		case AccessNCStore:

			event = System::event_nc_store;
			break;

		default:

			throw misc::Panic("Invalid access type");
		}
		break;

	case TypeLocalMemory:

		switch (access_type)
		{

		case AccessLoad:

			event = System::event_local_load;
			break;

		case AccessStore:

			event = System::event_local_store;
			break;

		default:

			throw misc::Panic("Invalid access type");
		}
		break;

	default:

		throw misc::Panic("Invalid module type");
	}

	// Schedule event
	esim::Engine *esim_engine = esim::Engine::getInstance();
	esim_engine->Call(event, frame);

	// Return frame ID
	return frame->getId();
}


void Module::StartAccess(Frame *frame, AccessType access_type)
{
	// Record access type
	frame->access_type = access_type;

	// Insert in access list
	frame->access_list_iterator = access_list.insert(access_list.end(),
			frame);

	// Insert in write access list
	if (access_type == AccessStore)
		frame->write_access_list_iterator = write_access_list.insert(
				write_access_list.end(),
				frame);

	// Insert in access hash table
	unsigned block_address = frame->getAddress() >> log_block_size;
	access_map.emplace(std::make_pair(block_address, frame));
}


void Module::FinishAccess(Frame *frame)
{
	// Remove from access list
	access_list.erase(frame->access_list_iterator);
	frame->access_list_iterator = access_list.end();

	// Remove from write access list
	assert(frame->access_type);
	if (frame->access_type == Module::AccessStore)
	{
		write_access_list.erase(frame->write_access_list_iterator);
		frame->write_access_list_iterator = write_access_list.end();
	}

	// Remove from hash table
	unsigned block_address = frame->getAddress() >> log_block_size;
	auto range = access_map.equal_range(block_address);
	bool found = false;
	for (auto it = range.first; it != range.second; ++it)
	{
		Frame *other_frame = it->second;
		if (frame == other_frame)
		{
			found = true;
			access_map.erase(it);
			break;
		}
	}

	// Must have been removed
	if (!found)
		throw misc::Panic("Frame not found");

	// If this was a coalesced access, update counter
	if (frame->coalesced)
	{
		assert(access_list_coalesced_count > 0);
		access_list_coalesced_count--;
	}
}


Frame *Module::getInFlightAddress(unsigned address,
		Frame *older_than_frame)
{
	// Look for address
	unsigned block_address = address >> log_block_size;
	auto range = access_map.equal_range(block_address);
	for (auto it = range.first; it != range.second; ++it)
	{
		// Get frame
		Frame *frame = it->second;

		// This frame is not older than 'older_than_frame'
		if (older_than_frame && frame->getId() >=
				older_than_frame->getId())
			continue;

		// Block address matches
		assert(frame->getAddress() >> log_block_size ==
				address >> log_block_size);
		return frame;
	}

	// Not found
	return nullptr;
}


Frame *Module::getInFlightWrite(Frame *older_than_frame)
{
	// No 'older_than_frame' given, return youngest write, or nullptr if
	// there is no in-flight write.
	if (!older_than_frame)
		return write_access_list.size() ?
				write_access_list.back() :
				nullptr;
	
	// Search
	auto it = older_than_frame->access_list_iterator;
	while (it != access_list.begin())
	{
		// Rewind
		--it;

		// Check if it is a write
		Frame *frame = *it;
		if (frame->access_type == AccessStore)
			return frame;
	}

	// Not found
	return nullptr;
}


Frame *Module::canCoalesce(AccessType access_type,
		unsigned address,
		Frame *older_than_frame)
{
	// Nothing if there is no in-flight access
	if (!access_list.size())
		return nullptr;

	// For efficiency, first check in the hash table of accesses
	// whether there is an access in flight to the same block.
	assert(access_type);
	if (!getInFlightAddress(address, older_than_frame))
		return nullptr;

	// Nothing if 'older_than_frame' is in the head of the in-flight
	// access list (i.e., there is nothing older).
	if (older_than_frame->access_list_iterator == access_list.begin())
		return nullptr;
	
	// Get iterator to youngest access older than 'older_than_frame', or an
	// iterator to the overall youngest access if 'older_than_frame' is
	// null.
	auto tail = older_than_frame ?
			older_than_frame->access_list_iterator :
			access_list.end();
	assert(tail != access_list.begin());
	--tail;

	// Coalesce depending on access type
	switch (access_type)
	{

	case AccessLoad:
	{
		auto it = tail;
		while (true)
		{
			// Only coalesce with groups of reads at the tail
			Frame *frame = *it;
			if (frame->access_type != AccessLoad)
				return nullptr;

			// Same block address, coalesce
			if (frame->getAddress() >> log_block_size ==
					address >> log_block_size)
				return frame->master_frame ?
						frame->master_frame :
						frame;
			
			// Done when head reached
			if (it == access_list.begin())
				break;

			// Previous
			--it;
		}
		break;
	}

	case AccessStore:
	{
		// Only coalesce with last access if it is a write
		Frame *frame = *tail;
		if (frame->access_type != AccessStore)
			return nullptr;
		
		// Only if it is an access to the same block
		if (frame->getAddress() >> log_block_size !=
				address >> log_block_size)
			return nullptr;

		// Only if previous write has not started yet
		if (frame->port_locked)
			return nullptr;

		// Coalesce
		return frame->master_frame ?
				frame->master_frame :
				frame;
	}

	case AccessNCStore:
	{
		// Only coalesce with last access if it is a non-coherent write
		Frame *frame = *tail;
		if (frame->access_type != AccessNCStore)
			return nullptr;

		// Only if it is an access to the same block
		if (frame->getAddress() >> log_block_size !=
				address >> log_block_size)
			return nullptr;

		// Only if previous write has not started yet
		if (frame->port_locked)
			return nullptr;

		// Coalesce
		return frame->master_frame ?
				frame->master_frame :
				frame;
	}

	default:

		throw misc::Panic("Invalid access type");
	}

	// No access found
	return nullptr;
}


void Module::Coalesce(Frame *master_frame, Frame *frame)
{
	// Get event-driven engine
	esim::Engine *esim_engine = esim::Engine::getInstance();

	// Debug
	System::debug << misc::fmt("  %lld %lld 0x%x %s coalesce with %lld\n",
			esim_engine->getTime(),
			frame->getId(),
			frame->getAddress(),
			name.c_str(),
			master_frame->getId());

	// Master frame must not have a parent. We only want one level of
	// coalesced accesses.
	assert(!master_frame->master_frame);

	// Access must have been recorded already, which sets the access
	// kind to a valid value.
	assert(frame->access_type);

	// Set slave frame as a coalesced access
	frame->coalesced = true;
	frame->master_frame = master_frame;
	assert(access_list_coalesced_count <= (int) access_list.size());

	// Record in-flight coalesced access in module
	access_list_coalesced_count++;
}


void Module::LockPort(Frame *frame, esim::Event *event)
{
	// No free port
	if (num_locked_ports >= num_ports)
	{
		// Event chain must not be suspended in a queue
		assert(!frame->isInQueue());

		// If the request to lock the port is down-up, give it priority
		// since it is possibly holding up a large portion of the memory
		// hierarchy.
		port_queue.Wait(event, frame->request_direction ==
				Frame::RequestDirectionDownUp);
		return;
	}

	// Get free port
	int port_index;
	Port *port = nullptr;
	for (port_index = 0; port_index < num_ports; port_index++)
	{
		port = &ports[port_index];
		if (!port->frame)
			break;
	}

	// Lock port
	assert(port && port_index < num_ports);
	port->frame = frame;
	frame->port = port;
	num_locked_ports++;

	// Debug
	esim::Engine *esim_engine = esim::Engine::getInstance();
	System::debug << misc::fmt("  %lld frame %lld %s port %d locked\n",
			esim_engine->getTime(),
			frame->getId(),
			name.c_str(),
			port_index);

	// Schedule event
	esim_engine->Next(event);
}


void Module::UnlockPort(Port *port, Frame *frame)
{
	// Checks
	assert(num_locked_ports > 0);
	assert(frame->port && port->frame);
	assert(frame->port == port && port->frame == frame);
	assert(frame->getModule() == this);

	// Unlock port
	frame->port = nullptr;
	port->frame = nullptr;
	num_locked_ports--;

	// Debug
	esim::Engine *esim_engine = esim::Engine::getInstance();
	System::debug << misc::fmt("  %lld %lld %s port unlocked\n",
			esim_engine->getTime(),
			frame->getId(),
			name.c_str());

	// Check if there was any access waiting for free port
	if (port_queue.isEmpty())
		return;

	// Lock port with an access waiting for a free port
	frame = misc::cast<Frame *>(port_queue.getHead());
	port->frame = frame;
	frame->port = port;
	num_locked_ports++;

	// Wake up access
	port_queue.WakeupOne();
	
	// Debug
	System::debug << misc::fmt("  %lld frame %lld %s port locked\n",
			esim_engine->getTime(),
			frame->getId(),
			name.c_str());
}


bool Module::FindBlock(unsigned address,
		int &set,
		int &way,
		int &tag,
		Cache::BlockState &state)
{
	// A transient tag is considered a hit if the block is locked in the
	// corresponding directory.
	tag = address & ~cache->getBlockMask();
	if (range_type == RangeInterleaved)
	{
		int num_modules = range.interleaved.mod;
		set = ((tag >> cache->getLogBlockSize()) / num_modules)
				% cache->getNumSets();
	}
	else if (range_type == RangeBounds)
	{
		set = (tag >> cache->getLogBlockSize())
				% cache->getNumSets();
	}
	else 
	{
		throw misc::Panic("Invalid range type");
	}

	// Find way in set
	for (way = 0; way < (int) cache->getNumWays(); way++)
	{
		// Get block
		Cache::Block *block = cache->getBlock(set, way);
		state = block->getState();

		// Permanent tag available with state other than invalid
		if (block->getTag() == (unsigned) tag && state)
			return true;

		// Transient tag available while directory entry is locked.
		// This is considered a hit, regardless of the state of the
		// block.
		if (block->getTransientTag() == (unsigned) tag &&
				directory->isEntryLocked(set, way))
			return true;
	}

	// Miss
	way = 0;
	state = Cache::BlockInvalid;
	return false;
}


void Module::Flush(int *witness)
{
	// Get pointer to esim engine
	esim::Engine *esim_engine = esim::Engine::getInstance();

	// Create a new event frame
	auto new_frame = misc::new_shared<Frame>(
			Frame::getNewId(),
			this,
			0);
	new_frame->witness = witness;

	// Schedule event
	esim_engine->Call(System::event_flush, new_frame);
}


void Module::RecursiveFlush()
{
	// Iterate through low_modules
	for (Module *low_module : low_modules)
	{
		// Skips main memory as it should not be flushed
		if (low_module->getType() != Module::TypeMainMemory)
			low_module->RecursiveFlush();
		else
			low_module->FlushCache();
	}
}


void Module::FlushCache()
{
	// Get pointer to event frame
	Frame *frame = misc::cast<Frame *>(frame);
	
	// Set up variables
	unsigned tag;
	Cache::BlockState state;

	// Invalidate all blocks
	for (int set = 0; set < directory_num_sets; set++)
	{
		for (int way = 0; way < directory_num_ways; way++)
		{
			// Get block
			cache->getBlock(set, way, tag, state);

			// Ignore if block has already been invalidated
			if (state == Cache::BlockState::BlockInvalid)
				continue;

			// One more pending request
			frame->pending++;

			// Get pointer to engine
			esim::Engine *esim_engine = esim::Engine::getInstance();

			// Create new frame
			auto new_frame = misc::new_shared<Frame>(
					frame->getId(),
					this,
					frame->tag);
			new_frame->set = set;
			new_frame->way = way;
			new_frame->witness = frame->witness;

			// Set up event
			esim::Event *event = System::event_invalidate;

			// Schedule event
			esim_engine->Call(event,
				new_frame);
		}
	}
}


void Module::UpdateStats(Frame *frame)
{
	// Record access type. I purposefully chose to record both hits and
	// misses separately here so that we can sanity check them against
	// the total number of accesses.
	if (frame->request_direction == Frame::RequestDirectionUpDown)
	{
		if (frame->read)
		{
			reads++;
			if (frame->retry)
				retry_reads++;
			if (frame->hit)
			{
				read_hits++;
				if (frame->retry)
					retry_read_hits++;
			}
			else
			{
				read_misses++;
				if (frame->retry)
					retry_read_misses++;
			}

		}
		else if (frame->nc_write)  // Must go after read
		{
			nc_writes++;
			if (frame->retry)
				retry_nc_writes++;
			if (frame->hit)
			{
				nc_write_hits++;
				if (frame->retry)
					retry_nc_write_hits++;
			}
			else
			{
				nc_write_misses++;
				if (frame->retry)
					retry_nc_write_misses++;
			}
		}
		else if (frame->write)
		{
			writes++;
			if (frame->retry)
				retry_writes++;
			if (frame->hit)
			{
				write_hits++;
				if (frame->retry)
					retry_write_hits++;
			}
			else
			{
				write_misses++;
				if (frame->retry)
					retry_write_misses++;
			}
		}
		else 
		{
			throw misc::Panic("Invalid memory operation type");
		}
	}
	else if (frame->request_direction == Frame::RequestDirectionDownUp)
	{
		assert(frame->hit);
		if (frame->write)
		{
			write_probes++;
			if (frame->retry)
				retry_write_probes++;
		}
		else if (frame->read)
		{
			read_probes++;
			if (frame->retry)
				retry_read_probes++;
		}
		else
		{
			throw misc::Panic("Invalid memory operation type");
		}
	}
	else
	{
		hlc_evictions++;
	}
}


int Module::getRetryLatency() const
{
	// To support a data latency of zero, we must ensure that at least
	// one of the following values is non-zero so that the modulo operation
	// will work.  Using two instead of one to avoid livelock situations.
	return random() % (data_latency + 2);
}


}  // namespace mem


