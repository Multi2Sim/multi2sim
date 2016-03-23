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

#include <lib/cpp/Misc.h>
#include <lib/cpp/Terminal.h>

#include "Arch.h"
#include "Emulator.h"
#include "Timing.h"


namespace comm
{


//
// Class ArchPool
//

std::unique_ptr<ArchPool> ArchPool::instance;


ArchPool *ArchPool::getInstance()
{
	// Return existing instance
	if (instance.get())
		return instance.get();
	
	// Create new architecture pool
	instance = misc::new_unique<ArchPool>();
	return instance.get();
}


Arch *ArchPool::Register(const std::string &name)
{
	// Check if architecture already exists, and return it if so.
	auto it = arch_map.find(name);
	if (it != arch_map.end())
		return it->second;
	
	// Create new architecture in place
	arch_list.emplace_back(misc::new_unique<Arch>(name));
	Arch *arch = arch_list.back().get();

	// Add to map
	arch_map[name] = arch;

	// Return created architecture
	return arch;
}


Arch *ArchPool::getByName(const std::string &name)
{
	// Search architecture
	for (auto &arch : arch_list)
		if (!strcasecmp(arch->getName().c_str(), name.c_str()))
			return arch.get();
	
	// Not found
	return nullptr;
}


std::string ArchPool::getArchNames()
{
	std::string comma;
	std::string names = "{";
	for (auto &arch : arch_list)
	{
		names += arch->getName() + comma;
		comma = "|";
	}
	names += "}";
	return names;
}


void ArchPool::Run(int &num_active_emulators, int &num_active_timing_simulators)
{
	// Reset active counters
	num_active_emulators = 0;
	num_active_timing_simulators = 0;

	// Run one iteration for each architecture
	for (auto &arch : arch_list)
	{
		switch (arch->getSimKind())
		{

		case Arch::SimFunctional:
		{
			// Get the emulatorlator. If none is available, skip this
			// architecture.
			Emulator *emulator = arch->getEmulator();
			if (!emulator)
				continue;

			// Run
			bool active = emulator->Run();
			arch->setActive(active);
                           
			// Increase number of active emulatorlations if the architecture
			// actually performed a useful emulatorlation iteration.
			if (active)
				num_active_emulators++;

			// Done
			break;
		}
		
		case Arch::SimDetailed:
		{
			// Get the timing simulator. If none is available, skip
			// this architecture.
			Timing *timing = arch->getTiming();
			assert(timing);

			// Get the architecture's frequency domain
			esim::FrequencyDomain *frequency_domain = timing->getFrequencyDomain();
			assert(frequency_domain);

			// Check whether the architecture should actually run an
			// iteration. If it is working at a slower frequency than
			// the main simulation loop, we must skip this call.
			long long cycle = frequency_domain->getCycle();
			if (cycle == timing->getLastSimulationCycle())
			{
				// We skip it, but the timing simulation is still
				// considered to be active.
				//
				// NOTE: This statement was added after finding
				// the problem that the simulation ends right
				// away when the frequency of all architectures
				// is lower than the memory.
				num_active_timing_simulators++;
				continue;
			}

			// Run
			bool active = timing->Run();
			arch->setActive(active);

			// ... but only update the last timing simulation cycle
			// if there was an effective execution of the iteration
			// loop. Otherwise, there is a deadlock: ESim time
			// will not advance (no call to esim::ProcessEvents)
			// because no architecture ran, and no architecture will
			// run because ESim time did not advance.
			if (active)
			{
				timing->SaveLastSimulationCycle();
				num_active_timing_simulators++;
			}

			// Done
			break;
		}

		default:

			throw misc::Panic("Invalid simulation kind");
		}
	}
}


void ArchPool::DumpSummary(std::ostream &os) const
{
	// Print in blue
	misc::Terminal::Blue(os);

	// Dump summary for each architecture
	for (auto &arch : arch_list)
	{
		// Get the emulator
		Emulator *emulator = arch->getEmulator();

		// Header
		os << "[ " << arch->getName() << " ]\n";

		// Print statistics summary for emulator even if the number
		// of executing instructions are zero
		emulator->DumpSummary(os);

		// Statistics summary for timing simulator
		if (arch->getSimKind() == Arch::SimDetailed)
		{
			Timing *timing = arch->getTiming();
			assert(timing);
			timing->DumpSummary(os);
		}

		// New line
		os << '\n';
	}

	// Reset color
	misc::Terminal::Reset(os);
}
	

void ArchPool::DumpReports()
{
	// Dump reports for each Timing simulator in ArchPool
	for (auto it = getTimingBegin(), end = getTimingEnd(); it != end; it++)
		(*it)->getTiming()->DumpReport();
}


//
// Class Arch
//

const misc::StringMap Arch::SimKindMap =
{
	{ "functional", SimFunctional },
	{ "detailed", SimDetailed }
};


}  // namespace comm
