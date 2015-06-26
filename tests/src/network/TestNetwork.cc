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

	// Expected string
	std::string expected_str = misc::fmt(".*%s: The value for 'Frequency' "
			"must be between 1MHz and 1000GHz.\n.*",
			ini_file.getPath().c_str());

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
	EXPECT_DEATH({std::cerr << message.c_str(); exit(1);}, 
		expected_str.c_str());
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

	// Expected string
	std::string expected_str = misc::fmt("%s: test:\nDefault values can not be"
				" zero/non-existent.\n.*", ini_file.getPath().c_str());

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
	EXPECT_DEATH({std::cerr << message.c_str(); exit(1);}, 
		expected_str.c_str());
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

	// Expected string
	std::string expected_str = misc::fmt("%s: test:\nDefault values can not be"
				" zero/non-existent.\n.*", ini_file.getPath().c_str());

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
	EXPECT_DEATH({std::cerr << message.c_str(); exit(1);},
		expected_str.c_str());
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

	// Expected string
	std::string expected_str = misc::fmt("%s: test:\nDefault values can not be"
				" zero/non-existent.\n.*", ini_file.getPath().c_str());

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
	EXPECT_DEATH({std::cerr << message.c_str(); exit(1);},
		expected_str.c_str());
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

	// Expected string
	std::string expected_str = misc::fmt("%s: test:\nDefault values can not be"
				" negative.\n.*", ini_file.getPath().c_str());

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
	EXPECT_DEATH({std::cerr << message.c_str(); exit(1);},
		expected_str.c_str());
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

	// Expected string
	std::string expected_str = misc::fmt("%s: test:\nDefault values can not be"
				" negative.\n.*", ini_file.getPath().c_str());

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
	EXPECT_DEATH({std::cerr << message.c_str(); exit(1);},
		expected_str.c_str());
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

	// Expected string
	std::string expected_str = misc::fmt("%s: Node type 'anything' is not "
			"supported.\n.*",	ini_file.getPath().c_str());

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
	EXPECT_DEATH({std::cerr << message.c_str(); exit(1);},
		expected_str.c_str());
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
		"Bandwidth = -1\n"
		"InputBufferSize = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Expected string
	std::string expected_str = misc::fmt("%s: Invalid buffer size for Node "
						"'N1'\n.*",	ini_file.getPath().c_str());

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
	EXPECT_DEATH({std::cerr << message.c_str(); exit(1);},
		expected_str.c_str());
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
		"Bandwidth = -1\n"
		"InputBufferSize = 1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Expected string
	std::string expected_str = misc::fmt("%s: Invalid argument for Switch "
						"'S1'\n.*",	ini_file.getPath().c_str());

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
	EXPECT_DEATH({std::cerr << message.c_str(); exit(1);},
		expected_str.c_str());
}
}
