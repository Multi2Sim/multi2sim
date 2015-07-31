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
#include <dram/System.h>
#include <lib/cpp/IniFile.h>
#include <lib/cpp/Error.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Trace.h>

namespace dram 
{

static void Cleanup()
{
}

TEST(TestSystemConfiguration, section_general_frequency)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 2000000";

	// Set up network instance
	System *dram_system = System::getInstance();
	EXPECT_TRUE(dram_system != nullptr);

	// Test body
	std::string message;
	try
	{
		dram_system->ParseConfiguration(config);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(".*: The value for 'Frequency' "
			"must be between 1MHz and 1000GHz.\n.*",	
			message.c_str());
}

}
