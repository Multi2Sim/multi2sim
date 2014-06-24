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

#include <arch/hsa/driver/Driver.h>
#include "Emu.h"

namespace HSA
{


//
// Configuration options
//

// Simulation kind
comm::Arch::SimKind Emu::sim_kind = comm::Arch::SimFunctional;

// Debug file
std::string Emu::hsa_debug_file;


//
// Static variables
//


// Debugger
misc::Debug Emu::hsa_debug;

// Singleton instance
std::unique_ptr<Emu> Emu::instance;



//
// Functions
//

void Emu::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("HSA");

	// Option --hsa-debug <file>
	command_line->RegisterString("--hsa-debug <file>", hsa_debug_file,
		"Dump debug information about hsa");
	
	// Option --hsa-sim <kind>
	command_line->RegisterEnum("--hsa-sim {functional|detailed} "
			"(default = functional)",
			(int &) sim_kind, comm::Arch::SimKindMap,
			"Level of accuracy of hsa simulation");
}


void Emu::ProcessOptions()
{
	hsa_debug.setPath(hsa_debug_file);
	//std::cout << "hsa_debug path " << hsa_debug_file << "\n";
}


Emu::Emu() : comm::Emu("hsa")
{
	pid = 100;
}


Emu *Emu::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	instance.reset(new Emu());
	return instance.get();
}

WorkItem *Emu::newWorkItem()
{
	// Create work item and add to work item list
	WorkItem *wi = new WorkItem();
	work_items.emplace_back(wi);
	return wi;
}

bool Emu::Run()
{
	// Stop if there is no more work items
	if(!work_items.size())
		return false;

	// Stop if maxmum number of CPU instructions exceeded
	// if(max_instructions && instructions >= max_instructions)
		// esim->Finish("hsaMaxInst");

	// Stop if any previous reason met
	if(esim->hasFinished())
		return true;

	// Currently I traverse all the work items. If requreied, I will change 
	// it to only traversing running work items;
	auto end = work_items.end();
	for(auto it = work_items.begin(); it != end; )
	{
		// Save the position of next work item
		auto next = it;
		++next;

		// Run one iteration
		WorkItem *wi = &(*it->get());
		wi->Execute();

		// Move to saved next work item
		it = next;
	}

	//TODO: add free work item function call

	// Process list of suspended work items
	// ProcessEvents();
		

	// Still running;
	return true;
}


void Emu::LoadProgram(const std::vector<std::string> &args,
		const std::vector<std::string> &env,
		const std::string &cwd,
		const std::string &stdin_file_name,
		const std::string &stdout_file_name)
{
	// Create new work item
	WorkItem *wi = newWorkItem();

	// Load the whole program
	wi->Load(args, env, cwd,
			stdin_file_name, stdout_file_name);
}


}  // namespace HSA

