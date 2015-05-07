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
	// Read memory configuration from '--mem-config', or create default
	// configuration for all architectures with a timing simulation.
	ConfigRead();


	//
	// NMOESI events
	//
	
	esim::Engine *esim = esim::Engine::getInstance();

	ev_load = esim->RegisterEventType("ev_load", evLoadHandler,
			frequency_domain);
	ev_load_lock = esim->RegisterEventType("ev_load_lock", evLoadHandler,
			frequency_domain);
	ev_load_action = esim->RegisterEventType("ev_load_action", evLoadHandler,
			frequency_domain);
	ev_load_miss = esim->RegisterEventType("ev_load_miss", evLoadHandler,
			frequency_domain);
	ev_load_unlock = esim->RegisterEventType("ev_load_unlock", evLoadHandler,
			frequency_domain);
	ev_load_finish = esim->RegisterEventType("ev_load_finish", evLoadHandler,
			frequency_domain);

	ev_store = esim->RegisterEventType("ev_store", evStoreHandler,
			frequency_domain);
	ev_store_lock = esim->RegisterEventType("ev_store_lock", evStoreHandler,
			frequency_domain);
	ev_store_action = esim->RegisterEventType("ev_store_action", evStoreHandler,
			frequency_domain);
	ev_store_unlock = esim->RegisterEventType("ev_store_unlock", evStoreHandler,
			frequency_domain);
	ev_store_finish = esim->RegisterEventType("ev_store_finish", evStoreHandler,
			frequency_domain);

	ev_nc_store = esim->RegisterEventType("ev_nc_store", evNCStoreHandler,
			frequency_domain);
	ev_nc_store_lock = esim->RegisterEventType("ev_nc_store_lock", evNCStoreHandler,
			frequency_domain);
	ev_nc_store_writeback = esim->RegisterEventType("ev_nc_store_writeback", evNCStoreHandler,
			frequency_domain);
	ev_nc_store_action = esim->RegisterEventType("ev_nc_store_action", evNCStoreHandler,
			frequency_domain);
	ev_nc_store_miss= esim->RegisterEventType("ev_nc_store_miss", evNCStoreHandler,
			frequency_domain);
	ev_nc_store_unlock = esim->RegisterEventType("ev_nc_store_unlock", evNCStoreHandler,
			frequency_domain);
	ev_nc_store_finish = esim->RegisterEventType("ev_nc_store_finish", evNCStoreHandler,
			frequency_domain);

	ev_prefetch = esim->RegisterEventType("ev_prefetch", evPrefetchHandler,
			frequency_domain);
	ev_prefetch_lock = esim->RegisterEventType("ev_prefetch_lock", evPrefetchHandler,
			frequency_domain);
	ev_prefetch_action = esim->RegisterEventType("ev_prefetch_action", evPrefetchHandler,
			frequency_domain);
	ev_prefetch_miss = esim->RegisterEventType("ev_prefetch_miss", evPrefetchHandler,
			frequency_domain);
	ev_prefetch_unlock = esim->RegisterEventType("ev_prefetch_unlock", evPrefetchHandler,
			frequency_domain);
	ev_prefetch_finish = esim->RegisterEventType("ev_prefetch_finish", evPrefetchHandler,
			frequency_domain);

	ev_find_and_lock = esim->RegisterEventType("ev_find_and_lock", evFindAndLockHandler,
			frequency_domain);
	ev_find_and_lock_port = esim->RegisterEventType("ev_find_and_lock_port", evFindAndLockHandler,
			frequency_domain);
	ev_find_and_lock_action = esim->RegisterEventType("ev_find_and_lock_action", evFindAndLockHandler,
			frequency_domain);
	ev_find_and_lock_finish = esim->RegisterEventType("ev_find_and_lock_finish", evFindAndLockHandler,
			frequency_domain);

	ev_evict = esim->RegisterEventType("ev_evict", evEvictHandler,
			frequency_domain);
	ev_evict_invalid = esim->RegisterEventType("ev_evict_invalid", evEvictHandler,
			frequency_domain);
	ev_evict_action = esim->RegisterEventType("ev_evict_action", evEvictHandler,
			frequency_domain);
	ev_evict_receive = esim->RegisterEventType("ev_evict_receive", evEvictHandler,
			frequency_domain);
	ev_evict_process = esim->RegisterEventType("ev_evict_process", evEvictHandler,
			frequency_domain);
	ev_evict_process_noncoherent = esim->RegisterEventType("ev_evict_process_noncoherent", evEvictHandler,
			frequency_domain);
	ev_evict_reply = esim->RegisterEventType("ev_evict_reply", evEvictHandler,
			frequency_domain);
	ev_evict_reply = esim->RegisterEventType("ev_evict_reply", evEvictHandler,
			frequency_domain);
	ev_evict_reply_receive = esim->RegisterEventType("ev_evict_reply_receive", evEvictHandler,
			frequency_domain);
	ev_evict_finish = esim->RegisterEventType("ev_evict_finish", evEvictHandler,
			frequency_domain);

	ev_write_request = esim->RegisterEventType("ev_write_request", evWriteRequestHandler,
			frequency_domain);
	ev_write_request_receive = esim->RegisterEventType("ev_write_request_receive", evWriteRequestHandler,
			frequency_domain);
	ev_write_request_action = esim->RegisterEventType("ev_write_request_action", evWriteRequestHandler,
			frequency_domain);
	ev_write_request_exclusive = esim->RegisterEventType("ev_write_request_exclusive", evWriteRequestHandler,
			frequency_domain);
	ev_write_request_updown = esim->RegisterEventType("ev_write_request_updown", evWriteRequestHandler,
			frequency_domain);
	ev_write_request_updown_finish = esim->RegisterEventType("ev_write_request_updown_finish", evWriteRequestHandler,
			frequency_domain);
	ev_write_request_downup = esim->RegisterEventType("ev_write_request_downup", evWriteRequestHandler,
			frequency_domain);
	ev_write_request_downup_finish = esim->RegisterEventType("ev_write_request_downup_finish", evWriteRequestHandler,
			frequency_domain);
	ev_write_request_reply = esim->RegisterEventType("ev_write_request_reply", evWriteRequestHandler,
			frequency_domain);
	ev_write_request_finish = esim->RegisterEventType("ev_write_request_finish", evWriteRequestHandler,
			frequency_domain);

	ev_read_request = esim->RegisterEventType("ev_read_request", evReadRequestHandler,
			frequency_domain);
	ev_read_request_receive = esim->RegisterEventType("ev_read_request_receive", evReadRequestHandler,
			frequency_domain);
	ev_read_request_action = esim->RegisterEventType("ev_read_request_action", evReadRequestHandler,
			frequency_domain);
	ev_read_request_updown = esim->RegisterEventType("ev_read_request_updown", evReadRequestHandler,
			frequency_domain);
	ev_read_request_updown_miss = esim->RegisterEventType("ev_read_request_updown_miss", evReadRequestHandler,
			frequency_domain);
	ev_read_request_updown_finish = esim->RegisterEventType("ev_read_request_updown_finish", evReadRequestHandler,
			frequency_domain);
	ev_read_request_downup = esim->RegisterEventType("ev_read_request_downup", evReadRequestHandler,
			frequency_domain);
	ev_read_request_downup_wait_for_reqs = esim->RegisterEventType("ev_read_request_downup_wait_for_reqs", evReadRequestHandler,
			frequency_domain);
	ev_read_request_downup_finish = esim->RegisterEventType("ev_read_request_downup_finish", evReadRequestHandler,
			frequency_domain);
	ev_read_request_reply = esim->RegisterEventType("ev_read_request_reply", evReadRequestHandler,
			frequency_domain);
	ev_read_request_finish = esim->RegisterEventType("ev_read_request_finish", evReadRequestHandler,
			frequency_domain);

	ev_invalidate = esim->RegisterEventType("ev_invalidate", evInvalidateHandler,
			frequency_domain);
	ev_invalidate_finish = esim->RegisterEventType("ev_invalidate_finish", evInvalidateHandler,
			frequency_domain);

	ev_peer_send = esim->RegisterEventType("ev_peer_send", evPeerHandler,
			frequency_domain);
	ev_peer_receive = esim->RegisterEventType("ev_peer_receive", evPeerHandler,
			frequency_domain);
	ev_peer_reply = esim->RegisterEventType("ev_peer_reply", evPeerHandler,
			frequency_domain);
	ev_peer_finish = esim->RegisterEventType("ev_peer_finish", evPeerHandler,
			frequency_domain);

	ev_message = esim->RegisterEventType("ev_message", evMessageHandler,
			frequency_domain);
	ev_message_receive = esim->RegisterEventType("ev_message_receive", evMessageHandler,
			frequency_domain);
	ev_message_action = esim->RegisterEventType("ev_message_action", evMessageHandler,
			frequency_domain);
	ev_message_reply = esim->RegisterEventType("ev_message_reply", evMessageHandler,
			frequency_domain);
	ev_message_finish = esim->RegisterEventType("ev_message_finish", evMessageHandler,
			frequency_domain);

	ev_flush = esim->RegisterEventType("ev_flush", evFlushHandler,
			frequency_domain);
	ev_flush_finish = esim->RegisterEventType("ev_flush_finish", evFlushHandler,
			frequency_domain);

	// Local memory event driven simulation

	ev_local_load = esim->RegisterEventType("ev_local_load", evLocalLoadHandler,
			frequency_domain);
	ev_local_load_lock = esim->RegisterEventType("ev_local_load_lock", evLocalLoadHandler,
			frequency_domain);
	ev_local_load_finish = esim->RegisterEventType("ev_local_load_finish", evLocalLoadHandler,
			frequency_domain);

	ev_local_store = esim->RegisterEventType("ev_local_store", evLocalStoreHandler,
			frequency_domain);
	ev_local_store_lock = esim->RegisterEventType("ev_local_store_lock", evLocalStoreHandler,
			frequency_domain);
	ev_local_store_finish = esim->RegisterEventType("ev_local_store_finish", evLocalStoreHandler,
			frequency_domain);

	ev_local_find_and_lock = esim->RegisterEventType("ev_local_find_and_lock", evLocalFindAndLockHandler,
			frequency_domain);
	ev_local_find_and_lock_port = esim->RegisterEventType("ev_local_find_and_lock_port", evLocalFindAndLockHandler,
			frequency_domain);
	ev_local_find_and_lock_action = esim->RegisterEventType("ev_local_find_and_lock_action", evLocalFindAndLockHandler,
			frequency_domain);
	ev_local_find_and_lock_finish = esim->RegisterEventType("ev_local_find_and_lock_finish", evLocalFindAndLockHandler,
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

