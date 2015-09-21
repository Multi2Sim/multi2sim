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


TimingParameters::TimingParameters(misc::IniFile *ini_file,
		const std::string &section)
{
	// Parse the configuration file section
	ParseTiming(ini_file, section);
}


void TimingParameters::ParseTiming(misc::IniFile *ini_file,
		const std::string &section)
{
	// Set the timing parameters if present, else default to timings of 
	// a typical DDR3 device at 1600Mhz, the default frequency
	time_rc = ini_file->ReadInt(section, "tRC", 49);
	time_rrd = ini_file->ReadInt(section, "tRRD", 5);
	time_rp = ini_file->ReadInt(section, "tRP", 11);
	time_rfc = ini_file->ReadInt(section, "tRFC", 128);
	time_ccd = ini_file->ReadInt(section, "tCCD", 4);
	time_rtrs = ini_file->ReadInt(section, "tRTRS", 1);
	time_cwd = ini_file->ReadInt(section, "tCWD", 5);
	time_wtr = ini_file->ReadInt(section, "tWTR", 6);
	time_cas = ini_file->ReadInt(section, "tCAS", 11);
	time_rcd = ini_file->ReadInt(section, "tRCD", 11);
	time_ost = ini_file->ReadInt(section, "tOST", 1);
	time_ras = ini_file->ReadInt(section, "tRAS", 28);
	time_wr = ini_file->ReadInt(section, "tWR", 12);
	time_rtp = ini_file->ReadInt(section, "tRTP", 6);
	time_burst = ini_file->ReadInt(section, "tBURST", 4);
}


}  // namespace dram
