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
	int tRC;
	int tRRD;
	int tRP;
	int tRFC;
	int tCCD;
	int tRTRS;
	int tCWD;
	int tWTR;
	int tCAS;
	int tRCD;
	int tOST;
	int tRAS;
	int tWR;
	int tRTP;
	int tBURST;

	/// Create a new TimingParameters instance with all parameters parsed
	/// from the configuration file MemoryController section.
	TimingParameters(misc::IniFile *ini_file, const std::string &section);

	/// Parse timings out of a MemoryController section of a dram
	/// configuration file.
	void ParseTiming(misc::IniFile *ini_file, const std::string &section);
};

}  // namespace dram

#endif
