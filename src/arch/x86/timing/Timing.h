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

#include <arch/common/Timing.h>

#include "CPU.h"

namespace x86
{

// Forward declaration
class CPU;

// Class Timing
class Timing : public comm::Timing
{
	// Unique instance of the singleton
	static std::unique_ptr<Timing> instance;

	// CPU associated with this timing simulator
	std::unique_ptr<CPU> cpu;

	// Configuration file name
	static std::string config_file;

	// Report file name
	static std::string report_file;

	// MMU report file name
	static std::string mmu_report_file;

	// Show CPU configuration file
	static bool help;

	// Message to display with '--x86-help'
	static const std::string help_message;

	// Flag to control whether the fast forward of OpenCL is enbaled
	static bool opencl_fast_forward;

	// Private constructor for singleton
	Timing();

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

	/// Create new CPU instance
	void NewCPU();

	/// Run one iteration of the cpu timing simuation.
	/// \return This function \c true if the iteration had a useful
	/// timing simulation, and \c false if all timing simulation finished execution.
	bool Run();

	/// Dump a default memory configuration for the architecture. This
	/// function is invoked by the memory system configuration parser when
	/// no specific memory configuration is given by the user for the
	/// architecture.
	void WriteMemoryConfiguration(misc::IniFile *ini_file);

	/// Check architecture-specific requirements for the memory
	/// memory configuration provided in the INI file. This function is
	/// invoked by the memory configuration parser.
	void CheckMemoryConfiguration(misc::IniFile *ini_file);

	//
	// Debuggers and configuration
	//

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Parse the configuration file
	void ParseConfiguration(std::string &config_file);
};

} //namespace x86

#endif // X86_ARCH_TIMING_TIMING_H
