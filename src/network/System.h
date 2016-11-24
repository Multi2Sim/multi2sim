/*
 *  Multi2Sim
 *  Copyright (C) 2014  Amir Kavyan Ziabari (aziabari@ece.neu.edu)
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

#ifndef NETWORK_SYSTEM_H
#define NETWORK_SYSTEM_H

#include <cassert>
#include <cmath>

#include <lib/cpp/Debug.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/IniFile.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Event.h>
#include <lib/esim/Trace.h>

#include "Network.h"
namespace net
{

class Network;


/// Class representing a runtime error in network system
class Error : public misc::Error
{
public:

	/// Constructor
	Error(const std::string &message) : misc::Error(message)
	{
		// Set module prefix
		AppendPrefix("Network");
	}
};


/// Network system singleton.
class System
{
	//
	// Static members
	//

	// Debugger file
	static std::string debug_file;

	// Configuration file
	static std::string config_file;

	// Configuration file
	static std::string report_file;

	// Static graph file
	static std::string graph_file;

	// Static router file
	static std::string route_file;

	// Show help for network configuration file
	static bool help;

	// Message to display with '--net-help'
	static const std::string help_message;

	// Stand-Alone Simulator Network Name
	static std::string sim_net_name;

	// Stand-alone simulation duration.
	static long long max_cycles;

	// Stand-alone message injection rate
	static double injection_rate;

	// Stand-alone simulator instantiator
	static bool stand_alone;

	// Unique instance of singleton
	static std::unique_ptr<System> instance;

	// General frequency if not specified in the network section
	static int frequency;
	
	/// Message size in stand alone network
	static int message_size;

	// Network trace version identifiers
	static const int trace_version_major;
	static const int trace_version_minor;

	// Get a exponential random valueclass Network;
	static double RandomExponential(double lambda);





	//
	// Event driven simulation
	//

	// Network event handlers
	static void EventTypeSendHandler(esim::Event *, esim::Frame *);
	static void EventTypeOutputBufferHandler(esim::Event *, esim::Frame *);
	static void EventTypeInputBufferHandler(esim::Event *, esim::Frame *);
	static void EventTypeReceiveHandler(esim::Event *, esim::Frame *);




	//
	// Class members
	//
	
	// Pointer to event-driven simulator, for efficiency
	esim::Engine *esim_engine = nullptr;

	// Network frequency domain
	esim::FrequencyDomain *frequency_domain = nullptr;

	// Hash table of networks indexed by their names
	std::unordered_map<std::string, Network *> network_map;

	// List of networks in the system
	std::vector<std::unique_ptr<Network>> networks;

public:

	//
	// Static members
	//

	//
	// Error messages
	//

	static const char *err_config_note;




	//
	// Event driven simulation event types
	//

	static esim::Event *event_send;
	static esim::Event *event_output_buffer;
	static esim::Event *event_input_buffer;
	static esim::Event *event_receive;

	/// Network system trace
	static esim::Trace trace;

	/// Debugger for network
	static misc::Debug debug;

	/// Get instance of singleton
	static System *getInstance();

	/// Returns true if the instance exists
	static bool hasInstance() { return instance.get(); }

	/// Destroy the singleton if allocated.
	static void Destroy() { instance = nullptr; }




	//
	// Configuration
	//

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Returns whether we are running as a stand alone simulator.
	static bool isStandAlone() { return stand_alone; }

	/// Return the message size for stand-alone simulation, as configured
	/// by the user.
	static int getMessageSize() { return message_size; }




	//
	// Class members
	//

	/// Constructor
	System();

	/// Find and returns a network in the network system given its name,
	/// or `nullptr` if no network exists with that name.
	Network *getNetworkByName(const std::string &name) const
	{
		auto it = network_map.find(name);
		return it == network_map.end() ? nullptr : it->second;
	}

	/// Return the current cycle in the network frequency domain.
	long long getCycle() const
	{
		return frequency_domain->getCycle();
	}

	/// Parse a configuration INI file
	void ParseConfiguration(misc::IniFile *ini_file);

	/// Output the report file
	void DumpReport();

	/// Output the route file
	void DumpRoutes();

	/// Output the static graph file
	void StaticGraph();

	/// Update trace file header
	void TraceHeader();

	// Initialize the network system by parsing the network configuration
	// file passed with '--net-config' by the user.
	void ReadConfiguration();

	// Uniform traffic simulation
	void UniformTrafficSimulation(Network *network);

	// Stand-Alone simulation
	void StandAlone();

    int getNumNetworks() const { return networks.size(); }

    Network *getNetwork(int index) const {
        assert(index >= 0 && index < (int) networks.size());
        return networks[index].get();
    }
};

}  // namespace net

#endif

