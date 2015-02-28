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

#ifndef ARCH_HSA_EMU_EMU_H
#define ARCH_HSA_EMU_EMU_H

#include <arch/common/Arch.h>
#include <arch/common/Emu.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>
#include <memory/Memory.h>
#include <memory/Manager.h>
//#include <arch/hsa/driver/runtime.h>

#include "AQLQueue.h"
#include "Component.h"

namespace HSA
{

class Component;


/// Exception thrown by HSA modules
class Error : public misc::Error
{
public: 
	
	/// Constructor
	Error(const std::string &message) : misc::Error(message)
	{
		AppendPrefix("HSA");
	}
};


/// HSA Emulator
class Emu : public comm::Emu
{
	// Debugger files
	static std::string hsa_debug_loader_file;
	static std::string hsa_debug_isa_file;
	static std::string hsa_debug_aql_file;

	// Maximum number of instructions
	// static long long max_instructions;

	// Simulation kind
	static comm::Arch::SimKind sim_kind;

	// Unique instance of HSA emulator
	static std::unique_ptr<Emu> instance;

	// Private constructor. The only possible instance of the HSA emulator
	// can be obtained with a call to getInstance()
	Emu();

	// Maps from a 64-bit identifier to component
	std::map<unsigned long long, std::unique_ptr<Component>> components;

	// Host CPU device, the execution has to be kicked start by the host CPU
	Component *host_cpu;

	// Host CPU component, the unique_ptr is kept in the list components.
	// Component *host_cpu;
	void InstallDefaultComponents();

	/// Set host CPU device
	void setHostCpuDevice(Component *cpu) { host_cpu = cpu; }

	// Flattened memory space, the only memory space of the virtual memory
	mem::Memory memory;

	// Global memory manager
	mem::Manager manager;

public:

	/// Destructor
	~Emu()
	{
		// Guarantee everything in guest memory freed before freeing 
		// the guest memory itself;
		components.clear();
	};

	/// The HSA emulator is a singleton class. The only possible instance
	/// of it will be allocated the first time this function is invoked
	static Emu *getInstance();

	/// Install the components of the virtual machine 
	/// Install the components according to the ini config file or install
	/// default setup
	void InstallComponents(const std::string& config_file);

	/// Get component by handler
	Component *getComponent(unsigned long long handler) const
	{
		auto it = components.find(handler);
		if (it == components.end())
			return nullptr;
		else
			return it->second.get();
	}

	/// Get the component whose handler is next to the value passed in
	Component *getNextComponent(unsigned long long handler) const
	{
		auto it = components.upper_bound(handler);
		if (it == components.end())
			return nullptr;
		else
			return it->second.get();
	}

	/// Dump component list for debug purpose
	void DumpComponentList(std::ostream &os) const;

	/// Run one iteration of the emulation loop
	/// \return This function \c true if the iteration had a useful emulation 
	/// and \c false if all work items finished execution
	bool Run();

	/// Debugger for HSA
	static misc::Debug loader_debug;
	static misc::Debug isa_debug;
	static misc::Debug aql_debug;

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Create a main work item and load a program. See comm::Emu::Load() 
	/// for details on the meaning of each argument.
	void LoadProgram(const std::vector<std::string> &args,
			const std::vector<std::string> &env = { },
			const std::string &cwd = "",
			const std::string &stdin_file_name = "",
			const std::string &stdout_file_name = "");

	/// Return the number of components
	// unsigned int getNumberOfComponent() const { return components.size(); }

	/// Return the global memory manager
	mem::Manager *getMemoryManager() { return &manager; };

	/// Return the pointer to the global memory
	mem::Memory *getMemory() { return &memory; };

};

}  // namespace HSA

#endif

