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

#include "System.h"


namespace mem
{
	
esim::EventType *System::ev_load;
esim::EventType *System::ev_load_lock;
esim::EventType *System::ev_load_action;
esim::EventType *System::ev_load_miss;
esim::EventType *System::ev_load_unlock;
esim::EventType *System::ev_load_finish;

esim::EventType *System::ev_store;
esim::EventType *System::ev_store_lock;
esim::EventType *System::ev_store_action;
esim::EventType *System::ev_store_unlock;
esim::EventType *System::ev_store_finish;

esim::EventType *System::ev_nc_store;
esim::EventType *System::ev_nc_store_lock;
esim::EventType *System::ev_nc_store_writeback;
esim::EventType *System::ev_nc_store_action;
esim::EventType *System::ev_nc_store_miss;
esim::EventType *System::ev_nc_store_unlock;
esim::EventType *System::ev_nc_store_finish;

esim::EventType *System::ev_prefetch;
esim::EventType *System::ev_prefetch_lock;
esim::EventType *System::ev_prefetch_action;
esim::EventType *System::ev_prefetch_miss;
esim::EventType *System::ev_prefetch_unlock;
esim::EventType *System::ev_prefetch_finish;

esim::EventType *System::ev_find_and_lock;
esim::EventType *System::ev_find_and_lock_port;
esim::EventType *System::ev_find_and_lock_action;
esim::EventType *System::ev_find_and_lock_finish;

esim::EventType *System::ev_evict;
esim::EventType *System::ev_evict_invalid;
esim::EventType *System::ev_evict_action;
esim::EventType *System::ev_evict_receive;
esim::EventType *System::ev_evict_process;
esim::EventType *System::ev_evict_process_noncoherent;
esim::EventType *System::ev_evict_reply;
esim::EventType *System::ev_evict_reply_receive;
esim::EventType *System::ev_evict_finish;

esim::EventType *System::ev_write_request;
esim::EventType *System::ev_write_request_receive;
esim::EventType *System::ev_write_request_action;
esim::EventType *System::ev_write_request_exclusive;
esim::EventType *System::ev_write_request_updown;
esim::EventType *System::ev_write_request_updown_finish;
esim::EventType *System::ev_write_request_downup;
esim::EventType *System::ev_write_request_downup_finish;
esim::EventType *System::ev_write_request_reply;
esim::EventType *System::ev_write_request_finish;

esim::EventType *System::ev_read_request;
esim::EventType *System::ev_read_request_receive;
esim::EventType *System::ev_read_request_action;
esim::EventType *System::ev_read_request_updown;
esim::EventType *System::ev_read_request_updown_miss;
esim::EventType *System::ev_read_request_updown_finish;
esim::EventType *System::ev_read_request_downup;
esim::EventType *System::ev_read_request_downup_wait_for_reqs;
esim::EventType *System::ev_read_request_downup_finish;
esim::EventType *System::ev_read_request_reply;
esim::EventType *System::ev_read_request_finish;

esim::EventType *System::ev_invalidate;
esim::EventType *System::ev_invalidate_finish;

esim::EventType *System::ev_peer_send;
esim::EventType *System::ev_peer_receive;
esim::EventType *System::ev_peer_reply;
esim::EventType *System::ev_peer_finish;

esim::EventType *System::ev_message;
esim::EventType *System::ev_message_receive;
esim::EventType *System::ev_message_action;
esim::EventType *System::ev_message_reply;
esim::EventType *System::ev_message_finish;

esim::EventType *System::ev_flush;
esim::EventType *System::ev_flush_finish;
	
esim::EventType *System::ev_local_load;
esim::EventType *System::ev_local_load_lock;
esim::EventType *System::ev_local_load_finish;

esim::EventType *System::ev_local_store;
esim::EventType *System::ev_local_store_lock;
esim::EventType *System::ev_local_store_finish;

esim::EventType *System::ev_local_find_and_lock;
esim::EventType *System::ev_local_find_and_lock_port;
esim::EventType *System::ev_local_find_and_lock_action;
esim::EventType *System::ev_local_find_and_lock_finish;


void System::evLoadHandler(esim::EventType *type, esim::EventFrame *frame)
{
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

