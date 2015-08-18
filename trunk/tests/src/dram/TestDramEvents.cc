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
	esim::Engine::Destroy();

	System::Destroy();
}

static std:string default_config = 
			"[ General ]\n"
			"Frequency = 100000\n"
			"[ MemoryController One ]\n";
			
TEST(TestSystemConfiguration, event_one_read)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		System::Read();
		esim::Engine *engine = esim::Engine::getInstance();
		engine->ProcessEvents();
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_TRUE(message.empty());
}

TEST(TestSystemConfiguration, event_one_write)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		System::Write();
		esim::Engine *engine = esim::Engine::getInstance();
		engine->ProcessEvents();
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_TRUE(message.empty());
}

}
