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

misc::Debug System::debug;

esim::Trace System::trace;

std::string System::sim_net_name;

long long System::max_cycles = 1000000;

int System::message_size = 1;

double System::injection_rate = 0.001;

bool System::stand_alone = false;

bool System::help = false;

int System::system_frequency = 1000;

std::unique_ptr<System> System::instance;


System *System::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new System);
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
}


void System::ProcessOptions()
{
	// Get the system
	System *net_system = System::getInstance();

	// Debugger
	if (help)
	{
		std::cerr << help_message;
		exit(1);
	}

	if (!debug_file.empty())
		debug.setPath(debug_file);

	// Stand-Alone activation
	if (!sim_net_name.empty())
		net_system->stand_alone = true;

	// Configuration
	if (!config_file.empty())
	{
		misc::IniFile ini_file(config_file);
		net_system->ParseConfiguration(&ini_file);
	}

	// Stand-Alone requires config file
	if (net_system->stand_alone)
	{
		// Check config file
		if (config_file.empty())
			throw Error(misc::fmt("Option --net-sim requires "
					" --net-config option "));

		System *system = getInstance();
		UniformTrafficSimulation(
				system->getNetworkByName(sim_net_name));
	}
}


void System::UniformTrafficSimulation(Network *network)
{
	// Init a list of double for injection time
	auto inject_time = misc::new_unique_array<double>(
			network->getNumNodes());
	for (int i = 0; i < network->getNumNodes(); i++)
	{
		inject_time[i] = 0.0f;
	}

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
			EndNode * dst_node = nullptr;
			while (1)
			{
				int num_nodes = network->getNumNodes();
				int index = random() % num_nodes;
				dst_node = dynamic_cast<EndNode *>(
						network->getNode(index));
				if (!dst_node || dst_node != node)
					break;
			}

			// Inject
			while (inject_time[i] < cycle)
			{
				// Dump debug information
				debug << misc::fmt("[Network] [cycle %lld] "
						"Injecting a message from node "
						"%s to node %s.\n",
						cycle,
						node->getName().c_str(),
						dst_node->getName().c_str());

				// Schedule next injection
				inject_time[i] += RandomExponential(
						injection_rate);
				debug << misc::fmt("[Network] [cycle %lld] "
						"[node %s] next injection time "
						"%f\n",
						cycle, node->getName().c_str(),
						inject_time[i]);

				// Send the packet
				if (network->CanSend(node, dst_node,
					System::message_size))
					network->Send(node, dst_node,
							System::message_size);
			}
		}

		// Next cycle
		esim_engine->ProcessEvents();
	}
}

}

