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
	
esim::Event *System::event_load;
esim::Event *System::event_load_lock;
esim::Event *System::event_load_action;
esim::Event *System::event_load_miss;
esim::Event *System::event_load_unlock;
esim::Event *System::event_load_finish;

esim::Event *System::event_store;
esim::Event *System::event_store_lock;
esim::Event *System::event_store_action;
esim::Event *System::event_store_unlock;
esim::Event *System::event_store_finish;

esim::Event *System::event_nc_store;
esim::Event *System::event_nc_store_lock;
esim::Event *System::event_nc_store_writeback;
esim::Event *System::event_nc_store_action;
esim::Event *System::event_nc_store_miss;
esim::Event *System::event_nc_store_unlock;
esim::Event *System::event_nc_store_finish;

esim::Event *System::event_find_and_lock;
esim::Event *System::event_find_and_lock_port;
esim::Event *System::event_find_and_lock_action;
esim::Event *System::event_find_and_lock_finish;

esim::Event *System::event_evict;
esim::Event *System::event_evict_invalid;
esim::Event *System::event_evict_action;
esim::Event *System::event_evict_receive;
esim::Event *System::event_evict_process;
esim::Event *System::event_evict_process_noncoherent;
esim::Event *System::event_evict_reply;
esim::Event *System::event_evict_reply_receive;
esim::Event *System::event_evict_finish;

esim::Event *System::event_write_request;
esim::Event *System::event_write_request_receive;
esim::Event *System::event_write_request_action;
esim::Event *System::event_write_request_exclusive;
esim::Event *System::event_write_request_updown;
esim::Event *System::event_write_request_updown_finish;
esim::Event *System::event_write_request_downup;
esim::Event *System::event_write_request_downup_finish;
esim::Event *System::event_write_request_reply;
esim::Event *System::event_write_request_finish;

esim::Event *System::event_read_request;
esim::Event *System::event_read_request_receive;
esim::Event *System::event_read_request_action;
esim::Event *System::event_read_request_updown;
esim::Event *System::event_read_request_updown_miss;
esim::Event *System::event_read_request_updown_finish;
esim::Event *System::event_read_request_downup;
esim::Event *System::event_read_request_downup_finish;
esim::Event *System::event_read_request_reply;
esim::Event *System::event_read_request_finish;

esim::Event *System::event_invalidate;
esim::Event *System::event_invalidate_finish;

esim::Event *System::event_message;
esim::Event *System::event_message_receive;
esim::Event *System::event_message_action;
esim::Event *System::event_message_reply;
esim::Event *System::event_message_finish;

esim::Event *System::event_flush;
esim::Event *System::event_flush_finish;
	
esim::Event *System::event_local_load;
esim::Event *System::event_local_load_lock;
esim::Event *System::event_local_load_finish;

esim::Event *System::event_local_store;
esim::Event *System::event_local_store_lock;
esim::Event *System::event_local_store_finish;

esim::Event *System::event_local_find_and_lock;
esim::Event *System::event_local_find_and_lock_port;
esim::Event *System::event_local_find_and_lock_action;
esim::Event *System::event_local_find_and_lock_finish;


void System::EventLoadHandler(esim::Event *event, esim::Frame *esim_frame)
{
	// Get engine, frame, and module
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Module *module = frame->getModule();
	Cache *cache = module->getCache();
	Directory *directory = module->getDirectory();

	// Event "load"
	if (event == event_load)
	{
		debug << misc::fmt("%lld A-%lld 0x%x %s load\n",
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
			master_frame->queue.Wait(event_load_finish);
			return;
		}

		// Next event
		esim_engine->Next(event_load_lock);
		return;
	}

	// Event "load_lock"
	if (event == event_load_lock)
	{
		debug << misc::fmt("  %lld A-%lld 0x%x %s load lock\n",
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
			debug << misc::fmt("    A-%lld wait for store A-%lld\n",
					frame->getId(),
					older_frame->getId());
			older_frame->queue.Wait(event_load_lock);
			return;
		}

		// If there is any older access to the same address that this
		// access could not be coalesced with, wait for it.
		older_frame = module->getInFlightAddress(
				frame->getAddress(),
				frame);
		if (older_frame)
		{
			debug << misc::fmt("    A-%lld wait for access A-%lld\n",
					frame->getId(),
					older_frame->getId());
			older_frame->queue.Wait(event_load_lock);
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
		esim_engine->Call(event_find_and_lock,
				new_frame,
				event_load_action);
		return;
	}


	// Event "load_action"
	if (event == event_load_action)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s load_action\n",
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
			esim_engine->Next(event_load_lock, retry_latency);
			return;
		}

		// Hit
		if (frame->state)
		{
			// Continue with 'load-unlock'
			esim_engine->Next(event_load_unlock);
			return;
		}

		// Miss
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				module,
				frame->tag);
		new_frame->target_module = module->getLowModuleServingAddress(frame->tag);
		new_frame->request_direction = Frame::RequestDirectionUpDown;
		esim_engine->Call(event_read_request,
				new_frame,
				event_load_miss);

		// Done
		return;
	}


	// Event "load_miss"
	if (event == event_load_miss)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s load_miss\n",
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
			directory->UnlockEntry(frame->set,
					frame->way,
					frame->getId());
			
			// Calculate retry latency
			int retry_latency = module->getRetryLatency();

			// Debug
			debug << misc::fmt("    lock error, retrying "
					"in %d cycles\n", retry_latency);

			// Continue with 'load-lock' after retry latency
			frame->retry = true;
			esim_engine->Next(event_load_lock, retry_latency);
			return;
		}

		// Set block state to E/S depending on return var 'shared'.
		// Also set the tag of the block.
		cache->setBlock(frame->set,
				frame->way,
				frame->tag,
				frame->shared ? Cache::BlockShared : Cache::BlockExclusive);

		// Continue
		esim_engine->Next(event_load_unlock);
		return;
	}


	// Event "load_unlock"
	if (event == event_load_unlock)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
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
		directory->UnlockEntry(frame->set,
				frame->way,
				frame->getId());

		// Stats
		module->incDataAccesses();

		// Continue with 'load-finish' after latency
		esim_engine->Next(event_load_finish,
				module->getDataLatency());
		return;
	}


	// Event "load_finish"
	if (event == event_load_finish)
	{
		// Debug and trace
		debug << misc::fmt("%lld A-%lld 0x%x %s load_finish\n",
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


void System::EventStoreHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Module *module = frame->getModule();
	Cache *cache = module->getCache();
	Directory *directory = module->getDirectory();

	// Event "store"
	if (event == event_store)
	{
		// Debug and trace
		debug << misc::fmt("%lld A-%lld 0x%x %s store\n",
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
			master_frame->queue.Wait(event_store_finish);

			// Increment witness
			if (frame->witness)
				(*frame->witness)++;

			// Done
			return;
		}

		// Continue
		esim_engine->Next(event_store_lock);
		return;
	}

	// Event "store_lock"
	if (event == event_store_lock)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s store_lock\n",
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
		auto it = frame->accesses_iterator;
		assert(it != module->getAccessListEnd());
		if (it != module->getAccessListBegin())
		{
			// Get older access
			--it;
			Frame *older_frame = *it;

			// Debug
			debug << misc::fmt("    A-%lld wait for access A-%lld\n",
					frame->getId(),
					older_frame->getId());

			// Enqueue
			older_frame->queue.Wait(event_store_lock);
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
		esim_engine->Call(event_find_and_lock,
				new_frame,
				event_store_action);
		return;
	}

	// Event "store_action"
	if (event == event_store_action)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s store_action\n",
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
			esim_engine->Next(event_store_lock, retry_latency);
			return;
		}

		// Hit - state=M/E
		if (frame->state == Cache::BlockModified ||
			frame->state == Cache::BlockExclusive)
		{
			// Continue with 'store-unlock'
			esim_engine->Next(event_store_unlock);
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
		esim_engine->Call(event_write_request,
				new_frame,
				event_store_unlock);
		return;
	}

	// Event "store_unlock"
	if (event == event_store_unlock)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s store_unlock\n",
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
			directory->UnlockEntry(frame->set,
					frame->way,
					frame->getId());

			// Continue with 'store-lock' after latency
			frame->retry = true;
			esim_engine->Next(event_store_lock, retry_latency);
			return;
		}

		// Update tag/state
		cache->setBlock(frame->set, frame->way, frame->tag,
				Cache::BlockModified);

		// Unlock directory entry
		directory->UnlockEntry(frame->set,
				frame->way,
				frame->getId());

		// Continue to 'store-finish' after data latency
		module->incDataAccesses();
		esim_engine->Next(event_store_finish,
				module->getDataLatency());
		return;
	}

	// Event "store_finish"
	if (event == event_store_finish)
	{
		// Debug and trace
		debug << misc::fmt("%lld A-%lld 0x%x %s store_finish\n",
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


void System::EventNCStoreHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Module *module = frame->getModule();
	Directory *directory = module->getDirectory();
	Cache *cache = module->getCache();

	// Event "nc_store"
	if (event == event_nc_store)
	{
		// Debug and trace
		debug << misc::fmt("%lld A-%lld 0x%x %s nc_store\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.new_access "
				"name=\"A-%lld\" "
				"type=\"nc_store\" "
				"state=\"%s:nc store\" "
				"addr=0x%x\n",
				frame->getId(),
				module->getName().c_str(),
				frame->getAddress());

		// Record access
		module->StartAccess(frame, Module::AccessNCStore);

		// Coalesce access
		Frame *master_frame = module->canCoalesce(
				Module::AccessNCStore,
				frame->getAddress(),
				frame);
		if (master_frame)
		{
			module->incCoalescedNCWrites();
			module->Coalesce(master_frame, frame);
			master_frame->queue.Wait(event_nc_store_finish);
			return;
		}

		// Continue with 'nc-store-lock'
		esim_engine->Next(event_nc_store_lock);
		return;
	}

	// Event "nc_store_lock"
	if (event == event_nc_store_lock)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s nc_store_lock\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:nc_store_lock\"\n",
				frame->getId(),
				module->getName().c_str());

		// If there is any older write, wait for it
		Frame *older_frame = module->getInFlightWrite(frame);
		if (older_frame)
		{
			// Debug
			debug << misc::fmt("    A-%lld wait for store A-%lld\n",
					frame->getId(),
					older_frame->getId());

			// Wait for access
			older_frame->queue.Wait(event_nc_store_lock);
			return;
		}

		// If there is any older access to the same address that this 
		// access could not be coalesced with, wait for it.
		older_frame = module->getInFlightAddress(frame->getAddress(),
				frame);
		if (older_frame)
		{
			// Debug
			debug << misc::fmt("    A-%lld wait for access A-%lld\n",
					frame->getId(),
					older_frame->getId());

			// Wait for it
			older_frame->queue.Wait(event_nc_store_lock);
			return;
		}

		// Call find and lock
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				module,
				frame->getAddress());
		new_frame->request_direction = Frame::RequestDirectionUpDown;
		new_frame->blocking = true;
		new_frame->nc_write = true;
		new_frame->retry = frame->retry;
		esim_engine->Call(event_find_and_lock,
				new_frame,
				event_nc_store_writeback);
		return;
	}

	// Event "nc_store_writeback"
	if (event == event_nc_store_writeback)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s nc_store_writeback\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:nc_store_writeback\"\n",
				frame->getId(),
				module->getName().c_str());

		// Error locking
		if (frame->error)
		{
			// Calculate retry latency
			int retry_latency = module->getRetryLatency();

			// Debug
			debug << misc::fmt("    lock error, retrying in "
					"%d cycles\n", retry_latency);

			// Retry access after latency
			frame->retry = true;
			esim_engine->Next(event_nc_store_lock, retry_latency);
			return;
		}

		// If the block has modified data, evict it so that the 
		// lower-level cache will have the latest copy
		if (frame->state == Cache::BlockModified ||
				frame->state == Cache::BlockOwned)
		{
			// Record that there was an eviction
			frame->eviction = true;

			// Call 'evict'
			auto new_frame = misc::new_shared<Frame>(
					frame->getId(),
					module,
					0);
			new_frame->set = frame->set;
			new_frame->way = frame->way;
			esim_engine->Call(event_evict,
					new_frame,
					event_nc_store_action);
			return;
		}

		// Continue with 'nc-store-action'
		esim_engine->Next(event_nc_store_action);
		return;
	}

	// Event "nc_store_action"
	if (event == event_nc_store_action)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s nc_store_action\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:nc_store_action\"\n",
				frame->getId(),
				module->getName().c_str());

		// Error locking
		if (frame->error)
		{
			// Calculate retry latency
			int retry_latency = module->getRetryLatency();

			// Debug
			debug << misc::fmt("    lock error, retrying in "
					"%d cycles\n", retry_latency);

			// Retry after latency
			frame->retry = true;
			esim_engine->Next(event_nc_store_lock, retry_latency);
			return;
		}

		// Main memory modules are a special case
		if (module->getType() == Module::TypeMainMemory)
		{
			// For non-coherent stores, finding an E or M for the
			// state of a cache block in the directory still
			// requires a message to the lower-level module so it
			// can update its owner field. These messages should not
			// be sent if the module is a main memory module.
			esim_engine->Next(event_nc_store_unlock);
			return;
		}

		// Check state
		switch (frame->state)
		{

		case Cache::BlockShared:
		case Cache::BlockNonCoherent:
		{
			// N/S are hit
			esim_engine->Next(event_nc_store_unlock);
			break;
		}

		case Cache::BlockExclusive:
		{
			// E state must tell the lower-level module to remove
			// this module as an owner. Call 'message'.
			auto new_frame = misc::new_shared<Frame>(
					frame->getId(),
					module,
					frame->tag);
			new_frame->message_type = Frame::MessageClearOwner;
			new_frame->target_module = module->getLowModuleServingAddress(frame->tag);
			esim_engine->Call(event_message,
					new_frame,
					event_nc_store_miss);
			break;
		}

		case Cache::BlockModified:
		case Cache::BlockOwned:
		case Cache::BlockInvalid:
		{
			// Modified and Owned states need to call read request
			// because we've already evicted the block so that the
			// lower-level cache will have the latest value before
			// it becomes non-coherent. Call 'read-request'.
			auto new_frame = misc::new_shared<Frame>(
					frame->getId(),
					module,
					frame->tag);
			new_frame->nc_write = true;
			new_frame->target_module = module->getLowModuleServingAddress(frame->tag);
			new_frame->request_direction = Frame::RequestDirectionUpDown;
			esim_engine->Call(event_read_request,
					new_frame,
					event_nc_store_miss);
			break;
		}

		default:
			
			throw misc::Panic("Invalid block state");
		}

		// Done
		return;
	}

	// Event "nc_store_miss"
	if (event == event_nc_store_miss)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s nc_store_miss\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:nc_store_miss\"\n",
				frame->getId(),
				module->getName().c_str());

		// Error on read request. Unlock block and retry nc store.
		if (frame->error)
		{
			// Calculate retry latency
			int retry_latency = module->getRetryLatency();

			// Unlock directory entry
			directory->UnlockEntry(frame->set,
					frame->way,
					frame->getId());

			// Debug
			debug << misc::fmt("    lock error, retrying in "
					"%d cycles\n", retry_latency);


			// Continue with 'nc-store-lock' after latency
			frame->retry = true;
			esim_engine->Next(event_nc_store_lock, retry_latency);
			return;
		}

		// Continue with 'nc-store-unlock'
		esim_engine->Next(event_nc_store_unlock);
		return;
	}

	// Event "nc_store_unlock"
	if (event == event_nc_store_unlock)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s nc_store_unlock\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:nc_store_unlock\"\n",
				frame->getId(),
				module->getName().c_str());

		// Set block state to E/S depending on return var 'shared'.
		// Also set the tag of the block.
		cache->setBlock(frame->set, frame->way, frame->tag,
				Cache::BlockNonCoherent);

		// Unlock directory entry
		directory->UnlockEntry(frame->set,
				frame->way,
				frame->getId());

		// Stats
		module->incDataAccesses();

		// Continue with 'store-finish' after access latency
		esim_engine->Next(event_nc_store_finish,
				module->getDataLatency());
		return;
	}

	// Event "nc_store_finish"
	if (event == event_nc_store_finish)
	{
		// Debug and trace
		debug << misc::fmt("%lld A-%lld 0x%x %s nc_store_finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:nc_store_finish\"\n",
				frame->getId(),
				module->getName().c_str());
		trace << misc::fmt("mem.end_access name=\"A-%lld\"\n",
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

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventFindAndLockHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Frame *parent_frame = misc::cast<Frame *>(esim_engine->getParentFrame());
	Module *module = frame->getModule();
	Cache *cache = module->getCache();
	Directory *directory = module->getDirectory();

	// Event "find_and_lock"
	if (event == event_find_and_lock)
	{
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"find_and_lock (blocking=%d)\n",
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
		module->LockPort(frame, event_find_and_lock_port);
		return;
	}

	// Event "find_and_lock_port"
	if (event == event_find_and_lock_port)
	{
		// Get locked port
		Module::Port *port = frame->port;
		assert(port);

		// Debug
		debug << misc::fmt("  %lld A-%lld 0x%x %s find_and_lock_port\n",
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
			debug << misc::fmt("    A-%lld 0x%x %s "
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
				debug << misc::fmt("        A-%lld "
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
				!frame->blocking)
		{
			// Debug
			debug << misc::fmt("    A-%lld 0x%x %s block locked at "
					"set=%d, "
					"way=%d "
					"by A-%lld - aborting\n",
					frame->getId(),
					frame->tag,
					module->getName().c_str(),
					frame->set,
					frame->way,
					directory->getEntryAccessId(frame->set,
							frame->way));

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
		if (!directory->LockEntry(frame->set,
				frame->way,
				event_find_and_lock,
				frame->getId()))
		{
			// Debug
			debug << misc::fmt("    A-%lld 0x%x %s block locked at "
					"set=%d, "
					"way=%d by "
					"A-%lld - waiting\n",
					frame->getId(), 
					frame->tag,
					module->getName().c_str(),
					frame->set,
					frame->way,
					directory->getEntryAccessId(frame->set,
							frame->way));

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
			debug << misc::fmt("    A-%lld 0x%x %s miss -> lru: "
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
		esim_engine->Next(event_find_and_lock_action,
				module->getDirectoryLatency());

		// Done
		return;
	}

	// Event "find_and_lock_action"
	if (event == event_find_and_lock_action)
	{
		// Get locked port
		Module::Port *port = frame->port;
		assert(port);

		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s find_and_lock_action\n",
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
			esim_engine->Call(event_evict,
					new_frame,
					event_find_and_lock_finish);
			return;
		}

		// Continue
		esim_engine->Next(event_find_and_lock_finish);
		return;
	}

	// Event "find_and_lock_finish"
	if (event == event_find_and_lock_finish)
	{
		// Cache and directory
		Cache *cache = module->getCache();
		Directory *directory = module->getDirectory();

		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"find_and_lock_finish (err=%d)\n",
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
			directory->UnlockEntry(frame->set,
					frame->way,
					frame->getId());
			
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


void System::EventEvictHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Engine, frame, module
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Frame *parent_frame = misc::cast<Frame *>(esim_engine->getParentFrame());
	Module *module = frame->getModule();
	Module *target_module = frame->target_module;
	Cache *cache = module->getCache();
	Directory *directory = module->getDirectory();

	// Event 'evict'
	if (event == event_evict)
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
		debug << misc::fmt("  %lld A-%lld 0x%x %s evict "
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
		esim_engine->Call(event_invalidate,
				new_frame,
				event_evict_invalid);
		return;
	}

	// Event "evict_invalid"
	if (event == event_evict_invalid)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s evict_invalid\n",
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
			esim_engine->Next(event_evict_finish);
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
			esim_engine->Next(event_evict_finish);
			return;
		}

		// Continue with 'evict-action'
		esim_engine->Next(event_evict_action);
		return;
	}

	// Event "evict_action"
	if (event == event_evict_action)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s evict_action\n",
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
			esim_engine->Next(event_evict_finish);
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
				event_evict_receive,
				event);
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

	// Event "evict_receive"
	if (event == event_evict_receive)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s evict_receive\n",
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
		net::EndNode *node = target_module->getHighNetworkNode();
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
			esim_engine->Call(event_find_and_lock,
					new_frame,
					event_evict_process_noncoherent);
		else
			esim_engine->Call(event_find_and_lock,
					new_frame,
					event_evict_process);
		return;
	}

	// Event "evict_process"
	if (event == event_evict_process)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s evict_process\n",
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
			esim_engine->Next(event_evict_reply);
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
		directory->UnlockEntry(frame->set,
				frame->way,
				frame->getId());

		// Stats
		target_module->incDataAccesses();

		// Continue with 'evict-reply', after data latency
		esim_engine->Next(event_evict_reply,
				target_module->getDataLatency());
		return;
	}

	// Event "evict_process_noncoherent"
	if (event == event_evict_process_noncoherent)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"evict_process_noncoherent\n",
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
			esim_engine->Next(event_evict_reply);
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
		directory->UnlockEntry(frame->set,
				frame->way,
				frame->getId());

		// Stats
		target_module->incDataAccesses();
		
		// Continue with 'evict-reply' after latency
		esim_engine->Next(event_evict_reply,
				target_module->getDataLatency());
		return;
	}

	// Event "evict_reply"
	if (event == event_evict_reply)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"evict_reply\n",
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
				event_evict_reply_receive,
				event);
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

	// Event "evict_reply_receive"
	if (event == event_evict_reply_receive)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"evict_reply_receive\n",
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
		net::EndNode *node = module->getLowNetworkNode();
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
		esim_engine->Next(event_evict_finish);
		return;
	}

	// Event "evict_finish"
	if (event == event_evict_finish)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s evict_finish\n",
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


void System::EventWriteRequestHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Frame *parent_frame = misc::cast<Frame *>(esim_engine->getParentFrame());
	Module *module = frame->getModule();
	Module *target_module = frame->target_module;
	Directory *target_directory = target_module->getDirectory();
	Cache *target_cache = target_module->getCache();

	// Event "write_request"
	if (event == event_write_request)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s write_request\n",
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
				event_write_request_receive,
				event);
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

	// Event "write_request_receive"
	if (event == event_write_request_receive)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"write_request_receive\n",
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
		net::EndNode *node;
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
		esim_engine->Call(event_find_and_lock,
				new_frame,
				event_write_request_action);
		return;
	}

	// Event "write_request_action"
	if (event == event_write_request_action)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s write_request_action\n", 
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
			esim_engine->Next(event_write_request_reply);
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
			esim_engine->Next(event_write_request_reply);
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
		esim_engine->Call(event_invalidate,
				new_frame,
				event_write_request_exclusive);
		return;
	}

	// Event "write_request_exclusive"
	if (event == event_write_request_exclusive)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"write_request_exclusive\n",
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
			esim_engine->Next(event_write_request_updown);
		else
			esim_engine->Next(event_write_request_downup);
		return;
	}

	// Event "write_request_updown"
	if (event == event_write_request_updown)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s write_request_updown\n",
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
			esim_engine->Next(event_write_request_updown_finish);
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
			esim_engine->Call(event_write_request,
					new_frame,
					event_write_request_updown_finish);
			break;
		}
		
		default:

			throw misc::Panic("Invalid block state");
		}

		return;
	}

	// Event "write_request_updown_finish"
	if (event == event_write_request_updown_finish)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"write_request_updown_finish\n",
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
			target_directory->UnlockEntry(frame->set,
					frame->way,
					frame->getId());

			// Continue with 'write-request-reply'
			esim_engine->Next(event_write_request_reply);
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
		target_directory->UnlockEntry(frame->set,
				frame->way,
				frame->getId());

		// Stats
		target_module->incDataAccesses();

		// Continue with 'write-request-reply' after data latency
		esim_engine->Next(event_write_request_reply,
				target_module->getDataLatency());
		return;
	}

	// Event "write_request_downup"
	if (event == event_write_request_downup)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s write_request_downup\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:write_request_downup\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Sanity
		assert(frame->state != Cache::BlockInvalid);
		assert(!target_directory->isBlockSharedOrOwned(frame->set, frame->way));

		// Compute reply size
		switch (frame->state)
		{

		case Cache::BlockExclusive:
		case Cache::BlockShared:
		{
			// E/S send an ack
			frame->reply_size = 8;
			parent_frame->setReplyIfHigher(Frame::ReplyAck);
			break;
		}

		case Cache::BlockNonCoherent:
		case Cache::BlockModified:
		case Cache::BlockOwned:
		{
			// N state sends data
			frame->reply_size = target_module->getBlockSize() + 8;
			parent_frame->setReplyIfHigher(Frame::ReplyAckData);
			break;
		}

		default:

			throw misc::Panic("Invalid block state");
		}

		// Continue with 'write-request-downup-finish'
		esim_engine->Next(event_write_request_downup_finish);
		return;
	}

	// Event "write_request_downup_finish"
	if (event == event_write_request_downup_finish)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"write_request_downup_finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:write_request_downup_finish\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Set state to I
		target_cache->setBlock(frame->set, frame->way, 0,
				Cache::BlockInvalid);

		// Unlock directory entry
		target_directory->UnlockEntry(frame->set,
				frame->way,
				frame->getId());

		// Latency will depend of reply kind
		switch (parent_frame->reply)
		{

		case Frame::ReplyAck:
		case Frame::ReplyAckError:
		{
			// No latency
			esim_engine->Next(event_write_request_reply);
			break;
		}

		case Frame::ReplyAckData:
		{
			// Data latency
			target_module->incDataAccesses();
			esim_engine->Next(event_write_request_reply,
					target_module->getDataLatency());
			break;
		}

		default:

			throw misc::Panic("Invalid reply type");
		}

		// Done
		return;
	}

	// Event "write_request_reply"
	if (event == event_write_request_reply)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"write_request_reply\n",
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
				event_write_request_finish,
				event);
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

	// Event "write_request_finish"
	if (event == event_write_request_finish)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"write_request_finish\n",
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
		net::EndNode *node;
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


void System::EventReadRequestHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Frame *parent_frame = misc::cast<Frame *>(esim_engine->getParentFrame());
	Module *module = frame->getModule();
	Module *target_module = frame->target_module;
	Directory *target_directory = target_module->getDirectory();
	Cache *target_cache = target_module->getCache();

	// Event "read_request"
	if (event == event_read_request)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s read_request\n",
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
				event_read_request_receive,
				event);
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
	if (event == event_read_request_receive)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s read_request_receive\n",
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
			net::EndNode *node = target_module->getHighNetworkNode();
			network->Receive(node, frame->message);
		}
		else
		{
			net::Network *network = target_module->getLowNetwork();
			net::EndNode *node = target_module->getLowNetworkNode();
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
		esim_engine->Call(event_find_and_lock,
				new_frame,
				event_read_request_action);
		return;
	}

	// Event "read_request_action"
	if (event == event_read_request_action)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s read_request_action\n",
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
			esim_engine->Next(event_read_request_reply);
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
			esim_engine->Next(event_read_request_reply);
			return;
		}

		// Continue with 'read-request-updown' or 'read-request-downup'
		esim_engine->Next(frame->request_direction == Frame::RequestDirectionUpDown ?
				event_read_request_updown :
				event_read_request_downup);
		return;
	}

	// Event "read_request_updown"
	if (event == event_read_request_updown)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s read_request_updown\n",
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
				esim_engine->Call(event_read_request,
						new_frame,
						event_read_request_updown_finish);
			}

			// Continue with 'read-request-updown-finish'
			esim_engine->Next(event_read_request_updown_finish);
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
			esim_engine->Call(event_read_request,
					new_frame,
					event_read_request_updown_miss);
		}
		return;
	}

	// Event "read_request_updown_miss"
	if (event == event_read_request_updown_miss)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"read_request_updown_miss\n",
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
			directory->UnlockEntry(frame->set,
					frame->way,
					frame->getId());

			// Return error
			parent_frame->error = true;
			parent_frame->setReplyIfHigher(Frame::ReplyAckError);

			// Continue with 'read-request-reply'
			frame->reply_size = 8;
			esim_engine->Next(event_read_request_reply);
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
		esim_engine->Next(event_read_request_updown_finish);
		return;
	}

	// Event "read_request_updown_finish"
	if (event == event_read_request_updown_finish)
	{
		// Ensure that a reply was received
		assert(frame->reply);

		// Ignore while pending requests
		assert(frame->pending > 0);
		frame->pending--;
		if (frame->pending)
			return;

		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"read_request_updown_finish\n",
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
		directory->UnlockEntry(frame->set,
				frame->way,
				frame->getId());

		// Stats
		target_module->incDataAccesses();

		// Continue with 'read-request-reply' after latency
		esim_engine->Next(event_read_request_reply,
				target_module->getDataLatency());
		return;
	}

	// Event "read_request_downup"
	if (event == event_read_request_downup)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s read_request_downup\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:read_request_downup\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Check: state must not be invalid or shared. By default, only
		// one pending request. Response depends on state.
		assert(frame->state != Cache::BlockInvalid);
		assert(frame->state != Cache::BlockShared);
		assert(frame->state != Cache::BlockNonCoherent);
		frame->pending = 1;

		// Send a read request to the owner of each subblock.
		for (int z = 0; z < target_directory->getNumSubBlocks(); z++)
		{
			unsigned directory_entry_tag = frame->tag + 
					z * (unsigned) target_module->getSubBlockSize();
			assert(directory_entry_tag < frame->tag +
					(unsigned) target_module->getBlockSize());
			Directory::Entry *entry = target_directory->getEntry(
					frame->set,
					frame->way,
					z);

			// No owner
			if (entry->getOwner() == Directory::NoOwner)
				continue;

			// Get owner module
			net::Network *network = target_module->getHighNetwork();
			net::Node *node = network->getNode(entry->getOwner());
			net::EndNode *end_node = misc::cast<net::EndNode *>(node);
			Module *owner = (Module *) end_node->getUserData();

			// Not the first sub-block
			if (directory_entry_tag % owner->getBlockSize())
				continue;

			// One more pending request
			frame->pending++;

			// Call 'read-request'
			auto new_frame = misc::new_shared<Frame>(
					frame->getId(),
					target_module,
					directory_entry_tag);
			new_frame->target_module = owner;
			new_frame->request_direction = Frame::RequestDirectionDownUp;
			esim_engine->Call(event_read_request,
					new_frame,
					event_read_request_downup_finish);
		}

		// Continue with 'read-request-downup-finish'
		esim_engine->Next(event_read_request_downup_finish);
		return;
	}

	// Event "read_request_downup_finish"
	if (event == event_read_request_downup_finish)
	{
		// Ignore while pending requests
		assert(frame->pending > 0);
		frame->pending--;
		if (frame->pending)
			return;
		
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"read_request_downup_finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				target_module->getName().c_str());
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:read_request_downup_finish\"\n",
				frame->getId(),
				target_module->getName().c_str());

		// Check reply type
		switch (frame->reply)
		{

		case Frame::ReplyAckData:
		{
			// Set state to S
			target_cache->setBlock(frame->set, frame->way,
					frame->tag, Cache::BlockShared);

			// State is changed to shared, set owner of 
			// sub-blocks to 0.
			for (int z = 0; z < target_directory->getNumSubBlocks(); z++)
				target_directory->setOwner(frame->set,
						frame->way,
						z,
						Directory::NoOwner);

			// Set reply size
			frame->reply_size = target_module->getBlockSize() + 8;
			parent_frame->setReplyIfHigher(Frame::ReplyAckData);

			// Done
			break;
		}

		case Frame::ReplyAck:
		{
			// Higher-level cache was exclusive with no
			// modifications above it.
			frame->reply_size = 8;

			// Set state to S
			target_cache->setBlock(frame->set, frame->way,
					frame->tag, Cache::BlockShared);

			// State is changed to shared, set owner of sub-blocks 
			// to 0
			for (int z = 0; z < target_directory->getNumSubBlocks(); z++)
				target_directory->setOwner(frame->set,
						frame->way,
						z,
						Directory::NoOwner);

			// Set reply size
			frame->reply_size = 8;
			parent_frame->setReplyIfHigher(Frame::ReplyAck);

			// Done
			break;
		}

		case Frame::ReplyNone:
		{
			// This block is not present in any higher-level 
			// caches. Check state.
			switch (frame->state)
			{

			case Cache::BlockExclusive:
			case Cache::BlockShared:

				frame->reply_size = 8;
				parent_frame->setReplyIfHigher(Frame::ReplyAck);
				break;

			case Cache::BlockOwned:
			case Cache::BlockModified:
			case Cache::BlockNonCoherent:
				
				frame->reply_size = target_module->getSubBlockSize() + 8;
				parent_frame->setReplyIfHigher(Frame::ReplyAckData);
				break;

			default:

				throw misc::Panic("Invalid block state");
			}

			// Set block to S
			target_cache->setBlock(frame->set, frame->way,
					frame->tag, Cache::BlockShared);

			// Done
			break;
		}

		default:

			throw misc::Panic("Invalid reply type");
		}

		// Unlock directory entry
		target_directory->UnlockEntry(frame->set,
				frame->way,
				frame->getId());

		// Stats
		target_module->incDataAccesses();

		// Continue with 'read-request-reply' after data latency
		esim_engine->Next(event_read_request_reply,
				target_module->getDataLatency());
		return;
	}

	// Event "read_request_reply"
	if (event == event_read_request_reply)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"read_request_reply\n",
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
				event_read_request_finish,
				event);
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

	// Event "read_request_finish"
	if (event == event_read_request_finish)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"read_request_finish\n",
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
		net::EndNode *node;
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


void System::EventInvalidateHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Module *module = frame->getModule();
	Cache *cache = module->getCache();
	Directory *directory = module->getDirectory();

	// Event "invalidate"
	if (event == event_invalidate)
	{
		// Get block info
		unsigned tag;
		cache->getBlock(frame->set, frame->way, tag, frame->state);
		frame->tag = tag;

		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s invalidate "
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

				// One more pending request
				frame->pending++;

				// Send write request upwards if beginning of block
				auto new_frame = misc::new_shared<Frame>(
						frame->getId(),
						module,
						directory_entry_tag);
				new_frame->target_module = sharer;
				new_frame->request_direction = Frame::RequestDirectionDownUp;
				esim_engine->Call(event_write_request,
						new_frame,
						event_invalidate_finish);
			}
		}

		// Continue with 'invalidate-finish' event
		esim_engine->Next(event_invalidate_finish);
		return;
	}

	// Event "invalidate_finish"
	if (event == event_invalidate_finish)
	{
		// Debug and trace
		debug << misc::fmt("  %lld A-%lld 0x%x %s invalidate_finish\n",
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


void System::EventMessageHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Frame *parent_frame = misc::cast<Frame *>(esim_engine->getParentFrame());
	Module *module = frame->getModule();
	Module *target_module = frame->target_module;
	Directory *target_directory = target_module->getDirectory();

	// Event "message"
	if (event == event_message)
	{
		// Memory debug
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"message\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());

		// Set reply
		frame->reply_size = 8;
		frame->reply = Frame::ReplyAck;

		// Default return value
		parent_frame->error = false;

		// Checks
		assert(frame->message_type != Frame::MessageNone);

		// Get source and destination node
		net::Network *network = module->getLowNetwork();
		net::EndNode *source_node = module->getLowNetworkNode();
		net::EndNode *destination_node = \
				target_module->getHighNetworkNode();

		// Send message
		frame->message = network->TrySend(source_node,
				destination_node,
				8,
				event_message_receive,
				event);

		// Trace
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

	// Event "message_receive"
	if (event == event_message_receive)
	{
		// Memory debug
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"message_receive\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());

		// Receive message
		net::Network *network = target_module->getHighNetwork();
		net::EndNode *node = target_module->getHighNetworkNode();
		network->Receive(node, frame->message);

		// Find and lock
		auto new_frame = misc::new_shared<Frame>(
					frame->getId(),
					target_module,
					frame->getAddress());
		new_frame->message_type = frame->message_type;
                new_frame->blocking = false;
		new_frame->retry = false;

		// Schedule event
		esim_engine->Call(event_find_and_lock,
				new_frame,
				event_message_action);

		return;
	}

	// Event "message_action"
	if (event == event_message_action)
	{
		// Memory debug
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"message_action\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());
		// Checks
		assert(frame->message);

		// Check block locking error
		debug << misc::fmt("frame error = %u\n", frame->error);
		if (frame->error)
		{
			parent_frame->error = true;
			parent_frame->setReplyIfHigher(Frame::ReplyAckError);
			frame->reply_size = 8;
			esim_engine->Next(event_message_reply, 0);
			return;
		}

		if (frame->message_type == Frame::MessageClearOwner)
		{
			// Remove owner
			for (int z = 0; z < target_module->getDirectorySize(); z++)
			{
				// Skip other subblocks
				if (int(frame->getAddress()) == frame->tag + z * target_module->getNumSubBlocks())
				{
					// Clear the owner
					Directory::Entry *dir_entry = target_directory->getEntry(frame->set, frame->way, z);
					dir_entry->setOwner(-1);
				}

			}

		}

		else
		{
			throw misc::Panic("Unexpected Message");
		}

		// Unlock directory entry
		target_directory->UnlockEntry(frame->set,
				frame->way,
				frame->getId());

		// Schedule event
		esim_engine->Next(event_message_reply, 0);

		return;
	}

	// Event "message_reply"
	if (event == event_message_reply)
	{
		// Memory debug
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"message_reply\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());

		// Get source and destination node
		net::Network *network = module->getLowNetwork();
		net::EndNode *source_node = target_module->getHighNetworkNode();
		net::EndNode *destination_node = \
				module->getLowNetworkNode();

		// Send message
		frame->message = network->TrySend(source_node,
				destination_node,
				frame->reply_size,
				event_message_finish,
				event);

		// Trace
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

	// Event "message_finish"
	if (event == event_message_finish)
	{
		// Memory debug
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"message_finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->tag,
				module->getName().c_str());

		// Receive message
		net::Network *network = module->getLowNetwork();
		net::EndNode *node = module->getLowNetworkNode();
		network->Receive(node, frame->message);

                // Return
                esim_engine->Return();
		return;
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventFlushHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Get objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Module *module = frame->getModule();

	// Event "flush"
	if (event == event_flush)
	{
		// Memory debug
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"flush\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());

		// Trace
		trace << misc::fmt("mem.new_access "
				"name=\"A-%lld\" "
				"type=\"flush\" "
				"state=\"%s:flush\" "
				"addr=0x%x\n",
				frame->getId(),
				module->getName().c_str(),
				frame->getAddress());

		// Set pending replies to 1
		frame->pending = 1;

		// Flush the cache if the module is a cache
                if (module->getType() == Module::TypeCache)
			module->RecursiveFlush();	

		// Schedule event
		esim_engine->Next(event_flush_finish, 0);

		return;
	}

	// Event "flush_finish"
	if (event == event_flush_finish)
	{
		// Ignore while pending requests
		assert(frame->pending > 0);
		frame->pending--;
		if (frame->pending)
			return;

		// Trace
		trace << misc::fmt("mem.end_access name=\"A-%lld\"\n",
				frame->getId());

		// Increment the witness pointer if one was provided
		if (frame->witness)
			(*frame->witness)++;
                
                // Return
		esim_engine->Return();
		return;
	}
	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventLocalLoadHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Module *module = frame->getModule();

	// Event "local_load"
	if (event == event_local_load)
	{
		// Memory debug
		debug << misc::fmt("%lld A-%lld 0x%x %s local_load\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());
		// Trace
		trace << misc::fmt("mem.new_access "
				"name=\"A-%lld\" "
				"type=\"store\" "
				"state=\"%s:store\" addr=0x%x\n",
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
			// Coalesce
			module->incCoalescedReads();
			module->Coalesce(master_frame, frame);
			master_frame->queue.Wait(event_local_load_finish);

			// Done
			return;
		}

		// Continue
		esim_engine->Next(event_local_load_lock);
		return;
	}

	// Event "local_load_lock"
	if (event == event_local_load_lock)
	{
		debug << misc::fmt("  %lld A-%lld 0x%x %s local_load_lock\n",
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
			debug << misc::fmt("    A-%lld wait for write A-%lld\n",
					frame->getId(),
					older_frame->getId());
			older_frame->queue.Wait(event_local_load_lock);
			return;
		}

		// If there is any older access to the same address that this
		// access could not be coalesced with, wait for it.
		older_frame = module->getInFlightAddress(
				frame->getAddress(),
				frame);
		if (older_frame)
		{
			debug << misc::fmt("    A-%lld wait for access A-%lld\n",
					frame->getId(),
					older_frame->getId());
			older_frame->queue.Wait(event_local_load_lock);
			return;
		}

		// Call "find_and_lock" event chain
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				module,
				frame->getAddress());
		new_frame->blocking = true;
		new_frame->read = true;
		new_frame->retry = frame->retry;
		esim_engine->Call(event_local_find_and_lock,
				new_frame,
				event_local_load_finish);

		return;
	}

	// Event "local_load_finish"
	if (event == event_local_load_finish)
	{
		// Memory debug
		debug << misc::fmt("%lld A-%lld 0x%x %s local_load_finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());

		// Trace
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:load_finish\"\n",
				frame->getId(),
				module->getName().c_str());

		// Trace
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

	// Invalid event
	throw misc::Panic("Invalid event");
}


void System::EventLocalStoreHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Module *module = frame->getModule();

	// Event "local_store"
	if (event == event_local_store)
	{
		// Memory debug
		debug << misc::fmt("%lld A-%lld 0x%x %s local_store\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());

		// Trace
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
			master_frame->queue.Wait(event_local_store_finish);

			// Increment witness
			if (frame->witness)
				(*frame->witness)++;

			// Done
			return;
		}

		// Continue
		esim_engine->Next(event_local_store_lock, 0);
		return;
	}

	// Event "local_store_lock"
	if (event == event_local_store_lock)
	{
		// Debug
		debug << misc::fmt("  %lld A-%lld 0x%x %s local_store_lock\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());

		// Trace
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:store_lock\"\n",
				frame->getId(),
				module->getName().c_str());

		// If there is any older access, wait for it
		auto it = frame->accesses_iterator;
		assert(it != module->getAccessListEnd());
		if (it != module->getAccessListBegin())
		{
			// Get older access
			--it;
			Frame *older_frame = *it;

			// Debug
			debug << misc::fmt("    A-%lld wait for access A-%lld\n",
					frame->getId(),
					older_frame->getId());

			// Enqueue
			older_frame->queue.Wait(event_local_store_lock);
			return;
		}

		// Call 'find-and-lock'
		auto new_frame = misc::new_shared<Frame>(
				frame->getId(),
				module,
				frame->getAddress());
		new_frame->blocking = true;
		new_frame->write = true;
		new_frame->retry = frame->retry;
		new_frame->witness = frame->witness;
		esim_engine->Call(event_local_find_and_lock,
				new_frame,
				event_local_store_finish);

		// Set witness to nullptr so that retries from the same frame
		// do not increment it multiple times
		frame->witness = nullptr;

		return;
	}

	// Event "local_store_finish"
	if (event == event_local_store_finish)
	{
		// Debug
		debug << misc::fmt("%lld A-%lld 0x%x %s local_store_finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());

		// Trace
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:store_finish\"\n",
				frame->getId(),
				module->getName().c_str());

		// Trace
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


void System::EventLocalFindAndLockHandler(esim::Event *event,
		esim::Frame *esim_frame)
{
	// Get useful objects
	esim::Engine *esim_engine = esim::Engine::getInstance();
	Frame *frame = misc::cast<Frame *>(esim_frame);
	Frame *parent_frame = misc::cast<Frame *>(esim_engine->getParentFrame());
	Module *module = frame->getModule();

	// Event "local_find_and_lock"
	if (event == event_local_find_and_lock)
	{
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"local_find_and_lock (blocking=%d)\n",
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
		module->LockPort(frame, event_local_find_and_lock_port);

		return;
	}

	// Event "local_find_and_lock_port"
	if (event == event_local_find_and_lock_port)
	{
		// Get locked port
		Module::Port *port = frame->port;
		assert(port);

		// Memory debug
		debug << misc::fmt("  %lld A-%lld 0x%x %s local_find_and_lock_port\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());

		// Trace
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:find_and_lock_port\"\n",
				frame->getId(),
				module->getName().c_str());

		// Set parent frame flag expressing that port has already been
		// locked. This flag is checked by new writes to find out if
		// it is already too late to coalesce.
		parent_frame->port_locked = true;

		// Statistics
		module->incAccesses();
		if (frame->retry)
		{
			module->incRetryAccesses();
		}

		if (frame->read)
		{
			module->UpdateStats(frame);
		}

		else
		{
			module->UpdateStats(frame);

			if (frame->witness)
			{
				(*frame->witness)++;
				frame->witness = nullptr;
			}
		}

		// Schedule event
		esim_engine->Next(event_local_find_and_lock_action,
				module->getDataLatency());

		return;
	}

	// Event "local_find_and_lock_action"
	if (event == event_local_find_and_lock_action)
	{
		// Get locked port
		Module::Port *port = frame->port;
		assert(port);

		// Memory debug
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"local_find_and_lock_action\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());

		// Trace
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:find_and_lock_action\"\n",
				frame->getId(),
				module->getName().c_str());

		// Release port
		module->UnlockPort(port, frame);
		parent_frame->port_locked = false;

		// Schedule event
		esim_engine->Next(event_local_find_and_lock_finish, 0);
		return;
	}

	// Event "local_find_and_lock_finish"
	if (event == event_local_find_and_lock_finish)
	{
		// Memory debug
		debug << misc::fmt("  %lld A-%lld 0x%x %s "
				"local_find_and_lock_finish\n",
				esim_engine->getTime(),
				frame->getId(),
				frame->getAddress(),
				module->getName().c_str());

		// Trace
		trace << misc::fmt("mem.access "
				"name=\"A-%lld\" "
				"state=\"%s:find_and_lock_finish\"\n",
				frame->getId(),
				module->getName().c_str());
		
		// Return esim engine
		esim_engine->Return();
		return;
	}

	// Invalid event
	throw misc::Panic("Invalid event");
}

}
