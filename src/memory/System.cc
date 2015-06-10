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

	// Create frequency domain
	esim::Engine *esim_engine = esim::Engine::getInstance();
	frequency_domain = esim_engine->RegisterFrequencyDomain("Memory");

	event_type_load = esim_engine->RegisterEventType("load",
			EventLoadHandler,
			frequency_domain);
	event_type_load_lock = esim_engine->RegisterEventType("load_lock",
			EventLoadHandler,
			frequency_domain);
	event_type_load_action = esim_engine->RegisterEventType("load_action",
			EventLoadHandler,
			frequency_domain);
	event_type_load_miss = esim_engine->RegisterEventType("load_miss",
			EventLoadHandler,
			frequency_domain);
	event_type_load_unlock = esim_engine->RegisterEventType("load_unlock",
			EventLoadHandler,
			frequency_domain);
	event_type_load_finish = esim_engine->RegisterEventType("load_finish",
			EventLoadHandler,
			frequency_domain);

	event_type_store = esim_engine->RegisterEventType("store",
			EventStoreHandler,
			frequency_domain);
	event_type_store_lock = esim_engine->RegisterEventType("store_lock",
			EventStoreHandler,
			frequency_domain);
	event_type_store_action = esim_engine->RegisterEventType("store_action",
			EventStoreHandler,
			frequency_domain);
	event_type_store_unlock = esim_engine->RegisterEventType("store_unlock",
			EventStoreHandler,
			frequency_domain);
	event_type_store_finish = esim_engine->RegisterEventType("store_finish",
			EventStoreHandler,
			frequency_domain);

	event_type_nc_store = esim_engine->RegisterEventType("nc_store",
			EventNCStoreHandler,
			frequency_domain);
	event_type_nc_store_lock = esim_engine->RegisterEventType("nc_store_lock",
			EventNCStoreHandler,
			frequency_domain);
	event_type_nc_store_writeback = esim_engine->RegisterEventType("nc_store_writeback",
			EventNCStoreHandler,
			frequency_domain);
	event_type_nc_store_action = esim_engine->RegisterEventType("nc_store_action",
			EventNCStoreHandler,
			frequency_domain);
	event_type_nc_store_miss= esim_engine->RegisterEventType("nc_store_miss",
			EventNCStoreHandler,
			frequency_domain);
	event_type_nc_store_unlock = esim_engine->RegisterEventType("nc_store_unlock",
			EventNCStoreHandler,
			frequency_domain);
	event_type_nc_store_finish = esim_engine->RegisterEventType("nc_store_finish",
			EventNCStoreHandler,
			frequency_domain);

	event_type_prefetch = esim_engine->RegisterEventType("prefetch",
			EventPrefetchHandler,
			frequency_domain);
	event_type_prefetch_lock = esim_engine->RegisterEventType("prefetch_lock",
			EventPrefetchHandler,
			frequency_domain);
	event_type_prefetch_action = esim_engine->RegisterEventType("prefetch_action",
			EventPrefetchHandler,
			frequency_domain);
	event_type_prefetch_miss = esim_engine->RegisterEventType("prefetch_miss",
			EventPrefetchHandler,
			frequency_domain);
	event_type_prefetch_unlock = esim_engine->RegisterEventType("prefetch_unlock",
			EventPrefetchHandler,
			frequency_domain);
	event_type_prefetch_finish = esim_engine->RegisterEventType("prefetch_finish",
			EventPrefetchHandler,
			frequency_domain);

	event_type_find_and_lock = esim_engine->RegisterEventType("find_and_lock",
			EventFindAndLockHandler,
			frequency_domain);
	event_type_find_and_lock_port = esim_engine->RegisterEventType("find_and_lock_port",
			EventFindAndLockHandler,
			frequency_domain);
	event_type_find_and_lock_action = esim_engine->RegisterEventType("find_and_lock_action",
			EventFindAndLockHandler,
			frequency_domain);
	event_type_find_and_lock_finish = esim_engine->RegisterEventType("find_and_lock_finish",
			EventFindAndLockHandler,
			frequency_domain);

	event_type_evict = esim_engine->RegisterEventType("evict",
			EventEvictHandler,
			frequency_domain);
	event_type_evict_invalid = esim_engine->RegisterEventType("evict_invalid",
			EventEvictHandler,
			frequency_domain);
	event_type_evict_action = esim_engine->RegisterEventType("evict_action",
			EventEvictHandler,
			frequency_domain);
	event_type_evict_receive = esim_engine->RegisterEventType("evict_receive",
			EventEvictHandler,
			frequency_domain);
	event_type_evict_process = esim_engine->RegisterEventType("evict_process",
			EventEvictHandler,
			frequency_domain);
	event_type_evict_process_noncoherent = esim_engine->RegisterEventType("evict_process_noncoherent",
			EventEvictHandler,
			frequency_domain);
	event_type_evict_reply = esim_engine->RegisterEventType("evict_reply",
			EventEvictHandler,
			frequency_domain);
	event_type_evict_reply = esim_engine->RegisterEventType("evict_reply",
			EventEvictHandler,
			frequency_domain);
	event_type_evict_reply_receive = esim_engine->RegisterEventType("evict_reply_receive",
			EventEvictHandler,
			frequency_domain);
	event_type_evict_finish = esim_engine->RegisterEventType("evict_finish",
			EventEvictHandler,
			frequency_domain);

	event_type_write_request = esim_engine->RegisterEventType("write_request",
			EventWriteRequestHandler,
			frequency_domain);
	event_type_write_request_receive = esim_engine->RegisterEventType("write_request_receive",
			EventWriteRequestHandler,
			frequency_domain);
	event_type_write_request_action = esim_engine->RegisterEventType("write_request_action",
			EventWriteRequestHandler,
			frequency_domain);
	event_type_write_request_exclusive = esim_engine->RegisterEventType("write_request_exclusive",
			EventWriteRequestHandler,
			frequency_domain);
	event_type_write_request_updown = esim_engine->RegisterEventType("write_request_updown",
			EventWriteRequestHandler,
			frequency_domain);
	event_type_write_request_updown_finish = esim_engine->RegisterEventType("write_request_updown_finish",
			EventWriteRequestHandler,
			frequency_domain);
	event_type_write_request_downup = esim_engine->RegisterEventType("write_request_downup",
			EventWriteRequestHandler,
			frequency_domain);
	event_type_write_request_downup_finish = esim_engine->RegisterEventType("write_request_downup_finish",
			EventWriteRequestHandler,
			frequency_domain);
	event_type_write_request_reply = esim_engine->RegisterEventType("write_request_reply",
			EventWriteRequestHandler,
			frequency_domain);
	event_type_write_request_finish = esim_engine->RegisterEventType("write_request_finish",
			EventWriteRequestHandler,
			frequency_domain);

	event_type_read_request = esim_engine->RegisterEventType("read_request",
			EventReadRequestHandler,
			frequency_domain);
	event_type_read_request_receive = esim_engine->RegisterEventType("read_request_receive",
			EventReadRequestHandler,
			frequency_domain);
	event_type_read_request_action = esim_engine->RegisterEventType("read_request_action",
			EventReadRequestHandler,
			frequency_domain);
	event_type_read_request_updown = esim_engine->RegisterEventType("read_request_updown",
			EventReadRequestHandler,
			frequency_domain);
	event_type_read_request_updown_miss = esim_engine->RegisterEventType("read_request_updown_miss",
			EventReadRequestHandler,
			frequency_domain);
	event_type_read_request_updown_finish = esim_engine->RegisterEventType("read_request_updown_finish",
			EventReadRequestHandler,
			frequency_domain);
	event_type_read_request_downup = esim_engine->RegisterEventType("read_request_downup",
			EventReadRequestHandler,
			frequency_domain);
	event_type_read_request_downup_wait_for_reqs = esim_engine->RegisterEventType("read_request_downup_wait_for_reqs",
			EventReadRequestHandler,
			frequency_domain);
	event_type_read_request_downup_finish = esim_engine->RegisterEventType("read_request_downup_finish",
			EventReadRequestHandler,
			frequency_domain);
	event_type_read_request_reply = esim_engine->RegisterEventType("read_request_reply",
			EventReadRequestHandler,
			frequency_domain);
	event_type_read_request_finish = esim_engine->RegisterEventType("read_request_finish",
			EventReadRequestHandler,
			frequency_domain);

	event_type_invalidate = esim_engine->RegisterEventType("invalidate",
			EventInvalidateHandler,
			frequency_domain);
	event_type_invalidate_finish = esim_engine->RegisterEventType("invalidate_finish",
			EventInvalidateHandler,
			frequency_domain);

	event_type_peer_send = esim_engine->RegisterEventType("peer_send",
			EventPeerHandler,
			frequency_domain);
	event_type_peer_receive = esim_engine->RegisterEventType("peer_receive",
			EventPeerHandler,
			frequency_domain);
	event_type_peer_reply = esim_engine->RegisterEventType("peer_reply",
			EventPeerHandler,
			frequency_domain);
	event_type_peer_finish = esim_engine->RegisterEventType("peer_finish",
			EventPeerHandler,
			frequency_domain);

	event_type_message = esim_engine->RegisterEventType("message",
			EventMessageHandler,
			frequency_domain);
	event_type_message_receive = esim_engine->RegisterEventType("message_receive",
			EventMessageHandler,
			frequency_domain);
	event_type_message_action = esim_engine->RegisterEventType("message_action",
			EventMessageHandler,
			frequency_domain);
	event_type_message_reply = esim_engine->RegisterEventType("message_reply",
			EventMessageHandler,
			frequency_domain);
	event_type_message_finish = esim_engine->RegisterEventType("message_finish",
			EventMessageHandler,
			frequency_domain);

	event_type_flush = esim_engine->RegisterEventType("flush",
			EventFlushHandler,
			frequency_domain);
	event_type_flush_finish = esim_engine->RegisterEventType("flush_finish",
			EventFlushHandler,
			frequency_domain);

	// Local memory event driven simulation

	event_type_local_load = esim_engine->RegisterEventType("local_load",
			EventLocalLoadHandler,
			frequency_domain);
	event_type_local_load_lock = esim_engine->RegisterEventType("local_load_lock",
			EventLocalLoadHandler,
			frequency_domain);
	event_type_local_load_finish = esim_engine->RegisterEventType("local_load_finish",
			EventLocalLoadHandler,
			frequency_domain);

	event_type_local_store = esim_engine->RegisterEventType("local_store",
			EventLocalStoreHandler,
			frequency_domain);
	event_type_local_store_lock = esim_engine->RegisterEventType("local_store_lock",
			EventLocalStoreHandler,
			frequency_domain);
	event_type_local_store_finish = esim_engine->RegisterEventType("local_store_finish",
			EventLocalStoreHandler,
			frequency_domain);

	event_type_local_find_and_lock = esim_engine->RegisterEventType("local_find_and_lock",
			EventLocalFindAndLockHandler,
			frequency_domain);
	event_type_local_find_and_lock_port = esim_engine->RegisterEventType("local_find_and_lock_port",
			EventLocalFindAndLockHandler,
			frequency_domain);
	event_type_local_find_and_lock_action = esim_engine->RegisterEventType("local_find_and_lock_action",
			EventLocalFindAndLockHandler,
			frequency_domain);
	event_type_local_find_and_lock_finish = esim_engine->RegisterEventType("local_find_and_lock_finish",
			EventLocalFindAndLockHandler,
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

