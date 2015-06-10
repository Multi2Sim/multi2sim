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

esim::EventType *System::event_type_prefetch;
esim::EventType *System::event_type_prefetch_lock;
esim::EventType *System::event_type_prefetch_action;
esim::EventType *System::event_type_prefetch_miss;
esim::EventType *System::event_type_prefetch_unlock;
esim::EventType *System::event_type_prefetch_finish;

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

esim::EventType *System::event_type_peer_send;
esim::EventType *System::event_type_peer_receive;
esim::EventType *System::event_type_peer_reply;
esim::EventType *System::event_type_peer_finish;

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
		throw misc::Panic("Not implemented");
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


void System::EventStoreHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}


void System::EventNCStoreHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}


void System::EventPrefetchHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
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
		throw misc::Panic("Not implemented");
	}

	// Event "find_and_lock_finish"
	if (event_type == event_type_find_and_lock_finish)
	{
		throw misc::Panic("Not implemented");
	}
}


void System::EventEvictHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}


void System::EventWriteRequestHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}


void System::EventReadRequestHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}


void System::EventInvalidateHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}


void System::EventPeerHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}


void System::EventMessageHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}


void System::EventFlushHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}


void System::EventLocalLoadHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}


void System::EventLocalStoreHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}


void System::EventLocalFindAndLockHandler(esim::EventType *type, esim::EventFrame *frame)
{
	throw misc::Panic("Not implemented");
}



}

