/*
 *  Multi2Sim
 *  Copyright (C) 2015 David English (english.d@husky.neu.edu)
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
#include <dram/Controller.h>
#include <dram/System.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/Error.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Trace.h>

namespace dram 
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

	// Set up dram instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*The value for 'Frequency' "
			"must be between 1MHz and 1000GHz.\n.*").c_str(),
			message.c_str());
}


TEST(TestSystemConfiguration, section_no_controller_name)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 20000\n"
			"[MemoryController]\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s: MemoryController must have a name "
			"in the form 'MemoryController <name>'.\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());

}

TEST(TestSystemConfiguration, section_negative_channels)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 20000\n"
			"[MemoryController One]\n"
			"NumChannels = -1\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s: NumChannels must be at least 1.\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, section_negative_ranks)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 20000\n"
			"[MemoryController One]\n"
			"NumRanks = -1\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s: NumRanks must be at least 1.\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, section_negative_banks)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 20000\n"
			"[MemoryController One]\n"
			"NumBanks = -1\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s: NumBanks must be at least 1.\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, section_negative_Rows)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 20000\n"
			"[MemoryController One]\n"
			"NumRows = -1\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s: NumRows must be at least 1.\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, section_negative_Columns)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 20000\n"
			"[MemoryController One]\n"
			"NumColumns = -1\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s: NumColumns must be at least 1.\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, section_negative_bits)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 20000\n"
			"[MemoryController One]\n"
			"NumBits = -1\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s: NumBits must be at least 1.\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, section_invalid_format)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 20000\n"
			"[MemoryController One]\n"
			"NumChannels = 'String'\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s:.*invalid format\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, section_invalid_vaiable)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 20000\n"
			"[MemoryController One]\n"
			"String = 4\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s:.*invalid variable.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, section_correct_timings_DDR3_1600)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 20000\n"
			"[MemoryController One]\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(&ini_file);
		Controller *controller = dram_system->getController(0);
		
		// Check Precharge timing
		EXPECT_TRUE(controller->
				getCommandDuration(CommandPrecharge) == 11);

		// Check Activate timing
		EXPECT_TRUE(controller->
				getCommandDuration(CommandActivate) == 11);

		// Check Read timing
		EXPECT_TRUE(controller->
				getCommandDuration(CommandRead) == 15);

		// Check Write timing
		EXPECT_TRUE(controller->
				getCommandDuration(CommandWrite) == 15);
	}
	catch (misc::Error &error)
	{
		error.Dump();
		FAIL();
	}
}

}
