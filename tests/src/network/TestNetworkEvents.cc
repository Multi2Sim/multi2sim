/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include "gtest/gtest.h"

#include <string>
#include <regex>
#include <exception>
#include <network/System.h>
#include <network/EndNode.h>
#include <lib/cpp/IniFile.h>
#include <lib/cpp/Error.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Trace.h>

namespace net
{

static void Cleanup()
{
	esim::Engine::Destroy();

	System::Destroy();
}

TEST(TestSystemConfiguration, event_0_same_src_dest_not_allowed)
{
	// cleanup singleton instance
	Cleanup();

	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the node
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		
		// Sending from the node to itself
		network->TrySend(src, src, 1);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Source and destination cannot "
			"be the same\\.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_0_no_route)
{
	// cleanup singleton instance
	Cleanup();

	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the source node
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));

		// Getting the destination node
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Sending from the node to itself
		network->TrySend(src, dst, 8);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("No route from 'n0' to 'n1'").c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, event_0_message_too_big)
{
	// cleanup singleton instance
	Cleanup();

	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.s0 ]\n"
			"Type = Switch\n"
			"\n"
			"[ Network.net0.Link.n0-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n0\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n1-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n1\n"
			"Dest = s0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the source node
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));

		// Getting the destination node
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Sending from the node to itself
		network->TrySend(src, dst, 8);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Buffer too small for the "
			"message size\\.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_0_packetized_message_too_big)
{
	// cleanup singleton instance
	Cleanup();

	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 3\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.s0 ]\n"
			"Type = Switch\n"
			"\n"
			"[ Network.net0.Link.n0-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n0\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n1-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n1\n"
			"Dest = s0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the source node
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));

		// Getting the destination node
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Sending from the node to itself
		network->TrySend(src, dst, 4);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Buffer too small for the "
			"message size\\.").c_str(), message.c_str());
}

}
