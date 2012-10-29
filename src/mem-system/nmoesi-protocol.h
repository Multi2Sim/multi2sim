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

#ifndef MEM_SYSTEM_NMOESI_PROTOCOL_H
#define MEM_SYSTEM_NMOESI_PROTOCOL_H


/* NMOESI Event-Driven Simulation */

extern int EV_MOD_NMOESI_LOAD;
extern int EV_MOD_NMOESI_LOAD_LOCK;
extern int EV_MOD_NMOESI_LOAD_ACTION;
extern int EV_MOD_NMOESI_LOAD_MISS;
extern int EV_MOD_NMOESI_LOAD_UNLOCK;
extern int EV_MOD_NMOESI_LOAD_FINISH;

extern int EV_MOD_NMOESI_STORE;
extern int EV_MOD_NMOESI_STORE_LOCK;
extern int EV_MOD_NMOESI_STORE_ACTION;
extern int EV_MOD_NMOESI_STORE_UNLOCK;
extern int EV_MOD_NMOESI_STORE_FINISH;

extern int EV_MOD_NMOESI_NC_STORE;
extern int EV_MOD_NMOESI_NC_STORE_LOCK;
extern int EV_MOD_NMOESI_NC_STORE_WRITEBACK;
extern int EV_MOD_NMOESI_NC_STORE_ACTION;
extern int EV_MOD_NMOESI_NC_STORE_MISS;
extern int EV_MOD_NMOESI_NC_STORE_UNLOCK;
extern int EV_MOD_NMOESI_NC_STORE_FINISH;

extern int EV_MOD_NMOESI_PREFETCH;
extern int EV_MOD_NMOESI_PREFETCH_LOCK;
extern int EV_MOD_NMOESI_PREFETCH_ACTION;
extern int EV_MOD_NMOESI_PREFETCH_MISS;
extern int EV_MOD_NMOESI_PREFETCH_UNLOCK;
extern int EV_MOD_NMOESI_PREFETCH_FINISH;

extern int EV_MOD_NMOESI_FIND_AND_LOCK;
extern int EV_MOD_NMOESI_FIND_AND_LOCK_PORT;
extern int EV_MOD_NMOESI_FIND_AND_LOCK_ACTION;
extern int EV_MOD_NMOESI_FIND_AND_LOCK_FINISH;

extern int EV_MOD_NMOESI_EVICT;
extern int EV_MOD_NMOESI_EVICT_INVALID;
extern int EV_MOD_NMOESI_EVICT_ACTION;
extern int EV_MOD_NMOESI_EVICT_RECEIVE;
extern int EV_MOD_NMOESI_EVICT_PROCESS;
extern int EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT;
extern int EV_MOD_NMOESI_EVICT_REPLY;
extern int EV_MOD_NMOESI_EVICT_REPLY_RECEIVE;
extern int EV_MOD_NMOESI_EVICT_FINISH;

extern int EV_MOD_NMOESI_WRITE_REQUEST;
extern int EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE;
extern int EV_MOD_NMOESI_WRITE_REQUEST_ACTION;
extern int EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE;
extern int EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN;
extern int EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH;
extern int EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP;
extern int EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH;
extern int EV_MOD_NMOESI_WRITE_REQUEST_REPLY;
extern int EV_MOD_NMOESI_WRITE_REQUEST_FINISH;

extern int EV_MOD_NMOESI_READ_REQUEST;
extern int EV_MOD_NMOESI_READ_REQUEST_RECEIVE;
extern int EV_MOD_NMOESI_READ_REQUEST_ACTION;
extern int EV_MOD_NMOESI_READ_REQUEST_UPDOWN;
extern int EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS;
extern int EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH;
extern int EV_MOD_NMOESI_READ_REQUEST_DOWNUP;
extern int EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS;
extern int EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH;
extern int EV_MOD_NMOESI_READ_REQUEST_REPLY;
extern int EV_MOD_NMOESI_READ_REQUEST_FINISH;

extern int EV_MOD_NMOESI_INVALIDATE;
extern int EV_MOD_NMOESI_INVALIDATE_FINISH;

extern int EV_MOD_NMOESI_PEER_SEND;
extern int EV_MOD_NMOESI_PEER_RECEIVE;
extern int EV_MOD_NMOESI_PEER_REPLY;
extern int EV_MOD_NMOESI_PEER_FINISH;

extern int EV_MOD_NMOESI_MESSAGE;
extern int EV_MOD_NMOESI_MESSAGE_RECEIVE;
extern int EV_MOD_NMOESI_MESSAGE_ACTION;
extern int EV_MOD_NMOESI_MESSAGE_REPLY;
extern int EV_MOD_NMOESI_MESSAGE_FINISH;


void mod_handler_nmoesi_find_and_lock(int event, void *data);
void mod_handler_nmoesi_load(int event, void *data);
void mod_handler_nmoesi_store(int event, void *data);
void mod_handler_nmoesi_prefetch(int event, void *data);
void mod_handler_nmoesi_nc_store(int event, void *data);
void mod_handler_nmoesi_evict(int event, void *data);
void mod_handler_nmoesi_write_request(int event, void *data);
void mod_handler_nmoesi_read_request(int event, void *data);
void mod_handler_nmoesi_invalidate(int event, void *data);
void mod_handler_nmoesi_peer(int event, void *data);
void mod_handler_nmoesi_message(int event, void *data);


#endif

