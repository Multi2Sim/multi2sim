/*
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
 *  You should have received stack copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MEM_SYSTEM_LOCAL_MEM_PROTOCOL_H
#define MEM_SYSTEM_LOCAL_MEM_PROTOCOL_H


extern int EV_MOD_LOCAL_MEM_LOAD;
extern int EV_MOD_LOCAL_MEM_LOAD_LOCK;
extern int EV_MOD_LOCAL_MEM_LOAD_FINISH;

extern int EV_MOD_LOCAL_MEM_STORE;
extern int EV_MOD_LOCAL_MEM_STORE_LOCK;
extern int EV_MOD_LOCAL_MEM_STORE_FINISH;

extern int EV_MOD_LOCAL_MEM_FIND_AND_LOCK;
extern int EV_MOD_LOCAL_MEM_FIND_AND_LOCK_PORT;
extern int EV_MOD_LOCAL_MEM_FIND_AND_LOCK_ACTION;
extern int EV_MOD_LOCAL_MEM_FIND_AND_LOCK_FINISH;


void mod_handler_local_mem_load(int event, void *data);
void mod_handler_local_mem_store(int event, void *data);
void mod_handler_local_mem_find_and_lock(int event, void *data);


#endif

