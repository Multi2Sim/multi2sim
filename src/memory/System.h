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

#include <list>
#include <map>
#include <memory>

#include <lib/cpp/Debug.h>
#include <lib/esim/Event.h>
#include <lib/esim/FrequencyDomain.h>
#include <lib/esim/Trace.h>
#include <lib/cpp/IniFile.h>
#include <network/Network.h>
#include <network/Node.h>

#include "Module.h"


namespace mem
{


// Forward declarations
class Module;


/// Memory exception
class Error : public misc::Error
{
public:

	/// Constructor
	Error(const std::string &message) : misc::Error(message)
	{
		/// Add module prefix
		AppendPrefix("Memory");
	}
};


/// Memory system
class System
{
	//
	// Static fields
	//

	// Unique instance of this class
	static std::unique_ptr<System> instance;
	
	// Configuration file name
	static std::string config_file;

	// Report file name
	static std::string report_file;

	// Debug file name
	static std::string debug_file;

	// Show memory configuration file
	static bool help;

	// Message to display with '--mem-help'
	static const std::string help_message;

	// Frequency of memory system in MHz
	static int frequency;
	
	// Error messages
	static const char *err_config_note;
	static const char *err_config_net;
	static const char *err_levels;
	static const char *err_block_size;
	static const char *err_connect;
	static const char *err_mem_disjoint;

	// Version of memory system trace producer.
	// See 'src/visual/memory/mem-system.c' for the trace consumer.
	static const int trace_version_major;
	static const int trace_version_minor;

	// Event handlers for NMOESI cache coherence protocol. These functions
	// are defined in SystemCoherenceProtocol.cc.
	static void EventLoadHandler(esim::Event *, esim::Frame *);
	static void EventStoreHandler(esim::Event *, esim::Frame *);
	static void EventNCStoreHandler(esim::Event *, esim::Frame *);
	static void EventFindAndLockHandler(esim::Event *, esim::Frame *);
	static void EventEvictHandler(esim::Event *, esim::Frame *);
	static void EventWriteRequestHandler(esim::Event *, esim::Frame *);
	static void EventReadRequestHandler(esim::Event *, esim::Frame *);
	static void EventInvalidateHandler(esim::Event *, esim::Frame *);
	static void EventMessageHandler(esim::Event *, esim::Frame *);
	static void EventFlushHandler(esim::Event *, esim::Frame *);
	static void EventLocalLoadHandler(esim::Event *, esim::Frame *);
	static void EventLocalStoreHandler(esim::Event *, esim::Frame *);
	static void EventLocalFindAndLockHandler(esim::Event *, esim::Frame *);




	//
	// Member functions
	//

	// Frequency domain for memory system
	esim::FrequencyDomain *frequency_domain = nullptr;

	// Construct a module and add it to the list and map of modules. The
	// arguments for this function match the arguments for the constructor
	// of the module. No module with the same name must exist.
	Module *addModule(const std::string &name,
			Module::Type type,
			int num_ports,
			int block_size,
			int data_latency)
	{
		assert(module_map.find(name) == module_map.end());
		modules.emplace_back(misc::new_unique<Module>(
				name,
				type,
				num_ports,
				block_size,
				data_latency));
		Module *module = modules.back().get();
		module_map[name] = module;
		return module;
	}

	// Construct a network and add it to the list and map of networks. No
	// pair of networks should have the same name.
	net::Network *addNetwork(const std::string &name)
	{
		assert(network_map.find(name) == network_map.end());
		networks.emplace_back(misc::new_unique<net::Network>(name));
		net::Network *network = networks.back().get();
		network_map[name] = network;
		return network;
	}




	//
	// Configuration
	//

	void ConfigReadGeneral(misc::IniFile *ini_file);

	void ConfigReadNetworks(misc::IniFile *ini_file);

	void ConfigInsertModuleInInternalNetwork(
			misc::IniFile *ini_file,
			Module *module,
			const std::string &network_name,
			const std::string &network_node_name,
			net::Network *&network,
			net::EndNode *&node);

	void ConfigInsertModuleInExternalNetwork(
			misc::IniFile *ini_file,
			Module *module,
			const std::string &network_name,
			const std::string &network_node_name,
			net::Network *&network,
			net::EndNode *&node);

	Module *ConfigReadCache(misc::IniFile *ini_file,
			const std::string &section);

	Module *ConfigReadMainMemory(misc::IniFile *ini_file,
			const std::string &section);

	void ConfigInvalidAddressRange(misc::IniFile *ini_file,
			Module *module);

	void ConfigReadModuleAddressRange(misc::IniFile *ini_file,
			Module *module,
			const std::string &section);

	void ConfigReadModules(misc::IniFile *ini_file);

	void ConfigCheckRouteToMainMemory(
			misc::IniFile *ini_file,
			Module *module,
			int block_size,
			int level);

	void ConfigReadLowModules(misc::IniFile *ini_file);

	void ConfigReadEntries(misc::IniFile *ini_file);
	
	void ConfigCreateSwitches(misc::IniFile *ini_file);
	
	void ConfigCheckRoutes(misc::IniFile *ini_file);

	void ConfigCalculateSubBlockSizes();

	void ConfigSetModuleLevel(Module *module, int level);

	void ConfigCalculateModuleLevels();

	void ConfigTrace();

	void ConfigReadCommands(misc::IniFile *ini_file);


	// List of networks
	std::list<std::unique_ptr<net::Network>> networks;

	// Map of networks, indexed by their name
	std::map<std::string, net::Network *> network_map;

	// List of modules
	std::list<std::unique_ptr<Module>> modules;

	// Map of modules, indexed by their name
	std::map<std::string, Module *> module_map;

public:

	/// Constructor
	System();

	/// Return a module given its name, or nullptr if no module with that
	/// name exists.
	Module *getModule(const std::string &name) const
	{
		auto it = module_map.find(name);
		return it == module_map.end() ? nullptr : it->second;
	}

	/// Return a network given its name, or nullptr if no network with that
	/// name exists.
	net::Network *getNetwork(const std::string &name) const
	{
		auto it = network_map.find(name);
		return it == network_map.end() ? nullptr : it->second;
	}




	//
	// Static fields
	//

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

	/// Returns true if the instance exists
	static bool hasInstance() { return instance.get(); }

	/// Destroy the singleton if allocated.
	static void Destroy() { instance = nullptr; }
	



	//
	// Event-driven simulation
	//

	static esim::Event *event_load;
	static esim::Event *event_load_lock;
	static esim::Event *event_load_action;
	static esim::Event *event_load_miss;
	static esim::Event *event_load_unlock;
	static esim::Event *event_load_finish;

	static esim::Event *event_store;
	static esim::Event *event_store_lock;
	static esim::Event *event_store_action;
	static esim::Event *event_store_unlock;
	static esim::Event *event_store_finish;

	static esim::Event *event_nc_store;
	static esim::Event *event_nc_store_lock;
	static esim::Event *event_nc_store_writeback;
	static esim::Event *event_nc_store_action;
	static esim::Event *event_nc_store_miss;
	static esim::Event *event_nc_store_unlock;
	static esim::Event *event_nc_store_finish;

	static esim::Event *event_find_and_lock;
	static esim::Event *event_find_and_lock_port;
	static esim::Event *event_find_and_lock_action;
	static esim::Event *event_find_and_lock_finish;

	static esim::Event *event_evict;
	static esim::Event *event_evict_invalid;
	static esim::Event *event_evict_action;
	static esim::Event *event_evict_receive;
	static esim::Event *event_evict_process;
	static esim::Event *event_evict_process_noncoherent;
	static esim::Event *event_evict_reply;
	static esim::Event *event_evict_reply_receive;
	static esim::Event *event_evict_finish;

	static esim::Event *event_write_request;
	static esim::Event *event_write_request_receive;
	static esim::Event *event_write_request_action;
	static esim::Event *event_write_request_exclusive;
	static esim::Event *event_write_request_updown;
	static esim::Event *event_write_request_updown_finish;
	static esim::Event *event_write_request_downup;
	static esim::Event *event_write_request_downup_finish;
	static esim::Event *event_write_request_reply;
	static esim::Event *event_write_request_finish;

	static esim::Event *event_read_request;
	static esim::Event *event_read_request_receive;
	static esim::Event *event_read_request_action;
	static esim::Event *event_read_request_updown;
	static esim::Event *event_read_request_updown_miss;
	static esim::Event *event_read_request_updown_finish;
	static esim::Event *event_read_request_downup;
	static esim::Event *event_read_request_downup_finish;
	static esim::Event *event_read_request_reply;
	static esim::Event *event_read_request_finish;

	static esim::Event *event_invalidate;
	static esim::Event *event_invalidate_finish;

	static esim::Event *event_message;
	static esim::Event *event_message_receive;
	static esim::Event *event_message_action;
	static esim::Event *event_message_reply;
	static esim::Event *event_message_finish;

	static esim::Event *event_flush;
	static esim::Event *event_flush_finish;
	
	static esim::Event *event_local_load;
	static esim::Event *event_local_load_lock;
	static esim::Event *event_local_load_finish;

	static esim::Event *event_local_store;
	static esim::Event *event_local_store_lock;
	static esim::Event *event_local_store_finish;

	static esim::Event *event_local_find_and_lock;
	static esim::Event *event_local_find_and_lock_port;
	static esim::Event *event_local_find_and_lock_action;
	static esim::Event *event_local_find_and_lock_finish;




	//
	// Configuration
	//

	// Initialize the memory system by parsing the memory configuration
	// file passed with '--mem-config' by the user. If this option was not
	// given, generate a default memory configuration for each architecture
	// with timing simulation.
	void ReadConfiguration();

	// Initialize the memory system from an existing memory configuration
	// INI file. This function is internally invoked by ReadConfiguration()
	// or externally invoked for unit testing purposes.
	void ReadConfiguration(misc::IniFile *ini_file);




	// 
	// Memory report
	//

	/// Dump the memory report in the report file
	void DumpReport();

	/// Dump function for report
	void Dump(std::ostream &os = std::cout) const;

};

}  // namespace mem

#endif

