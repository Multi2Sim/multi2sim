/*
 * Multi2Sim
 * Copyright (C) 2014 Agamemnon Despopoulos (agdespopoulos@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <vector>
#include <algorithm>
#include <iostream>

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/IniFile.h>
#include <lib/cpp/String.h>
#include <lib/esim/Engine.h>

#include "Action.h"
#include "Address.h"
#include "Bank.h"
#include "Controller.h"
#include "Channel.h"
#include "Scheduler.h"
#include "System.h"


namespace dram
{

//
// Configuration Options
//

std::string debug_file;

std::string activity_file;

std::string config_file;

bool stand_alone_setting = false;


//
// Static variables
//

misc::Debug System::debug;

misc::Debug System::activity;

std::unique_ptr<System> System::instance;

esim::FrequencyDomain *System::DRAM_DOMAIN(nullptr);

esim::EventType *System::ACTION_REQUEST(nullptr);

esim::EventType *System::COMMAND_RETURN(nullptr);


System *System::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new System());
	return instance.get();
}


System::System()
{
	debug << "Dram simulator initialized\n";
}


void System::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("DRAM");

	// Debugger for dram
	command_line->RegisterString("--dram-debug <file>",
			debug_file,
			"Dump debug information related with the DRAM "
			"simulation.");

	// Activity log for dram
	command_line->RegisterString("--dram-debug-activity <file>",
			activity_file,
			"Dump debug information related with DRAM activity "
			"during simulation.");

	// Dram system configuration
	command_line->RegisterString("--dram-config <file>",
			config_file,
			"DRAM configuration file. Memory controllers and "
			"their components can be defined here.");

	// Stand-alone simulator
	command_line->RegisterBool("--dram-sim",
			stand_alone_setting,
			"Runs a DRAM simulation using the actions provided "
			"in the DRAM configuration file (option "
			"'--dram-config').");
}


void System::ProcessOptions()
{
	System *dram = System::getInstance();

	// Debugger
	if (!debug_file.empty())
		setDebugPath(debug_file);

	// Activity Debugger
	if (!activity_file.empty())
		setActivityDebugPath(activity_file);

	// Configuration
	if (!config_file.empty())
		dram->ParseConfiguration(config_file);

	// Stand-alone simulator
	if (stand_alone_setting)
	{
		dram->Run();
	}
}


void System::ParseConfiguration(const std::string &path)
{
	esim::Engine *esim = esim::Engine::getInstance();
	misc::IniFile ini_file(path);

	// Get the frequency and make the FrequencyDomain.
	int frequency = ini_file.ReadInt("General", "Frequency", 1000);
	DRAM_DOMAIN = esim->RegisterFrequencyDomain(
			"DRAM_DOMAIN", frequency);

	// Create events used by the entire system
	COMMAND_RETURN = esim->RegisterEventType("COMMAND_RETURN",
			Controller::CommandReturnHandler, DRAM_DOMAIN);

	// Iterate through each section.
	// Parse it if it is a MemoryController section.
	int num_controller = 0;
	for (int i = 0; i < ini_file.getNumSections(); i++)
	{
		std::string section_name = ini_file.getSection(i);
		if (misc::StringPrefix(section_name, "MemoryController"))
		{
			controllers.emplace_back(new Controller(num_controller,
					section_name, ini_file));
			num_controller++;
		}
	}

	// All controllers and all the memory hierarchy under them has been
	// made, so now calculate the sizes of address components.
	GenerateAddressSizes();

	// Parse actions if the section exists.
	if (ini_file.Exists("Actions"))
	{
		Actions *actions = Actions::getInstance();
		actions->ParseConfiguration(ini_file);
	}
}


void System::Run()
{
	// Running a simulation separate from the rest of m2s.
	stand_alone = true;

	// Get the simulation engine and actions.
	esim::Engine *engine = esim::Engine::getInstance();
	Actions *actions = Actions::getInstance();

	// Run a simulation with 1000 cycles.
	// FIXME: Make this dependent on actions.
	for (int i = 0; i < 1000; i++)
	{
		engine->ProcessEvents();
	}

	// Dump the system and actions to the debug file.
	dump(debug);
	actions->dump(debug);

	// Run checks from actions.
	actions->DoChecks();
}


int System::getNextCommandId()
{
	next_command_id++;
	return next_command_id;
}


void System::AddRequest(std::shared_ptr<Request> request)
{
	// Decode the address and move the request to the correct controller.
	Address *address = request->getAddress();
	controllers[address->getPhysical()]->AddRequest(request);

	// Debug
	debug << misc::fmt("[%lld] Adding request for 0x%llx to controller %d\n",
			DRAM_DOMAIN->getCycle(), address->getEncoded(),
			address->getPhysical());
}


int System::Log2(unsigned num)
{
	// Find the index of the highest set bit of num by shifting num to the
	// right repeatedly until it equals 0.
	int result = 0;
	while (num >>= 1)
		result++;
	return result;
}


void System::GenerateAddressSizes()
{
	// Set physical size based on number of controllers in the system.
	physical_size = Log2(controllers.size());

	// Find the largest of each component size because one address format
	// must be able to decode to any location in the system.
	// Controllers can potentially each have different sizes for everything
	// but all sizes under the controller are uniform.
	for (auto const& controller : controllers)
	{
		logical_size = std::max(logical_size,
				Log2(controller->getNumChannels()));
		rank_size = std::max(rank_size,
				Log2(controller->getNumRanks()));
		bank_size = std::max(bank_size,
				Log2(controller->getNumBanks()));
		row_size = std::max(row_size,
				Log2(controller->getNumRows()));
		column_size = std::max(column_size,
				Log2(controller->getNumColumns()));
	}
}


void System::dump(std::ostream &os) const
{
	// Print header
	os << "\n\n--------------------\n\n";
	os << "Dumping DRAM system\n";

	// Print controllers in the system
	os << misc::fmt("%d Controllers\nController dump:\n",
			(int) controllers.size());
	for (auto &controller : controllers)
		controller->dump(os);
}


}  //namespace dram
