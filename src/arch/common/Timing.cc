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

#include <lib/cpp/Error.h>

#include "Arch.h"
#include "Timing.h"


namespace comm
{

Timing::Timing(const std::string &name) :
		name(name)
{
	// Register timing simulator in architecture pool
	ArchPool *arch_pool = ArchPool::getInstance();
	arch_pool->RegisterTiming(name, this);
}


void Timing::ConfigureFrequencyDomain(int frequency)
{
	esim::Engine *engine = esim::Engine::getInstance();
	assert(!frequency_domain);
	frequency_domain = engine->RegisterFrequencyDomain(name, frequency);
}

	
void Timing::WriteMemoryConfiguration(misc::IniFile *ini_file)
{
	throw misc::Panic("Architecture does not have a default memory "
			"configuration");
}


void Timing::CheckMemoryConfiguration(misc::IniFile *ini_file)
{
	throw misc::Panic("Architecture does not have a memory configuration "
			"checker");
}


void Timing::ParseMemoryConfigurationEntry(misc::IniFile *ini_file,
			const std::string &section)
{
	throw misc::Panic("Architeture does not have a parser for the "
			"[Entry] section of the memory configuration file");
}


int Timing::getNumEntryModules()
{
	throw misc::Panic("Not implemented");
}


mem::Module *Timing::getEntryModule(int index)
{
	throw misc::Panic("Not implemented");
}


}

