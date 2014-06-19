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

#ifndef NETWORK_SYSTEM_H
#define NETWORK_SYSTEM_H

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>
#include <lib/cpp/IniFile.h>


namespace net
{


class System
{
	//
	// Configuration options
	//

	// Debugger file
	static std::string debug_file;

	// Configuration file
	static std::string config_file;

	// Stand-alone simulation duration.
	static long long max_cycles;



public:
	/// Debugger for network
	static misc::Debug debug;

	/// External Networks configuration file
	static misc::IniFile config;

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();
};

}  // namespace net

#endif
