/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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
 *  You should have received as copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_COMMON_TIMING_H
#define ARCH_COMMON_TIMING_H

#include <lib/cpp/IniFile.h>


namespace comm
{

class Timing
{
	// Current cycle
	long long cycle;

	// Frequency domain info
	int frequency;
	int frequency_domain;

public:

	/// Run one cycle for the timing simulator. This function returns \a
	/// true if a valid simulation was performed by the architecture. The
	/// function must be implemented by every derived class.
	virtual bool Run() = 0;

	/// Set the frequency domain identifier where this architecture
	/// belongs to.
	void setFrequencyDomain(int config_frequency_domain) { frequency_domain = config_frequency_domain; }

	/// Set the frequency identifier where this architecture
	/// belongs to.
	void setFrequency(int config_frequency) { frequency = config_frequency; }

	/// Return the frequency domain identifier where this architecture
	/// belongs to.
	int getFrequencyDomain() const { return frequency_domain; }

	/// Return the frequency identifier where this architecture
	/// belongs to.
	int getFrequency() const { return frequency; }

	/// Increase the cycle
	void IncCycle() { cycle++; }

	/// Return the cycle
	long long getCycle() const { return cycle; }

	/// Dump a default memory configuration for the architecture. This
	/// function is invoked by the memory system configuration parser when
	/// no specific memory configuration is given by the user for the
	/// architecture.
	virtual void WriteMemoryConfiguration(misc::IniFile *ini_file);

	/// Check architecture-specific requirements for the memory
	/// memory configuration provided in the INI file. This function is
	/// invoked by the memory configuration parser.
	virtual void CheckMemoryConfiguration(misc::IniFile *ini_file);
};

}

#endif

