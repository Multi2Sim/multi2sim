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

#ifndef MEMORY_SYSTEM_H
#define MEMORY_SYSTEM_H

#include <memory>

#include <lib/cpp/Debug.h>
#include <lib/esim/Event.h>
#include <lib/esim/FrequencyDomain.h>
#include <lib/esim/Trace.h>
#include <lib/cpp/IniFile.h>


namespace mem
{


// Forward declarations
class Module;

/// Memory system
class System
{
	// Unique instance of this class
	static std::unique_ptr<System> instance;

	// Frequency domain for memory system
	esim::FrequencyDomain *frequency_domain;

	// Private constructor
	System();

	
	
	//
	// Memory system configuration. These functions are defined in
	// SystemConfig.cc
	//
	
	// Configuration file name
	static std::string config_file;

	// Debug file name
	static std::string debug_file;

	// Show memory configuration file
	static bool help;

	static const std::string help_message;

	// Parse memory hierarchy configuration file, or generate default if
	// user didn't pass '--mem-config' command-line option.
	void ConfigRead();

	void ConfigReadGeneral(misc::IniFile *ini_file);

	void ConfigReadNetworks(misc::IniFile *ini_file);

	void ConfigReadCache(misc::IniFile *ini_file,
			const std::string &section);

	void ConfigReadMainMemory(misc::IniFile *ini_file,
			const std::string &section);

	void ConfigReadModuleAddressRange(misc::IniFile *ini_file,
			Module *module,
			const std::string &section);

	void ConfigReadModules(misc::IniFile *ini_file);

	void ConfigCheckRouteToMainMemory(Module *module,
			int block_size,
			int level);

	void ConfigReadLowModules(misc::IniFile *ini_file);

	void ConfigReadEntries(misc::IniFile *ini_file);
	
	void ConfigCreateSwitches(misc::IniFile *ini_file);
	
	void ConfigCheckRoutes();

	void CalculateSubBlockSizes();

	void ConfigSetModuleLevel(Module *module, int level);

	void ConfigCalculateModuleLevels();

	void ConfigTrace();

	void ConfigReadCommands(misc::IniFile *ini_file);


	//
	// Event handlers for NMOESI cache coherence protocol. These functions
	// are defined in SystemCoherenceProtocol.cc.
	//

	static void evLoadHandler(esim::EventType *, esim::EventFrame *);
	static void evStoreHandler(esim::EventType *, esim::EventFrame *);
	static void evNCStoreHandler(esim::EventType *, esim::EventFrame *);
	static void evPrefetchHandler(esim::EventType *, esim::EventFrame *);
	static void evFindAndLockHandler(esim::EventType *, esim::EventFrame *);
	static void evEvictHandler(esim::EventType *, esim::EventFrame *);
	static void evWriteRequestHandler(esim::EventType *, esim::EventFrame *);
	static void evReadRequestHandler(esim::EventType *, esim::EventFrame *);
	static void evInvalidateHandler(esim::EventType *, esim::EventFrame *);
	static void evPeerHandler(esim::EventType *, esim::EventFrame *);
	static void evMessageHandler(esim::EventType *, esim::EventFrame *);
	static void evFlushHandler(esim::EventType *, esim::EventFrame *);
	static void evLocalLoadHandler(esim::EventType *, esim::EventFrame *);
	static void evLocalStoreHandler(esim::EventType *, esim::EventFrame *);
	static void evLocalFindAndLockHandler(esim::EventType *, esim::EventFrame *);

public:

	// NMOESI event types
	static esim::EventType *ev_load;
	static esim::EventType *ev_load_lock;
	static esim::EventType *ev_load_action;
	static esim::EventType *ev_load_miss;
	static esim::EventType *ev_load_unlock;
	static esim::EventType *ev_load_finish;

	static esim::EventType *ev_store;
	static esim::EventType *ev_store_lock;
	static esim::EventType *ev_store_action;
	static esim::EventType *ev_store_unlock;
	static esim::EventType *ev_store_finish;

	static esim::EventType *ev_nc_store;
	static esim::EventType *ev_nc_store_lock;
	static esim::EventType *ev_nc_store_writeback;
	static esim::EventType *ev_nc_store_action;
	static esim::EventType *ev_nc_store_miss;
	static esim::EventType *ev_nc_store_unlock;
	static esim::EventType *ev_nc_store_finish;

	static esim::EventType *ev_prefetch;
	static esim::EventType *ev_prefetch_lock;
	static esim::EventType *ev_prefetch_action;
	static esim::EventType *ev_prefetch_miss;
	static esim::EventType *ev_prefetch_unlock;
	static esim::EventType *ev_prefetch_finish;

	static esim::EventType *ev_find_and_lock;
	static esim::EventType *ev_find_and_lock_port;
	static esim::EventType *ev_find_and_lock_action;
	static esim::EventType *ev_find_and_lock_finish;

	static esim::EventType *ev_evict;
	static esim::EventType *ev_evict_invalid;
	static esim::EventType *ev_evict_action;
	static esim::EventType *ev_evict_receive;
	static esim::EventType *ev_evict_process;
	static esim::EventType *ev_evict_process_noncoherent;
	static esim::EventType *ev_evict_reply;
	static esim::EventType *ev_evict_reply_receive;
	static esim::EventType *ev_evict_finish;

	static esim::EventType *ev_write_request;
	static esim::EventType *ev_write_request_receive;
	static esim::EventType *ev_write_request_action;
	static esim::EventType *ev_write_request_exclusive;
	static esim::EventType *ev_write_request_updown;
	static esim::EventType *ev_write_request_updown_finish;
	static esim::EventType *ev_write_request_downup;
	static esim::EventType *ev_write_request_downup_finish;
	static esim::EventType *ev_write_request_reply;
	static esim::EventType *ev_write_request_finish;

	static esim::EventType *ev_read_request;
	static esim::EventType *ev_read_request_receive;
	static esim::EventType *ev_read_request_action;
	static esim::EventType *ev_read_request_updown;
	static esim::EventType *ev_read_request_updown_miss;
	static esim::EventType *ev_read_request_updown_finish;
	static esim::EventType *ev_read_request_downup;
	static esim::EventType *ev_read_request_downup_wait_for_reqs;
	static esim::EventType *ev_read_request_downup_finish;
	static esim::EventType *ev_read_request_reply;
	static esim::EventType *ev_read_request_finish;

	static esim::EventType *ev_invalidate;
	static esim::EventType *ev_invalidate_finish;

	static esim::EventType *ev_peer_send;
	static esim::EventType *ev_peer_receive;
	static esim::EventType *ev_peer_reply;
	static esim::EventType *ev_peer_finish;

	static esim::EventType *ev_message;
	static esim::EventType *ev_message_receive;
	static esim::EventType *ev_message_action;
	static esim::EventType *ev_message_reply;
	static esim::EventType *ev_message_finish;

	static esim::EventType *ev_flush;
	static esim::EventType *ev_flush_finish;
	
	static esim::EventType *ev_local_load;
	static esim::EventType *ev_local_load_lock;
	static esim::EventType *ev_local_load_finish;

	static esim::EventType *ev_local_store;
	static esim::EventType *ev_local_store_lock;
	static esim::EventType *ev_local_store_finish;

	static esim::EventType *ev_local_find_and_lock;
	static esim::EventType *ev_local_find_and_lock_port;
	static esim::EventType *ev_local_find_and_lock_action;
	static esim::EventType *ev_local_find_and_lock_finish;

	/// Memory system trace
	static esim::Trace trace;

	/// Memory system debugger
	static misc::Debug debug;

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();
	
	/// Obtain singleton instance.
	static System *getInstance();
};

}  // namespace mem

#endif

