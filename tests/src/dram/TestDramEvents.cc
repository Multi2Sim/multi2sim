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

#include <string>
#include <regex>
#include <exception>

#include <dram/Address.h>
#include <dram/Bank.h>
#include <dram/Channel.h>
#include <dram/Controller.h>
#include <dram/Rank.h>
#include <dram/System.h>
#include <gtest/gtest.h>
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

static std::string default_config = 
			"[ General ]\n"
			"Frequency = 100000\n"
			"[ MemoryController One ]\n";
			
TEST(TestSystemEvents, section_one_read)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after a single read
	std::array<std::string, 2> expected = { "Activate", "Read" };

	// Initialze array to store test results
	std::array<std::string, 2>  results;

	// Test body
	std::string message;
	try
	{
		// Submit a single read request
		dram_system->Read(0);
		esim::Engine *engine = esim::Engine::getInstance();
		engine->ProcessEvents();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Step through cycle by cycle
		for (unsigned i = 0; i < results.size(); i++)
		{
			engine->ProcessEvents();

			// Store Current Bank Command in results
			results[i] = bank->getCommandInQueueType();
		}

		// Check that command queue is now empty
		engine->ProcessEvents();
		EXPECT_TRUE(bank->getNumCommandsInQueue() == 0);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
	
	// Compare results to expected
	EXPECT_TRUE(results == expected);
}

TEST(TestSystemEvents, section_one_write)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after a single write
	std::array<std::string, 2> expected = { "Activate", "Write" };

	// Initialize array to store test results
	std::array<std::string, 2> results;

	// Test body
	std::string message;
	try
	{
		// Submit a single write request
		dram_system->Write(0);
		esim::Engine *engine = esim::Engine::getInstance();
		engine->ProcessEvents();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Step through cycle by cycle
		for (unsigned i = 0; i < results.size(); i++)
		{
			engine->ProcessEvents();

			// Store Current Bank Command in results
			results[i] = bank->getCommandInQueueType();
		}

		// Check that command queue is now empty
		engine->ProcessEvents();
		EXPECT_TRUE(bank->getNumCommandsInQueue() == 0);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
	
	// Compare results to expected
	EXPECT_TRUE(results == expected);
}

TEST(TestSystemEvents, section_two_reads_same_address)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after two reads to same address
	std::array<std::string, 3> expected = { "Activate", "Read", "Read" };

	// Initialze array to store test results
	std::array<std::string, 3>  results;

	// Test body
	std::string message;
	try
	{
		// Submit two read requests
		dram_system->Read(0);
		dram_system->Read(0);
		esim::Engine *engine = esim::Engine::getInstance();
		engine->ProcessEvents();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Step through cycle by cycle
		for (unsigned i = 0; i < results.size(); i++)
		{
			engine->ProcessEvents();

			// Store Current Bank Command in results
			results[i] = bank->getCommandInQueueType();
		}

		// Check that command queue is now empty
		engine->ProcessEvents();
		EXPECT_TRUE(bank->getNumCommandsInQueue() == 0);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
	
	// Compare results to expected
	EXPECT_TRUE(results == expected);
}

TEST(TestSystemEvents, section_two_writes_same_address)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after two writes to same address
	std::array<std::string, 3> expected = { "Activate", "Write", "Write" };

	// Initialze array to store test results
	std::array<std::string, 3>  results;

	// Test body
	std::string message;
	try
	{
		// Submit two write requests
		dram_system->Write(0);
		dram_system->Write(0);
		esim::Engine *engine = esim::Engine::getInstance();
		engine->ProcessEvents();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Step through cycle by cycle
		for (unsigned i = 0; i < results.size(); i++)
		{
			engine->ProcessEvents();

			// Store Current Bank Command in results
			results[i] = bank->getCommandInQueueType();
		}

		// Check that command queue is now empty
		engine->ProcessEvents();
		EXPECT_TRUE(bank->getNumCommandsInQueue() == 0);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
	
	// Compare results to expected
	EXPECT_TRUE(results == expected);
}

TEST(TestSystemEvents, section_two_reads_different_row)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after two reads to different rows
	std::array<std::string, 5> expected = { "Activate", "Read",
		"Precharge", "Activate", "Read" };

	// Initialze array to store test results
	std::array<std::string, 5>  results;

	// Test body
	std::string message;
	try
	{
		// Submit two read requests one row apart
		dram_system->Read(0);
		dram_system->Read(1024);
		esim::Engine *engine = esim::Engine::getInstance();
		engine->ProcessEvents();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Step through cycle by cycle
		for (unsigned i = 0; i < results.size(); i++)
		{
			engine->ProcessEvents();

			// Store Current Bank Command in results
			results[i] = bank->getCommandInQueueType();
		}

		// Check that command queue is now empty
		engine->ProcessEvents();
		EXPECT_TRUE(bank->getNumCommandsInQueue() == 0);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
	
	// Compare results to expected
	EXPECT_TRUE(results == expected);
}

TEST(TestSystemEvents, section_two_writes_different_row)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after two writes to different rows
	std::array<std::string, 5> expected = { "Activate", "Write",
		"Precharge", "Activate", "Write" };

	// Initialze array to store test results
	std::array<std::string, 5>  results;

	// Test body
	std::string message;
	try
	{
		// Submit two write requests one row apart
		dram_system->Write(0);
		dram_system->Write(1024);
		esim::Engine *engine = esim::Engine::getInstance();
		engine->ProcessEvents();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Step through cycle by cycle
		for (unsigned i = 0; i < results.size(); i++)
		{
			engine->ProcessEvents();

			// Store Current Bank Command in results
			results[i] = bank->getCommandInQueueType();
		}

		// Check that command queue is now empty
		engine->ProcessEvents();
		EXPECT_TRUE(bank->getNumCommandsInQueue() == 0);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
	
	// Compare results to expected
	EXPECT_TRUE(results == expected);
}

}
