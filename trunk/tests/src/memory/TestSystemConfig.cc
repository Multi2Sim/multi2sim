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

#include <lib/cpp/IniFile.h>
#include <lib/cpp/Error.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Trace.h>
#include <memory/System.h>

namespace mem
{

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

	// Expected string
	std::string expected_str = misc::fmt(".*%s: The value for 'Frequency' "
			"must be between 1MHz and 1000GHz.\n.*",
			ini_file.getPath().c_str());

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
	EXPECT_DEATH({std::cerr << actual_str.c_str(); exit(1);}, expected_str.c_str());
}


TEST(TestSystemConfiguration, section_module_type)
{
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

	// Expected string
	std::string expected_str = misc::fmt(".*%s: .*: invalid or missing "
			"value for 'Type'.\n.*",
			ini_file.getPath().c_str());

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
	EXPECT_DEATH({std::cerr << actual_str.c_str(); exit(1);}, expected_str.c_str());
}

}
