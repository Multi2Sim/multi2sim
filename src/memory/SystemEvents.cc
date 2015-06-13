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
#include "System.h"


namespace mem
{
	
esim::EventType *System::event_type_load;
esim::EventType *System::event_type_load_lock;
esim::EventType *System::event_type_load_action;
esim::EventType *System::event_type_load_miss;
esim::EventType *System::event_type_load_unlock;
esim::EventType *System::event_type_load_finish;

esim::EventType *System::event_type_store;
esim::EventType *System::event_type_store_lock;
esim::EventType *System::event_type_store_action;
esim::EventType *System::event_type_store_unlock;
esim::EventType *System::event_type_store_finish;

esim::EventType *System::event_type_nc_store;
esim::EventType *System::event_type_nc_store_lock;
esim::EventType *System::event_type_nc_store_writeback;
esim::EventType *System::event_type_nc_store_action;
esim::EventType *System::event_type_nc_store_miss;
esim::EventType *System::event_type_nc_store_unlock;
esim::EventType *System::event_type_nc_store_finish;

esim::EventType *System::event_type_find_and_lock;
esim::EventType *System::event_type_find_and_lock_port;
esim::EventType *System::event_type_find_and_lock_action;
esim::EventType *System::event_type_find_and_lock_finish;

esim::EventType *System::event_type_evict;
esim::EventType *System::event_type_evict_invalid;
esim::EventType *System::event_type_evict_action;
esim::EventType *System::event_type_evict_receive;
esim::EventType *System::event_type_evict_process;
esim::EventType *System::event_type_evict_process_noncoherent;
esim::EventType *System::event_type_evict_reply;
esim::EventType *System::event_type_evict_reply_receive;
esim::EventType *System::event_type_evict_finish;

esim::EventType *System::event_type_write_request;
esim::EventType *System::event_type_write_request_receive;
esim::EventType *System::event_type_write_request_action;
esim::EventType *System::event_type_write_request_exclusive;
esim::EventType *System::event_type_write_request_updown;
esim::EventType *System::event_type_write_request_updown_finish;
esim::EventType *System::event_type_write_request_downup;
esim::EventType *System::event_type_write_request_downup_finish;
esim::EventType *System::event_type_write_request_reply;
esim::EventType *System::event_type_write_request_finish;

esim::EventType *System::event_type_read_request;
esim::EventType *System::event_type_read_request_receive;
esim::EventType *System::event_type_read_request_action;
esim::EventType *System::event_type_read_request_updown;
esim::EventType *System::event_type_read_request_updown_miss;
esim::EventType *System::event_type_read_request_updown_finish;
esim::EventType *System::event_type_read_request_downup;
esim::EventType *System::event_type_read_request_downup_wait_for_reqs;
esim::EventType *System::event_type_read_request_downup_finish;
esim::EventType *System::event_type_read_request_reply;
esim::EventType *System::event_type_read_request_finish;

esim::EventType *System::event_type_invalidate;
esim::EventType *System::event_type_invalidate_finish;

esim::EventType *System::event_type_message;
esim::EventType *System::event_type_message_receive;
esim::EventType *System::event_type_message_action;
esim::EventType *System::event_type_message_reply;
esim::EventType *System::event_type_message_finish;

esim::EventType *System::event_type_flush;
esim::EventType *System::event_type_flush_finish;
	
esim::EventType *System::event_type_local_load;
esim::EventType *System::event_type_local_load_lock;
esim::EventType *System::event_type_local_load_finish;

esim::EventType *System::event_type_local_store;
esim::EventType *System::event_type_local_store_lock;
esim::EventType *System::event_type_local_store_finish;

esim::EventType *System::event_type_local_find_and_lock;
esim::EventType *System::event_type_local_find_and_lock_port;
esim::EventType *System::event_type_local_find_and_lock_action;
esim::EventType *System::event_type_local_find_and_lock_finish;


void System::EventLoadHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Get engine, frame, and module
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(event_frame);
	Module *module = frame->getModule();

	// Event "load"
	if (event_type == event_type_load)
	{
		debug << misc::fmt("%lld %lld 0x%x %s load\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.new_access "
				"name=\"A-%lld\" "
				"type=\"load\" "
				"state=\"%s:load\" "
				"addr=0x%x\n",
				frame->getId(),
				module->getName().c_str(),
				frame->getAddress());

		// Record access
		module->StartAccess(frame, Module::AccessLoad);

		// Coalesce access
		Frame *master_frame = module->canCoalesce(
				Module::AccessLoad,
				frame->getAddress(),
				frame);
		if (master_frame)
		{
			module->incCoalescedReads();
			module->Coalesce(master_frame, frame);
			master_frame->queue.Wait(event_type_load_finish);
			return;
		}

		// Next event
		esim_engine->Next(event_type_load_lock);
		return;
	}

	// Event "load_lock"
	if (event_type == event_type_load_lock)
	{
		debug << misc::fmt("  %lld %lld 0x%x %s load lock\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:load_lock\"\n",
				frame->getId(),
				module->getName().c_str());

		// If there is any older write, wait for it
		Frame *older_frame = module->getInFlightWrite(frame);
		if (older_frame)
		{
			debug << misc::fmt("    %lld wait for write %lld\n",
					frame->getId(),
					older_frame->getId());
			older_frame->queue.Wait(event_type_load_lock);
			return;
		}

		// If there is any older access to the same address that this
		// access could not be coalesced with, wait for it.
		older_frame = module->getInFlightAddress(
				frame->getAddress(),
				frame);
		if (older_frame)
		{
			debug << misc::fmt("    %lld wait for access %lld\n",
					frame->getId(),
					older_frame->getId());
			older_frame->queue.Wait(event_type_load_lock);
			return;
		}

		// Call "find_and_lock" event chain
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				module,
				frame->getAddress());
		new_frame->request_direction = Frame::RequestDirectionUpDown;
		new_frame->blocking = true;
		new_frame->read = true;
		new_frame->retry = frame->retry;
		esim_engine->Call(event_type_find_and_lock,
				new_frame,
				event_type_load_action);
		return;
	}


	// Event "load_action"
	if (event_type == event_type_load_action)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s load action\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access name=\"A-%lld\" "
				"state=\"%s:load_action\"\n",
				frame->getId(),
				module->getName().c_str());

		// Error locking
		if (frame->error)
		{
			// Calculate a retry latency
			int retry_latency = module->getRetryLatency();

			// Debug
			debug << misc::fmt("    lock error, retrying in "
					"%d cycles\n",
					retry_latency);

			// Reschedule 'load-lock'
			frame->retry = true;
			esim_engine->Next(event_type_load_lock, retry_latency);
			return;
		}

		// Hit
		if (frame->state)
		{
			// Continue with 'load-unlock'
			esim_engine->Next(event_type_load_unlock);
			return;
		}

		// Miss
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				module,
				frame->tag);
		new_frame->target_module = module->getLowModuleServingAddress(frame->tag);
		new_frame->request_direction = Frame::RequestDirectionUpDown;
		esim_engine->Call(event_type_read_request,
				new_frame,
				event_type_load_miss);

		// Done
		return;
	}


	// Event "load_miss"
	if (event_type == event_type_load_miss)
	{
		throw misc::Panic("Not implemented");
	}


	// Event "load_unlock"
	if (event_type == event_type_load_unlock)
	{
		throw misc::Panic("Not implemented");
	}


	// Event "load_finish"
	if (event_type == event_type_load_finish)
	{
		throw misc::Panic("Not implemented");
	}

}


void System::EventStoreHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Event "store"
	if (event_type == event_type_store)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "store_lock"
	if (event_type == event_type_store_lock)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "store_action"
	if (event_type == event_type_store_action)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "store_unlock"
	if (event_type == event_type_store_unlock)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "store_finish"
	if (event_type == event_type_store_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventNCStoreHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Event "nc_store"
	if (event_type == event_type_nc_store)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "nc_store_lock"
	if (event_type == event_type_nc_store_lock)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "nc_store_writeback"
	if (event_type == event_type_nc_store_writeback)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "nc_store_action"
	if (event_type == event_type_nc_store_action)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "nc_store_miss"
	if (event_type == event_type_nc_store_miss)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "nc_store_unlock"
	if (event_type == event_type_nc_store_unlock)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "nc_store_finish"
	if (event_type == event_type_nc_store_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventFindAndLockHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Get engine, frame, and module
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(event_frame);
	Frame *parent_frame = misc::cast<Frame *>(frame->getParentFrame().get());
	Module *module = frame->getModule();
	Cache *cache = module->getCache();
	Directory *directory = module->getDirectory();

	// Event "find_and_lock"
	if (event_type == event_type_find_and_lock)
	{
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"find and lock (blocking=%d)\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str(),
				frame->blocking);
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:find_and_lock\"\n",
				frame->getId(),
				module->getName().c_str());

		// Default return values
		parent_frame->error = false;

		// If this access has already been assigned a way, keep using it
		frame->way = parent_frame->way;

		// Get a port
		module->LockPort(frame, event_type_find_and_lock_port);
		return;
	}

	// Event "find_and_lock_port"
	if (event_type == event_type_find_and_lock_port)
	{
		// Get locked port
		Module::Port *port = frame->port;
		assert(port);

		// Debug
		debug << misc::fmt("  %lld %lld 0x%x %s find and lock port\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:find_and_lock_port\"\n",
				frame->getId(),
				module->getName().c_str());

		// Statistics
		module->incAccesses();
		if (frame->retry)
			module->incRetryAccesses();

		// Set parent frame flag expressing that port has already been 
		// locked. This flag is checked by new writes to find out if 
		// it is already too late to coalesce.
		parent_frame->port_locked = true;

		// Look for block
		frame->hit = module->FindBlock(frame->getAddress(),
				frame->set,
				frame->way,
				frame->tag,
				frame->state);
		if (frame->hit)
		{
			assert(frame->state);
			debug << misc::fmt("    %lld 0x%x %s "
					"hit: set=%d, way=%d, "
					"state=%s\n",
					frame->getId(),
					frame->tag,
					module->getName().c_str(),
					frame->set,
					frame->way,
					Cache::BlockStateMap[frame->state]);
		}

		// If a store access hits in the cache, we can be sure
		// that the it will complete and can allow processing to
		// continue by incrementing the witness pointer.  Misses
		// cannot do this without violating consistency, so their
		// witness pointer is updated in the write request logic.
		if (frame->write && frame->hit && frame->witness)
			(*frame->witness)++;

		// Check if miss
		if (!frame->hit)
		{
			// If the request is down-up and the block was not
			// found, it must be because it was previously
			// evicted.  We must stop here because going any
			// further would result in a new space being allocated
			// for it.
			if (frame->request_direction == Frame::RequestDirectionDownUp)
			{
				debug << misc::fmt("        %lld "
						"block not found",
						frame->getId());
				parent_frame->block_not_found = true;
				module->UnlockPort(port, frame);
				parent_frame->port_locked = false;
				esim_engine->Return();
				return;
			}

			// Find a victim to evict, only in up-down accesses.
			assert(!frame->way);
			frame->way = cache->ReplaceBlock(frame->set);
		}
		assert(frame->way >= 0);

		// If directory entry is locked and the call to find-and-lock
		// is not blocking, release port and return error.
		if (directory->isEntryLocked(frame->set, frame->way) &&
				frame->blocking)
		{
			// Debug
			debug << misc::fmt("    %lld 0x%x %s block locked at "
					"set=%d, "
					"way=%d "
					"by A-%lld - aborting\n",
					frame->getId(),
					frame->tag,
					module->getName().c_str(),
					frame->set,
					frame->way,
					directory->getEntryFrame(frame->set,
							frame->way)->getId());

			// Return error code to parent frame
			parent_frame->error = true;
			module->UnlockPort(port, frame);
			parent_frame->port_locked = false;
			esim_engine->Return();

			// Statistics
			module->incDirectoryEntryConflicts();
			if (frame->retry)
				module->incRetryDirectoryEntryConflicts();

			// Done
			return;
		}

		// Lock directory entry. If lock fails, port needs to be 
		// released to prevent deadlock.  When the directory entry 
		// is released, locking port and directory entry will be 
		// retried.
		if (!directory->LockEntry(frame->set, frame->way,
				event_type_find_and_lock, frame))
		{
			// Debug
			debug << misc::fmt("    %lld 0x%x %s block locked at "
					"set=%d, "
					"way=%d by "
					"A-%lld - waiting\n",
					frame->getId(), 
					frame->tag,
					module->getName().c_str(),
					frame->set,
					frame->way,
					directory->getEntryFrame(frame->set,
							frame->way)->getId());

			// Unlock port
			module->UnlockPort(port, frame);
			parent_frame->port_locked = false;

			// Statistics
			module->incDirectoryEntryConflicts();
			if (frame->retry)
				module->incRetryDirectoryEntryConflicts();

			// Done
			return;
		}

		// Miss
		if (!frame->hit)
		{
			// Find victim
			unsigned tag;
			cache->getBlock(frame->set,
					frame->way,
					tag,
					frame->state);
			assert(frame->state || !directory->isBlockSharedOrOwned(
					frame->set, frame->way));
			
			// Debug
			debug << misc::fmt("    %lld 0x%x %s miss -> lru: "
					"set=%d, "
					"way=%d, "
					"state=%s\n",
					frame->getId(),
					frame->tag,
					module->getName().c_str(),
					frame->set,
					frame->way,
					Cache::BlockStateMap[frame->state]);
		}

		// Statistics
		module->UpdateStats(frame);

		// Entry is locked. Record the transient tag so that a 
		// subsequent lookup detects that the block is being brought.
		// Also, update LRU counters here.
		cache->setTransientTag(frame->set, frame->way, frame->tag);
		cache->AccessBlock(frame->set, frame->way);

		// Access latency
		module->incDirectoryAccesses();
		esim_engine->Next(event_type_find_and_lock_action,
				module->getDirectoryLatency());

		// Done
		return;
	}

	// Event "find_and_lock_action"
	if (event_type == event_type_find_and_lock_action)
	{
		// Get locked port
		Module::Port *port = frame->port;
		assert(port);

		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s find and lock action\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:find_and_lock_action\"\n",
				frame->getId(),
				module->getName().c_str());

		// Release port
		module->UnlockPort(port, frame);
		parent_frame->port_locked = false;

		// On miss, evict if victim is a valid block.
		if (!frame->hit && frame->state)
		{
			// Record eviction
			frame->eviction = true;
			module->incConflictInvalidations();

			// Call 'evict'
			auto new_frame = misc::new_shared<Frame>(
					frame->getId(),
					module,
					0);
			new_frame->set = frame->set;
			new_frame->way = frame->way;
			esim_engine->Call(event_type_evict,
					new_frame,
					event_type_find_and_lock_finish);
			return;
		}

		// Continue
		esim_engine->Next(event_type_find_and_lock_finish);
		return;
	}

	// Event "find_and_lock_finish"
	if (event_type == event_type_find_and_lock_finish)
	{
		// Cache and directory
		Cache *cache = module->getCache();
		Directory *directory = module->getDirectory();

		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"find and lock finish (err=%d)\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str(),
				frame->error);
		trace << misc::fmt("mem.access name=\"A-%lld\" "
				"state=\"%s:find_and_lock_finish\"\n",
				frame->getId(),
				module->getName().c_str());

		// If evict produced error, return this error
		if (frame->error)
		{
			// Get block
			unsigned tag;
			cache->getBlock(frame->set, frame->way, tag,
					frame->state);
			assert(frame->state);
			assert(frame->eviction);

			// Unlock directory entry
			directory->UnlockEntry(frame->set, frame->way);
			
			// Return error
			parent_frame->error = true;
			esim_engine->Return();
			return;
		}

		// Eviction
		if (frame->eviction)
		{
			// Stats
			module->incEvictions();

			// Get cache block
			unsigned tag;
			cache->getBlock(frame->set, frame->way, tag,
					frame->state);
			assert(frame->state == Cache::BlockInvalid ||
					frame->state == Cache::BlockShared ||
					frame->state == Cache::BlockExclusive);

			// After an eviction, set the block to invalid
			cache->setBlock(frame->set, frame->way, 0,
					Cache::BlockInvalid);
		}

		// If this is a main memory, the block is here. A previous miss
		// was just a miss in the directory.
		if (module->getType() == Module::TypeMainMemory
				&& !frame->state)
		{
			frame->state = Cache::BlockExclusive;
			cache->setBlock(frame->set, frame->way, frame->tag,
					frame->state);
		}

		// Return
		parent_frame->error = 0;
		parent_frame->set = frame->set;
		parent_frame->way = frame->way;
		parent_frame->state = frame->state;
		parent_frame->tag = frame->tag;
		esim_engine->Return();
		return;
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventEvictHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Engine, frame, module
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(event_frame);
	Frame *parent_frame = misc::cast<Frame *>(frame->getParentFrame().get());
	Module *module = frame->getModule();
	Cache *cache = module->getCache();
	Directory *directory = module->getDirectory();

	// Event 'evict'
	if (event_type == event_type_evict)
	{
		// Default return value
		parent_frame->error = false;

		// Get block info
		unsigned tag;
		cache->getBlock(frame->set, frame->way, tag, frame->state);
		frame->tag = tag;
		assert(frame->state || !directory->isBlockSharedOrOwned(
				frame->set, frame->way));

		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s evict "
				"(set=%d, way=%d, state=%s)\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str(),
				frame->set,
				frame->way,
				Cache::BlockStateMap[frame->state]);
		trace << misc::fmt("mem.access name=\"A-%lld\" "
				"state=\"%s:evict\"\n",
				frame->getId(),
				module->getName().c_str());

		// Save some data
		frame->src_set = frame->set;
		frame->src_way = frame->way;
		frame->src_tag = frame->tag;
		frame->target_module = module->getLowModuleServingAddress(frame->tag);

		// Send write request to all sharers
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				module,
				0);
		new_frame->except_module = nullptr;
		new_frame->set = frame->set;
		new_frame->way = frame->way;
		esim_engine->Call(event_type_invalidate,
				new_frame,
				event_type_evict_invalid);
		return;
	}

	// Event "evict_invalid"
	if (event_type == event_type_evict_invalid)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "evict_action"
	if (event_type == event_type_evict_action)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "evict_receive"
	if (event_type == event_type_evict_receive)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "evict_process"
	if (event_type == event_type_evict_process)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "evict_process_noncoherent"
	if (event_type == event_type_evict_process_noncoherent)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "evict_reply"
	if (event_type == event_type_evict_reply)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "evict_reply_receive"
	if (event_type == event_type_evict_reply_receive)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "evict_finish"
	if (event_type == event_type_evict_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventWriteRequestHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Event "write_request"
	if (event_type == event_type_write_request)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "write_request_receive"
	if (event_type == event_type_write_request_receive)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "write_request_action"
	if (event_type == event_type_write_request_action)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "write_request_exclusive"
	if (event_type == event_type_write_request_exclusive)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "write_request_updown"
	if (event_type == event_type_write_request_updown)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "write_request_updown_finish"
	if (event_type == event_type_write_request_updown_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "write_request_downup"
	if (event_type == event_type_write_request_downup)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "write_request_downup_finish"
	if (event_type == event_type_write_request_downup_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "write_request_reply"
	if (event_type == event_type_write_request_reply)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "write_request_finish"
	if (event_type == event_type_write_request_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventReadRequestHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(event_frame);
	Frame *parent_frame = misc::cast<Frame *>(frame->getParentFrame().get());
	Module *module = frame->getModule();
	Module *target_module = frame->target_module;

	// Event "read_request"
	if (event_type == event_type_read_request)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s read request\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:read_request\"\n",
				frame->getId(),
				module->getName().c_str());

		// Default return values
		parent_frame->shared = false;
		parent_frame->error = false;

		// Sanity
		assert(frame->request_direction);
		assert(module->getLowModuleServingAddress(frame->getAddress())
				== target_module ||
				frame->request_direction ==
				Frame::RequestDirectionDownUp);
		assert(target_module->getLowModuleServingAddress(
				frame->getAddress()) == module ||
				frame->request_direction ==
				Frame::RequestDirectionUpDown);

		// Get source and destination nodes
		net::Network *network;
		net::Node *source_node;
		net::Node *destination_node;
		if (frame->request_direction == Frame::RequestDirectionUpDown)
		{
			network = module->getLowNetwork();
			source_node = module->getLowNetworkNode();
			destination_node = target_module->getHighNetworkNode();
		}
		else
		{
			network = module->getHighNetwork();
			source_node = module->getHighNetworkNode();
			destination_node = target_module->getLowNetworkNode();
		}

		// Send message
		frame->message = network->TrySend(source_node,
				destination_node,
				8,
				event_type_read_request_receive,
				event_type);
		if (frame->message)
			net::System::trace << misc::fmt("net.msg_access "
					"net=\"%s\" "
					"name=\"M-%lld\" "
					"access=\"A-%lld\"\n",
					network->getName().c_str(),
					frame->message->getId(),
					frame->getId());
		return;
	}

	// Event "read_request_receive"
	if (event_type == event_type_read_request_receive)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "read_request_action"
	if (event_type == event_type_read_request_action)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "read_request_updown"
	if (event_type == event_type_read_request_updown)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "read_request_updown_miss"
	if (event_type == event_type_read_request_updown_miss)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "read_request_updown_finish"
	if (event_type == event_type_read_request_updown_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "read_request_downup"
	if (event_type == event_type_read_request_downup)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "read_request_downup_wait_for_reqs"
	if (event_type == event_type_read_request_downup_wait_for_reqs)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "read_request_downup_finish"
	if (event_type == event_type_read_request_downup_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "read_request_reply"
	if (event_type == event_type_read_request_reply)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "read_request_finish"
	if (event_type == event_type_read_request_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventInvalidateHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(event_frame);
	Module *module = frame->getModule();
	Cache *cache = module->getCache();
	Directory *directory = module->getDirectory();

	// Event "invalidate"
	if (event_type == event_type_invalidate)
	{
		// Get block info
		unsigned tag;
		cache->getBlock(frame->set, frame->way, tag, frame->state);
		frame->tag = tag;

		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s invalidate "
				"(set=%d, way=%d, state=%s)\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str(),
				frame->set,
				frame->way,
				Cache::BlockStateMap[frame->state]);
		trace << misc::fmt("mem.access name=\"A-%lld\" "
				"state=\"%s:invalidate\"\n",
				frame->getId(),
				module->getName().c_str());

		// At least one pending reply
		frame->pending = 1;
		
		// Send write request to all upper level sharers except
		// 'except_module'.
		for (int z = 0; z < directory->getNumSubBlocks(); z++)
		{
			unsigned directory_entry_tag = frame->tag +
					z * module->getSubBlockSize();
			assert(directory_entry_tag < frame->tag +
					(unsigned) module->getSubBlockSize());
			Directory::Entry *directory_entry = directory->getEntry(
					frame->set, frame->way, z);
			for (int i = 0; i < directory->getNumNodes(); i++)
			{
				// Skip non-sharers and 'except_module'
				if (!directory->isSharer(frame->set,
						frame->way,
						z,
						i))
					continue;

				net::Network *high_network = module->getHighNetwork();
				net::Node *node = high_network->getNode(i);
				Module *sharer = (Module *) node->getUserData();
				if (sharer == frame->except_module)
					continue;

				// Clear sharer and owner
				directory->clearSharer(frame->set,
						frame->way,
						z,
						i);
				if (directory_entry->getOwner() == i)
					directory->setOwner(frame->set,
							frame->way,
							z,
							Directory::NoOwner);

				// Skip mid-block sub-blocks
				if (directory_entry_tag % sharer->getBlockSize())
					continue;
				
				// Send write request upwards if beginning of block
				auto new_frame = misc::new_shared<Frame>(
						frame->getId(),
						module,
						directory_entry_tag);
				new_frame->target_module = sharer;
				new_frame->request_direction = Frame::RequestDirectionDownUp;
				esim_engine->Call(event_type_write_request,
						new_frame,
						event_type_invalidate_finish);
			}
		}

		// Continue with 'invalidate-finish' event
		esim_engine->Next(event_type_invalidate_finish);
		return;
	}

	// Event "invalidate_finish"
	if (event_type == event_type_invalidate_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventMessageHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Event "message"
	if (event_type == event_type_message)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "message_receive"
	if (event_type == event_type_message_receive)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "message_action"
	if (event_type == event_type_message_action)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "message_reply"
	if (event_type == event_type_message_reply)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "message_finish"
	if (event_type == event_type_message_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventFlushHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Event "flush"
	if (event_type == event_type_flush)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "flush_finish"
	if (event_type == event_type_flush_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventLocalLoadHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Event "local_load"
	if (event_type == event_type_local_load)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "local_load_lock"
	if (event_type == event_type_local_load_lock)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "local_load_finish"
	if (event_type == event_type_local_load_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventLocalStoreHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Event "local_store"
	if (event_type == event_type_local_store)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "local_store_lock"
	if (event_type == event_type_local_store_lock)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "local_store_finish"
	if (event_type == event_type_local_store_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventLocalFindAndLockHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Event "local_find_and_lock"
	if (event_type == event_type_local_find_and_lock)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "local_find_and_lock_port"
	if (event_type == event_type_local_find_and_lock_port)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "local_find_and_lock_action"
	if (event_type == event_type_local_find_and_lock_action)
	{
		throw misc::Panic("Not implemented");
	}

	// Event "local_find_and_lock_finish"
	if (event_type == event_type_local_find_and_lock_finish)
	{
		throw misc::Panic("Not implemented");
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}



}

