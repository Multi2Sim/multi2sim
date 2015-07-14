/*
 *  Multi2Sim
 *  Copyright (C) 2014  Spencer Hance (hance.s@husky.neu.edu)
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

#include <arch/x86/timing/Timing.h>
#include <arch/common/Arch.h>
#include <lib/cpp/IniFile.h>
#include <lib/cpp/Error.h>
#include <lib/esim/Engine.h>
#include <memory/System.h>
#include <memory/Frame.h>
#include <memory/Module.h>
#include <network/System.h>

namespace mem
{

const std::string mem_config_0 =
		"; 1 mm\n"
		"\n"
		"[Module mod-mm]\n"
		"Type = MainMemory\n"
		"BlockSize = 128\n"
		"Latency = 200\n"
		"\n"
		"\n"
		"[Entry core-0]\n"
		"Arch = x86\n"
		"Core = 0\n"
		"Thread = 0\n"
		"DataModule = mod-mm\n"
		"InstModule = mod-mm";

const std::string x86_config_0 =
		"[ General ]\n"
		"Cores = 1\n"
		"Threads = 1\n";

static void Cleanup()
{
	esim::Engine::Destroy();

	net::System::Destroy();

	System::Destroy();

	x86::Timing::Destroy();

	comm::ArchPool::Destroy();
}

// This test checks the isInFlightAccess() function.
// The test schedules an access and then checks that the function
// returns the right values throughout the execution.
TEST(TestModule, is_in_flight_access)
{
	try
	{
		// Clean up singleton instances
		Cleanup();
		
		// Load configuration file
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config_0);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get Module
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_mm, nullptr);

		// Set up access
		int address = 0x400;
		module_mm->Access(Module::AccessLoad, address);
		EXPECT_FALSE(module_mm->isInFlightAccess(address));

		// Simulation loop for 200 cycles
		esim::Engine *esim_engine = esim::Engine::getInstance();
		for (int i = 0; i < 201; i++)
			esim_engine->ProcessEvents();
			EXPECT_TRUE(module_mm->isInFlightAccess(address));

		// Check that the access finishes after 1 more cycle
		esim_engine->ProcessEvents();
		EXPECT_FALSE(module_mm->isInFlightAccess(address));
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}

}

