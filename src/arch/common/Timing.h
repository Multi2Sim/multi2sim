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
#include <lib/esim/FrequencyDomain.h>
#include <lib/esim/Engine.h>


namespace comm
{

class Timing
{

	// Frequency domain pointer
	esim::FrequencyDomain *frequency_domain = nullptr;

public:

	/// Run one cycle for the timing simulator. This function returns \a
	/// true if a valid simulation was performed by the architecture. The
	/// function must be implemented by every derived class.
	virtual bool Run() = 0;

	/// Configure the frequency domain
	void setFrequencyDomain(const std::string &name, int frequency)
	{
		esim::Engine *engine = esim::Engine::getInstance();
		frequency_domain = engine->RegisterFrequencyDomain(name, frequency);
	}

	/// Return the frequency domain identifier
	esim::FrequencyDomain *getFrequencyDomain() { return frequency_domain; }

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

