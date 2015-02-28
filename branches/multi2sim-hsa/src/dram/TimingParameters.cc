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
	// Set the default timing parameters if set.  If a default is not set,
	// the parameters are all set to 0 and the user must set all of them.
	std::string set_default = ini_file.ReadString(section, "Default", "");
	if (set_default == "DDR3_1600")
		DefaultDDR3_1600();

	// Read the timing parameters set by the user.
	tRC = ini_file.ReadInt(section, "tRC", tRC);
	tRRD = ini_file.ReadInt(section, "tRRD", tRRD);
	tRP = ini_file.ReadInt(section, "tRP", tRP);
	tRFC = ini_file.ReadInt(section, "tRFC", tRFC);
	tCCD = ini_file.ReadInt(section, "tCCD", tCCD);
	tRTRS = ini_file.ReadInt(section, "tRTRS", tRTRS);
	tCWD = ini_file.ReadInt(section, "tCWD", tCWD);
	tWTR = ini_file.ReadInt(section, "tWTR", tWTR);
	tCAS = ini_file.ReadInt(section, "tCAS", tCAS);
	tRCD = ini_file.ReadInt(section, "tRCD", tRCD);
	tOST = ini_file.ReadInt(section, "tOST", tOST);
	tRAS = ini_file.ReadInt(section, "tRAS", tRAS);
	tWR = ini_file.ReadInt(section, "tWR", tWR);
	tRTP = ini_file.ReadInt(section, "tRTP", tRTP);
	tBURST = ini_file.ReadInt(section, "tBURST", tBURST);
}


void TimingParameters::DefaultDDR3_1600()
{
	tRC = 49;
	tRRD = 5;
	tRP = 11;
	tRFC = 128;
	tCCD = 4;
	tRTRS = 1;
	tCWD = 5;
	tWTR = 6;
	tCAS = 11;
	tRCD = 11;
	tOST = 1;
	tRAS = 28;
	tWR = 12;
	tRTP = 6;
	tBURST = 4;
}


}  // namespace dram
