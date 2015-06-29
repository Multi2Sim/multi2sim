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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_TIMING_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_TIMING_H

#include <arch/common/Arch.h>
#include <arch/common/Timing.h>

#include "Gpu.h"


namespace SI
{

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

	// The GPU object
	Gpu gpu;

	// List of entry modules to the memory hierarchy
	std::vector<mem::Module *> entry_modules;

public:

	//
	// Error class
	//

	/// User error
	class Error : public misc::Error
	{
	public:

		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("Southern Islands timing simulator");
		}
	};




	//
	// Static fields
	//

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Parse the configuration file
	static void ParseConfiguration(misc::IniFile *ini_file);

	/// Return the simulation level set by command-line option
	/// '--si-sim-kind'.
	static comm::Arch::SimKind getSimKind() { return sim_kind; }




	//
	// Class members
	//
	
	/// Constructor
	Timing();

	/// Return unique instance of the X86 timing simulator singleton.
	static Timing *getInstance();

	/// Run one iteration of the Southern Islands timing simulator. See
	/// comm::Timing::Run() for details.
	bool Run() override;

	/// Dump a default memory configuration for the architecture. See
	/// comm::Timing::WriteMemoryConfiguration() for details.
	void WriteMemoryConfiguration(misc::IniFile *ini_file) override;

	/// Check architecture-specific requirements for the memory
	/// memory configuration provided in the INI file.
	/// See comm::Timing::CheckMemoryConfiguration() for details.
	void CheckMemoryConfiguration(misc::IniFile *ini_file) override;

	/// Parse an entry in the memory configuration file.
	/// See comm::Timing::ParseMemoryConfigurationEntry() for details.
	void ParseMemoryConfigurationEntry(misc::IniFile *ini_file,
			const std::string &section) override;
	
	/// Return the number of entry modules.
	/// See comm::Timing::getNumEntryModules() for details.
	int getNumEntryModules() override
	{
		return entry_modules.size();
	}

	/// Return an entry module.
	/// See comm::Timing::getEntryModule() for details.
	mem::Module *getEntryModule(int index) override
	{
		assert(index >= 0 && index < (int) entry_modules.size());
		return entry_modules[index];
	}
};


}

#endif

