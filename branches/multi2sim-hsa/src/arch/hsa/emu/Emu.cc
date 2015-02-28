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

#include <arch/hsa/asm/Asm.h>

#include "Emu.h"
#include "AQLQueue.h"
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


Emu::Emu() :
		comm::Emu("hsa"),
		manager(&memory)
{
	InstallComponents("");
	Asm::getInstance()->DisableIndentation();
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


void Emu::InstallComponents(const std::string& config_file = "")
{
	if (config_file != "")
	{
		throw misc::Panic("User defined component config file is "
				"not supported yet");
		return;
	}
	InstallDefaultComponents();
}


void Emu::InstallDefaultComponents()
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


void Emu::DumpComponentList(std::ostream &os = std::cout) const
{
	os << "Components installed.\n";
	for (auto it = components.begin(); it != components.end(); it++)
	{
		os << *(it->second.get());
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
		bool running = it->second->Execute();
		if (running)
			stillRunning = true;
	}

	// Process list of suspended contexts
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

	// Create a simple queue and add it to host cpu
	auto queue = misc::new_unique<AQLQueue>(2, HSA_QUEUE_TYPE_MULTI);
	AQLQueue *queue_ptr = queue.get();
	host_cpu->addQueue(std::move(queue));

	//Prepare the dispatch packet
	AQLDispatchPacket *packet = new AQLDispatchPacket();
	packet->setDimension(1);
	packet->setGridSize(1, 1, 1);
	packet->setWorkGroupSize(1, 1, 1);
	ProgramLoader *loader = ProgramLoader::getInstance();
	Function *main_function = loader->getMainFunction();
	unsigned int function_dir = main_function->getFunctionDirective()->getOffset();
	packet->setKernalObjectAddress((unsigned long long)function_dir);
	packet->setPrivateSegmentSizeBytes(0x10000);
	packet->setGroupSegmentSizeBytes(0x10000);

	// Enqueue the packet
	aql_debug << "Packet created and enqueued: \n" << *packet;
	queue_ptr->Enqueue(packet);
}

}  // namespace HSA

