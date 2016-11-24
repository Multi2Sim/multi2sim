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

#ifndef DRAM_TIMING_PARAMETERS_H
#define DRAM_TIMING_PARAMETERS_H

#include <lib/cpp/IniFile.h>


namespace dram
{

class TimingParameters
{
	// These values are initialized in the contructor while parcing the
	// IniFile
	int time_rc;
	int time_rrd;
	int time_rp;
	int time_rfc;
	int time_ccd;
	int time_rtrs;
	int time_cwd;
	int time_wtr;
	int time_cas;
	int time_rcd;
	int time_ost;
	int time_ras;
	int time_wr;
	int time_rtp;
	int time_burst;

public:

	/// Create a new TimingParameters instance with all parameters parsed
	/// from the configuration file MemoryController section.
	TimingParameters(misc::IniFile *ini_file, const std::string &section);

	/// Parse timings out of a MemoryController section of a dram
	/// configuration file.
	void ParseTiming(misc::IniFile *ini_file, const std::string &section);
	
	// Getters for the timing values
	int getTimeRc() { return time_rc; }
	int getTimeRrd() { return time_rrd; }
	int getTimeRp() { return time_rp; }
	int getTimeRfc() { return time_rfc; }
	int getTimeCcd() { return time_ccd; }
	int getTimeRtrs() { return time_rtrs; }
	int getTimeCwd() { return time_cwd; }
	int getTimeWtr() { return time_wtr; }
	int getTimeCas() { return time_cas; }
	int getTimeRcd() { return time_rcd; }
	int getTimeOst() { return time_ost; }
	int getTimeRas() { return time_ras; }
	int getTimeWr() { return time_wr; }
	int getTimeRtp() { return time_rtp; }
	int getTimeBurst() { return time_burst; }
};

}  // namespace dram

#endif
