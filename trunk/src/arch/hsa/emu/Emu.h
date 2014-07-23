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
#include <arch/hsa/asm/BrigFile.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>
#include <arch/hsa/asm/BrigFile.h>

#include "ProgramLoader.h"
#include "Component.h"
#include "WorkGroup.h"
#include "WorkItem.h"
#include "RuntimeLibrary.h"


namespace HSA
{

class Component;
class WorkItem;


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

	// Maximum number of instructions
	// static long long max_instructions;

	// Simulation kind
	static comm::Arch::SimKind sim_kind;

	// Unique instance of HSA emulator
	static std::unique_ptr<Emu> instance;

	// Private constructor. The only possible instance of the HSA emulator
	// can be obtained with a call to getInstance()
	Emu();

	// list of available components
	std::list<std::unique_ptr<Component>> components;

	// Host CPU component, the unique_ptr is kept in the list components.
	Component *host_cpu;

	// Process ID to be assigned next. Process IDs are assigned in
	// increasing order, using function Emu::getPid()
	int pid;

public:

	/// Destructor
	virtual ~Emu(){};

	/// The HSA emulator is a singleton class. The only possible instance
	/// of it will be allocated the first time this function is invoked
	static Emu *getInstance();

	/// Return a unique process ID. Work items can call this function when 
	/// created to obtain their unique identifier
	/// TODO: work items should not have a process id, instead, they should 
	/// 	have work group ids and work item ids. 
	int getPid() { return pid++; }

	/// Set component list with default devices. A single core CPU and a
	/// simple GPU will be installed on the virtual machine
	void setDefaultComponentList();

	/// Set host CPU component
	void setHostCPUComponent(Component *cpu) { host_cpu = cpu; }

	/// Dump component list for debug purpose
	void DumpComponentList(std::ostream &os) const;

	/// Run one iteration of the emulation loop
	/// \return This function \c true if the iteration had a useful emulation 
	/// and \c false if all work items finished execution
	bool Run();

	/// Debugger for HSA
	static misc::Debug loader_debug;
	static misc::Debug isa_debug;

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

	/// Returns the number of components
	unsigned int getNumberOfComponent() const { return components.size(); }

};

}  // namespace HSA

#endif

