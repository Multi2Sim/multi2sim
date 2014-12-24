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

namespace net
{

int System::net_system_frequency = 1000;


void System::ParseConfiguration(const std::string &path)
{
	misc::IniFile ini_file(path);
	System::debug << misc::fmt("Loading Network Configuration file \"%s\"\n",
			path.c_str());

	std::string section = "General";

	// Default Frequency
	ini_file.ReadInt(section, "Frequency", net_system_frequency);

	// First configuration look-up is for networks
	for (int i = 0; i < ini_file.getNumSections(); i++)
	{
		// Get section
		section = ini_file.getSection(i);

		std::vector<std::string> tokens;
		misc::StringTokenize(section, tokens, ".");

		if (tokens.size() != 2 ||
				strcasecmp(tokens[0].c_str(), "Network"))
			continue;

		std::string network_name = tokens[1];

		Network * net = new Network(network_name,
				section, ini_file);
		networks.emplace_back(net);
		network_map.emplace(network_name,net);
	}

}

}
