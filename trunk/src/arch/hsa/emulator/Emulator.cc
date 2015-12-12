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

#include <cstring>

#include <lib/cpp/Misc.h>
#include <arch/hsa/disassembler/Disassembler.h>

#include "Emulator.h"
#include "AQLQueue.h"

namespace HSA
{
//
// Configuration options
//

// Simulation kind
comm::Arch::SimKind Emulator::sim_kind = comm::Arch::SimFunctional;

// Debug file
std::string Emulator::hsa_debug_loader_file;
std::string Emulator::hsa_debug_isa_file;
std::string Emulator::hsa_debug_aql_file;




//
// Static variables
//

// Debugger
misc::Debug Emulator::loader_debug;
misc::Debug Emulator::isa_debug;
misc::Debug Emulator::aql_debug;

// Singleton instance
std::unique_ptr<Emulator> Emulator::instance;




//
// Functions
//

void Emulator::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("HSA");

	// Option --hsa-debug-loader <file>
	command_line->RegisterString("--hsa-debug-loader <file>", 
			hsa_debug_loader_file,
			"Dump debug information about HSA program loader");
	
	// Option --hsa-debug-isa <file>
	command_line->RegisterString("--hsa-debug-isa <file>", 
			hsa_debug_isa_file,
			"Dump debug information about HSA isa implementation");

	// Option --hsa-debug-aql <file>
	command_line->RegisterString("--hsa-debug-aql <file>",
			hsa_debug_aql_file,
			"Dump debug information about AQL queues and packets");

	// Option --hsa-sim <kind>
	command_line->RegisterEnum("--hsa-sim {functional|detailed} "
			"(default = functional)",
			(int &) sim_kind, comm::Arch::SimKindMap,
			"Level of accuracy of HSA simulation");
}


void Emulator::ProcessOptions()
{
	loader_debug.setPath(hsa_debug_loader_file);
	isa_debug.setPath(hsa_debug_isa_file);
	aql_debug.setPath(hsa_debug_aql_file);
}


Emulator::Emulator() : comm::Emulator("HSA")
{
	InstallComponents("");
	//memory = null;
	//manager.reset(new mem::Manager(memory.get()));
	Disassembler::getInstance()->DisableIndentation();
	StartTimer();
	if (loader_debug)
	{
		DumpComponentList(loader_debug);
	}
}


Emulator *Emulator::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	instance.reset(new Emulator());
	return instance.get();
}


void Emulator::InstallComponents(const std::string& config_file = "")
{
	if (config_file != "")
	{
		throw misc::Panic("User defined component config file is "
				"not supported yet");
		return;
	}
	InstallDefaultComponents();
}


void Emulator::InstallDefaultComponents()
{
	// Add a CPU device
	auto cpu = Component::getDefaultCPUComponent(1);
	components.insert(std::make_pair(1, std::move(cpu)));

	// Add a simple GPU component
	auto gpu = Component::getDefaultGPUComponent(2);
	components.insert(std::make_pair(2, std::move(gpu)));

	// Set the CPU as the host component
	setHostCpuDevice(components.at(1).get());
}


void Emulator::DumpComponentList(std::ostream &os = std::cout) const
{
	os << "Components installed.\n";
	for (auto it = components.begin(); it != components.end(); it++)
	{
		os << *(it->second.get());
	}
}


bool Emulator::Run()
{
	// Stop if maxmum number of CPU instructions exceeded
	// if(max_instructions && instructions >= max_instructions)
		// esim->Finish("hsaMaxInst");

	// Stop if any previous reason met
	if (esim->hasFinished())
		return false;

	bool active = false;

	// Let all components to execute their own task
	for (auto it = components.begin(); it != components.end(); it++)
	{
		bool running = it->second->Execute();
		if (running)
			active = true;
	}

	// Process list of suspended contexts
	// ProcessEvents();
		
	// Still running;
	return active;
}


void Emulator::LoadProgram(const std::vector<std::string> &args,
		const std::vector<std::string> &env,
		const std::string &cwd,
		const std::string &stdin_file_name,
		const std::string &stdout_file_name)
{
	// This function is kept blank, because the HSA kernel is 
	// always launched from HSA runtime.
}

}  // namespace HSA

