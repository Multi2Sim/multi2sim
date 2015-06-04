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

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Misc.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Event.h>
#include <lib/esim/FrequencyDomain.h>

#include "System.h"


namespace mem
{


// Configuration options
std::string System::config_file;
std::string System::debug_file;
bool System::help = false;
int System::frequency = 1000;


esim::Trace System::trace;

misc::Debug System::debug;

std::unique_ptr<System> System::instance;


System *System::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new System());
	return instance.get();
}


System::System()
{
	//
	// NMOESI events
	//
	
	esim::Engine *esim = esim::Engine::getInstance();

	event_type_load = esim->RegisterEventType("load", evLoadHandler,
			frequency_domain);
	event_type_load_lock = esim->RegisterEventType("load_lock", evLoadHandler,
			frequency_domain);
	event_type_load_action = esim->RegisterEventType("load_action", evLoadHandler,
			frequency_domain);
	event_type_load_miss = esim->RegisterEventType("load_miss", evLoadHandler,
			frequency_domain);
	event_type_load_unlock = esim->RegisterEventType("load_unlock", evLoadHandler,
			frequency_domain);
	event_type_load_finish = esim->RegisterEventType("load_finish", evLoadHandler,
			frequency_domain);

	event_type_store = esim->RegisterEventType("store", evStoreHandler,
			frequency_domain);
	event_type_store_lock = esim->RegisterEventType("store_lock", evStoreHandler,
			frequency_domain);
	event_type_store_action = esim->RegisterEventType("store_action", evStoreHandler,
			frequency_domain);
	event_type_store_unlock = esim->RegisterEventType("store_unlock", evStoreHandler,
			frequency_domain);
	event_type_store_finish = esim->RegisterEventType("store_finish", evStoreHandler,
			frequency_domain);

	event_type_nc_store = esim->RegisterEventType("nc_store", evNCStoreHandler,
			frequency_domain);
	event_type_nc_store_lock = esim->RegisterEventType("nc_store_lock", evNCStoreHandler,
			frequency_domain);
	event_type_nc_store_writeback = esim->RegisterEventType("nc_store_writeback", evNCStoreHandler,
			frequency_domain);
	event_type_nc_store_action = esim->RegisterEventType("nc_store_action", evNCStoreHandler,
			frequency_domain);
	event_type_nc_store_miss= esim->RegisterEventType("nc_store_miss", evNCStoreHandler,
			frequency_domain);
	event_type_nc_store_unlock = esim->RegisterEventType("nc_store_unlock", evNCStoreHandler,
			frequency_domain);
	event_type_nc_store_finish = esim->RegisterEventType("nc_store_finish", evNCStoreHandler,
			frequency_domain);

	event_type_prefetch = esim->RegisterEventType("prefetch", evPrefetchHandler,
			frequency_domain);
	event_type_prefetch_lock = esim->RegisterEventType("prefetch_lock", evPrefetchHandler,
			frequency_domain);
	event_type_prefetch_action = esim->RegisterEventType("prefetch_action", evPrefetchHandler,
			frequency_domain);
	event_type_prefetch_miss = esim->RegisterEventType("prefetch_miss", evPrefetchHandler,
			frequency_domain);
	event_type_prefetch_unlock = esim->RegisterEventType("prefetch_unlock", evPrefetchHandler,
			frequency_domain);
	event_type_prefetch_finish = esim->RegisterEventType("prefetch_finish", evPrefetchHandler,
			frequency_domain);

	event_type_find_and_lock = esim->RegisterEventType("find_and_lock", evFindAndLockHandler,
			frequency_domain);
	event_type_find_and_lock_port = esim->RegisterEventType("find_and_lock_port", evFindAndLockHandler,
			frequency_domain);
	event_type_find_and_lock_action = esim->RegisterEventType("find_and_lock_action", evFindAndLockHandler,
			frequency_domain);
	event_type_find_and_lock_finish = esim->RegisterEventType("find_and_lock_finish", evFindAndLockHandler,
			frequency_domain);

	event_type_evict = esim->RegisterEventType("evict", evEvictHandler,
			frequency_domain);
	event_type_evict_invalid = esim->RegisterEventType("evict_invalid", evEvictHandler,
			frequency_domain);
	event_type_evict_action = esim->RegisterEventType("evict_action", evEvictHandler,
			frequency_domain);
	event_type_evict_receive = esim->RegisterEventType("evict_receive", evEvictHandler,
			frequency_domain);
	event_type_evict_process = esim->RegisterEventType("evict_process", evEvictHandler,
			frequency_domain);
	event_type_evict_process_noncoherent = esim->RegisterEventType("evict_process_noncoherent", evEvictHandler,
			frequency_domain);
	event_type_evict_reply = esim->RegisterEventType("evict_reply", evEvictHandler,
			frequency_domain);
	event_type_evict_reply = esim->RegisterEventType("evict_reply", evEvictHandler,
			frequency_domain);
	event_type_evict_reply_receive = esim->RegisterEventType("evict_reply_receive", evEvictHandler,
			frequency_domain);
	event_type_evict_finish = esim->RegisterEventType("evict_finish", evEvictHandler,
			frequency_domain);

	event_type_write_request = esim->RegisterEventType("write_request", evWriteRequestHandler,
			frequency_domain);
	event_type_write_request_receive = esim->RegisterEventType("write_request_receive", evWriteRequestHandler,
			frequency_domain);
	event_type_write_request_action = esim->RegisterEventType("write_request_action", evWriteRequestHandler,
			frequency_domain);
	event_type_write_request_exclusive = esim->RegisterEventType("write_request_exclusive", evWriteRequestHandler,
			frequency_domain);
	event_type_write_request_updown = esim->RegisterEventType("write_request_updown", evWriteRequestHandler,
			frequency_domain);
	event_type_write_request_updown_finish = esim->RegisterEventType("write_request_updown_finish", evWriteRequestHandler,
			frequency_domain);
	event_type_write_request_downup = esim->RegisterEventType("write_request_downup", evWriteRequestHandler,
			frequency_domain);
	event_type_write_request_downup_finish = esim->RegisterEventType("write_request_downup_finish", evWriteRequestHandler,
			frequency_domain);
	event_type_write_request_reply = esim->RegisterEventType("write_request_reply", evWriteRequestHandler,
			frequency_domain);
	event_type_write_request_finish = esim->RegisterEventType("write_request_finish", evWriteRequestHandler,
			frequency_domain);

	event_type_read_request = esim->RegisterEventType("read_request", evReadRequestHandler,
			frequency_domain);
	event_type_read_request_receive = esim->RegisterEventType("read_request_receive", evReadRequestHandler,
			frequency_domain);
	event_type_read_request_action = esim->RegisterEventType("read_request_action", evReadRequestHandler,
			frequency_domain);
	event_type_read_request_updown = esim->RegisterEventType("read_request_updown", evReadRequestHandler,
			frequency_domain);
	event_type_read_request_updown_miss = esim->RegisterEventType("read_request_updown_miss", evReadRequestHandler,
			frequency_domain);
	event_type_read_request_updown_finish = esim->RegisterEventType("read_request_updown_finish", evReadRequestHandler,
			frequency_domain);
	event_type_read_request_downup = esim->RegisterEventType("read_request_downup", evReadRequestHandler,
			frequency_domain);
	event_type_read_request_downup_wait_for_reqs = esim->RegisterEventType("read_request_downup_wait_for_reqs", evReadRequestHandler,
			frequency_domain);
	event_type_read_request_downup_finish = esim->RegisterEventType("read_request_downup_finish", evReadRequestHandler,
			frequency_domain);
	event_type_read_request_reply = esim->RegisterEventType("read_request_reply", evReadRequestHandler,
			frequency_domain);
	event_type_read_request_finish = esim->RegisterEventType("read_request_finish", evReadRequestHandler,
			frequency_domain);

	event_type_invalidate = esim->RegisterEventType("invalidate", evInvalidateHandler,
			frequency_domain);
	event_type_invalidate_finish = esim->RegisterEventType("invalidate_finish", evInvalidateHandler,
			frequency_domain);

	event_type_peer_send = esim->RegisterEventType("peer_send", evPeerHandler,
			frequency_domain);
	event_type_peer_receive = esim->RegisterEventType("peer_receive", evPeerHandler,
			frequency_domain);
	event_type_peer_reply = esim->RegisterEventType("peer_reply", evPeerHandler,
			frequency_domain);
	event_type_peer_finish = esim->RegisterEventType("peer_finish", evPeerHandler,
			frequency_domain);

	event_type_message = esim->RegisterEventType("message", evMessageHandler,
			frequency_domain);
	event_type_message_receive = esim->RegisterEventType("message_receive", evMessageHandler,
			frequency_domain);
	event_type_message_action = esim->RegisterEventType("message_action", evMessageHandler,
			frequency_domain);
	event_type_message_reply = esim->RegisterEventType("message_reply", evMessageHandler,
			frequency_domain);
	event_type_message_finish = esim->RegisterEventType("message_finish", evMessageHandler,
			frequency_domain);

	event_type_flush = esim->RegisterEventType("flush", evFlushHandler,
			frequency_domain);
	event_type_flush_finish = esim->RegisterEventType("flush_finish", evFlushHandler,
			frequency_domain);

	// Local memory event driven simulation

	event_type_local_load = esim->RegisterEventType("local_load", evLocalLoadHandler,
			frequency_domain);
	event_type_local_load_lock = esim->RegisterEventType("local_load_lock", evLocalLoadHandler,
			frequency_domain);
	event_type_local_load_finish = esim->RegisterEventType("local_load_finish", evLocalLoadHandler,
			frequency_domain);

	event_type_local_store = esim->RegisterEventType("local_store", evLocalStoreHandler,
			frequency_domain);
	event_type_local_store_lock = esim->RegisterEventType("local_store_lock", evLocalStoreHandler,
			frequency_domain);
	event_type_local_store_finish = esim->RegisterEventType("local_store_finish", evLocalStoreHandler,
			frequency_domain);

	event_type_local_find_and_lock = esim->RegisterEventType("local_find_and_lock", evLocalFindAndLockHandler,
			frequency_domain);
	event_type_local_find_and_lock_port = esim->RegisterEventType("local_find_and_lock_port", evLocalFindAndLockHandler,
			frequency_domain);
	event_type_local_find_and_lock_action = esim->RegisterEventType("local_find_and_lock_action", evLocalFindAndLockHandler,
			frequency_domain);
	event_type_local_find_and_lock_finish = esim->RegisterEventType("local_find_and_lock_finish", evLocalFindAndLockHandler,
			frequency_domain);
}


void System::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("Memory System");

	// Debug information
	command_line->RegisterString("--mem-debug <file>", debug_file,
			"Debug information related with the memory system, "
			"including caches and coherence protocol.");

	// Configuration INI file for memory system
	command_line->RegisterString("--mem-config <file>", config_file,
			"Memory configuration file. For a detailed description "
			"of the sections and variables supported for this file "
			"use option '--mem-help'.");
	
	// Help message for memory system
	command_line->RegisterBool("--mem-help",
			help,
			"Print help message describing the memory configuration"
			" file, passed in option '--mem-config <file>'.");
	command_line->setIncompatible("--mem-help");

}


void System::ProcessOptions()
{
	// Show help message
	if (help)
	{
		std::cerr << help_message;
		exit(1);
	}

	// Debug file
	debug.setPath(debug_file);
}



}  // namespace mem

