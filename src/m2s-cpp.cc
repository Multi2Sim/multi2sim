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
	/////////////////
	return;
	/////////////////

	// Read command line
	CommandLine command_line(argc, argv);
	command_line.setErrorMessage("Please type 'm2s --help' for a list of "
			"valid Multi2Sim command-line options.\n");

	// Command-line option to run C++ version of Multi2Sim
	bool cpp_multi2sim = false;
	command_line.Register("--cpp", cpp_multi2sim,
			"Run C++ version of Multi2Sim");

	// Test command-line option of type 'int'
	int x = 0;
	command_line.RegisterInt32("--x", x, "Help for x");

	// Process command line
	command_line.Process();

	// Finish if C++ version of Multi2Sim is not activated
	if (!cpp_multi2sim)
		return;

	// Multi2Sim C++
	std::cout << "This is C++ Multi2Sim\n";
	std::cout << "x = " << x << '\n';

	// Rest
	std::cout << "Program name = '" << command_line.getProgramName() << "'\n";
	for (int i = 0; i < command_line.getNumArguments(); i++)
		std::cout << "Argument " << i << " = '" <<
				command_line.getArgument(i) << "'\n";

	// End
	exit(0);
}

