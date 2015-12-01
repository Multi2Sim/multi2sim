/*
 *  Multi2Sim
 *  Copyright (C) 2015  David English (english.d@husky.neu.edu)
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

// Testing config used to simplify testing that is not intended to verify
// timings.
static std::string zero_time_config = default_config + 
			"tRC = 0\n"
			"tRRD = 0\n"
			"tRP = 0\n"
			"tRFC = 0\n"
			"tCCD = 0\n"
			"tRTRS = 0\n"
			"tCWD = 0\n"
			"tWTR = 0\n"
			"tCAS = 0\n"
			"tRCD = 0\n"
			"tOST = 0\n"
			"tRAS = 0\n"
			"tWR = 0\n"
			"tRTP = 0\n"
			"tBURST = 0\n";
			
TEST(TestSystemEvents, section_one_read)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(zero_time_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after a single read
	std::array<std::string, 2> expected = { "Activate", "Read" };

	// Initialze array to store test results
	std::array<std::string, 2>  results;

	// Test body
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
	ini_file.LoadFromString(zero_time_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after a single write
	std::array<std::string, 2> expected = { "Activate", "Write" };

	// Initialize array to store test results
	std::array<std::string, 2> results;

	// Test body
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
	ini_file.LoadFromString(zero_time_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after two reads to same address
	std::array<std::string, 3> expected = { "Activate", "Read", "Read" };

	// Initialze array to store test results
	std::array<std::string, 3>  results;

	// Test body
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
	ini_file.LoadFromString(zero_time_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after two writes to same address
	std::array<std::string, 3> expected = { "Activate", "Write", "Write" };

	// Initialze array to store test results
	std::array<std::string, 3>  results;

	// Test body
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
	ini_file.LoadFromString(zero_time_config);

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
	ini_file.LoadFromString(zero_time_config);

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

TEST(TestSystemEvents, section_two_reads_same_row_closed_page_policy)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	std::string closed_page_config = 
			zero_time_config + "PagePolicy = Closed\n";
	ini_file.LoadFromString(closed_page_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after two reads to the same row
	std::array<std::string, 6> expected = { "Activate", "Read",
		"Precharge", "Activate", "Read", "Precharge" };

	// Initialze array to store test results
	std::array<std::string, 6>  results;

	// Test body
	try
	{
		// Submit two read requests one row apart
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

TEST(TestSystemEvents, section_two_writes_same_row_closed_page_policy)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	std::string closed_page_config = 
			zero_time_config + "PagePolicy = Closed\n";
	ini_file.LoadFromString(closed_page_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// TODO: Verify that these are the expected commands 
	// after two writes to the same row
	std::array<std::string, 6> expected = { "Activate", "Write",
		"Precharge", "Activate", "Write", "Precharge" };

	// Initialze array to store test results
	std::array<std::string, 6>  results;

	// Test body
	try
	{
		// Submit two write requests one row apart
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

TEST(TestSystemEvents, section_test_precharge_timing)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// Test body
	try
	{
		// Submit two read requests one row apart
		// to trigger a precharge
		dram_system->Read(0);
		dram_system->Read(1024);
		esim::Engine *engine = esim::Engine::getInstance();

		// Three ProcessEvents to get the precharge scheduled
		engine->ProcessEvents();
		engine->ProcessEvents();
		engine->ProcessEvents();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Go to precharge command
		while(System::frequency_domain->getCycle()
			       	< bank->getFrontCommandTiming()){
			engine->ProcessEvents();
		}

		// Ensure we are at the first cycle where prcharge in at front
		EXPECT_TRUE(bank->getCommandInQueueType() != "Precharge");
		engine->ProcessEvents();
		EXPECT_TRUE(bank->getCommandInQueueType() == "Precharge");

		// TODO: Verify Precharge should take 16 cycles
		EXPECT_TRUE(bank->getFrontCommandTiming() - 
			System::frequency_domain->getCycle() == 16);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

TEST(TestSystemEvents, section_test_activate_timing_same_bank)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// Test body
	try
	{
		// Submit two read requests one row apart
		// to trigger an activate
		dram_system->Read(0);
		dram_system->Read(1024);
		esim::Engine *engine = esim::Engine::getInstance();

		// Three ProcessEvents to get the activate scheduled
		engine->ProcessEvents();
		engine->ProcessEvents();
		engine->ProcessEvents();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Skip over precharge command
		while(System::frequency_domain->getCycle()
			       	< bank->getFrontCommandTiming()){
			engine->ProcessEvents();
		}
		engine->ProcessEvents();

		// Go to activate command
		while(System::frequency_domain->getCycle()
			       	< bank->getFrontCommandTiming()){
			engine->ProcessEvents();
		}

		// Ensure we are at the first cycle where activate in at front
		EXPECT_TRUE(bank->getCommandInQueueType() != "Activate");
		engine->ProcessEvents();
		EXPECT_TRUE(bank->getCommandInQueueType() == "Activate");

		// TODO: Verify activate should take 20 cycles
		EXPECT_TRUE(bank->getFrontCommandTiming() - 
			System::frequency_domain->getCycle() == 20);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

TEST(TestSystemEvents, section_test_single_read_timing)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// Test body
	try
	{
		// Submit one read
		dram_system->Read(0);
		esim::Engine *engine = esim::Engine::getInstance();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Schedule the command
		engine->ProcessEvents();
		engine->ProcessEvents();

		// ProcessEvents until the command queue is empty
		while(bank->getNumCommandsInQueue() > 0){
			engine->ProcessEvents();
		}

		// 15 cycles to do single read
		EXPECT_TRUE(System::frequency_domain->getCycle() == 15);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

TEST(TestSystemEvents, section_test_single_write_timing)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// Test body
	try
	{
		// Submit one write
		dram_system->Write(0);
		esim::Engine *engine = esim::Engine::getInstance();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Schedule the commands
		engine->ProcessEvents();
		engine->ProcessEvents();

		// ProcessEvents until the command queue is empty
		while(bank->getNumCommandsInQueue() > 0){
			engine->ProcessEvents();
		}

		// 15 cycles to do single write
		EXPECT_TRUE(System::frequency_domain->getCycle() == 15);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

TEST(TestSystemEvents, section_test_negtive_address)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// Test body
	std::string message;
	try
	{
		// Submit a write to a negaive address
		dram_system->Write(-1);
		esim::Engine *engine = esim::Engine::getInstance();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Schedule the commands
		engine->ProcessEvents();
		engine->ProcessEvents();

		// ProcessEvents until the command queue is empty
		while(bank->getNumCommandsInQueue() > 0){
			engine->ProcessEvents();
		}

	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Invalid Address").c_str(),
			message.c_str());
}

TEST(TestSystemEvents, section_test_address_out_of_bounds)
{
	// cleanup singleton instance
	Cleanup();

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(default_config);

	// Set up dram instance
	System *dram_system = System::getInstance();
	dram_system->ParseConfiguration(&ini_file);

	// Test body
	std::string message;
	try
	{
		// Submit a write to an address that is too large
		dram_system->Write(10000000000);
		esim::Engine *engine = esim::Engine::getInstance();

		// Get relevent bank
		Bank *bank = dram_system->getController(0)->
				getChannel(0)->getRank(0)->getBank(0);

		// Schedule the commands
		engine->ProcessEvents();
		engine->ProcessEvents();

		// ProcessEvents until the command queue is empty
		while(bank->getNumCommandsInQueue() > 0){
			engine->ProcessEvents();
		}

	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Invalid Address").c_str(),
			message.c_str());
}
}
