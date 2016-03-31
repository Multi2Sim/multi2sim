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

#include <lib/cpp/Misc.h>

#include "Network.h"

namespace net
{


const std::string System::help_message =
		"The network configuration file is a plain-text file following the\n"
		"IniFile format. It specifies a set of networks, their nodes, and\n"
		"connections between them. The following set of sections and\n"
		"variables are allowed:\n"
		"\n"
		"Section '[ General ]' contains configuration parameters affecting\n"
		"the whole network system.\n"
		"\n"
		"  Frequency = <value> (Default = 1000)\n"
		"      Frequency for the network system in MHz.\n"
		"\n"
		"Section '[ Network.<name> ]' defines a network. The string specified\n"
		" in <name> can be used in other configuration files to refer to\n"
		"this network.\n"
		"\n"
		"  DefaultInputBufferSize = <size> (Required)\n"
		"      Default size for input buffers in nodes and switches,\n"
		"      specified in number of packets. When a node/switch is\n"
		"      created in the network this size will be used if it is\n"
		"      not specified.\n"
		"  DefaultOutputBufferSize = <size> (Required)\n"
		"      Default size for output buffers in nodes and switches in\n"
		"      number or packets.\n"
		"  DefaultBandwidth = <bandwidth> (Required)\n"
		"      Default bandwidth for links in the network, specified in\n"
		"      number of bytes per cycle. If a link's bandwidth is\n"
		"      not specified, this value will be used.\n"
		"  DefaultPacketSize = <packet_size> (Optional)\n"
		"      If set, the messages in the networks are packetized to\n"
		"      smaller packets, and an individual packet is transfered\n"
		"      over the communication medium (link,bus) at each cycle.\n"
		"  Ideal = <true/false> (Optional)\n"
		"      If set to true, the network is an ideal network with a\n"
		"      single cycle latency between send and receive. The \n"
		"      network topology becomes ineffective.\n"
		"      The ideal option still requires a network to connect the\n"
		"      end-nodes to each other\n"
		"  FixLatency = <fix_latency> (Optional)\n"
		"      If set, the network transfers every message without\n"
		"      packetizing, with the fix_latency, regardless of\n"
		"      the network topology. The ideal option still requires a\n"
		"      network to connect the end-nodes to each other\n"
		"\n"
		"Sections '[ Network.<network>.Node.<node> ]' are used to \n"
		"define nodes in network '<network>'.\n"
		"\n"
		"  Type = {EndNode|Switch} (Required)\n"
		"      Type of node. End nodes can send and receive packets, while\n"
		"      switches are used to forward packets between other switches\n"
		"      and end nodes.\n"
		"  InputBufferSize = <size> (Default = <network>.DefaultInputBufferSize)\n"
		"      Size of input buffer in bytes.\n"
		"  OutputBufferSize = <size> (Default = <network>.DefaultOutputBufferSize)\n"
		"      Size of output buffer in bytes.\n"
		"  Bandwidth = <bandwidth> (Default = <network>.DefaultBandwidth)\n"
		"      For switches, bandwidth of internal crossbar communicating\n"
		"       input with output buffers. For end nodes, this variable\n"
		"       is ignored.\n"
		"\n"
		"Sections '[ Network.<network>.Link.<link> ]' are used to define \n"
		"links in network <network>. A link connects an output buffer of\n"
		"a source node with an input buffer of a destination node.\n"
		"\n"
		"  Source = <node> (Required)\n"
		"      Source node. The node name is a node declared in a\n"
		"      'Network.<network>.Node.<node>' section.\n"
		"  Dest = <node> (Required)\n"
		"      Destination node.\n"
		"  Type = {Unidirectional|Bidirectional} (Default = Unidirectional)\n"
		"      Link direction. Choosing a bidirectional link has the same\n"
		"      effect as creating two unidirectional links in opposite\n"
		"      directions.\n"
		"  Bandwidth = <bandwidth> (Default = <network>.DefaultBandwidth)\n"
		"      Bandwidth of the link in bytes per cycle.\n"
		"  VC = <virtual channels> (Default = 1)\n"
		"       Number of virtual channels a link can have.\n"
		"  InputBufferSize = <size> (Default = <network>.DefaultInputBufferSize)\n"
		"	Size of the link's input buffer(s) in bytes.\n"
		"  OutputBufferSize = <size> (Default = <network>/DefaultOutputBufferSize)\n"
		"	Size of the link's output buffer(s) in bytes.\n"
		"\n"
		"Section '[ Network.<network>.Bus.<bus> ]' are used to define\n"
		"a bus in network <network>. A bus connect multiple output buffers\n"
		"of multiple source nodes to an input buffer of a destination node.\n"
		"However, each different port to the bus should be identified\n"
		"separately in section '[ Network.<network>.BusPort.<port> ]'\n"
		"\n"
		"  Bandwidth = <bandwidth> (Default = <network>.DefaultBandwidth)\n"
		"	For buses, the bandwidth of the each lane of the bus is\n"
		"	identified in this field.\n"
		"  Lanes = <lanes> (Default = 1)\n"
		"	The number of lanes that each bus has. Using multiple lanes\n"
		"	allows source nodes to have multiple options to transmit to\n"
		"	the destination nodes, specially if other lanes are occupied\n" 
		"\n"
		"Section '[ Network.<network>.BusPort.<port> ]' are used to define\n"
		"a port between a node to a bus. The node can be sender, receiver,\n"
		"or both.\n"
		"\n" 
		"  Bus = <bus>\n"
		"	The bus that the port is created for. The Node will connect\n"
		"	to this bus through the port\n"
		"  Node = <node>\n"
		"	The Node that connects to the bus.\n"
		"  Type = <Bidirectional/Sender/Receiver> (Default= Bidirectional)\n"
		"	The connection from the node to the bus can be either\n"
		"	 bidirectional (creating two ports). Alternatively, the\n"
		"	node can be only a sender to the bus, or a receiver from it\n"
		"  BufferSize = <buffer_size>\n"
		"	(Default = <network>.DefaultInputBufferSize, if node is a receiver\n"
		"	and <network>.DefaultOutputBufferSize if node is a sender)\n"
		"	Sets the buffer that connects the node and bus together\n"
		"\n"
		"Section '[ Network.<network>.Routes ]' can be used (Optional)\n"
		"to define routes and manually configure the routing table.\n"
		"For a route between two end-nodes every route step from source to\n"
		"destination should be identified. Each unidirectional route step\n"
		"follows the pattern:\n"
		"  <node_A>.to.<node_C> = <node_B>:<Virtual Channel>\n"
		"  node_A. Source node of a route step \n"
		"  node_C. Destination node of a route step \n"
		"  node_B. Immediate next node that each packet must go\n"
		"      through to get from node_A to node_C\n"
		"  Virtual Channel. Is an optional field to choose a virtual channel\n"
		"   on the link between node_A and node_B. \n" "\n" "\n";

const char *System::err_config_note =
		"Please run 'm2s --net-help' or consult the Multi2Sim Guide for "
		"a description of the network system configuration file format.";


void System::ParseConfiguration(misc::IniFile *ini_file)
{
	// Debug
	System::debug << ini_file->getPath() << ": Loading network "
			"Configuration file\n";

	// Default Frequency
	std::string section = "General";
	frequency = ini_file->ReadInt(section, "Frequency", frequency);
	if (!esim::Engine::isValidFrequency(frequency))
		throw Error(misc::fmt("%s: The value for 'Frequency' "
				"must be between 1MHz and 1000GHz.\n%s",
				ini_file->getPath().c_str(),
				err_config_note));

	// Register frequency domain
	esim::Engine *esim = esim::Engine::getInstance();
	frequency_domain = esim->RegisterFrequencyDomain("network", 
			frequency);

	// First configuration look-up is for networks
	for (int i = 0; i < ini_file->getNumSections(); i++)
	{
		// Get section
		section = ini_file->getSection(i);

		// Split section name in tokens
		std::vector<std::string> tokens;
		misc::StringTokenize(section, tokens, ".");

		// Skip if it is not a network section
		if (tokens.size() != 2 || misc::StringCaseCompare(tokens[0],
				"Network"))
			continue;

		// Check that there is not a network with that name
		std::string name = tokens[1];
		if (network_map.find(name) != network_map.end())
			throw Error(misc::fmt("%s: %s: Duplicated network",
					ini_file->getPath().c_str(),
					name.c_str()));

		// Create network
		networks.emplace_back(misc::new_unique<Network>(name));
		Network *network = networks.back().get();
		network_map[name] = network;

		// Setup network by configuration
		network->ParseConfiguration(ini_file, section);
	}

	ini_file->Check();
}


}

