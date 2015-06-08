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


void System::evLoadHandler(esim::EventType *event_type,
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
		/*
		FIXME
		master_stack = mod_can_coalesce(mod, mod_access_load, stack->addr, stack);
		if (master_stack)
		{
			mod->coalesced_reads++;
			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_NMOESI_LOAD_FINISH);
			return;
		}
		*/

		// Next event
		esim_engine->Next(event_type_load_lock);
		return;
	}

	// Event "load_lock"
	if (event_type == event_type_load_lock)
	{
		return;
	}


	// Event "load_action"
	if (event_type == event_type_load_action)
	{
		return;
	}


	// Event "load_miss"
	if (event_type == event_type_load_miss)
	{
		return;
	}


	// Event "load_unlock"
	if (event_type == event_type_load_unlock)
	{
		return;
	}


	// Event "load_finish"
	if (event_type == event_type_load_finish)
	{
		return;
	}

}


void System::evStoreHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evNCStoreHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evPrefetchHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evFindAndLockHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evEvictHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evWriteRequestHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evReadRequestHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evInvalidateHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evPeerHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evMessageHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evFlushHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evLocalLoadHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evLocalStoreHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evLocalFindAndLockHandler(esim::EventType *type, esim::EventFrame *frame)
{
}



}

