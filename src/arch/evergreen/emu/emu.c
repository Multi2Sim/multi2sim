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

#include <driver/opencl-old/evergreen/device.h>
#include <driver/opencl-old/evergreen/platform.h>
#include <driver/opencl-old/evergreen/repo.h>
#include <lib/esim/esim.h>
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
#include "machine.h"
#include "ndrange.h"
#include "work-group.h"


/*
 * Public
 */


long long evg_emu_max_cycles;
long long evg_emu_max_inst;
int evg_emu_max_kernels;

char *evg_emu_opencl_binary_name = "";
char *evg_emu_report_file_name = "";
FILE *evg_emu_report_file = NULL;

int evg_emu_wavefront_size = 64;




/*
 * Class 'EvgEmu'
 */

void EvgEmuCreate(EvgEmu *self, EvgAsm *as)
{
	/* Parent */
	EmuCreate(asEmu(self), "Evergreen");

	/* Initialize */
	self->as = as;

	/* Instruction execution functions */
#define DEFINST(_name, _fmt_str, _fmt0, _fmt1, _fmt2, _category, _opcode, _flags) \
	self->inst_func[EVG_INST_##_name] = evg_isa_##_name##_impl;
#include <arch/evergreen/asm/asm.dat>
#undef DEFINST

	/* Memories */
	self->const_mem = mem_create();
	self->const_mem->safe = 0;
	self->global_mem = mem_create();
	self->global_mem->safe = 0;

	/* Repository of deferred tasks */
	self->write_task_repos = repos_create(sizeof(struct evg_isa_write_task_t),
		"evg_emu->write_task_repos");

	/* Open report file */
	if (*evg_emu_report_file_name)
	{
		evg_emu_report_file = file_open_for_write(evg_emu_report_file_name);
		if (!evg_emu_report_file)
			fatal("%s: cannot open report for Evergreen emulator",
				evg_emu_report_file_name);
	}

	/* Virtual functions */
	asObject(self)->Dump = EvgEmuDump;
	asEmu(self)->DumpSummary = EvgEmuDumpSummary;
	asEmu(self)->Run = EvgEmuRun;
}


void EvgEmuDestroy(EvgEmu *self)
{
	/* Free ND-Ranges */
	while (self->ndrange_list_count)
		delete(self->ndrange_list_head);

	/* Finalize GPU kernel */
	mem_free(self->const_mem);
	mem_free(self->global_mem);

	/* Repository of deferred tasks */
	repos_free(self->write_task_repos);

	/* Emulator report */
	if (evg_emu_report_file)
		fclose(evg_emu_report_file);
}


void EvgEmuDump(Object *self, FILE *f)
{
	/* Call parent */
	EmuDump(self, f);
}


void EvgEmuDumpSummary(Emu *self, FILE *f)
{
	EvgEmu *emu = asEvgEmu(self);

	/* Call parent */
	EmuDumpSummary(self, f);

	/* More statistics */
	fprintf(f, "NDRangeCount = %d\n", emu->ndrange_count);
}


int EvgEmuRun(Emu *self)
{
	EvgEmu *emu = asEvgEmu(self);

	EvgNDRange *ndrange;
	EvgNDRange *ndrange_next;

	EvgWorkGroup *work_group;
	EvgWorkGroup *work_group_next;

	EvgWavefront *wavefront;
	EvgWavefront *wavefront_next;

	/* Exit if there are no ND-Ranges to emulate */
	if (!emu->ndrange_list_count)
		return FALSE;

	/* Start any ND-Range in state 'pending' */
	while ((ndrange = emu->pending_ndrange_list_head))
	{
		/* Set all ready work-groups to running */
		while ((work_group = ndrange->pending_list_head))
		{
			EvgWorkGroupClearState(work_group, EvgWorkGroupPending);
			EvgWorkGroupSetState(work_group, EvgWorkGroupRunning);
		}

		/* Set is in state 'running' */
		EvgNDRangeClearState(ndrange, EvgNDRangePending);
		EvgNDRangeSetState(ndrange, EvgNDRangeRunning);
	}

	/* Run one instruction of each wavefront in each work-group of each
	 * ND-Range that is in status 'running'. */
	for (ndrange = emu->running_ndrange_list_head; ndrange; ndrange = ndrange_next)
	{
		/* Save next ND-Range in state 'running'. This is done because the state
		 * might change during the execution of the ND-Range. */
		ndrange_next = ndrange->running_ndrange_list_next;

		/* Execute an instruction from each work-group */
		for (work_group = ndrange->running_list_head; work_group; work_group = work_group_next)
		{
			/* Save next running work-group */
			work_group_next = work_group->running_list_next;

			/* Run an instruction from each wavefront */
			for (wavefront = work_group->running_list_head; wavefront; wavefront = wavefront_next)
			{
				/* Save next running wavefront */
				wavefront_next = wavefront->running_list_next;

				/* Execute instruction in wavefront */
				EvgWavefrontExecute(wavefront);
			}
		}
	}

	/* Free ND-Ranges that finished */
	while ((ndrange = emu->finished_ndrange_list_head))
	{
		/* Dump ND-Range report */
		EvgNDRangeDump(ndrange, evg_emu_report_file);

		/* Stop if maximum number of kernels reached */
		if (evg_emu_max_kernels && emu->ndrange_count >= evg_emu_max_kernels)
			esim_finish = esim_finish_evg_max_kernels;

		/* Extract from list of finished ND-Ranges and free */
		delete(ndrange);
	}

	/* Still emulating */
	return TRUE;
}



void EvgEmuConstMemWrite(EvgEmu *self, int bank, int vector, int elem, void *pvalue, int size)
{
	unsigned int addr;

	/* Mark CB0[0..8].{x,y,z,w} positions as initialized */
	if (!bank && vector < 9)
		self->const_mem_cb0_init[vector * 4 + elem] = 1;

	/* Write */
	addr = bank * 16384 + vector * 16 + elem * 4;
	mem_write(self->const_mem, addr, size, pvalue);
}


void EvgEmuConstMemRead(EvgEmu *self, int bank, int vector, int elem, void *pvalue)
{
	unsigned int addr;

	/* Warn if a position within CB[0..8].{x,y,z,w} is used uninitialized */
	if (!bank && vector < 9 && !self->const_mem_cb0_init[vector * 4 + elem])
		warning("CB0[%d].%c is used uninitialized", vector, "xyzw"[elem]);

	/* Read */
	addr = bank * 16384 + vector * 16 + elem * 4;
	mem_read(self->const_mem, addr, 4, pvalue);
}
