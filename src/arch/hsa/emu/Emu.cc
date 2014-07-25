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

//#include <arch/hsa/driver/Driver.h>

#include "Emu.h"
#include "ProgramLoader.h"

namespace HSA
{
//
// Configuration options
//

// Simulation kind
comm::Arch::SimKind Emu::sim_kind = comm::Arch::SimFunctional;

// Debug file
std::string Emu::hsa_debug_loader_file;
std::string Emu::hsa_debug_isa_file;
std::string Emu::hsa_debug_aql_file;




//
// Static variables
//

// Debugger
misc::Debug Emu::loader_debug;
misc::Debug Emu::isa_debug;
misc::Debug Emu::aql_debug;

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


void Emu::ProcessOptions()
{
	loader_debug.setPath(hsa_debug_loader_file);
	isa_debug.setPath(hsa_debug_isa_file);
	aql_debug.setPath(hsa_debug_aql_file);
}


Emu::Emu() : comm::Emu("hsa"),
		manager(&memory)
{
	// FIXME remove everything related to pid
	pid = 100;

	// FIXME: Allow user to set up customized HSA virtual machine
	setDefaultComponentList();
	if (loader_debug)
	{
		DumpComponentList(loader_debug);
	}
}


Emu *Emu::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	instance.reset(new Emu());
	return instance.get();
}


void Emu::setDefaultComponentList()
{
	// Add a CPU device
	Component *cpu = Component::getDefaultCPUComponent();
	components.push_back(std::unique_ptr<Component>(cpu));

	// Add a simple GPU component
	Component *gpu = Component::getDefaultGPUComponent();
	components.push_back(std::unique_ptr<Component>(gpu));

	// Set the CPU as the host component
	setHostCPUComponent(cpu);

}


void Emu::DumpComponentList(std::ostream &os = std::cout) const
{
	os << "Components installed.\n";
	for (auto it = components.begin(); it != components.end(); it++)
	{
		os << *((*it).get());
	}
}


bool Emu::Run()
{
	// Stop if maxmum number of CPU instructions exceeded
	// if(max_instructions && instructions >= max_instructions)
		// esim->Finish("hsaMaxInst");

	// Stop if any previous reason met
	if (esim->hasFinished())
		return true;

	bool stillRunning = false;

	// Let all components to execute their own task
	for (auto it = components.begin(); it != components.end(); it++)
	{
		bool running = (*it)->Execute();
		if (running)
			stillRunning = true;
	}

	// Process list of suspended work items
	// ProcessEvents();
		
	// Still running;
	return stillRunning;
}


void Emu::LoadProgram(const std::vector<std::string> &args,
		const std::vector<std::string> &env,
		const std::string &cwd,
		const std::string &stdin_file_name,
		const std::string &stdout_file_name)
{
	// Load the whole program binary
	ProgramLoader::LoadProgram(args, env, cwd,
			stdin_file_name, stdout_file_name);

	// Create the default queue for the host device
	AQLQueue *queue = RuntimeLibrary::CreateQueue(host_cpu, 2,
			HSAQueueMulti);

	//Prepare the dispatch packet
	AQLDispatchPacket *packet = new AQLDispatchPacket();
	packet->setDimension(1);
	packet->setGridSizeX(1);
	packet->setWorkGroupSizeX(1);
	ProgramLoader *loader = ProgramLoader::getInstance();
	Function *main_function = loader->getMainFunction();
	packet->setKernalObjectAddress((unsigned long long)main_function);

	// Enqueue the packet
	aql_debug << "Packet created and enqueued: \n" << *packet;
	queue->Enqueue(packet);
}

}  // namespace HSA

