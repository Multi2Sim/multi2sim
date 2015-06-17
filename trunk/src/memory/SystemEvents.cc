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

#include <network/EndNode.h>

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
	Cache *cache = module->getCache();
	Directory *directory = module->getDirectory();

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
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s load miss\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:load_miss\"\n",
				frame->getId(),
				module->getName().c_str());

		// Error on read request. Unlock block and retry load.
		if (frame->error)
		{
			// Unlock directory entry
			directory->UnlockEntry(frame->set, frame->way);
			
			// Calculate retry latency
			int retry_latency = module->getRetryLatency();

			// Debug
			debug << misc::fmt("    lock error, retrying "
					"in %d cycles\n", retry_latency);

			// Continue with 'load-lock' after retry latency
			frame->retry = true;
			esim_engine->Next(event_type_load_lock, retry_latency);
			return;
		}

		// Set block state to E/S depending on return var 'shared'.
		// Also set the tag of the block.
		cache->setBlock(frame->set,
				frame->way,
				frame->tag,
				frame->shared ? Cache::BlockShared : Cache::BlockExclusive);

		// Continue
		esim_engine->Next(event_type_load_unlock);
		return;
	}


	// Event "load_unlock"
	if (event_type == event_type_load_unlock)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"load unlock\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:load_unlock\"\n",
				frame->getId(),
				module->getName().c_str());

		// Unlock directory entry
		directory->UnlockEntry(frame->set, frame->way);

		// Stats
		module->incDataAccesses();

		// Continue with 'load-finish' after latency
		esim_engine->Next(event_type_load_finish,
				module->getDataLatency());
		return;
	}


	// Event "load_finish"
	if (event_type == event_type_load_finish)
	{
		// Debug and trace
		debug << misc::fmt("%lld %lld 0x%x %s load finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:load_finish\"\n",
				frame->getId(),
				module->getName().c_str());
		trace << misc::fmt("mem.end_access "
				"name=\"A-%lld\"\n",
				frame->getId());

		// Increment witness variable
		if (frame->witness)
			(*frame->witness)++;

		// Finish access
		module->FinishAccess(frame);

		// Return
		esim_engine->Return();
		return;
	}

}


void System::EventStoreHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(event_frame);
	Module *module = frame->getModule();
	Cache *cache = module->getCache();
	Directory *directory = module->getDirectory();

	// Event "store"
	if (event_type == event_type_store)
	{
		// Debug and trace
		debug << misc::fmt("%lld %lld 0x%x %s store\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.new_access "
				"name=\"A-%lld\" "
				"type=\"store\" "
				"state=\"%s:store\" addr=0x%x\n",
				frame->getId(),
				module->getName().c_str(),
				frame->getAddress());

		// Record access
		module->StartAccess(frame, Module::AccessStore);

		// Coalesce access
		Frame *master_frame = module->canCoalesce(
				Module::AccessStore,
				frame->getAddress(),
				frame);
		if (master_frame)
		{
			// Coalesce
			module->incCoalescedWrites();
			module->Coalesce(master_frame, frame);
			master_frame->queue.Wait(event_type_store_finish);

			// Increment witness
			if (frame->witness)
				(*frame->witness)++;

			// Done
			return;
		}

		// Continue
		esim_engine->Next(event_type_store_lock);
		return;
	}

	// Event "store_lock"
	if (event_type == event_type_store_lock)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s store lock\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:store_lock\"\n",
				frame->getId(),
				module->getName().c_str());

		// If there is any older access, wait for it
		auto it = frame->access_list_iterator;
		assert(it != module->getAccessListEnd());
		if (it != module->getAccessListBegin())
		{
			// Get older access
			--it;
			Frame *older_frame = *it;

			// Debug
			debug << misc::fmt("    %lld wait for access %lld\n",
					frame->getId(),
					older_frame->getId());

			// Enqueue
			older_frame->queue.Wait(event_type_store_lock);
			return;
		}

		// Call 'find-and-lock'
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				module,
				frame->getAddress());
		new_frame->request_direction = Frame::RequestDirectionUpDown;
		new_frame->blocking = true;
		new_frame->write = true;
		new_frame->retry = frame->retry;
		new_frame->witness = frame->witness;
		esim_engine->Call(event_type_find_and_lock,
				new_frame,
				event_type_store_action);
		return;
	}

	// Event "store_action"
	if (event_type == event_type_store_action)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s store action\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:store_action\"\n",
				frame->getId(),
				module->getName().c_str());

		// Error locking
		if (frame->error)
		{
			// Calculate retry latency
			int retry_latency = module->getRetryLatency();

			// Debug
			debug << misc::fmt("    lock error, retrying in "
					"%d cycles\n",
					retry_latency);

			// Reschedule 'store-lock' after lantecy
			frame->retry = true;
			esim_engine->Next(event_type_store_lock, retry_latency);
			return;
		}

		// Hit - state=M/E
		if (frame->state == Cache::BlockModified ||
			frame->state == Cache::BlockExclusive)
		{
			// Continue with 'store-unlock'
			esim_engine->Next(event_type_store_unlock);
			return;
		}

		// Miss - state=O/S/I/N
		// Call 'write-request'
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				module,
				frame->tag);
		new_frame->target_module = module->getLowModuleServingAddress(frame->tag);
		new_frame->request_direction = Frame::RequestDirectionUpDown;
		new_frame->witness = frame->witness;
		esim_engine->Call(event_type_write_request,
				new_frame,
				event_type_store_unlock);
		return;
	}

	// Event "store_unlock"
	if (event_type == event_type_store_unlock)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s store unlock\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:store_unlock\"\n",
				frame->getId(),
				module->getName().c_str());

		// Error in write request, unlock block and retry store.
		if (frame->error)
		{
			// Calculate retry latency
			int retry_latency = module->getRetryLatency();

			// Debug
			debug << misc::fmt("    lock error, retrying in "
					"%d cycles\n", retry_latency);

			// Unlock directory entry
			directory->UnlockEntry(frame->set, frame->way);

			// Continue with 'store-lock' after latency
			frame->retry = true;
			esim_engine->Next(event_type_store_lock, retry_latency);
			return;
		}

		// Update tag/state and unlock
		cache->setBlock(frame->set, frame->way, frame->tag,
				Cache::BlockModified);
		directory->UnlockEntry(frame->set, frame->way);

		// Continue to 'store-finish' after data latency
		module->incDataAccesses();
		esim_engine->Next(event_type_store_finish,
				module->getDataLatency());
		return;
	}

	// Event "store_finish"
	if (event_type == event_type_store_finish)
	{
		// Debug and trace
		debug << misc::fmt("%lld %lld 0x%x %s store finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:store_finish\"\n",
				frame->getId(),
				module->getName().c_str());
		trace << misc::fmt("mem.end_access "
				"name=\"A-%lld\"\n",
				frame->getId());

		// Finish access
		module->FinishAccess(frame);

		// Return
		esim_engine->Return();
		return;
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
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(event_frame);
	Frame *parent_frame = misc::cast<Frame *>(esim_engine->getParentFrame());
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
	Frame *parent_frame = misc::cast<Frame *>(esim_engine->getParentFrame());
	Module *module = frame->getModule();
	Module *target_module = frame->target_module;
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
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s evict invalid\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:evict_invalid\"\n",
				frame->getId(),
				module->getName().c_str());

		// Update the cache state since it may have changed after its 
		// higher-level modules were invalidated.
		unsigned tag;
		cache->getBlock(frame->set, frame->way, tag, frame->state);
		
		// If module is main memory, we just need to set the block 
		// as invalid, and finish.
		if (module->getType() == Module::TypeMainMemory)
		{
			cache->setBlock(frame->src_set,
					frame->src_way,
					0,
					Cache::BlockInvalid);

			// Continue with 'evict-finish'
			esim_engine->Next(event_type_evict_finish);
			return;
		}

		// Note: if state is invalid (happens during invalidate?)
		// just skip to FINISH also?
		// Just set the block to invalid if there is no data to
		// return, and let the protocol deal with catching up later.
		if (module->getType() == Module::TypeCache &&
				(frame->state == Cache::BlockShared ||
				frame->state == Cache::BlockExclusive))
		{
			cache->setBlock(frame->src_set,
					frame->src_way,
					0,
					Cache::BlockInvalid);
			frame->state = Cache::BlockInvalid;
			esim_engine->Next(event_type_evict_finish);
			return;
		}

		// Continue with 'evict-action'
		esim_engine->Next(event_type_evict_action);
		return;
	}

	// Event "evict_action"
	if (event_type == event_type_evict_action)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s evict action\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:evict_action\"\n",
				frame->getId(),
				module->getName().c_str());

		// Get low node
		Module *low_module = frame->target_module;
		net::EndNode *low_node = low_module->getHighNetworkNode();
		assert(low_module != module);
		assert(low_module == module->getLowModuleServingAddress(frame->tag));
		assert(low_node && low_node->getUserData() == low_module);

		// State = I
		if (frame->state == Cache::BlockInvalid)
		{
			esim_engine->Next(event_type_evict_finish);
			return;
		}

		// If state is M/O/N, data must be sent to lower level module
		if (frame->state == Cache::BlockModified ||
			frame->state == Cache::BlockOwned ||
			frame->state == Cache::BlockNonCoherent)
		{
			// Need to transmit data to low module.
			frame->reply = Frame::ReplyAckData;
		}

		// States E/S shouldn't happen
		else 
		{
			throw misc::Panic("Unexpected E or S states");
		}

		// Send message
		int message_size = 8 + module->getBlockSize();
		net::Network *network = module->getLowNetwork();
		net::EndNode *source_node = module->getLowNetworkNode();
		frame->message = network->TrySend(source_node,
				low_node,
				message_size,
				event_type_evict_receive,
				event_type);
		if (frame->message)
			trace << misc::fmt("net.msg_access "
					"net=\"%s\" "
					"name=\"M-%lld\" "
					"access=\"A-%lld\"\n",
					network->getName().c_str(),
					frame->message->getId(),
					frame->getId());
		return;
	}

	// Event "evict_receive"
	if (event_type == event_type_evict_receive)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s evict receive\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:evict_receive\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Receive message
		net::Network *network = target_module->getHighNetwork();
		net::Node *node = target_module->getHighNetworkNode();
		network->Receive(node, frame->message);

		// Call find-and-lock
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				target_module,
				frame->src_tag);
		new_frame->blocking = false;
		new_frame->request_direction = Frame::RequestDirectionDownUp;
		new_frame->write = true;
		new_frame->retry = false;
		if (frame->state == Cache::BlockNonCoherent)
			esim_engine->Call(event_type_find_and_lock,
					new_frame,
					event_type_evict_process_noncoherent);
		else
			esim_engine->Call(event_type_find_and_lock,
					new_frame,
					event_type_evict_process);
		return;
	}

	// Event "evict_process"
	if (event_type == event_type_evict_process)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s evict process\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:evict_process\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Error locking block
		if (frame->error)
		{
			parent_frame->error = true;
			esim_engine->Next(event_type_evict_reply);
			return;
		}

		// If data was received, set the block to modified
		if (frame->reply == Frame::ReplyAck)
		{
			// Nothing to do
		}
		else if (frame->reply == Frame::ReplyAckData)
		{
			if (frame->state == Cache::BlockExclusive)
			{
				Cache *target_cache = target_module->getCache();
				target_cache->setBlock(frame->set,
						frame->way,
						frame->tag,
						Cache::BlockModified);
			}
			else if (frame->state == Cache::BlockModified)
			{
				// Nothing to do
			}
			else
			{
				throw misc::Panic("Invalid block state");
			}
		}
		else 
		{
			throw misc::Panic("Invalid cache reply");
		}

		// Remove sharer and owner
		Directory *directory = target_module->getDirectory();
		for (int z = 0; z < directory->getNumSubBlocks(); z++)
		{
			// Skip other sub-blocks
			unsigned directory_entry_tag = frame->tag + 
					z * target_module->getSubBlockSize();
			assert(directory_entry_tag < frame->tag + (unsigned) target_module->getBlockSize());
			if (directory_entry_tag < (unsigned) frame->src_tag || 
					directory_entry_tag >=
					frame->src_tag + 
					(unsigned) module->getBlockSize())
				continue;

			Directory::Entry *directory_entry = directory->getEntry(
					frame->set, frame->way, z);
			directory->clearSharer(frame->set,
					frame->way,
					z,
					module->getLowNetworkNode()->getIndex());
			if (directory_entry->getOwner() == module->
					getLowNetworkNode()->getIndex())
				directory->setOwner(frame->set,
						frame->way,
						z,
						Directory::NoOwner);
		}

		// Unlock the directory entry
		directory->UnlockEntry(frame->set, frame->way);

		// Stats
		target_module->incDataAccesses();

		// Continue with 'evict-reply', after data latency
		esim_engine->Next(event_type_evict_reply,
				target_module->getDataLatency());
		return;
	}

	// Event "evict_process_noncoherent"
	if (event_type == event_type_evict_process_noncoherent)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"evict process noncoherent\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:evict_process_noncoherent\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Error locking block
		if (frame->error)
		{
			parent_frame->error = true;
			esim_engine->Next(event_type_evict_reply);
			return;
		}

		// If data was received, set the block to modified
		Cache *target_cache = target_module->getCache();
		if (frame->reply == Frame::ReplyAckData)
		{
			switch (frame->state)
			{
			case Cache::BlockExclusive:

				target_cache->setBlock(frame->set,
						frame->way,
						frame->tag,
						Cache::BlockModified);
				break;
			
			case Cache::BlockOwned:
			case Cache::BlockModified:

				// Nothing to do
				break;
			
			case Cache::BlockShared:
			case Cache::BlockNonCoherent:

				target_cache->setBlock(frame->set,
						frame->way,
						frame->tag,
						Cache::BlockNonCoherent);
				break;
			
			default:

				throw misc::Panic("Invalid cache block state");
			}
		}
		else 
		{
			throw misc::Panic("Invalid cache reply");
		}

		// Remove sharer and owner
		Directory *directory = target_module->getDirectory();
		for (int z = 0; z < directory->getNumSubBlocks(); z++)
		{
			// Skip other sub-blocks
			unsigned directory_entry_tag = frame->tag + z *
					target_module->getSubBlockSize();
			assert(directory_entry_tag < frame->tag + (unsigned)
					target_module->getBlockSize());
			if (directory_entry_tag < (unsigned) frame->src_tag || 
					directory_entry_tag >= frame->src_tag +
					(unsigned) module->getBlockSize())
				continue;

			// Set sharer and owner
			Directory::Entry *entry = directory->getEntry(
					frame->set,
					frame->way,
					z);
			int index = module->getLowNetworkNode()->getIndex();
			directory->clearSharer(frame->set,
					frame->way,
					z,
					index);
			if (entry->getOwner() == index)
				directory->setOwner(frame->set,
						frame->way,
						z,
						Directory::NoOwner);
		}

		// Unlock the directory entry
		directory->UnlockEntry(frame->set, frame->way);

		// Stats
		target_module->incDataAccesses();
		
		// Continue with 'evict-reply' after latency
		esim_engine->Next(event_type_evict_reply,
				target_module->getDataLatency());
		return;
	}

	// Event "evict_reply"
	if (event_type == event_type_evict_reply)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"evict reply\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:evict_reply\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Send message
		net::Network *network = target_module->getHighNetwork();
		net::EndNode *source_node = target_module->getHighNetworkNode();
		net::EndNode *destination_node = module->getLowNetworkNode();
		frame->message = network->TrySend(source_node,
				destination_node,
				8,
				event_type_evict_reply_receive,
				event_type);
		if (frame->message)
			trace << misc::fmt("net.msg_access "
					"net=\"%s\" "
					"name=\"M-%lld\" "
					"access=\"A-%lld\"\n",
					network->getName().c_str(),
					frame->message->getId(),
					frame->getId());
		return;
	}

	// Event "evict_reply_receive"
	if (event_type == event_type_evict_reply_receive)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"evict reply receive\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:evict_reply_receive\"\n",
				frame->getId(),
				module->getName().c_str());

		// Receive message
		net::Network *network = module->getLowNetwork();
		net::Node *node = module->getLowNetworkNode();
		network->Receive(node, frame->message);

		// Invalidate block if there was no error
		if (!frame->error)
			cache->setBlock(frame->src_set,
					frame->src_way,
					0,
					Cache::BlockInvalid);
		
		// Sanity
		assert(!directory->isBlockSharedOrOwned(frame->src_set, frame->src_way));

		// Continue with 'evict-finish'
		esim_engine->Next(event_type_evict_finish);
		return;
	}

	// Event "evict_finish"
	if (event_type == event_type_evict_finish)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s evict finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:evict_finish\"\n",
				frame->getId(),
				module->getName().c_str());

		// Return
		esim_engine->Return();
		return;
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventWriteRequestHandler(esim::EventType *event_type,
		esim::EventFrame *event_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(event_frame);
	Frame *parent_frame = misc::cast<Frame *>(esim_engine->getParentFrame());
	Module *module = frame->getModule();
	Module *target_module = frame->target_module;
	Directory *target_directory = target_module->getDirectory();

	// Event "write_request"
	if (event_type == event_type_write_request)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s write request\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:write_request\"\n",
				frame->getId(),
				module->getName().c_str());

		// Default return values
		parent_frame->error = false;

		// For write requests, we need to set the initial reply size 
		// because in updown, peer transfers must be allowed to 
		// decrease this value (during invalidate). If the request 
		// turns out to be downup, then these values will get 
		// overwritten.
		frame->reply_size = module->getBlockSize() + 8;
		frame->setReplyIfHigher(Frame::ReplyAckData);

		// Sanity
		assert(frame->request_direction);
		assert(module->getLowModuleServingAddress(frame->getAddress()) == target_module ||
				frame->request_direction == Frame::RequestDirectionDownUp);
		assert(target_module->getLowModuleServingAddress(frame->getAddress()) == module ||
				frame->request_direction == Frame::RequestDirectionUpDown);

		// Get source and destination nodes
		net::Network *network;
		net::EndNode *source_node;
		net::EndNode *destination_node;
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
				event_type_write_request_receive,
				event_type);
		if (frame->message)
			trace << misc::fmt("net.msg_access "
					"net=\"%s\" "
					"name=\"M-%lld\" "
					"access=\"A-%lld\"\n",
					network->getName().c_str(),
					frame->message->getId(),
					frame->getId());
		return;
	}

	// Event "write_request_receive"
	if (event_type == event_type_write_request_receive)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"write request receive\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:write_request_receive\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Receive message
		net::Network *network;
		net::Node *node;
		if (frame->request_direction == Frame::RequestDirectionUpDown)
		{
			network = target_module->getHighNetwork();
			node = target_module->getHighNetworkNode();
		}
		else
		{
			network = target_module->getLowNetwork();
			node = target_module->getLowNetworkNode();
		}
		network->Receive(node, frame->message);
		
		// Call 'find-and-lock'
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				target_module,
				frame->getAddress());
		new_frame->blocking = frame->request_direction ==
				Frame::RequestDirectionDownUp;
		new_frame->request_direction = frame->request_direction;
		new_frame->write = true;
		new_frame->retry = false;
		esim_engine->Call(event_type_find_and_lock,
				new_frame,
				event_type_write_request_action);
		return;
	}

	// Event "write_request_action"
	if (event_type == event_type_write_request_action)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s write request action\n", 
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:write_request_action\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Check lock error. If write request is down-up, there should
		// have been no error.
		if (frame->error)
		{
			// Return error
			assert(frame->request_direction == Frame::RequestDirectionUpDown);
			parent_frame->error = true;
			
			// Adjust reply size
			frame->reply_size = 8;

			// Continue with 'write-request-reply'
			esim_engine->Next(event_type_write_request_reply);
			return;
		}

		// If we have a down-up write request, it is possible that the
		// block has already been evicted by the higher-level cache if
		// it was in an unmodified state.
		if (frame->block_not_found)
		{
			// TODO Create a reply_ack_removed message so that
			// the cache can update its metadata.
			assert(frame->request_direction == Frame::RequestDirectionDownUp);

			// Simply send an ack
			parent_frame->setReplyIfHigher(Frame::ReplyAck);

			// Adjust reply size
			frame->reply_size = 8;

			// Continue with 'write-request-reply'
			esim_engine->Next(event_type_write_request_reply);
			return;
		}

		// Invalidate the rest of higher-level sharers.
		// Call 'invalidate' event chain.
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				target_module,
				0);
		new_frame->except_module = module;
		new_frame->set = frame->set;
		new_frame->way = frame->way;
		esim_engine->Call(event_type_invalidate,
				new_frame,
				event_type_write_request_exclusive);
		return;
	}

	// Event "write_request_exclusive"
	if (event_type == event_type_write_request_exclusive)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"write request exclusive\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:write_request_exclusive\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Continue with 'write-request-updown' or
		// 'write-request-downup', depending on direction.
		if (frame->request_direction == Frame::RequestDirectionUpDown)
			esim_engine->Next(event_type_write_request_updown);
		else
			esim_engine->Next(event_type_write_request_downup);
		return;
	}

	// Event "write_request_updown"
	if (event_type == event_type_write_request_updown)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s write request updown\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:write_request_updown\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Check state
		switch (frame->state)
		{

		// State = M/E
		case Cache::BlockModified:
		case Cache::BlockExclusive:
		{
			esim_engine->Next(event_type_write_request_updown_finish);
			break;
		}
		
		// State = O/S/I/N
		case Cache::BlockOwned:
		case Cache::BlockShared:
		case Cache::BlockInvalid:
		case Cache::BlockNonCoherent:
		{
			auto new_frame = misc::new_shared<Frame>(
					frame->getId(),
					target_module,
					frame->tag);
			new_frame->target_module = target_module->
					getLowModuleServingAddress(frame->tag);
			new_frame->request_direction = Frame::RequestDirectionUpDown;
			esim_engine->Call(event_type_write_request,
					new_frame,
					event_type_write_request_updown_finish);
			break;
		}
		
		default:

			throw misc::Panic("Invalid block state");
		}

		return;
	}

	// Event "write_request_updown_finish"
	if (event_type == event_type_write_request_updown_finish)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"write request updown finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:write_request_updown_finish\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Ensure that a reply was received
		assert(frame->reply);

		// Error in write request to next cache level
		if (frame->error)
		{
			// Return error
			parent_frame->error = true;
			parent_frame->setReplyIfHigher(Frame::ReplyAckError);

			// Adjust reply size
			frame->reply_size = 8;

			// Unlock directory entry
			target_directory->UnlockEntry(frame->set, frame->way);

			// Continue with 'write-request-reply'
			esim_engine->Next(event_type_write_request_reply);
			return;
		}

		// Check that address is a multiple of the module's block size.
		// Set module as sharer and owner. */
		for (int z = 0; z < target_directory->getNumSubBlocks(); z++)
		{
			assert(frame->getAddress() % module->getBlockSize() == 0);
			unsigned directory_entry_tag = frame->tag +
					z * target_module->getSubBlockSize();
			assert(directory_entry_tag < frame->tag + 
					(unsigned) target_module->getBlockSize());
			if (directory_entry_tag < frame->getAddress() || 
					directory_entry_tag >=
					frame->getAddress() +
					(unsigned) module->getBlockSize())
				continue;

			// Set sharer and owner
			int index = module->getLowNetworkNode()->getIndex();
			Directory::Entry *entry = target_directory->getEntry(
					frame->set,
					frame->way,
					z);
			target_directory->setSharer(frame->set,
					frame->way,
					z,
					index);
			target_directory->setOwner(frame->set,
					frame->way,
					z,
					index);
			assert(entry->getNumSharers() == 1);
		}

		// Set state to E
		Cache *target_cache = target_module->getCache();
		target_cache->setBlock(frame->set,
				frame->way,
				frame->tag,
				Cache::BlockExclusive);

		// If blocks were sent directly to the peer, the reply size 
		// would have been decreased.  Based on the final size, we can
		// tell whether to send more data up or simply ack
		if (frame->reply_size == 8) 
			parent_frame->setReplyIfHigher(Frame::ReplyAck);
		else if (frame->reply_size > 8)
			parent_frame->setReplyIfHigher(Frame::ReplyAckData);
		else 
			throw misc::Panic("Invalid reply size");

		// Unlock directory entry
		target_directory->UnlockEntry(frame->set, frame->way);

		// Stats
		target_module->incDataAccesses();

		// Continue with 'write-request-reply' after data latency
		esim_engine->Next(event_type_write_request_reply,
				target_module->getDataLatency());
		return;
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
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"write request reply\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:write_request_reply\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Sanity
		assert(frame->reply_size);
		assert(module->getLowModuleServingAddress(frame->getAddress()) == target_module ||
				target_module->getLowModuleServingAddress(frame->getAddress()) == module);

		// Get network and nodes
		net::Network *network;
		net::EndNode *source_node;
		net::EndNode *destination_node;
		if (frame->request_direction == Frame::RequestDirectionUpDown)
		{
			network = module->getLowNetwork();
			source_node = target_module->getHighNetworkNode();
			destination_node = module->getLowNetworkNode();
		}
		else
		{
			network = module->getHighNetwork();
			source_node = target_module->getLowNetworkNode();
			destination_node = module->getHighNetworkNode();
		}

		// Send message
		frame->message = network->TrySend(source_node,
				destination_node,
				frame->reply_size,
				event_type_write_request_finish,
				event_type);
		if (frame->message)
			trace << misc::fmt("net.msg_access "
					"net=\"%s\" "
					"name=\"M-%lld\" "
					"access=\"A-%lld\"\n",
					network->getName().c_str(),
					frame->message->getId(),
					frame->getId());
		return;
	}

	// Event "write_request_finish"
	if (event_type == event_type_write_request_finish)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"write request finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:write_request_finish\"\n",
				frame->getId(),
				module->getName().c_str());

		// Receive message
		net::Network *network;
		net::Node *node;
		if (frame->request_direction == Frame::RequestDirectionDownUp)
		{
			network = module->getLowNetwork();
			node = module->getLowNetworkNode();
		}
		else
		{
			network = module->getHighNetwork();
			node = module->getHighNetworkNode();
		}
		network->Receive(node, frame->message);
		
		// If the write request was generated from a store, we can
		// be sure that it will complete at this point and so can
		// increment the witness pointer to allow processing to 
		// continue while assuring consistency.
		if (frame->request_direction == Frame::RequestDirectionUpDown
				&& frame->witness)
			(*frame->witness)++;

		// Return
		esim_engine->Return();
		return;
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
	Frame *parent_frame = misc::cast<Frame *>(esim_engine->getParentFrame());
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
		net::EndNode *source_node;
		net::EndNode *destination_node;
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
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s read request receive\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:read_request_receive\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Receive message
		if (frame->request_direction == Frame::RequestDirectionUpDown)
		{
			net::Network *network = target_module->getHighNetwork();
			net::Node *node = target_module->getHighNetworkNode();
			network->Receive(node, frame->message);
		}
		else
		{
			net::Network *network = target_module->getLowNetwork();
			net::Node *node = target_module->getLowNetworkNode();
			network->Receive(node, frame->message);
		}
		
		// Call 'find-and-lock'
		// TODO Read requests should always be able to be blocking.  
		// It's impossible to get into a livelock situation because we 
		// only need to hit and not have ownership.  We would never 
		// cross paths with a request coming down-up because we would
		// hit before that.
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				target_module,
				frame->getAddress());
		new_frame->request_direction = frame->request_direction;
		new_frame->blocking = frame->request_direction ==
				Frame::RequestDirectionDownUp;
		new_frame->read = true;
		new_frame->retry = false;
		esim_engine->Call(event_type_find_and_lock,
				new_frame,
				event_type_read_request_action);
		return;
	}

	// Event "read_request_action"
	if (event_type == event_type_read_request_action)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s read request action\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:read_request_action\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Check block locking error. If read request is down-up, 
		// there should not have been any error while locking.
		if (frame->error)
		{
			assert(frame->request_direction == Frame::RequestDirectionUpDown);
			parent_frame->error = true;
			parent_frame->setReplyIfHigher(Frame::ReplyAckError);
			frame->reply_size = 8;

			// Continue with 'read-request-reply'
			esim_engine->Next(event_type_read_request_reply);
			return;
		}

		// If we have a down-up read request, it is possible that the 
		// block has already been evicted by the higher-level cache if
		// it was in an unmodified state.
		if (frame->block_not_found)
		{
			// TODO Create a reply_ack_removed message so that
			// the cache can update its metadata
			assert(frame->request_direction == Frame::RequestDirectionDownUp);

			// Simply send an ack
			parent_frame->setReplyIfHigher(Frame::ReplyAck);
			frame->reply_size = 8;

			// Continue with 'read-request-reply'
			esim_engine->Next(event_type_read_request_reply);
			return;
		}

		// Continue with 'read-request-updown' or 'read-request-downup'
		esim_engine->Next(frame->request_direction == Frame::RequestDirectionUpDown ?
				event_type_read_request_updown :
				event_type_read_request_downup);
		return;
	}

	// Event "read_request_updown"
	if (event_type == event_type_read_request_updown)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s read request updown\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:read_request_updown\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// One pending request initially
		frame->pending = 1;

		// Set the initial reply message and size.  This will be 
		// adjusted later if a transfer occurs between peers.
		frame->reply_size = module->getBlockSize() + 8;
		frame->setReplyIfHigher(Frame::ReplyAckData);
	
		// Check state
		if (frame->state)
		{
			// Status = M/O/E/S/N
			// Check: address is a multiple of requester's 
			// block_size. Send read request to owners other than
			// 'module' for all sub-blocks.
			assert(frame->getAddress() % module->getBlockSize() == 0);
			Directory *directory = target_module->getDirectory();
			for (int z = 0; z < directory->getNumSubBlocks(); z++)
			{
				// Check that address is a multiple of block
				// size.
				unsigned directory_entry_tag = frame->tag + z * target_module->getSubBlockSize();
				assert(directory_entry_tag < frame->tag + (unsigned) target_module->getBlockSize());

				// Get directory entry
				Directory::Entry *directory_entry = directory->getEntry(
						frame->set,
						frame->way,
						z);

				// No owner, skip
				if (directory_entry->getOwner() == Directory::NoOwner)
					continue;

				// Owner is 'module', skip
				if (directory_entry->getOwner() == module->getLowNetworkNode()->getIndex())
					continue;

				// Get owner module
				net::Network *network = target_module->getHighNetwork();
				net::Node *node = network->getNode(directory_entry->getOwner());
				assert(dynamic_cast<net::EndNode *>(node));
				Module *owner_module = (Module *) node->getUserData();
				assert(owner_module);

				// Not the first sub-block, skip
				if (directory_entry_tag % owner_module->getBlockSize())
					continue;

				// One more pending request
				frame->pending++;

				// Call 'read-request'
				auto new_frame = misc::new_shared<Frame>(
						frame->getId(),
						target_module,
						directory_entry_tag);
				new_frame->target_module = owner_module;
				new_frame->request_direction = Frame::RequestDirectionDownUp;
				esim_engine->Call(event_type_read_request,
						new_frame,
						event_type_read_request_updown_finish);
			}

			// Continue with 'read-request-updown-finish'
			esim_engine->Next(event_type_read_request_updown_finish);
		}
		else
		{
			// State = I
			Directory *directory = target_module->getDirectory();
			assert(!directory->isBlockSharedOrOwned(frame->set, frame->way));

			// Call 'read-request'
			auto new_frame = misc::new_shared<Frame>(
					frame->getId(),
					target_module,
					frame->tag);
			new_frame->target_module = target_module->getLowModuleServingAddress(frame->tag);
			new_frame->request_direction = Frame::RequestDirectionUpDown;
			esim_engine->Call(event_type_read_request,
					new_frame,
					event_type_read_request_updown_miss);
		}
		return;
	}

	// Event "read_request_updown_miss"
	if (event_type == event_type_read_request_updown_miss)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"read request updown miss\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:read_request_updown_miss\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Check error
		if (frame->error)
		{
			// Unlock directory entry
			Directory *directory = target_module->getDirectory();
			directory->UnlockEntry(frame->set, frame->way);

			// Return error
			parent_frame->error = true;
			parent_frame->setReplyIfHigher(Frame::ReplyAckError);

			// Continue with 'read-request-reply'
			frame->reply_size = 8;
			esim_engine->Next(event_type_read_request_reply);
			return;
		}

		// Set block state to E/S depending on the return value 'shared'
		// that comes from a read request into the next cache level.
		// Also set the tag of the block.
		Cache *target_cache = target_module->getCache();
		target_cache->setBlock(frame->set,
				frame->way,
				frame->tag,
				frame->shared ? Cache::BlockShared : Cache::BlockExclusive);

		// Continue with 'read-request-updown-finish'
		esim_engine->Next(event_type_read_request_updown_finish);
		return;
	}

	// Event "read_request_updown_finish"
	if (event_type == event_type_read_request_updown_finish)
	{
		// Ensure that a reply was received
		assert(frame->reply);

		// Ignore while pending requests
		assert(frame->pending > 0);
		frame->pending--;
		if (frame->pending)
			return;

		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"read request updown finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:read_request_updown_finish\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// If blocks were sent directly to the peer, the reply size
		// would have been decreased.  Based on the final size, we can
		// tell whether to send more data or simply ack.
		if (frame->reply_size == 8) 
		{
			parent_frame->setReplyIfHigher(Frame::ReplyAck);
		}
		else if (frame->reply_size > 8)
		{
			parent_frame->setReplyIfHigher(Frame::ReplyAckData);
		}
		else 
		{
			throw misc::Panic("Invalid reply size");
		}

		// Get directory
		Directory *directory = target_module->getDirectory();
		bool shared = false;

		// With the Owned state, the directory entry may remain owned by
		// the sender.
		if (!frame->retain_owner)
		{
			// Set owner to 0 for all directory entries not owned by
			// the module.
			for (int z = 0; z < directory->getNumSubBlocks(); z++)
			{
				Directory::Entry *entry = directory->getEntry(
						frame->set,
						frame->way,
						z);
				if (entry->getOwner() != module->getLowNetworkNode()->getIndex())
					directory->setOwner(frame->set,
							frame->way,
							z,
							Directory::NoOwner);
			}
		}

		// For each sub-block requested by the module, set it as sharer,
		// and check whether there is other cache sharing it. */
		for (int z = 0; z < directory->getNumSubBlocks(); z++)
		{
			unsigned directory_entry_tag = frame->tag + z * target_module->getSubBlockSize();
			if (directory_entry_tag < frame->getAddress() ||
					directory_entry_tag >= frame->getAddress()
					+ (unsigned) module->getBlockSize())
				continue;
			
			Directory::Entry *entry = directory->getEntry(frame->set,
					frame->way,
					z);
			directory->setSharer(frame->set,
					frame->way,
					z,
					module->getLowNetworkNode()->getIndex());
			if (entry->getNumSharers() > 1 || frame->nc_write || frame->shared)
				shared = true;

			// If the block is owned, non-coherent, or shared,  
			// 'module' (the higher-level cache) should never be E.
			if (frame->state == Cache::BlockOwned ||
					frame->state == Cache::BlockNonCoherent ||
					frame->state == Cache::BlockShared)
				shared = true;
		}

		// If no sub-block requested by 'module' is shared by other
		// cache, set 'module' as owner of all of them. Otherwise,
		// notify requester that the block is shared by setting the
		// 'shared' return value to true.
		parent_frame->shared = shared;
		if (!shared)
		{
			for (int z = 0; z < directory->getNumSubBlocks(); z++)
			{
				unsigned directory_entry_tag = frame->tag + z * target_module->getSubBlockSize();
				if (directory_entry_tag < frame->getAddress() ||
						directory_entry_tag >= frame->getAddress()
						+ (unsigned) module->getBlockSize())
					continue;

				// Set owner
				directory->setOwner(
						frame->set,
						frame->way,
						z,
						module->getLowNetworkNode()->getIndex());
			}
		}

		// Unlock directory entry
		directory->UnlockEntry(frame->set, frame->way);

		// Stats
		target_module->incDataAccesses();

		// Continue with 'read-request-reply' after latency
		esim_engine->Next(event_type_read_request_reply,
				target_module->getDataLatency());
		return;
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
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"read request reply\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:read_request_reply\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Checks
		assert(frame->reply_size);
		assert(frame->request_direction);
		assert(module->getLowModuleServingAddress(frame->getAddress())
				== target_module ||
				target_module->getLowModuleServingAddress(frame->getAddress())
				== module);

		// Get network and nodes
		net::Network *network;
		net::EndNode *source_node;
		net::EndNode *destination_node;
		if (frame->request_direction == Frame::RequestDirectionUpDown)
		{
			network = module->getLowNetwork();
			source_node = target_module->getHighNetworkNode();
			destination_node = module->getLowNetworkNode();
		}
		else
		{
			network = module->getHighNetwork();
			source_node = target_module->getLowNetworkNode();
			destination_node = module->getHighNetworkNode();
		}

		// Send message
		frame->message = network->TrySend(
				source_node,
				destination_node,
				frame->reply_size,
				event_type_read_request_finish,
				event_type);
		if (frame->message)
			trace << misc::fmt("net.msg_access "
					"net=\"%s\" "
					"name=\"M-%lld\" "
					"access=\"A-%lld\"\n",
					network->getName().c_str(),
					frame->message->getId(),
					frame->getId());
		return;
	}

	// Event "read_request_finish"
	if (event_type == event_type_read_request_finish)
	{
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s "
				"read request finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:read_request_finish\"\n",
				frame->getId(),
				module->getName().c_str());

		// Receive message
		net::Network *network;
		net::Node *node;
		if (frame->request_direction == Frame::RequestDirectionUpDown)
		{
			network = module->getLowNetwork();
			node = module->getLowNetworkNode();
		}
		else
		{
			network = module->getHighNetwork();
			node = module->getHighNetworkNode();
		}
		network->Receive(node, frame->message);

		// Return
		esim_engine->Return();
		return;
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
					(unsigned) module->getBlockSize());
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
		// Debug and trace
		debug << misc::fmt("  %lld %lld 0x%x %s invalidate finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:invalidate_finish\"\n",
				frame->getId(),
				module->getName().c_str());

		// The following line updates the block state.  We must
		// be sure that the directory entry is always locked if we
		// allow this to happen.
		if (frame->reply == Frame::ReplyAckData)
			cache->setBlock(frame->set, frame->way, frame->tag,
					Cache::BlockModified);

		// Ignore while pending
		assert(frame->pending > 0);
		frame->pending--;
		if (frame->pending)
			return;

		// Return to parent event chain
		esim_engine->Return();
		return;
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

