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

#include <arch/southern-islands/asm/Wrapper.h>
#include <arch/southern-islands/asm/opengl-bin-file.h>
#include <arch/x86/emu/emu.h>
#include <driver/opencl/opencl.h>
#include <driver/opengl/opengl.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/repos.h>
#include <lib/util/string.h>
#include <memory/memory.h>

#include "emu.h"
#include "isa.h"
#include "ndrange.h"
#include "sx.h"
#include "wavefront.h"
#include "work-group.h"


/*
 * Class 'SIEmu'
 */

void SIEmuCreate(SIEmu *self, struct SIAsmWrap *as)
{
	/* Parent */
	EmuCreate(asEmu(self), "SouthernIslands");

	/* Initialize */
	self->as = as;
	self->video_mem = mem_create();
	self->video_mem->safe = 1;
	self->video_mem_top = 0x80000000;  /////////////////////////
	
	/* Set global memory to video memory by default */
	self->global_mem = self->video_mem;

	/* Shader Export module */
	self->sx = new(SISX, self);

	/* Repository of deferred tasks */
	self->write_task_repos = repos_create(sizeof(struct si_isa_write_task_t),
		"SIEmu.write_task_repos");
	
	/* Virtual functions */
	asObject(self)->Dump = SIEmuDump;
	asEmu(self)->DumpSummary = SIEmuDumpSummary;
	asEmu(self)->Run = SIEmuRun;
}


void SIEmuDestroy(SIEmu *self)
{
	/* Free emulator memory */
	mem_free(self->video_mem);

	/* Free SX module */
	delete(self->sx);

	/* Repository of deferred tasks */
	repos_free(self->write_task_repos);
}


void SIEmuDump(Object *self, FILE *f)
{
	/* Call parent */
	EmuDump(self, f);
}


void SIEmuDumpSummary(Emu *self, FILE *f)
{
	SIEmu *emu = asSIEmu(self);

	/* Call parent */
	EmuDumpSummary(self, f);

	/* More statistics */
	fprintf(f, "NDRangeCount = %d\n", emu->ndrange_count);
	fprintf(f, "WorkGroupCount = %lld\n", emu->work_group_count);
	fprintf(f, "BranchInstructions = %lld\n", emu->branch_inst_count);
	fprintf(f, "LDSInstructions = %lld\n", emu->lds_inst_count);
	fprintf(f, "ScalarALUInstructions = %lld\n", emu->scalar_alu_inst_count);
	fprintf(f, "ScalarMemInstructions = %lld\n", emu->scalar_mem_inst_count);
	fprintf(f, "VectorALUInstructions = %lld\n", emu->vector_alu_inst_count);
	fprintf(f, "VectorMemInstructions = %lld\n", emu->vector_mem_inst_count);
}


int SIEmuRun(Emu *self)
{
	SIEmu *emu = asSIEmu(self);
	OpenclDriver *opencl_driver;
	SINDRange *ndrange;
	SIWavefront *wavefront;
	SIWorkGroup *work_group;
	struct list_t *ndrange_list;

	int ndrange_index;
	int wg_index;
	int wavefront_id;
	long work_group_id;

	opencl_driver = emu->opencl_driver;
	assert(opencl_driver);

#ifdef HAVE_OPENGL
	OpenglDriver *opengl_driver;
	opengl_driver = emu->opengl_driver;
	assert(opengl_driver);

	/* 
	 * FIXME : need to find a better way to decide which NDRange list is 
	 * in use, especially when we need to run CL/GL mixed programs 
	 */

	/* For efficiency when no Southern Islands emulation is selected, 
	 * exit here if the list of existing ND-Ranges is empty. */
	if (!list_count(opencl_driver->si_ndrange_list))
	{
		if (list_count(opengl_driver->opengl_si_ndrange_list))
			ndrange_list = opengl_driver->opengl_si_ndrange_list;
		else
			return FALSE;		
	}
	else
		ndrange_list = opencl_driver->si_ndrange_list;
#else
	if (!list_count(opencl_driver->si_ndrange_list))
		return FALSE;		
	ndrange_list = opencl_driver->si_ndrange_list;
#endif 

	/* Iterate over each nd-range */
	LIST_FOR_EACH(ndrange_list, ndrange_index)
	{
		ndrange = list_get(ndrange_list, 
			ndrange_index);

		/* Move waiting work groups to running work groups */
		if (list_count(ndrange->waiting_work_groups))
		{
			work_group_id = (long)list_dequeue(
				ndrange->waiting_work_groups);
			list_enqueue(ndrange->running_work_groups, 
				(void*)work_group_id);
		}

		/* If there's no work groups to run, go to next nd-range */
		if (!list_count(ndrange->running_work_groups))
			continue;

		/* Iterate over running work groups */
		LIST_FOR_EACH(ndrange->running_work_groups, wg_index)
		{
			/* Instantiate the next work-group */
			work_group_id = (long) list_dequeue(
				ndrange->running_work_groups);
			work_group = new(SIWorkGroup, work_group_id, ndrange);

			/* Execute the work-group to completion */
			while (!work_group->finished_emu)
			{
				SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, 
					wavefront_id)
				{
					wavefront = work_group->wavefronts[
						wavefront_id];

					if (wavefront->finished || 
						wavefront->at_barrier)
					{
						continue;
					}

					/* Execute instruction in wavefront */
					SIWavefrontExecute(wavefront);
				}
			}

			/* Free work group */
			delete(work_group);
		}

		/* Let driver know that all work-groups from this nd-range
		 * have been run */
		if (opencl_driver && (ndrange_list == opencl_driver->si_ndrange_list))
		{
			OpenclDriverRequestWork(opencl_driver, ndrange);
		}

#ifdef HAVE_OPENGL
		/* Let driver know that all work-groups from this nd-range
		 * have been run */
		if (opengl_driver && (ndrange_list == opengl_driver->opengl_si_ndrange_list))
		{
			OpenglDriverRequestWork(opengl_driver, ndrange);
		}
#endif

	}

	/* Still emulating */
	return TRUE;
}



/*
 * Public
 */

long long si_emu_max_cycles = 0;
long long si_emu_max_inst = 0;
int si_emu_max_kernels = 0;

char *si_emu_opengl_binary_name = "";

int si_emu_wavefront_size = 64;
int si_emu_num_mapped_const_buffers = 2;  /* CB0, CB1 by default */

