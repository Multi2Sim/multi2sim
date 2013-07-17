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


#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "isa.h"
#include "grid.h"
#include "warp.h"
#include "thread-block.h"


/*
 * Class 'FrmEmu'
 */

CLASS_IMPLEMENTATION(FrmEmu);

void FrmEmuCreate(FrmEmu *self)
{
	/* Parent */
	EmuCreate(asEmu(self), "Fermi");

        /* Initialize */
	self->grids = list_create();
	self->pending_grids = list_create();
	self->running_grids = list_create();
	self->finished_grids = list_create();
        self->global_mem = mem_create();
        self->global_mem->safe = 0;
        self->global_mem_top = 0;
        self->total_global_mem_size = 1 << 31; /* 2GB */
        self->free_global_mem_size = 1 << 31; /* 2GB */
        self->const_mem = mem_create();
        self->const_mem->safe = 0;

        /* Virtual functions */
        asObject(self)->Dump = FrmEmuDump;
        asEmu(self)->DumpSummary = FrmEmuDumpSummary;
        asEmu(self)->Run = FrmEmuRun;
}


void FrmEmuDestroy(FrmEmu *self)
{
	/* Free grids */
	list_free(self->grids);
	list_free(self->pending_grids);
	list_free(self->running_grids);
	list_free(self->finished_grids);
        mem_free(self->const_mem);
        mem_free(self->global_mem);
}


void FrmEmuDump(Object *self, FILE *f)
{
	/* Call parent */
	EmuDump(self, f);
}


void FrmEmuDumpSummary(Emu *self, FILE *f)
{
	/* Call parent */
	EmuDumpSummary(self, f);
}


int FrmEmuRun(Emu *self)
{
	FrmEmu *emu = asFrmEmu(self);

	struct frm_grid_t *grid;
	struct frm_thread_block_t *thread_block;
	struct frm_warp_t *warp;

	/* Stop emulation if no grid needs running */
	if (!list_count(emu->grids))
		return FALSE;

	/* Remove grid and its thread blocks from pending list, and add them to
	 * running list */
	while ((grid = list_head(emu->pending_grids)))
	{
		while ((thread_block = list_head(grid->pending_thread_blocks)))
		{
			list_remove(grid->pending_thread_blocks, thread_block);
			list_add(grid->running_thread_blocks, thread_block);
		}

		list_remove(emu->pending_grids, grid);
		list_add(emu->running_grids, grid);
	}

	/* Run one instruction */
	while ((grid = list_head(emu->running_grids)))
	{
		while ((thread_block = list_head(grid->running_thread_blocks)))
		{
			while ((warp = list_head(thread_block->running_warps)))
			{
				if (warp->finished || warp->at_barrier)
					continue;

				frm_warp_execute(warp);
			}
		}
	}

	/* Free finished grids */
	assert(list_count(emu->pending_grids) == 0 &&
			list_count(emu->running_grids) == 0);
	while ((grid = list_head(emu->finished_grids)))
	{
		/* Dump grid report */
		frm_grid_dump(grid, frm_emu_report_file);

		/* Remove grid from finished list */
		list_remove(emu->finished_grids, grid);

		/* Free grid */
		frm_grid_free(grid);
	}

	/* Continue emulation */
	return TRUE;
}




/*
 * Non-Class Stuff
 */

long long frm_emu_max_cycles;
long long frm_emu_max_inst;
int frm_emu_max_functions;

char *frm_emu_cuda_binary_name = "";
char *frm_emu_report_file_name = "";
FILE *frm_emu_report_file;

int frm_emu_warp_size = 32;

FrmEmu *frm_emu;


void frm_emu_init(void)
{
	/* Classes */
	CLASS_REGISTER(FrmEmu);

	/* Open report file */
	if (*frm_emu_report_file_name)
	{
		frm_emu_report_file = file_open_for_write(frm_emu_report_file_name);
		if (!frm_emu_report_file)
			fatal("%s: cannot open report for Fermi emulator", 
				frm_emu_report_file_name);
	}

	/* Create emulator */
	frm_emu = new(FrmEmu);

	/* Initialize disassembler (decoding tables...) */
	frm_disasm_init();

	/* Initialize ISA (instruction execution tables...) */
	frm_isa_init();
}


void frm_emu_done(void)
{
	/* Report */
	if (frm_emu_report_file)
		fclose(frm_emu_report_file);

	/* Finalize disassembler */
	frm_disasm_done();

	/* Finalize ISA */
	frm_isa_done();

	/* Free emulator */
	delete(frm_emu);
}

