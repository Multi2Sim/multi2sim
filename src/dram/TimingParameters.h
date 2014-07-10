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

#ifndef DRAM_TIMINGPARAMETERS_H
#define DRAM_TIMINGPARAMETERS_H

#include <lib/cpp/IniFile.h>


namespace dram
{

struct TimingParameters
{
	int tRC = 0;
	int tRRD = 0;
	int tRP = 0;
	int tRFC = 0;
	int tCCD = 0;
	int tRTRS = 0;
	int tCWD = 0;
	int tWTR = 0;
	int tCAS = 0;
	int tRCD = 0;
	int tOST = 0;
	int tRAS = 0;
	int tWR = 0;
	int tRTP = 0;
	int tBURST = 0;

	/// Create a new TimingParameters instance with all parameters set to 0
	TimingParameters()
	{
	}

	/// Create a new TimingParameters instance with all parameters
	/// parsed from the configuration file MemoryController section.
	TimingParameters(const std::string &section,
			misc::IniFile &ini_file);

	/// Parse timings out of a MemoryController section of a dram
	/// configuration file.
	void ParseTiming(const std::string &section,
			misc::IniFile &ini_file);

	/// Set the parameters to what they would in a system with DRR3-1600.
	void DefaultDDR3_1600();
};

}  // namespace dram

#endif
