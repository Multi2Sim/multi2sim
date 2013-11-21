/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include <cstdlib>
#include <iostream>

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Misc.h>

#include "Wrapper.h"

using namespace misc;


void main_cpp(int argc, char **argv)
{
	// Read command line
	CommandLine command_line(argc, argv);
	command_line.setErrorMessage("Please type 'm2s --help' for a list of "
			"valid Multi2Sim command-line options.\n");

	// Command-line option to run C++ version of Multi2Sim
	bool cpp_multi2sim = false;
	command_line.Register("--cpp", cpp_multi2sim,
			"Run C++ version of Multi2Sim");

	// Register three sample command-line options
	long long m2s_max_time = 0;
	command_line.RegisterInt64("--max-time", m2s_max_time,
			"Maximum simulation time in seconds. The simulator "
			"will stop once this time is exceeded. A value of 0 "
			"(default) means no time limit.");
	
	std::string m2s_trace_file;
	command_line.RegisterString("--trace", m2s_trace_file,
			"Generate a trace file with debug information on the "
			"configuration of the modeled CPUs, GPUs, and memory "
			"system, as well as their dynamic simulation. The "
			"trace is a compressed plain-text file in format. The "
			"user should watch the size of the generated trace as "
			"simulation runs, since the trace file can quickly "
			"become extremely large.");
	
	std::string m2s_visual_file;
	command_line.RegisterString("--visual", m2s_visual_file,
			"Run the Multi2Sim Visualization Tool. This option "
			"consumes a file generated with the '--trace' option "
			"in a previous simulation. This option is only "
			"available on systems with support for GTK 3.0 or "
			"higher.");

	bool m2s_show_help = false;
	command_line.Register("--help", m2s_show_help,
			"Show this help message.");

	// Process command line. Return to C version of Multi2Sim if a
	// command-line option was not recognized.
	if (!command_line.Process(false))
		return;

	// Finish if C++ version of Multi2Sim is not activated
	if (!cpp_multi2sim)
		return;

	// Show help message
	if (m2s_show_help)
	{
		command_line.Help(std::cout);
		exit(0);
	}

	// Multi2Sim C++
	std::cerr << "; Multi2Sim C++\n";
	std::cerr << "m2s_max_time = " << m2s_max_time << '\n';
	std::cerr << "m2s_trace_file = " << m2s_trace_file << '\n';
	std::cerr << "m2s_visual_file = " << m2s_visual_file << '\n';

	// End
	exit(0);
}

