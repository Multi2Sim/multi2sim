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

#include <arch/southern-islands/asm/Asm.h>
#include <arch/southern-islands/emu/WorkGroup.h>
#include <arch/southern-islands/emu/Wavefront.h>
#include <arch/southern-islands/emu/WorkItem.h>
#include <driver/opencl/OpenCLDriver.h>
#include <driver/opengl/OpenGLDriver.h>
#include <driver/opengl/southern-islands/ShaderExport.h>
#include <lib/cpp/ELFReader.h>
#include <lib/cpp/Misc.h>


#include "Emu.h"
#include "NDRange.h"

namespace SI
{

// Singleton
std::unique_ptr<Emu> Emu::instance;

// Debugger
misc::Debug Emu::debug;

// Configuration
EmuConfig Emu::config;

Emu *Emu::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Emu());
	return instance.get();	
}

Emu::Emu()
{
	// Disassemler
	this->as.reset(new Asm());
	
	// GPU memories
	this->video_mem.reset(new mem::Memory());
	this->video_mem->setSafe(true);

	this->shared_mem.reset(new mem::Memory());
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

	// NDRange list is shared by CL/GL driver
	for (auto ndr_i = opencl_driver->getNDRangeBegin(); 
		ndr_i < opencl_driver->getNDRangeEnd(); ++ndr_i)
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
			std::unique_ptr<WorkGroup> workgroup(new WorkGroup((*ndr_i).get(), (*wg_i)));

			for (auto wf_i = workgroup->WavefrontsBegin(), 
				wf_e = workgroup->WavefrontsEnd(); wf_i != wf_e; ++wf_i)
				(*wf_i)->Execute();

			workgroup.reset();
		}

#ifdef HAVE_OPENGL
		// Notify corresponding driver
		if ((*ndr_i)->getStage() == NDRangeStageCompute)
		{
			// Let OpenCL driver know that all work-groups from this nd-range
			// have been run
//			opencl_driver->RequestWork((*ndr_i).get());
		}
		else
			opengl_driver->RequestWork((*ndr_i).get());
#else
		// Let OpenCL driver know that all work-groups from this nd-range
		// have been run
		// opencl_driver->RequestWork((*ndr_i).get());
#endif
	}
}

}  // namespace SI
