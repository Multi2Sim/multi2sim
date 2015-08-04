/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#include <gtest/gtest.h>

#include <arch/southern-islands/timing/Timing.h>
#include <lib/cpp/IniFile.h>
#include <lib/esim/Engine.h>

namespace SI 
{

static void Cleanup()
{
        esim::Engine::Destroy();
        Timing::Destroy();
        comm::ArchPool::Destroy();
}


// This test checks to see if the correct error message is returned when
// a frequency is passed that is larger than the acceptable bounds
TEST(TestTiming, config_section_device_frequency_0)
{
	// Cleanup singleton instances
	Cleanup();

	// Create config file
	std::string config =
			"[ Device ]\n"
			"Frequency = 10000000";
	
	// Load config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Try ParseConfiguration for invalid frequency
	std::string message;
	try
	{
		Timing::ParseConfiguration(&ini_file);
	}
	catch(misc::Error &error)
	{
		message = error.getMessage();
	}

	// Check error message
	EXPECT_REGEX_MATCH(misc::fmt(".*%s: The value for 'Frequency' "
			"must be between 1MHz and 1000GHz.\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}


// This test checks to see if the correct error message is returned when
// a frequency is passed that is smaller than the acceptable bounds
TEST(TestTiming, config_section_device_frequency_1)
{
	// Cleanup singleton instances
	Cleanup();

	// Try frequency lower than bounds
	std::string config =
		"[ Device ]\n"
		"Frequency = 0";

	// Load config file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Try ParseConfiguration for invalid frequency
	std::string message;
	try
	{
		Timing::ParseConfiguration(&ini_file);
	}
	catch(misc::Error &error)
	{
		message = error.getMessage();
	}

	// Check error message
	EXPECT_REGEX_MATCH(misc::fmt(".*%s: The value for 'Frequency' "
			"must be between 1MHz and 1000GHz.\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}


} // namespace SI
