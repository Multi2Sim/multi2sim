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

#include <lib/cpp/CommandLine.h>
#include <lib/esim/Engine.h>
#include <lib/cpp/Misc.h>

#include "Network.h"
#include "Node.h"
#include "EndNode.h"
#include "System.h"

namespace net
{

std::string System::config_file;

std::string System::debug_file;

std::string System::report_file;

std::string System::graph_file;

std::string System::route_file;

misc::Debug System::debug;

esim::Trace System::trace;

std::string System::sim_net_name;

long long System::max_cycles = 1000000;

int System::message_size = 1;

double System::injection_rate = 0.001;

bool System::stand_alone = false;

bool System::help = false;

int System::frequency = 1000;

const int System::trace_version_major = 1;
const int System::trace_version_minor = 10;

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


double System::RandomExponential(double lambda)
{
	double x = (double) random() / RAND_MAX;
	double ret = log(1 - x) / -lambda;
	return ret;
}


System::System()
{
	// Create frequency domain
	esim_engine = esim::Engine::getInstance();
	frequency_domain = esim_engine->RegisterFrequencyDomain("Network");

	// Register events here
	event_send = esim_engine->RegisterEvent("send", 
			EventTypeSendHandler, frequency_domain);
	event_output_buffer = esim_engine->RegisterEvent(
			"output_buffer", EventTypeOutputBufferHandler, 
			frequency_domain);
	event_input_buffer = esim_engine->RegisterEvent(
			"input_buffer", EventTypeInputBufferHandler, 
			frequency_domain);
	event_receive = esim_engine->RegisterEvent("receive", 
			EventTypeReceiveHandler, frequency_domain);


}


void System::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("Network");

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

	// Message size for network stand-alone simulator
	command_line->RegisterInt32("--net-msg-size <number> (default = 1 Byte)",
			message_size,
			"For network simulation, packet size in bytes. An entire"
			" packet is assumed to fit in a node's buffer, but its "
			"transfer latency through a link will depend on the "
			"message size and the link bandwidth. This option must "
			"be used together with '--net-sim'.");

	// Injection rate for stand-alone simulator
	command_line->RegisterDouble("--net-injection-rate <number> (default 0.001)",
			injection_rate,
			"For network simulation, packet injection rate for nodes"
			" (e.g. 0.001 means one packet every 1000 cycles on "
			"average. Nodes will inject packets into the network "
			"using random delays with exponential distribution with "
			"lambda = <rate>. This option must be used together "
			"with '--net-sim'.");

	// Stand-alone simulator
	command_line->RegisterString("--net-sim <network name>",
			sim_net_name,
			"Runs a network simulation using synthetic traffic, "
			"where <network> is the name of a network specified "
			"in the network configuration file (option "
			"'--net-config')");

	// Help message for network configuration
	command_line->RegisterBool("--net-help",
			help,
			"Print help message describing the network configuration"
			" file, passed in option '--net-config <file>'.");

	// Report file
	command_line->RegisterString("--net-report <file>", report_file,
			"File to dump detailed statistics for each network defined "
			"in the network configuration file (option '--net-config'). "
			"The report includes statistics on bandwidth utilization, "
			"network traffic, etc.)");

	// Static graph file
	command_line->RegisterString("--net-visual <file>", graph_file,
			"File for graphically representing the interconnection network. "
			"This file is an input for a supplementary tool called "
			"'graphplot' which is located in samples/network folder "
			"in multi2sim trunk.");

	// Dumping the route file
	command_line->RegisterString("--net-dump-routes <file>", route_file,
			"Files for representing the routing table of each individual "
			"network. The input is a string that consequently creates "
			"an individual file for each network.");
}



void System::ProcessOptions()
{
	// Network help
	if (help)
	{
		std::cerr << help_message;
		exit(1);
	}

	// Debugger
	if (!debug_file.empty())
		debug.setPath(debug_file);

	// Stand-Alone activation
	if (!sim_net_name.empty())
		stand_alone = true;

	// Stand-Alone requires config file
	if (stand_alone && config_file.empty())
		throw Error(misc::fmt("Option --net-sim requires "
				" --net-config option "));
}


void System::ReadConfiguration()
{
	// Load network configuration file
	if (!config_file.empty())
	{
		// Load and parse the configuration file
		misc::IniFile ini_file(config_file);
		ParseConfiguration(&ini_file);

		// Currently network trace info will be created only if
		// we have external configuration file. Here we activate the
		// trace, since we have a configuration file, but the trace
		// will be updated only if the traceSystem is active as well.
		trace.On();
		if ((trace) && (stand_alone))
			TraceHeader();
	}
}


void System::TraceHeader()
{
	// Update the trace header with network information
	trace.Header(misc::fmt("net.init version=\"%d.%d\"\n",
				trace_version_major, trace_version_minor));

	// Trace header for each network
	for (auto &network : networks)
		network->TraceHeader();
}


void System::UniformTrafficSimulation(Network *network)
{
	// Initiate a list of double for injection time
	auto inject_time = misc::new_unique_array<double>(network->getNumNodes());

	// Loop from the beginning to the end the simulation
	while (1)
	{
		// Get current cycle and check max cycles
		esim::Engine *esim_engine = esim::Engine::getInstance();
		long long cycle = System::getInstance()->getCycle();
		if (cycle >= max_cycles)
			break;

		// Traverse all nodes to check if some nodes need injection
		for (int i = 0; i < network->getNumNodes(); i++)
		{
			// Get end node
			EndNode *node = dynamic_cast<EndNode *>(
					network->getNode(i));
			if (!node)
				continue;

			// Check turn for next injection
			if (inject_time[i] > cycle)
				continue;

			// Get a random destination node
			EndNode *destination_node = nullptr;
			while (1)
			{
				int num_nodes = network->getNumNodes();
				int index = random() % num_nodes;
				destination_node = dynamic_cast<EndNode *>(
						network->getNode(index));
				if (destination_node && destination_node != 
						node)
					break;
			}

			// Inject
			while (inject_time[i] < cycle)
			{
				// Schedule next injection
				inject_time[i] += RandomExponential(
						injection_rate);

				// Send the packet
				if (network->CanSend(node, destination_node,
					System::message_size))
					network->Send(node, destination_node,
							System::message_size);
			}
		}

		// Next cycle
		debug << misc::fmt("___ cycle %lld ___\n", cycle);	
		esim_engine->ProcessEvents();
	}
}

void System::StandAlone()
{
	// Generate a uniform traffic pattern on the network.
	// Later, this function can be easily extended (via a switch)
	// to support other traffic patterns, hence the stand-alone
	// function is not combined with the uniform traffic function.
	Network *network = getNetworkByName(sim_net_name);
	if (!network)
		throw Error(misc::fmt("%s: The network does not exist for "
				"stand-alone simulation\n",
				config_file.c_str()));
	UniformTrafficSimulation(network);
}


void System::DumpReport()
{
	// Dump report files
	if (!report_file.empty())
	{
		for (auto &network : networks)
			network->DumpReport(report_file);
	}
}


void System::StaticGraph()
{
	// Dumping the graph file for each network
	if (!graph_file.empty())
	{
		for (auto &network : networks)
			network->StaticGraph(graph_file);
	}
}


void System::DumpRoutes()
{
	// Dump the route file for each network
	if (!route_file.empty())
	{
		for (auto &network: networks)
		{
			std::string net_path = network->getName() +
					"_" + route_file;
			network->getRoutingTable()->DumpRoutes(net_path);
		}
	}
}

}

