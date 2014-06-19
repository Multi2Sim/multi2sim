/*
 *  Multi2Sim
 *  Copyright (C) 2014  Amir Kavyan Ziabari (aziabari@ece.neu.edu)
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

#include "System.h"

namespace net
{

std::string System::debug_file;

// maximum number of cycmes
long long System::max_cycles;

void System::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();
	// Category
	command_line->setCategory("net");

	// Debug information
	command_line->RegisterString("--net-debug <file>", debug_file,
			"Debug information related with interconnection "
			"networks including packet transfers, link usage, etc");

	// Configuration Inifile for External Networks
	command_line->RegisterString("--net-config <file>", config_file,
			"Network configuration file. Networks in the memory "
			"hierarchy can be defined here and referenced in other"
			" configuration files. For a description of the format,"
			" use option '--net-help'");

	// Maximum simulation time in cycles
	command_line->RegisterInt64("--net-max-cycles <number> (default = 1M)",
			max_cycles,
			"Maximum number of cycles for network simulation."
			"together with option '--net-sim.");



}

misc::Debug System::debug;

misc::IniFile System::config;

void System::ProcessOptions()
{
	// Debugger
	if (debug_file.empty())
		debug.setPath(debug_file);

}

}
