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

#include "Arch.h"
#include "Emu.h"
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
	instance.reset(new ArchPool());
	return instance.get();
}
	
void ArchPool::Register(const std::string &name,
		Asm *as,
		Emu *emu,
		Timing *timing)
{
	// Create new architecture in place
	Arch *arch = new Arch(name, as, emu, timing);
	arch_list.emplace_back(arch);
}

void ArchPool::Run(int &num_emu_active, int &num_timing_active)
{
	// Reset active counters
	num_emu_active = 0;
	num_timing_active = 0;

	// Run one iteration for each architecture
	for (auto &arch : arch_list)
	{
		switch (arch->getSimKind())
		{
		case Arch::SimFunctional:
		{
			// Get the emulator. If none is available, skip this
			// architecture.
			Emu *emu = arch->getEmu();
			if (!emu)
				continue;


			// Run
			bool active = emu->Run();
			arch->setActive(active);
                           
			// Increase number of active emulations if the architecture
			// actually performed a useful emulation iteration.
			if (active)
				num_emu_active++;

			// Done
			break;
		}
		
		case Arch::SimDetailed:
		{
			throw misc::Panic("Timing simulation not supported");
#if 0
			// Get the timing simulator. If none is available, skip
			// this architecture.
			Timing *timing = arch->getTiming();
			if (!timing)
				continue;

			// Check whether the architecture should actually run an
			// iteration. If it is working at a slower frequency than
			// the main simulation loop, we must skip this call.
			int frequency_domain = timing->getFrequencyDomain();
			unsigned long long cycle = esim_domain_cycle(frequency_domain);
			if (cycle == arch->last_timing_cycle)
				continue;

			// Run
			bool active = timing->Run(timing);
			arch->setActive(active);

			// ... but only update the last timing simulation cycle
			// if there was an effective execution of the iteration
			// loop. Otherwise, there is a deadlock: 'esim_time'
			// will not advance (no call to 'esim_process_events')
			// because no architecture ran, and no architecture will
			// run because 'esim_time' did not advance.
			if (active)
			{
				arch->last_timing_cycle = cycle;
				num_timing_active++;
			}

			// Done
			break;
#endif
		}

		default:

			throw misc::Panic("Invalid simulation kind");
		}
	}
}
	



//
// Class Arch
//

const misc::StringMap Arch::SimKindMap =
{
	{ "functional", SimFunctional },
	{ "detailed", SimDetailed }
};


Arch::Arch(const std::string &name, Asm *as, Emu *emu, Timing *timing) :
		name(name),
		as(as),
		emu(emu),
		timing(timing)
{
}



}  // namespace comm
