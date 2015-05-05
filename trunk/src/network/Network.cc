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

#include <cstring>
#include <csignal>

#include "Network.h"

namespace net
{

Network::Network(const std::string &name)
:
				name(name)
{

}


void Network::ParseConfiguration(const std::string &section,
		misc::IniFile &config)
{
	// Non-empty values
	default_output_buffer_size = config.ReadInt(section,
			"DefaultOutputBufferSize", 0);
	default_input_buffer_size = config.ReadInt(section,
			"DefaultInputBufferSize",0);
	default_bandwidth = config.ReadInt(section,
			"DefaultBandwidth",0);
	if (!default_output_buffer_size || !default_input_buffer_size ||
			!default_bandwidth)
	{
		throw misc::Error(misc::fmt(
				"%s:%s:\nDefault values can not be "
				"zero/non-existent.\n", __FILE__, 
				__FUNCTION__));
	}

	// Packet size and frequency
	packet_size = config.ReadInt(section, "DefaultPacketSize", 0);
	net_frequency = config.ReadInt(section, "Frequency", 0);

	// Debug information
	System::debug << misc::fmt("Network found: %s\n",name.c_str());
}


void Network::Dump(std::ostream &os = std::cout) const
{
	// Dump network information
	os << misc::fmt("\n***** Network %s *****\n", name.c_str());
	os << misc::fmt("\tDefault input buffer size: %d\n", 
			default_input_buffer_size);
	os << misc::fmt("\tDefault output buffer size: %d\n", 
			default_output_buffer_size);
	os << misc::fmt("\tDefault bandwidth: %d\n", 
			default_bandwidth);
}

}
