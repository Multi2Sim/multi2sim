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

Context *Emu::newContext()
{
	// Create context and add to context list
	Context *context = new Context();
	contexts.emplace_back(context);
	return context;
}

bool Emu::Run()
{
	// Stop if there is no more contexts
	if(!contexts.size())
		return false;

	// Stop if maxmum number of CPU instructions exceeded
	// if(max_instructions && instructions >= max_instructions)
		// esim->Finish("hsaMaxInst");

	// Stop if any previous reason met
	if(esim->hasFinished())
		return true;

	// Currently I traverse all the contexts. If requreied, I will change 
	// it to only traversing running contexts;
	auto end = contexts.end();
	for(auto it = contexts.begin(); it != end; )
	{
		// Save the position of next context
		auto next = it;
		++next;

		// Run one iteration
		Context *context = &(*it->get());
		context->Execute();

		// Move to saved next context
		it = next;
	}

	//TODO: add free context function call

	// Process list of suspended contexts
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
	// Create new context
	Context *context = newContext();
	context->Load(args, env, cwd,
			stdin_file_name, stdout_file_name);
}


}  // namespace HSA

