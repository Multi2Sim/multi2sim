/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef X86_ARCH_TIMING_TIMING_H
#define X86_ARCH_TIMING_TIMING_H

#include <lib/cpp/String.h>
#include <lib/cpp/Debug.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>

#include <arch/common/Arch.h>
#include <arch/common/Timing.h>

#include "BranchPredictor.h"
#include "TraceCache.h"
#include "CPU.h"

namespace x86
{

// Forward declaration
class CPU;

// Class Timing
class Timing : public comm::Timing
{
	//
	// Static fields
	//

	// Unique instance of the singleton
	static std::unique_ptr<Timing> instance;

	// Simulation kind
	static comm::Arch::SimKind sim_kind;

	// Configuration file name
	static std::string config_file;

	// Report file name
	static std::string report_file;

	// MMU report file name
	static std::string mmu_report_file;

	// If true
	// how a message describing the format for the x86 configuration file
	// Passed with option --x86-help
	static bool help;

	// Message to display with '--x86-help'
	static const std::string help_message;

	// Frequency of memory system in MHz
	static int frequency;

	
	
	//
	// Member fields
	//

	// CPU associated with this timing simulator
	std::unique_ptr<CPU> cpu;

	// List of entry modules to the memory hierarchy
	std::vector<mem::Module *> entry_modules;

	// Private constructor for singleton
	Timing() : comm::Timing("x86")
	{
		setFrequencyDomain("x86", frequency);
		cpu.reset(new CPU());
	}

public:

	/// Exception for X86 timing simulator
	class Error : public misc::Error
	{
	public:

		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("X86 timing simulator");
		}
	};

	/// Return unique instance of the X86 timing simulator singleton.
	static Timing *getInstance();

	/// get CPU instance
	CPU *getCPU() { return cpu.get(); }

	/// Run one iteration of the cpu timing simuation.
	/// \return This function \c true if the iteration had a useful
	/// timing simulation, and \c false if all timing simulation finished execution.
	bool Run() override;

	/// Dump a default memory configuration for the architecture. This
	/// function is invoked by the memory system configuration parser when
	/// no specific memory configuration is given by the user for the
	/// architecture.
	void WriteMemoryConfiguration(misc::IniFile *ini_file) override;

	/// Check architecture-specific requirements for the memory
	/// memory configuration provided in the INI file. This function is
	/// invoked by the memory configuration parser.
	void CheckMemoryConfiguration(misc::IniFile *ini_file) override;

	/// Parse an [Entry] section in the memory configuration file for this
	/// architecture. These sections specify the entry points from the
	/// architecture into the memory hierarchy.
	void ParseMemoryConfigurationEntry(misc::IniFile *ini_file,
			const std::string &section) override;
	
	/// Return the number of entry modules from this architecture into the
	/// memory hierarchy.
	int getNumEntryModules() override
	{
		return entry_modules.size();
	}

	/// Return the entry module from the architecture into the memory
	/// hierarchy given its index. The index must be a value between 0 and
	/// getNumEntryModules() - 1.
	mem::Module *getEntryModule(int index) override
	{
		assert(index >= 0 && index < (int) entry_modules.size());
		return entry_modules[index];
	}
	
	
	
	
	//
	// Configuration
	//

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Parse the configuration file
	static void ParseConfiguration(misc::IniFile &ini_file);

	/// Return the simulation level set by command-line options '--x86-sim'
	static comm::Arch::SimKind getSimKind() { return sim_kind; }
};

} //namespace x86

#endif // X86_ARCH_TIMING_TIMING_H
