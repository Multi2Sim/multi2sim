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
#include <network/EndNode.h>
#include <network/RoutingTable.h>
#include <network/System.h>
#include <lib/cpp/Misc.h>
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

TEST(TestSystemConfiguration, section_general_frequency)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 2000000";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *network_system = System::getInstance();
	EXPECT_TRUE(network_system != nullptr);

	// Test body
	std::string message;
	try
	{
		network_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s: The value for 'Frequency' "
			"must be between 1MHz and 1000GHz.\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, section_network_two_defaults_missing)
{
	// cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 1000\n"
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 10";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: test:\nDefault values can not be"
					" zero/non-existent.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_network_missing_over_negative)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: test:\nDefault values can not be"
					" zero/non-existent.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_network_non_existant_default)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: test:\nDefault values can not be"
					" zero/non-existent.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_network_default_negative)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = -5\n"
			"DefaultBandwidth = 1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: test:\nDefault values can not be negative.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_network_negative_packet_size)
{
	// cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: test:\nDefault values can not be negative.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_node_unknown_fixlatency)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 1\n"
			"FixLatency = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_TRUE(system->getNetworkByName("test") != nullptr);
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Network test cannot have a negative "
					"fix latency",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}


TEST(TestSystemConfiguration, section_node_unknown_type)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 1\n"
			"\n"
			"[Network.test.Node.N1]\n"
			"Type = anything";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_TRUE(system->getNetworkByName("test") != nullptr);
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Node type 'anything' is not "
					"supported.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_section_unknown)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"\n"
			"[Network.test.anything]";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_TRUE(system->getNetworkByName("test") != nullptr);
	EXPECT_REGEX_MATCH(misc::fmt(
			"%s: Invalid section (.Network\\.test\\.anything.)",
			ini_file.getPath().c_str()).c_str(), message.c_str());

}

TEST(TestSystemConfiguration, section_node_no_preset_net)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[Network.test.Node.N1]\n"
			"Type = Switch\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	// FIXME
	// This should produce an error saying network not defined
	// but currently produces an error for each variable
	// no matter they are valid or not.
	EXPECT_TRUE(system->getNetworkByName("test") == nullptr);
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Section (.Network\\.test\\.Node\\.N1.), "
					"invalid variable 'Type'",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_node_buffer_size_vs_msg_size)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"InputBufferSize = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_TRUE(system->getNetworkByName("test") != nullptr);
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Invalid argument for Node 'N1'\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_node_switch_bandwidth)
{
	// cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"\n"
			"[Network.test.Node.S1]\n"
			"Type = Switch\n"
			"InputBufferSize = 1\n"
			"Bandwidth = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);


	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_FALSE(network->getNodeByName("S1") != nullptr);
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Invalid argument for Node 'S1'\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_type)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = anything";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Link type 'anything' is not supported.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_wrong_variable)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Bidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"anything = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Section (.Network\\.test\\.Link\\.S1-S2.), invalid"
					" variable 'anything'",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_wrong_source)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Bidirectional\n"
			"Source = anything\n"
			"Dest = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Source node 'anything' is invalid"
					" for link 'S1-S2'.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_no_source)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Bidirectional\n"
			"Dest = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Source node is not provided "
					"for link 'S1-S2'.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_wrong_destination)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Dest = anything\n"
			"Source = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Destination node 'anything' is "
					"invalid for link 'S1-S2'.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_no_destination)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Destination node is not provided "
					"for link 'S1-S2'.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_same_src_dst)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Link 'S1-S2', source "
					"and destination cannot be the same.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_bandwidth)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"Bandwidth = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Link 'S1-S2', bandwidth cannot "
					"be zero/negative.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_between_end_nodes)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("Network 'test': Link 'S1-S2' cannot connect "
					"two end-nodes.").c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, config_1_net_4_node_1_switch)
{
	// Cleanup singleton instance
	Cleanup();

	// Setting up the configuration file
	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 2\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n2 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n3 ]\n"
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
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n2-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n2\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n3-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n3\n"
			"Dest = s0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();


	// Parse the configuration file
	network_system->ParseConfiguration(&ini_file);

	// Assert the system
	EXPECT_TRUE(network_system != nullptr);

	// Assert the network
	Network * net0 = network_system->getNetworkByName("net0");
	EXPECT_TRUE(net0 != nullptr);

	// Assert the nodes
	EXPECT_TRUE(net0->getNodeByName("n0") != nullptr);
	EXPECT_TRUE(net0->getNodeByName("n1") != nullptr);
	EXPECT_TRUE(net0->getNodeByName("n2") != nullptr);
	EXPECT_TRUE(net0->getNodeByName("n3") != nullptr);
	EXPECT_TRUE(net0->getNodeByName("s0") != nullptr);
	EXPECT_TRUE(net0->getNumNodes() == 5);

	// Assert the links
	EXPECT_TRUE(net0->getConnectionByName("link_n0_s0") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_s0_n0") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_n1_s0") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_s0_n1") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_n2_s0") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_s0_n2") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_n3_s0") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_s0_n3") != nullptr);
	EXPECT_TRUE(net0->getNumConnections() == 8);
}

TEST(TestSystemConfiguration, section_bus_unknown_varialbe)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Bus.B0]\n"
			"anything = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Section (.Network\\.test\\.Bus\\.B0.), "
					"invalid variable 'anything'",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_bus_negative_bandwdith)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Bus.B0]\n"
			"Bandwidth = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Bus 'B0', bandwidth cannot be "
					"zero/negative.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_bus_negative_lanes)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Bus.B0]\n"
			"Bandwidth = 2\n"
			"Lanes = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Bus 'B0', number of lanes cannot be "
					"zero/negative.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_busport_no_bus)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N2]\n"
			"Type = Switch\n"
			"[Network.test.Busport.port0]";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("N1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("N2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Bus name should be included for "
					"the port.",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_busport_wrong_bus)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N2]\n"
			"Type = Switch\n"
			"[Network.test.Busport.port0]\n"
			"Bus = B0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("N1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("N2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Bus 'B0' does not exist in "
					"the network.",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_busport_no_node)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N2]\n"
			"Type = Switch\n"
			"[Network.test.Bus.B0]\n"
			"[Network.test.Busport.port0]\n"
			"Bus = B0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("N1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("N2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Section (.Network\\.test\\.Busport\\.port0.): "
					"Node is not set for the busport.",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_busport_wrong_node)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N2]\n"
			"Type = Switch\n"
			"[Network.test.Bus.B0]\n"
			"[Network.test.Busport.port0]\n"
			"Bus = B0\n"
			"Node = anything";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("N1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("N2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Section (.Network\\.test\\.Busport\\.port0.): "
					"Node 'anything' does not exist in the network.",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_busport_wrong_size)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N2]\n"
			"Type = Switch\n"
			"[Network.test.Bus.B0]\n"
			"[Network.test.Busport.port0]\n"
			"Bus = B0\n"
			"Node = N1\n"
			"BufferSize = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("N1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("N2") != nullptr);
	EXPECT_TRUE(network->getConnectionByName("B0") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Section (.Network\\.test\\.Busport\\.port0.): "
					"Buffer size cannot be less than 1.",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_busport_wrong_type)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N2]\n"
			"Type = Switch\n"
			"[Network.test.Bus.B0]\n"
			"[Network.test.Busport.port0]\n"
			"Type = anything\n"
			"Bus = B0\n"
			"Node = N1\n"
			"BufferSize = 2";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("N1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("N2") != nullptr);
	EXPECT_TRUE(network->getConnectionByName("B0") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Section (.Network\\.test\\.Busport\\.port0.): "
					"Type 'anything' is not recognized",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_bus_config)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N2]\n"
			"Type = Switch\n"
			"[Network.test.Bus.B0]\n"
			"[Network.test.Busport.port0]\n"
			"Bus = B0\n"
			"Node = N1\n"
			"BufferSize = 2";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	// Sanity checks
	Network *network = system->getNetworkByName("test");
	Node *N1 = network->getNodeByName("N1");
	Node *N2 = network->getNodeByName("N2");
	Bus *B0 = dynamic_cast<Bus *>(network->getConnectionByName("B0"));
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(N1 != nullptr);
	EXPECT_TRUE(N2 != nullptr);
	EXPECT_TRUE(B0 != nullptr);
	EXPECT_EQ(N1->getNumOutputBuffers(), 1);
	EXPECT_EQ(N1->getNumInputBuffers(), 1);
	EXPECT_EQ(B0->getNumSourceBuffers(), 1);
	EXPECT_EQ(B0->getNumDestinationBuffers(), 1);

}

TEST(TestSystemConfiguration, routes_wrong_source_node)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S0]\n"
			"Type = Switch\n"
			"[Network.test.Node.S1]\n"
			"Type = Switch\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.N0-S0]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S0\n"
			"[Network.test.Link.N0-S1]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S1\n"
			"[Network.test.Link.S0-S2]\n"
			"Type = Unidirectional\n"
			"Source = S0\n"
			"Dest = S2\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"[Network.test.Link.S2-N1]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = N1\n"
			"[Network.test.Routes]\n"
			"anything.to.N1 = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Section (.Network\\.test\\.Routes.), "
					"invalid variable 'anything.to.N1'",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, routes_wrong_destination_node)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S0]\n"
			"Type = Switch\n"
			"[Network.test.Node.S1]\n"
			"Type = Switch\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.N0-S0]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S0\n"
			"[Network.test.Link.N0-S1]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S1\n"
			"[Network.test.Link.S0-S2]\n"
			"Type = Unidirectional\n"
			"Source = S0\n"
			"Dest = S2\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"[Network.test.Link.S2-N1]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = N1\n"
			"[Network.test.Routes]\n"
			"N1.to.anything = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Section (.Network\\.test\\.Routes.), "
					"invalid variable 'N1.to.anything'",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, routes_to_switch)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S0]\n"
			"Type = Switch\n"
			"[Network.test.Node.S1]\n"
			"Type = Switch\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.N0-S0]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S0\n"
			"[Network.test.Link.N0-S1]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S1\n"
			"[Network.test.Link.S0-S2]\n"
			"Type = Unidirectional\n"
			"Source = S0\n"
			"Dest = S2\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"[Network.test.Link.S2-N1]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = N1\n"
			"[Network.test.Routes]\n"
			"S1.to.S2 = S2";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Section (.Network\\.test\\.Routes.), "
					"invalid variable 'S1.to.S2'",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, routes_same_src_dst)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S0]\n"
			"Type = Switch\n"
			"[Network.test.Node.S1]\n"
			"Type = Switch\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.N0-S0]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S0\n"
			"[Network.test.Link.N0-S1]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S1\n"
			"[Network.test.Link.S0-S2]\n"
			"Type = Unidirectional\n"
			"Source = S0\n"
			"Dest = S2\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"[Network.test.Link.S2-N1]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = N1\n"
			"[Network.test.Routes]\n"
			"N0.to.N0 = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH("Network test: route N0.to.N0: "
					"source and destination cannot be the same\n",
					message.c_str());
}

TEST(TestSystemConfiguration, routes_no_possible_route)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S0]\n"
			"Type = Switch\n"
			"[Network.test.Node.S1]\n"
			"Type = Switch\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.N0-S0]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S0\n"
			"[Network.test.Link.N0-S1]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S1\n"
			"[Network.test.Link.S0-S2]\n"
			"Type = Unidirectional\n"
			"Source = S0\n"
			"Dest = S2\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"[Network.test.Link.S2-N1]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = N1\n"
			"[Network.test.Routes]\n"
			"N0.to.N1 = N1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH("Network test: route N0.to.N1: "
					"missing connection\n",
					message.c_str());
}

TEST(TestSystemConfiguration, routes_wrong_virtual_channel)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S0]\n"
			"Type = Switch\n"
			"[Network.test.Node.S1]\n"
			"Type = Switch\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.N0-S0]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S0\n"
			"[Network.test.Link.N0-S1]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S1\n"
			"[Network.test.Link.S0-S2]\n"
			"Type = Unidirectional\n"
			"Source = S0\n"
			"Dest = S2\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"[Network.test.Link.S2-N1]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = N1\n"
			"[Network.test.Routes]\n"
			"N0.to.N1 = S1:1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH("Network test: route N0.to.N1: "
					"wrong virtual channel\n",
					message.c_str());
}

TEST(TestSystemConfiguration, routes_negative_virtual_channel)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S0]\n"
			"Type = Switch\n"
			"[Network.test.Node.S1]\n"
			"Type = Switch\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.N0-S0]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S0\n"
			"[Network.test.Link.N0-S1]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S1\n"
			"[Network.test.Link.S0-S2]\n"
			"Type = Unidirectional\n"
			"Source = S0\n"
			"Dest = S2\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"[Network.test.Link.S2-N1]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = N1\n"
			"[Network.test.Routes]\n"
			"N0.to.N1 = S1:0\n"
			"S1.to.N1 = S2:-1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH("Network test: route S1.to.N1: "
					"virtual channel cannot be negative\n",
					message.c_str());
}

TEST(TestSystemConfiguration, routes_wrong_format_for_next_node)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S0]\n"
			"Type = Switch\n"
			"[Network.test.Node.S1]\n"
			"Type = Switch\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.N0-S0]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S0\n"
			"[Network.test.Link.N0-S1]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S1\n"
			"[Network.test.Link.S0-S2]\n"
			"Type = Unidirectional\n"
			"Source = S0\n"
			"Dest = S2\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"[Network.test.Link.S2-N1]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = N1\n"
			"[Network.test.Routes]\n"
			"N0.to.N1 = S1:0\n"
			"S1.to.N1 = S2:1:0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH("Network test: route S1.to.N1: "
					"wrong format for next node\n",
					message.c_str());
}

TEST(TestSystemConfiguration, routes_wrong_next_node)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S0]\n"
			"Type = Switch\n"
			"[Network.test.Node.S1]\n"
			"Type = Switch\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.N0-S0]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S0\n"
			"[Network.test.Link.N0-S1]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S1\n"
			"[Network.test.Link.S0-S2]\n"
			"Type = Unidirectional\n"
			"Source = S0\n"
			"Dest = S2\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"[Network.test.Link.S2-N1]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = N1\n"
			"[Network.test.Routes]\n"
			"N0.to.N1 = S1:0\n"
			"S1.to.N1 = S3";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH("Network test: route S1.to.N1: "
					"invalid node name 'S3'\n",
					message.c_str());
}


///
/// Test topology
///
///  ----     ----     ----     ----     ---- 
///  |S0| ==> |S1| ==> |S2| --> |S3| --> |S4|
///  |  | <-- |  | <-- |  | <== |  | <== |  |
///  ----     ----     ----     ----     ----
///   ^        ^  ^     ^      ^  ^        ^
///   |        |   \    |     /   |        |
///   |        |    \   |    /    |        |
///   v        v      v v   v     v        v
///  ----     ----     ----     ----     ----
///  |N0|     |N1|     |N2|     |N3|     |N4|
///  ----     ----     ----     ----     ----
///
/// Distributing traffic to different partitions of the network, 
/// using manual routing and virtual channel 
TEST(TestSystemConfiguration, thorough_manual_routing)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.net0.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.N2]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.N3]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.N4]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.s0]\n"
			"Type = Switch\n"
			"[Network.net0.Node.s1]\n"
			"Type = Switch\n"
			"[Network.net0.Node.s2]\n"
			"Type = Switch\n"
			"[Network.net0.Node.s3]\n"
			"Type = Switch\n"
			"[Network.net0.Node.s4]\n"
			"Type = Switch\n"
			"[Network.net0.Link.N0-s0]\n"
			"Type = Bidirectional\n"
			"Source = N0\n"
			"Dest = s0\n"
			"[Network.net0.Link.N1-s1]\n"
			"Type = Bidirectional\n"
			"Source = N1\n"
			"Dest = s1\n"
			"[Network.net0.Link.N2-s2]\n"
			"Type = Bidirectional\n"
			"Source = N2\n"
			"Dest = s2\n"
			"[Network.net0.Link.N3-s3]\n"
			"Type = Bidirectional\n"
			"Source = N3\n"
			"Dest = s3\n"
			"[Network.net0.Link.N4-s4]\n"
			"Type = Bidirectional\n"
			"Source = N4\n"
			"Dest = s4\n"
			"[Network.net0.Link.s0-s1]\n"
			"Type = Unidirectional\n"
			"Source = s0\n"
			"Dest = s1\n"
			"VC = 2\n"
			"[Network.net0.Link.s1-s0]\n"
			"Type = Unidirectional\n"
			"Source = s1\n"
			"Dest = s0\n"
			"VC = 1\n"
			"[Network.net0.Link.s1-s2]\n"
			"Type = Unidirectional\n"
			"Source = s1\n"
			"Dest = s2\n"
			"VC = 2\n"
			"[Network.net0.Link.s2-s1]\n"
			"Type = Unidirectional\n"
			"Source = s2\n"
			"Dest = s1\n"
			"VC = 1\n"
			"[Network.net0.Link.s2-s3]\n"
			"Type = Unidirectional\n"
			"Source = s2\n"
			"Dest = s3\n"
			"VC = 1\n"
			"[Network.net0.Link.s3-s2]\n"
			"Type = Unidirectional\n"
			"Source = s3\n"
			"Dest = s2\n"
			"VC = 2\n"
			"[Network.net0.Link.s3-s4]\n"
			"Type = Unidirectional\n"
			"Source = s3\n"
			"Dest = s4\n"
			"VC = 1\n"
			"[Network.net0.Link.s4-s3]\n"
			"Type = Unidirectional\n"
			"Source = s4\n"
			"Dest = s3\n"
			"VC = 2\n"
			"[Network.net0.Link.s2-N1]\n"
			"Type = Bidirectional\n"
			"Source = s2\n"
			"Dest = N1\n"
			"VC = 1\n"
			"[Network.net0.Link.s2-N3]\n"
			"Type = Bidirectional\n"
			"Source = s2\n"
			"Dest = N3\n"
			"VC = 1\n"
			"[Network.net0.Routes]\n"
			"N0.to.N1 = s0\n"
			"s0.to.N1 = s1:0\n"
			"N0.to.N2 = s0\n"
			"s0.to.N2 = s1:0\n"
			"s1.to.N2 = s2:0\n"
			"N0.to.N3 = s0\n"
			"s0.to.N3 = s1:1\n"
			"s1.to.N3 = s2:1\n"
			"s2.to.N3 = s3\n"
			"N0.to.N4 = s0\n"
			"s0.to.N4 = s1:1\n"
			"s1.to.N4 = s2:1\n"
			"s2.to.N4 = s3\n"
			"s3.to.N4 = s4\n"
			"N1.to.N0 = s1\n"
			"s1.to.N0 = s0\n"
			"N1.to.N2 = s1\n"
			"N1.to.N3 = s1\n"
			"N1.to.N4 = s1\n"
			"N2.to.N0 = s2\n"
			"s2.to.N0 = s1\n"
			"N2.to.N1 = s2\n"
			"s2.to.N1 = s1\n"
			"N2.to.N3 = s2\n"
			"N2.to.N4 = s2\n"
			"N3.to.N0 = s3\n"
			"s3.to.N0 = s2:1\n"
			"N3.to.N1 = s3\n"
			"s3.to.N1 = s2:1\n"
			"N3.to.N2 = s3\n"
			"s3.to.N2 = s2:0\n"
			"N3.to.N4 = s3\n"
			"N4.to.N0 = s4\n"
			"s4.to.N0 = s3:1\n"
			"N4.to.N1 = s4\n"
			"s4.to.N1 = s3:1\n"
			"N4.to.N2 = s4\n"
			"s4.to.N2 = s3:0\n"
			"N4.to.N3 = s4\n"
			"s4.to.N3 = s3:0\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	try
	{
		// Parse the configuration file
		system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the routing table
		RoutingTable *table = network->getRoutingTable();
	
		// Getting all the nodes
		Node *N0 = network->getNodeByName("N0");
		Node *N1 = network->getNodeByName("N1");
		Node *N2 = network->getNodeByName("N2");
		Node *N3 = network->getNodeByName("N3");
		Node *N4 = network->getNodeByName("N4");
		Node *S0 = network->getNodeByName("s0");
		Node *S1 = network->getNodeByName("s1");
		Node *S2 = network->getNodeByName("s2");
		Node *S3 = network->getNodeByName("s3");
		Node *S4 = network->getNodeByName("s4");

		// Checking the table one entry at a time -- N0 to N1
		RoutingTable::Entry *entry = table->RoutingTable::Lookup(N0, N1);
		EXPECT_EQ(entry->cost, 3);
		EXPECT_EQ(entry->getNextNode(), S0);
		entry = table->RoutingTable::Lookup(S0, N1);
		EXPECT_EQ(entry->cost, 2);
		EXPECT_EQ(entry->getNextNode(), S1);
		Connection *connection = entry->getBuffer()->getConnection();
		EXPECT_EQ(entry->getBuffer(), connection->getSourceBuffer(0));
		entry = table->RoutingTable::Lookup(S1, N1);
		EXPECT_EQ(entry->cost, 1);
		EXPECT_EQ(entry->getNextNode(), N1);

		// Checking the table one entry at a time -- N1 to N4
		entry = table->RoutingTable::Lookup(N1, N4);
		EXPECT_EQ(entry->cost, 5);
		EXPECT_EQ(entry->getNextNode(), S1);
		entry = table->RoutingTable::Lookup(S1, N4);
		EXPECT_EQ(entry->cost, 4);
		EXPECT_EQ(entry->getNextNode(), S2);
		connection = entry->getBuffer()->getConnection();
		EXPECT_EQ(entry->getBuffer(), connection->getSourceBuffer(1));
		entry = table->RoutingTable::Lookup(S2, N4);
		EXPECT_EQ(entry->cost, 3);
		EXPECT_EQ(entry->getNextNode(), S3);
		entry = table->RoutingTable::Lookup(S3, N4);
		EXPECT_EQ(entry->cost, 2);
		EXPECT_EQ(entry->getNextNode(), S4);
		entry = table->RoutingTable::Lookup(S4, N4);
		EXPECT_EQ(entry->cost, 1);
		EXPECT_EQ(entry->getNextNode(), N4);

		// Checking the table on entry at a time -- N3 to N2
		entry = table->RoutingTable::Lookup(N3, N2);	
		EXPECT_EQ(entry->cost, 3);
		EXPECT_EQ(entry->getNextNode(), S3);
		entry = table->RoutingTable::Lookup(S3, N2);
		EXPECT_EQ(entry->cost, 2);
		EXPECT_EQ(entry->getNextNode(), S2);
		connection = entry->getBuffer()->getConnection();
		EXPECT_EQ(entry->getBuffer(), connection->getSourceBuffer(0));
		entry = table->RoutingTable::Lookup(S2, N2);
		EXPECT_EQ(entry->cost, 1);
		EXPECT_EQ(entry->getNextNode(), N2);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

}
