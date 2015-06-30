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

TEST(TestSystemConfiguration, section_general_frequency)
{
	// cleanup singleton instance
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
	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: test:\nDefault values can not be"
						" zero/non-existent.\n.*",
						ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_network_missing_over_negative)
{
	// cleanup singleton instance
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
	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: test:\nDefault values can not be"
						" zero/non-existent.\n.*",
						ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_network_non_existant_default)
{
	// cleanup singleton instance
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
	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: test:\nDefault values can not be"
						" zero/non-existent.\n.*",
						ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_network_default_negative)
{
	// cleanup singleton instance
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
	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: test:\nDefault values can not be negative.\n.*",
						ini_file.getPath().c_str()))));
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
	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: test:\nDefault values can not be negative.\n.*",
						ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_node_unknown_type)
{
	// cleanup singleton instance
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
	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: Node type 'anything' is not "
						"supported.\n.*",
						ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_section_unknown)
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
	// FIXME
	// regular Bracket in regex. \[ and \\[ result in regex_error
	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: invalid section (.Network\\.test\\.anything.)",
						ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_section_no_net)
{
	// cleanup singleton instance
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
	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: section (.Network\\.test\\.Node\\.N1.), "
					"invalid variable 'Type'",
						ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_node_buffer_size_vs_msg_size)
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
	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: Invalid argument for Node 'N1'\n.*",
						ini_file.getPath().c_str()))));
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

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_FALSE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: Invalid argument for Node 'S1'\n.*",
						ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_link_type)
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

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: Link type 'anything' is not supported.\n.*",
					ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_link_wrong_variable)
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
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
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

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: section (.Network\\.test\\.Link\\.S1-S2.), invalid"
					" variable 'anything'",
								ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_link_wrong_source)
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

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: Source node 'anything' is invalid"
								" for link 'S1-S2'.\n",
								ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_link_no_source)
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

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: Source node is not provided "
								"for link 'S1-S2'.\n",
								ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_link_wrong_destination)
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

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: Destination node 'anything' is "
						"invalid for link 'S1-S2'.\n",
						ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_link_no_destination)
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

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: Destination node is not provided "
								"for link 'S1-S2'.\n",
								ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_link_same_src_dst)
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

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: Link 'S1-S2', source "
					"and destination cannot be the same.\n",
					ini_file.getPath().c_str()))));
}

TEST(TestSystemConfiguration, section_link_bandwidth)
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
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
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

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_TRUE(std::regex_match(message, std::regex(
			misc::fmt("%s: Link 'S1-S2', bandwidth cannot "
						"be zero/negative.\n",
					ini_file.getPath().c_str()))));
}
}
