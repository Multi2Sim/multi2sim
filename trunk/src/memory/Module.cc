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
	ports.reserve(num_ports);

	// Block size
	assert(!(block_size & (block_size - 1)) && block_size >= 4);
	log_block_size = misc::LogBase2(block_size);
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
	esim::EventType *event_type;
	switch (type)
	{

	case TypeCache:
	case TypeMainMemory:

		switch (access_type)
		{

		case AccessLoad:

			event_type = System::event_type_load;
			break;

		case AccessStore:

			event_type = System::event_type_store;
			break;

		case AccessNCStore:

			event_type = System::event_type_nc_store;
			break;

		default:

			throw misc::Panic("Invalid access type");
		}
		break;

	case TypeLocalMemory:

		switch (access_type)
		{

		case AccessLoad:

			event_type = System::event_type_local_load;
			break;

		case AccessStore:

			event_type = System::event_type_local_store;
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
	esim_engine->Call(event_type, frame);

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


}  // namespace mem

