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
#include <cmath>

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/IniFile.h>
#include <lib/cpp/String.h>
#include <lib/esim/Engine.h>

#include "Action.h"
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

std::string config_file;

bool stand_alone = false;


//
// Static variables
//

misc::Debug System::debug;

std::unique_ptr<System> System::instance;

esim::FrequencyDomain *System::DRAM_DOMAIN(nullptr);

esim::EventType *System::ACTION_REQUEST(nullptr);


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
	command_line->setCategory("dram");
	
	// Debugger for dram
	command_line->RegisterString("--debug-dram <file>",
			debug_file,
			"Dump debug information related with the dram "
			"simulation.");

	// Dram system configuration
	command_line->RegisterString("--dram-config <file>",
			config_file,
			"Dram configuration file. Memory controllers and "
			"their components can be defined here.");

	// Stand-alone simulator
	command_line->RegisterBool("--dram-sim",
			stand_alone,
			"Runs a dram simulation using the actions provided "
			"in the dram configuration file (option "
			"'--dram-config').");
}


void System::ProcessOptions()
{
	System *dram = System::getInstance();

	// Debugger
	if (!debug_file.empty())
		setDebugPath(debug_file);

	// Configuration
	if (!config_file.empty())
		dram->ParseConfiguration(config_file);

	// Stand-alone simulator
	if (stand_alone)
	{
		dram->Run();
	}
}


void System::ParseConfiguration(const std::string &path)
{
	esim::Engine *esim = esim::Engine::getInstance();
	misc::IniFile config(path);

	// Get the frequency and make the FrequencyDomain.
	int frequency = config.ReadInt("General", "Frequency", 1000);
	DRAM_DOMAIN = esim->RegisterFrequencyDomain(
			"DRAM_DOMAIN", frequency);

	// Iterate through each section.
	// Parse it if it is a MemoryController section.
	int num_controller = 0;
	for (int i = 0; i < config.getNumSections(); i++)
	{
		std::string section_name = config.getSection(i);
		if (misc::StringPrefix(section_name, "MemoryController"))
		{
			controllers.emplace_back(new Controller(num_controller,
					section_name, config));
			num_controller++;
		}
	}

	// Parse actions if the section exists.
	if (config.Exists("Actions"))
	{
		Actions *actions = Actions::getInstance();
		actions->ParseConfiguration(config);
	}
}


void System::Run()
{
	debug << "It's happening.\n";

	esim::Engine *engine = esim::Engine::getInstance();
	for (int i = 0; i < 1000; i++)
	{
		engine->ProcessEvents();
	}

	dump(debug);
}


void System::AddRequest(std::shared_ptr<Request> request)
{
	// Decode the address and move the request to the correct controller.
	request->DecodeAddress();
	controllers[request->getAddress()->physical]->AddRequest(request);

	// Debug
	debug << misc::fmt("[%lld] Adding request for 0x%llx to controller %d\n",
			DRAM_DOMAIN->getCycle(), request->getAddress()->encoded,
			request->getAddress()->physical);
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

void System::DecodeAddress(Address &address)
{
	// Get the sizes of each address component in number of bits required
	// to represent it.
	int physical_size = Log2(controllers.size());
	int logical_size = 0;
	int rank_size = 0;
	int bank_size = 0;
	int row_size = 0;
	int column_size = 0;

	// Find the largest of each component size.
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

	// Make a local copy of the address.  Don't use the one in the address
	// struct because it will be altered during decoding.
	long long decoding = address.encoded;

	// Step through the address to parse out the components.
	// This will be configurable, but for now the address mapping is
	// (from MSB to LSB) physical:logical:rank:bank:row:column.
	address.column = decoding & int(pow(2, column_size) - 1);
	// Step to row.
	decoding >>= column_size;
	address.row = decoding & int(pow(2, row_size) - 1);
	// Step to bank.
	decoding >>= row_size;
	address.bank = decoding & int(pow(2, bank_size) - 1);
	// Step to rank.
	decoding >>= bank_size;
	address.rank = decoding & int(pow(2, rank_size) - 1);
	// Step to logical.
	decoding >>= rank_size;
	address.logical = decoding & int(pow(2, logical_size) - 1);
	// Step to physical.
	decoding >>= logical_size;
	address.physical = decoding & int(pow(2, physical_size) - 1);

	// Debug
	// debug << misc::fmt("Sizes: %d, %d, %d, %d, %d, %d\n", physical_size,
	// 		logical_size, rank_size, bank_size, row_size,
	// 		column_size);
	// debug << address;
}


void System::dump(std::ostream &os) const
{
	os << "\n\n--------------------\n\n";
	os << "Dumping DRAM system\n";
	os << misc::fmt("%d Controllers\nController dump:\n",
			(int) controllers.size());

	for (auto &controller : controllers)
		controller->dump(os);
}


void Address::dump(std::ostream &os) const
{
	os << misc::fmt("0x%llx = %d : %d : %d : %d : %d : %d\n", encoded,
			physical, logical, rank, bank, row, column);
}


}  //namespace dram
