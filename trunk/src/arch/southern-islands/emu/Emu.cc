/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <arch/southern-islands/emu/WorkGroup.h>
#include <arch/southern-islands/emu/Wavefront.h>
#include <arch/southern-islands/emu/WorkItem.h>
#include <src/driver/opencl/OpenCLDriver.h>

#include "Emu.h"
#include "NDRange.h"

namespace SI
{

Emu::Emu(Asm *as)
{
	// Disassemler
	this->as = as;
	
	// GPU memories
	this->video_mem.reset(new Memory::Memory());
	this->shared_mem.reset(new Memory::Memory());
	this->global_mem = video_mem.get();
}


void Emu::Dump(std::ostream &os) const
{
	// FIXME: basic statistics, such as instructions, time...

	// More statistics 
	os << "NDRangeCount = " << ndrange_count << std::endl;
	os << "WorkGroupCount = " << work_group_count << std::endl;
	os << "BranchInstructions = " << branch_inst_count << std::endl;
	os << "LDSInstructions = " << lds_inst_count << std::endl;
	os << "ScalarALUInstructions = " << scalar_alu_inst_count << std::endl;
	os << "ScalarMemInstructions = " << scalar_mem_inst_count << std::endl;
	os << "VectorALUInstructions = " << vector_alu_inst_count << std::endl;
	os << "VectorMemInstructions = " << vector_mem_inst_count << std::endl;
}

void Emu::Run()
{

	// For efficiency when no Southern Islands emulation is selected, 
	// exit here if the list of existing ND-Ranges is empty. 
	if (opencl_driver->isNDRangeListEmpty())
		return;

	for (auto ndr_i = opencl_driver->NDRangesBegin(), 
		ndr_e = opencl_driver->NDRangesEnd(); ndr_i < ndr_e; ++ndr_i)
	{
		// Move waiting work groups to running work groups 
		(*ndr_i)->WaitingToRunning();

		// If there's no work groups to run, go to next nd-range 
		if ((*ndr_i)->isRunningWorkGroupsEmpty())
			continue;

		// Iterate over running work groups
		for (auto wg_i = (*ndr_i)->RunningWorkGroupBegin(), 
			wg_e = (*ndr_i)->RunningWorkGroupEnd(); wg_i != wg_e; ++wg_i)
		{
			// FIXME: workgroups are instantiated in driver
			// might need to instantiate at here as in C version

			for (auto wf_i = (*wg_i)->WavefrontsBegin(), 
				wf_e = (*wg_i)->WavefrontsEnd(); wf_i != wf_e; ++wf_i)
			{
				(*wf_i)->Execute();
			}
		}

		// Let driver know that all work-groups from this nd-range
		// have been run
		opencl_driver->RequestWork((*ndr_i).get());
	}
}

}  // namespace SI
