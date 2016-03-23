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

#ifndef ARCH_X86_TIMING_TIMING_H
#define ARCH_X86_TIMING_TIMING_H

#include <lib/cpp/String.h>
#include <lib/cpp/Debug.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>

#include <arch/common/Arch.h>
#include <arch/common/Timing.h>

#include "BranchPredictor.h"
#include "Cpu.h"
#include "TraceCache.h"


namespace x86
{

// Forward declaration
class Cpu;

// Class Timing
class Timing : public comm::Timing
{
	//
	// Static fields
	//

	// Error message
	static const char *error_fast_forward;

	// Unique instance of the singleton
	static std::unique_ptr<Timing> instance;

	// Simulation kind
	static comm::Arch::SimKind sim_kind;

	// Configuration file name
	static std::string config_file;

	// Report file name
	static std::string report_file;

	// If true, show a message describing the format for the x86
	// configuration file. Passed with option --x86-help.
	static bool help;

	// Message to display with '--x86-help'
	static const std::string help_message;

	// Frequency of memory system in MHz
	static int frequency;

	
	
	//
	// Class members
	//

	// CPU object
	std::unique_ptr<Cpu> cpu;

	// List of entry modules to the memory hierarchy
	std::vector<mem::Module *> entry_modules;

	// Dump a specific part of a statistics report related with uops.
	void DumpUopReport(std::ostream &os, const long long *uop_stats,
			const std::string &prefix, int peak_ipc) const;

public:

	//
	// Class Error
	//

	/// Exception for X86 timing simulator
	class Error : public misc::Error
	{
	public:

		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("X86 timing simulator");
		}
	};




	//
	// Static members
	//

	/// Return unique instance of the X86 timing simulator singleton.
	static Timing *getInstance();

	/// Destroy the singleton if allocated.
	static void Destroy() { instance = nullptr; }
	
	/// Timing simulator trace
	static esim::Trace trace;

	/// Major and Minor versions of the trace
	static const int trace_version_major;
	static const int trace_version_minor;



	//
	// Class members
	//

	/// Constructor
	Timing();

	/// Get pointer to the CPU object of this timing simulator
	Cpu *getCpu() const
	{
		assert(cpu.get());
		return cpu.get();
	}

	/// Fast forward instructions set up by the user
	void FastForward();

	/// Run one iteration of the cpu timing simuation.
	/// \return This function \c true if the iteration had a useful
	/// timing simulation, and \c false if all timing simulation finished
	/// execution.
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

	/// Dump the statistics summary for the timing simulator.
	void DumpSummary(std::ostream &os) const override;

	/// Dump a report of statistics collected during x86 simulation
	void DumpReport() const override;

	/// Dump the configuration of the CPU
	void DumpConfiguration(std::ofstream &os) const;

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
	static void ParseConfiguration(misc::IniFile *ini_file);

	/// Return the simulation level set by command-line options '--x86-sim'
	static comm::Arch::SimKind getSimKind() { return sim_kind; }
};

} //namespace x86

#endif // ARCH_X86_TIMING_TIMING_H
