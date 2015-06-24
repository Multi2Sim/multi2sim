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
	instance = misc::new_unique<System>();
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

	event_load = esim_engine->RegisterEvent("load",
			EventLoadHandler,
			frequency_domain);
	event_load_lock = esim_engine->RegisterEvent("load_lock",
			EventLoadHandler,
			frequency_domain);
	event_load_action = esim_engine->RegisterEvent("load_action",
			EventLoadHandler,
			frequency_domain);
	event_load_miss = esim_engine->RegisterEvent("load_miss",
			EventLoadHandler,
			frequency_domain);
	event_load_unlock = esim_engine->RegisterEvent("load_unlock",
			EventLoadHandler,
			frequency_domain);
	event_load_finish = esim_engine->RegisterEvent("load_finish",
			EventLoadHandler,
			frequency_domain);

	event_store = esim_engine->RegisterEvent("store",
			EventStoreHandler,
			frequency_domain);
	event_store_lock = esim_engine->RegisterEvent("store_lock",
			EventStoreHandler,
			frequency_domain);
	event_store_action = esim_engine->RegisterEvent("store_action",
			EventStoreHandler,
			frequency_domain);
	event_store_unlock = esim_engine->RegisterEvent("store_unlock",
			EventStoreHandler,
			frequency_domain);
	event_store_finish = esim_engine->RegisterEvent("store_finish",
			EventStoreHandler,
			frequency_domain);

	event_nc_store = esim_engine->RegisterEvent("nc_store",
			EventNCStoreHandler,
			frequency_domain);
	event_nc_store_lock = esim_engine->RegisterEvent("nc_store_lock",
			EventNCStoreHandler,
			frequency_domain);
	event_nc_store_writeback = esim_engine->RegisterEvent("nc_store_writeback",
			EventNCStoreHandler,
			frequency_domain);
	event_nc_store_action = esim_engine->RegisterEvent("nc_store_action",
			EventNCStoreHandler,
			frequency_domain);
	event_nc_store_miss= esim_engine->RegisterEvent("nc_store_miss",
			EventNCStoreHandler,
			frequency_domain);
	event_nc_store_unlock = esim_engine->RegisterEvent("nc_store_unlock",
			EventNCStoreHandler,
			frequency_domain);
	event_nc_store_finish = esim_engine->RegisterEvent("nc_store_finish",
			EventNCStoreHandler,
			frequency_domain);

	event_find_and_lock = esim_engine->RegisterEvent("find_and_lock",
			EventFindAndLockHandler,
			frequency_domain);
	event_find_and_lock_port = esim_engine->RegisterEvent("find_and_lock_port",
			EventFindAndLockHandler,
			frequency_domain);
	event_find_and_lock_action = esim_engine->RegisterEvent("find_and_lock_action",
			EventFindAndLockHandler,
			frequency_domain);
	event_find_and_lock_finish = esim_engine->RegisterEvent("find_and_lock_finish",
			EventFindAndLockHandler,
			frequency_domain);

	event_evict = esim_engine->RegisterEvent("evict",
			EventEvictHandler,
			frequency_domain);
	event_evict_invalid = esim_engine->RegisterEvent("evict_invalid",
			EventEvictHandler,
			frequency_domain);
	event_evict_action = esim_engine->RegisterEvent("evict_action",
			EventEvictHandler,
			frequency_domain);
	event_evict_receive = esim_engine->RegisterEvent("evict_receive",
			EventEvictHandler,
			frequency_domain);
	event_evict_process = esim_engine->RegisterEvent("evict_process",
			EventEvictHandler,
			frequency_domain);
	event_evict_process_noncoherent = esim_engine->RegisterEvent("evict_process_noncoherent",
			EventEvictHandler,
			frequency_domain);
	event_evict_reply = esim_engine->RegisterEvent("evict_reply",
			EventEvictHandler,
			frequency_domain);
	event_evict_reply = esim_engine->RegisterEvent("evict_reply",
			EventEvictHandler,
			frequency_domain);
	event_evict_reply_receive = esim_engine->RegisterEvent("evict_reply_receive",
			EventEvictHandler,
			frequency_domain);
	event_evict_finish = esim_engine->RegisterEvent("evict_finish",
			EventEvictHandler,
			frequency_domain);

	event_write_request = esim_engine->RegisterEvent("write_request",
			EventWriteRequestHandler,
			frequency_domain);
	event_write_request_receive = esim_engine->RegisterEvent("write_request_receive",
			EventWriteRequestHandler,
			frequency_domain);
	event_write_request_action = esim_engine->RegisterEvent("write_request_action",
			EventWriteRequestHandler,
			frequency_domain);
	event_write_request_exclusive = esim_engine->RegisterEvent("write_request_exclusive",
			EventWriteRequestHandler,
			frequency_domain);
	event_write_request_updown = esim_engine->RegisterEvent("write_request_updown",
			EventWriteRequestHandler,
			frequency_domain);
	event_write_request_updown_finish = esim_engine->RegisterEvent("write_request_updown_finish",
			EventWriteRequestHandler,
			frequency_domain);
	event_write_request_downup = esim_engine->RegisterEvent("write_request_downup",
			EventWriteRequestHandler,
			frequency_domain);
	event_write_request_downup_finish = esim_engine->RegisterEvent("write_request_downup_finish",
			EventWriteRequestHandler,
			frequency_domain);
	event_write_request_reply = esim_engine->RegisterEvent("write_request_reply",
			EventWriteRequestHandler,
			frequency_domain);
	event_write_request_finish = esim_engine->RegisterEvent("write_request_finish",
			EventWriteRequestHandler,
			frequency_domain);

	event_read_request = esim_engine->RegisterEvent("read_request",
			EventReadRequestHandler,
			frequency_domain);
	event_read_request_receive = esim_engine->RegisterEvent("read_request_receive",
			EventReadRequestHandler,
			frequency_domain);
	event_read_request_action = esim_engine->RegisterEvent("read_request_action",
			EventReadRequestHandler,
			frequency_domain);
	event_read_request_updown = esim_engine->RegisterEvent("read_request_updown",
			EventReadRequestHandler,
			frequency_domain);
	event_read_request_updown_miss = esim_engine->RegisterEvent("read_request_updown_miss",
			EventReadRequestHandler,
			frequency_domain);
	event_read_request_updown_finish = esim_engine->RegisterEvent("read_request_updown_finish",
			EventReadRequestHandler,
			frequency_domain);
	event_read_request_downup = esim_engine->RegisterEvent("read_request_downup",
			EventReadRequestHandler,
			frequency_domain);
	event_read_request_downup_finish = esim_engine->RegisterEvent("read_request_downup_finish",
			EventReadRequestHandler,
			frequency_domain);
	event_read_request_reply = esim_engine->RegisterEvent("read_request_reply",
			EventReadRequestHandler,
			frequency_domain);
	event_read_request_finish = esim_engine->RegisterEvent("read_request_finish",
			EventReadRequestHandler,
			frequency_domain);

	event_invalidate = esim_engine->RegisterEvent("invalidate",
			EventInvalidateHandler,
			frequency_domain);
	event_invalidate_finish = esim_engine->RegisterEvent("invalidate_finish",
			EventInvalidateHandler,
			frequency_domain);

	event_message = esim_engine->RegisterEvent("message",
			EventMessageHandler,
			frequency_domain);
	event_message_receive = esim_engine->RegisterEvent("message_receive",
			EventMessageHandler,
			frequency_domain);
	event_message_action = esim_engine->RegisterEvent("message_action",
			EventMessageHandler,
			frequency_domain);
	event_message_reply = esim_engine->RegisterEvent("message_reply",
			EventMessageHandler,
			frequency_domain);
	event_message_finish = esim_engine->RegisterEvent("message_finish",
			EventMessageHandler,
			frequency_domain);

	event_flush = esim_engine->RegisterEvent("flush",
			EventFlushHandler,
			frequency_domain);
	event_flush_finish = esim_engine->RegisterEvent("flush_finish",
			EventFlushHandler,
			frequency_domain);

	// Local memory event driven simulation

	event_local_load = esim_engine->RegisterEvent("local_load",
			EventLocalLoadHandler,
			frequency_domain);
	event_local_load_lock = esim_engine->RegisterEvent("local_load_lock",
			EventLocalLoadHandler,
			frequency_domain);
	event_local_load_finish = esim_engine->RegisterEvent("local_load_finish",
			EventLocalLoadHandler,
			frequency_domain);

	event_local_store = esim_engine->RegisterEvent("local_store",
			EventLocalStoreHandler,
			frequency_domain);
	event_local_store_lock = esim_engine->RegisterEvent("local_store_lock",
			EventLocalStoreHandler,
			frequency_domain);
	event_local_store_finish = esim_engine->RegisterEvent("local_store_finish",
			EventLocalStoreHandler,
			frequency_domain);

	event_local_find_and_lock = esim_engine->RegisterEvent("local_find_and_lock",
			EventLocalFindAndLockHandler,
			frequency_domain);
	event_local_find_and_lock_port = esim_engine->RegisterEvent("local_find_and_lock_port",
			EventLocalFindAndLockHandler,
			frequency_domain);
	event_local_find_and_lock_action = esim_engine->RegisterEvent("local_find_and_lock_action",
			EventLocalFindAndLockHandler,
			frequency_domain);
	event_local_find_and_lock_finish = esim_engine->RegisterEvent("local_find_and_lock_finish",
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

