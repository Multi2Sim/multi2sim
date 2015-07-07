/*
 *  Multi2Sim
 *  Copyright (C) 2014  Shi Dong (dong.sh@husky.neu.edu)
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

#include <lib/cpp/IniFile.h>
#include <lib/cpp/Error.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Trace.h>
#include <memory/System.h>

namespace mem
{

static void Cleanup()
{
	esim::Engine::Destroy();

	net::System::Destroy();

	System::Destroy();
}

TEST(TestSystemConfiguration, section_general_frequency)
{
	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 2000000";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s: The value for 'Frequency' "
			"must be between 1MHz and 1000GHz.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_type)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Anything";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s: .*: invalid or missing "
			"value for 'Type'.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_cache_replacement_policy)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"Policy = anything";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: Cache test: anything: "
			"Invalid block replacement policy.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_cache_write_policy)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"WritePolicy = anything";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: Cache test: anything: "
			"Invalid write policy.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_cache_num_sets_1)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"Sets = 0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: cache test: number of sets must be a "
			"power of two greater than 1.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_cache_num_sets_2)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"Sets = 13";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: cache test: number of sets must be a "
			"power of two greater than 1.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_cache_num_ways_1)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"Assoc = 0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: cache test: associativity must be a "
			"power of two and > 1.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_cache_num_ways_2)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"Assoc = 13";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: cache test: associativity must be a "
			"power of two and > 1.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_cache_block_size_1)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"BlockSize = 3";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: cache test: block size must be power "
			"of two and at least 4.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_cache_block_size_2)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"BlockSize = 13";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: cache test: block size must be power "
			"of two and at least 4.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_cache_directory_latency)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"DirectoryLatency = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: cache test: invalid value for "
			"variable 'DirectoryLatency'.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_cache_latency)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"Latency = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: cache test: invalid value for "
			"variable 'Latency'.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_mshr_size)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"MSHR = 0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: cache test: invalid value for "
			"variable 'MSHR'.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}


TEST(TestSystemConfiguration, section_module_num_ports)
{
	// Cleanup singleton instances
	Cleanup();

	// Setup configuration file
	std::string config =
		"[ General ]\n"
		"Frequency = 1000\n"
		"[ Module test ]\n"
		"Type = Cache\n"
		"Geometry = cacheTest\n"
		"[ CacheGeometry cacheTest ]\n"
		"Ports = 0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up memory system instance
	System *memory_system = System::getInstance();

	// Test body
	std::string actual_str;
	try
	{
		memory_system->ReadConfiguration(&ini_file);
	}
	catch (misc::Error &actual_error)
	{
		actual_str = actual_error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt("%s: cache test: invalid value for "
			"variable 'Ports'.\n.*",
			ini_file.getPath().c_str()).c_str(),
			actual_str.c_str());
}

}

