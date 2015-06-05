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

#include <lib/cpp/Misc.h>

#include "Network.h"

namespace net
{

int System::net_system_frequency = 1000;

void System::ParseConfiguration(misc::IniFile *ini_file)
{
	// Debug
	System::debug << ini_file->getPath() << ": Loading network "
			"Configuration file\n";

	// Default Frequency
	std::string section = "General";
	ini_file->ReadInt(section, "Frequency", net_system_frequency);

	// First configuration look-up is for networks
	for (int i = 0; i < ini_file->getNumSections(); i++)
	{
		// Get section
		section = ini_file->getSection(i);

		// Split section name in tokens
		std::vector<std::string> tokens;
		misc::StringTokenize(section, tokens, ".");

		// Skip if it is not a network section
		if (tokens.size() != 2 || misc::StringCaseCompare(tokens[0],
				"Network"))
			continue;

		// Check that there is not network with that name
		std::string name = tokens[1];
		if (network_map.find(name) != network_map.end())
			throw Error(misc::fmt("%s: %s: Duplicated network",
					ini_file->getPath().c_str(),
					name.c_str()));

		// Create network
		networks.emplace_back(misc::new_unique<Network>(name));
		Network *network = networks.back().get();
		network_map[name] = network;
		network->ParseConfiguration(ini_file, section);
	}
}


}

