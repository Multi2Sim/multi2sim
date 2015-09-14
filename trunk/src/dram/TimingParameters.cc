/*
 * Multi2Sim
 * Copyright (C) 2014 Agamemnon Despopoulos (agdespopoulos@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include "TimingParameters.h"


namespace dram
{


TimingParameters::TimingParameters(const std::string &section,
		misc::IniFile &ini_file)
{
	// Parse the configuration file section
	ParseTiming(section, ini_file);
}


void TimingParameters::ParseTiming(const std::string &section,
		misc::IniFile &ini_file)
{
	// Set the timing parameters if present, else default to timings of 
	// a typical DDR3 device at 1600Mhz, the default frequency
	tRC = ini_file.ReadInt(section, "tRC", 49);
	tRRD = ini_file.ReadInt(section, "tRRD", 5);
	tRP = ini_file.ReadInt(section, "tRP", 11);
	tRFC = ini_file.ReadInt(section, "tRFC", 128);
	tCCD = ini_file.ReadInt(section, "tCCD", 4);
	tRTRS = ini_file.ReadInt(section, "tRTRS", 1);
	tCWD = ini_file.ReadInt(section, "tCWD", 5);
	tWTR = ini_file.ReadInt(section, "tWTR", 6);
	tCAS = ini_file.ReadInt(section, "tCAS", 11);
	tRCD = ini_file.ReadInt(section, "tRCD", 11);
	tOST = ini_file.ReadInt(section, "tOST", 1);
	tRAS = ini_file.ReadInt(section, "tRAS", 28);
	tWR = ini_file.ReadInt(section, "tWR", 12);
	tRTP = ini_file.ReadInt(section, "tRTP", 6);
	tBURST = ini_file.ReadInt(section, "tBURST", 4);
}


}  // namespace dram
