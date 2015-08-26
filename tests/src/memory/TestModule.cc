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

const std::string mem_config_1 =
                "; 1 mm\n"
		"\n"
                "[CacheGeometry geo-l1]\n"
                "Sets = 128\n"
                "Assoc = 2\n"
                "BlockSize = 256\n"
                "Latency = 5\n"
		"DirectoryLatency = 5\n"
		"MSHR = 2\n"
                "Policy = LRU\n"
                "Ports = 2\n"
                "; 1 l1\n"
                "[Module mod-l1-0]\n"
                "Type = Cache\n"
                "Geometry = geo-l1\n"
                "LowNetwork = l1-mm\n"
                "LowModules = mod-mm\n"
		"\n"
                "[Module mod-mm]\n"
		"Type = MainMemory\n"
		"BlockSize = 256\n"
		"Latency = 200\n"
                "HighNetwork = l1-mm\n"
                "\n"
		"[Entry core-0]\n"
		"Arch = x86\n"
		"Core = 0\n"
		"Thread = 0\n"
		"DataModule = mod-l1-0\n"
		"InstModule = mod-l1-0\n"
                "\n"
                "[Network l1-mm]\n"
                "DefaultInputBufferSize = 1024\n" 
                "DefaultOutputBufferSize = 1024\n"
                "DefaultBandwidth = 256"; 


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

// This test checks the isInFlightAddress() function.
// The test schedules an access and then checks that the function
// returns true until the access has completed
//
// The latency of the MM module is 200 cycles so that is how long
// it takes for the access to complete.
TEST(TestModule, is_in_flight_address)
{
	try
	{
                // Cleanup singleton instances
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

		// Set up block
		module_mm->getCache()->getBlock(4, 15)->setStateTag(Cache::BlockExclusive, 0x400);

		// Set up access
		module_mm->Access(Module::AccessLoad, 0x400);
		EXPECT_FALSE(module_mm->isInFlightAddress(0x400));

		// Simulation loop for 200 cycles
		esim::Engine *esim_engine = esim::Engine::getInstance();
		for (int i = 0; i < 201; i++)
		{
			esim_engine->ProcessEvents();
			EXPECT_TRUE(module_mm->isInFlightAddress(0x400));
		}

		// Check that the access finishes after 1 more cycle
		esim_engine->ProcessEvents();
		EXPECT_FALSE(module_mm->isInFlightAddress(0x400));
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// This test checks the isInFlightAccess() function. The test schedules an
// access and then checks that the function returns true until the access has
// completed.
//
// The latency of the MM module is 200 cycles so that is how long
// it takes for the access to complete.
TEST(TestModule, is_in_flight_access)
{
	try
	{
                // Cleanup singleton instances
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

		// Set up block
		module_mm->getCache()->getBlock(4, 15)->setStateTag(Cache::BlockExclusive, 0x400);

		// Set up access
		long long id = module_mm->Access(Module::AccessLoad, 0x400);
		EXPECT_FALSE(module_mm->isInFlightAccess(id));

		// Simulation loop for 200 cycles
		esim::Engine *esim_engine = esim::Engine::getInstance();
		for (int i = 0; i < 201; i++)
		{
			esim_engine->ProcessEvents();
			EXPECT_TRUE(module_mm->isInFlightAccess(id));
		}

		// Check that the access finishes after 1 more cycle
		esim_engine->ProcessEvents();
		EXPECT_FALSE(module_mm->isInFlightAccess(id));
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// Tests a situation where all module ports are locked, and canAccess()
// returns false. As soon as one port is ready, canAccess() returns true.
// There are no cache misses so the only relevant latency is the
// directory latency.
TEST(TestModule, can_access_0)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration file
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		ini_file_mem.LoadFromString(mem_config_1);
		ini_file_x86.LoadFromString(x86_config_0);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get Modules
		Module *module_mm = memory_system->getModule("mod-mm");
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		ASSERT_NE(module_mm, nullptr);
		ASSERT_NE(module_l1_0, nullptr);

		// Set up blocks
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l1_0->getCache()->getBlock(4, 1)->setStateTag(Cache::BlockExclusive, 0x400);
		module_l1_0->getCache()->getBlock(6, 1)->setStateTag(Cache::BlockExclusive, 0x600);
		module_l1_0->getCache()->getBlock(8, 1)->setStateTag(Cache::BlockExclusive, 0x800);

		// Set up accesses
		module_l1_0->Access(Module::AccessLoad, 0x400);
                module_l1_0->Access(Module::AccessLoad, 0x600);
                module_l1_0->Access(Module::AccessLoad, 0x800);

		// Simulation loop
		EXPECT_TRUE(module_l1_0->canAccess(0x400));
		esim::Engine *esim_engine = esim::Engine::getInstance();

		// Run simulation for 10 cycles, CanAccess() should return false
		// since the directory latency is 5 cycles and there are two
		// accesses
		for (int i = 0; i < 10; i++)
		{
			esim_engine->ProcessEvents();
			EXPECT_FALSE(module_l1_0->canAccess(0x0));
		}

		// Finish accesses and check that the ports have been freed
		for (int i = 0; i < 6; i++)
		{
			esim_engine->ProcessEvents();
			EXPECT_TRUE(module_l1_0->canAccess(0x0));
		}
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// Tests a situation where there are enough ports, but the size of the MSHR
// (miss status holding register, which keeps track of in-flight cache misses)
// does not allow for new in-flight accesses, and canAccess() returns false.
// It returns true as soon as one access completes.
//
// The test uses 3 accesses which are all cache misses.  Since the MSHR is
// configured here to hold 2 misses, canAccess() will return false until one of
// them completes.
TEST(TestModule, can_access_1)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration file
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		ini_file_mem.LoadFromString(mem_config_1);
		ini_file_x86.LoadFromString(x86_config_0);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get Modules
		Module *module_mm = memory_system->getModule("mod-mm");
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		ASSERT_NE(module_mm, nullptr);
		ASSERT_NE(module_l1_0, nullptr);

		// Set up blocks
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(4, 15)->setStateTag(Cache::BlockExclusive, 0x400);
		module_mm->getCache()->getBlock(6, 15)->setStateTag(Cache::BlockExclusive, 0x600);
		module_mm->getCache()->getBlock(8, 15)->setStateTag(Cache::BlockExclusive, 0x800);

		// Set up accesses
		module_l1_0->Access(Module::AccessLoad, 0x400);
                module_l1_0->Access(Module::AccessLoad, 0x600);
                module_l1_0->Access(Module::AccessLoad, 0x800);

		// Simulation loop
		EXPECT_TRUE(module_l1_0->canAccess(0x0));
		esim::Engine *esim_engine = esim::Engine::getInstance();

		// Run simulation for 224 cycles.  CanAccess() should return false
		// since none of the accesses should have completed.
		for (int i = 0; i < 224; i++)
		{
			esim_engine->ProcessEvents();
			EXPECT_FALSE(module_l1_0->canAccess(0x0));
		}

		// Run simulation for 1 more cycle, CanAccess() should return true since
		// the first access has completed
		esim_engine->ProcessEvents();
		EXPECT_TRUE(module_l1_0->canAccess(0x0));

	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// A situation where there are two coalesced accesses, which are not
// considered toward the MSHR size limit.  canAccess() returns true even
// though the MSHR would be full.  This is because the
// two accesses to module_l1_0 0x400 are coalesced.
TEST(TestModule, can_access_2)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration file
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		ini_file_mem.LoadFromString(mem_config_1);
		ini_file_x86.LoadFromString(x86_config_0);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get Modules
		Module *module_mm = memory_system->getModule("mod-mm");
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		ASSERT_NE(module_mm, nullptr);
		ASSERT_NE(module_l1_0, nullptr);

		// Set up blocks
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(4, 15)->setStateTag(Cache::BlockExclusive, 0x400);

		// Set up accesses
		module_l1_0->Access(Module::AccessLoad, 0x400);
                module_l1_0->Access(Module::AccessLoad, 0x400);

		// Get esim engine
		esim::Engine *esim_engine = esim::Engine::getInstance();

		// Run simulation
		for (int i = 0; i < 223; i++)
		{
			EXPECT_TRUE(module_l1_0->canAccess(0x0));
			esim_engine->ProcessEvents();
		}

	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


} // Namespace mem

