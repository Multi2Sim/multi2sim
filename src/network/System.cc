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

#include "System.h"

namespace net
{

// Command Map
misc::StringMap System::command_map =
{
		{ "command",     TrafficPatternCommand },
		{ "uniform",     TrafficPatternUniform }
};

//
// Configuration Options
//

// Network Debug
std::string System::debug_file;
misc::Debug System::debug;

// Network Report
std::string System::report_file;

// Visual File name
std::string System::visual_file;

// Network Routing Table print
std::string System::routing_table_file;

// maximum number of cycles
long long System::max_cycles = 1000000;

// default message size for stand alone network
int System::msg_size = 1;

// default injection rate for stand alone network
int System::injection_rate = 0.001;

// snapshot period
int System::snapshot_period = 0;

// Printing the Network Help
bool System::net_help = false;

// Stand-alone simulator instantiation
bool System::stand_alone = false;


//
// Static variables
//

// Network System singleton
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
	// Initialize class members

}

void System::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("net");

	// Debug information
	command_line->RegisterString("--net-debug <file>", debug_file,
			"Debug information related with interconnection "
			"networks including packet transfers, link usage, etc");

	// Configuration Inifile for External Networks
	command_line->RegisterString("--net-config <file>", config_file,
			"Network configuration file. Networks in the memory "
			"hierarchy can be defined here and referenced in other"
			" configuration files. For a description of the format,"
			" use option '--net-help'");

	// Maximum simulation time in cycles
	command_line->RegisterInt64("--net-max-cycles <number> (default = 1M)",
			max_cycles,
			"Maximum number of cycles for network simulation."
			"together with option '--net-sim.");

	// Network Help Message
	command_line->RegisterBool("--net-help",
			net_help,
			"Print help message describing the network configuration"
			" file, passed to the simulator "
			"with option '--net-config <file>'.");

	// Message size for network stand-alone simulator
	command_line->RegisterInt32("--net-msg-size <number> (default = 1 Byte",
			msg_size,
			"For network simulation, packet size in bytes. An entire"
			" packet is assumed to fit in a node's buffer, but its "
			"transfer latency through a link will depend on the "
			"message size and the link bandwidth. This option must "
			"be used together with '--net-sim'.");

	// Report file for each Network
	command_line->RegisterString("--net-report <file>",
			report_file,
			"File to dump detailed statistics for each network "
			"defined in the network configuration file (option "
			"'--net-config'). The report includes statistics"
			"on bandwidth utilization, network traffic, etc. and "
			"will be presented as <network_name>_<file>");

	// Visual File for each Network
	command_line->RegisterString("--net-visual <file>",
			visual_file,
			"File for graphically representing the interconnection "
			"networks. It we be presented as a file with name "
			"<network_name>_<file>. This file is an input for a "
			"supplementary tool called 'graphplot' which is "
			"located in samples/network folder in multi2sim trunk.");

	// Injection rate for stand-alone simulator
	command_line->RegisterInt32("--net-injection-rate <number> (default 0.001)"
			, injection_rate,
			"For network simulation, packet injection rate for nodes"
			" (e.g. 0.001 means one packet every 1000 cycles on "
			"average. Nodes will inject packets into the network "
			"using random delays with exponential distribution with "
			"lambda = <rate>. This option must be used together "
			"with '--net-sim'. (see '--net-traffic-pattern'");

	// Stand-alone simulator
	command_line->RegisterBool("--net-sim <network name>",
			stand_alone,
			"Runs a network simulation using synthetic traffic, "
			"where <network> is the name of a network specified "
			"in the network configuration file (option "
			"'--net-config')");

	// Network traffic snapshot
	command_line->RegisterInt32("--net-snapshot <number> (default = 0)",
			snapshot_period,
			"Accumulates the network traffic in sepcified periods "
			"and creates a plot for each individual network, "
			"showing the traffic pattern ");

	// Network Routing Table representation
	command_line->RegisterString("--net-dump-routes <file>",
			routing_table_file,
			"Prints a table that shows the connection between"
			"all the nodes in each individual network. The provided"
			"file would be in <network_name>_<file> format");

	// Network Traffic Pattern
	bool err;
	std::string pattern;
	command_line->RegisterString("--net-traffic-pattern {uniform|command} "
			"(default = uniform) ",
			pattern,
			"If set on uniform, messages are injected in the network"
			"uniformly with random delays with exponential "
			"distribution. If set on command, the Commands section"
			"of configuration file is activated and messages are"
			"inserted in the network on certain cycles indicated in"
			"commands (use '--net-help' for learning more about"
			"commands");

	traffic_pattern = (System::TrafficPattern)
			command_map.MapString(pattern, err);
	if (err)
	{
		misc::fatal("%s: unrecognized command for network traffic "
				"pattern", __FUNCTION__);
	}

}

void System::ProcessOptions()
{
	// Debugger
	if (debug_file.empty())
		debug.setPath(debug_file);

}

}
