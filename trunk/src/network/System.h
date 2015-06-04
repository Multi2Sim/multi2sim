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
#include <lib/esim/Event.h>

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

class System
{
	// Traffic pattern states for network system
	enum TrafficPattern
	{
		TrafficPatternInvalid = 0,
		TrafficPatternUniform,
		TrafficPatternCommand
	};

	// String map for traffic pattern
	static const misc::StringMap TrafficPatternMap;

	// Debugger file
	static std::string debug_file;

	// Configuration file
	static std::string config_file;

	// Report file
	static std::string report_file;

	// Visual file
	static std::string visual_file;

	// Routing table print file
	static std::string routing_table_file;

	// Network Traffic Pattern
	static TrafficPattern traffic_pattern;

	// Stand-Alone Simulator Network Name
	static std::string sim_net_name;

	// Stand-alone simulation duration.
	static long long max_cycles;

	// Stand-alone message injection rate
	static int injection_rate;

	// Network snapshot sampling period
	static int snapshot_period;

	// Network Help
	static bool net_help;

	// Stand-alone simulator instantiator
	static bool stand_alone;

	// Networks configuration file
	static misc::IniFile ini_file;

	// Unique instance of singleton
	static std::unique_ptr<System> instance;

	// Hash table of networks indexed by their names
	std::unordered_map<std::string, Network *> network_map;

	// List of networks in the system
	std::vector<std::unique_ptr<Network>> networks;

	// General frequency if not specified in the network section
	static int net_system_frequency;

	// Get a exponential random valueclass Network;
	static double RandomExponential(double lambda)
	{
		double x = (double) random() / RAND_MAX;
		return log(1 - x) / -lambda;
	}

	// Uniform traffic simulation
	static void UniformTrafficSimulation(Network *network);




	//
	// Event driven simulation
	//

	// Network frequency domain
	static esim::FrequencyDomain *frequency_domain;

	// Handle the send event
	static void evNetSendHandler(esim::EventType *, esim::EventFrame *);

	// Handle the event of message arriving in output buffer
	static void evNetSendOutputBuffer(esim::EventType *, 
			esim::EventFrame *);

	// Handle the event of message arriving in input buffer
	static void evNetSendInputBuffer(esim::EventType *, 
			esim::EventFrame *);

	// Handle the message receive event
	static void evNetSendReceive(esim::EventType *, esim::EventFrame *);

public:

	// Message Size in stand alone network
	static int message_size;

	//
	// Event driven simulation event types
	//

	static esim::EventType *ev_net_send;
	static esim::EventType *ev_net_output_buffer;
	static esim::EventType *ev_net_input_buffer;
	static esim::EventType *ev_net_receive;
	

	/// Debugger for network
	static misc::Debug debug;

	/// Get instance of singleton
	static System *getInstance();

	/// Constructor
	System() { assert(instance == nullptr); }



	/// Find and returns a network in the network system given its name.
	///
	/// \param name
	///	Network name
	Network *getNetworkByName(const std::string &name);

	/// Create a new network.
	///
	/// \param name
	///	Name of the new network.
	Network *newNetwork(const std::string &name);

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Parse a configuration INI file
	void ParseConfiguration(misc::IniFile *ini_file);

	/// Returns whether or not Network is running as a stand alone simulator.
	bool isStandAlone() { return stand_alone; }

};

}  // namespace net

#endif

