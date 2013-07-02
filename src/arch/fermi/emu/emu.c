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

#include <arch/common/arch.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/timer.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "isa.h"
#include "grid.h"
#include "warp.h"
#include "thread-block.h"


/*
 * Global variables
 */

struct frm_emu_t *frm_emu;

long long frm_emu_max_cycles = 0;
long long frm_emu_max_inst = 0;
int frm_emu_max_functions = 0;

char *frm_emu_cuda_binary_name = "";
char *frm_emu_report_file_name = "";
FILE *frm_emu_report_file = NULL;

int frm_emu_warp_size = 32;




void frm_emu_init(void)
{
	/* Open report file */
	if (*frm_emu_report_file_name)
	{
		frm_emu_report_file = file_open_for_write(
			frm_emu_report_file_name);
		if (!frm_emu_report_file)
		{
			fatal("%s: cannot open report for Fermi emulator", 
				frm_emu_report_file_name);
		}
	}

        /* Initialize */
        frm_emu = xcalloc(1, sizeof(struct frm_emu_t));
	frm_emu->grids = list_create();
	frm_emu->pending_grids = list_create();
	frm_emu->running_grids = list_create();
	frm_emu->finished_grids = list_create();
        frm_emu->global_mem = mem_create();
        frm_emu->global_mem->safe = 0;
        frm_emu->global_mem_top = 0;
        frm_emu->total_global_mem_size = 1 << 31; /* 2GB */
        frm_emu->free_global_mem_size = 1 << 31; /* 2GB */
        frm_emu->const_mem = mem_create();
        frm_emu->const_mem->safe = 0;

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

	/* Free grids */
	list_free(frm_emu->grids);
	list_free(frm_emu->pending_grids);
	list_free(frm_emu->running_grids);
	list_free(frm_emu->finished_grids);
        mem_free(frm_emu->const_mem);
        mem_free(frm_emu->global_mem);

	/* Finalize GPU emulator */
        free(frm_emu);
}


void frm_emu_dump(FILE *f)
{
}


/* One iteration of emulator. Return TRUE if emulation is still running. */
int frm_emu_run(void)
{
	struct frm_grid_t *grid;
	struct frm_thread_block_t *thread_block;
	struct frm_warp_t *warp;

	/* Stop emulation if no grid needs running */
	if (!list_count(frm_emu->grids))
		return FALSE;

	/* Remove grid and its thread blocks from pending list, and add them to
	 * running list */
	while ((grid = list_head(frm_emu->pending_grids)))
	{
		while ((thread_block = list_head(grid->pending_thread_blocks)))
		{
			list_remove(grid->pending_thread_blocks, thread_block);
			list_add(grid->running_thread_blocks, thread_block);
		}

		list_remove(frm_emu->pending_grids, grid);
		list_add(frm_emu->running_grids, grid);
	}

	/* Run one instruction of each warp in each thread block of each running
	 * grid */
	while ((grid = list_head(frm_emu->running_grids)))
	{
		/* Execute an instruction from each thread block */
		while ((thread_block = list_head(grid->running_thread_blocks)))
		{
			/* Run an instruction from each warp */
			while ((warp = list_head(thread_block->running_warps)))
			{
				/* Execute instruction in warp */
				frm_warp_execute(warp);
			}
		}
	}

	/* Free finished grids */
	while ((grid = list_head(frm_emu->finished_grids)))
	{
		/* Dump grid report */
		frm_grid_dump(grid, frm_emu_report_file);

		/* Stop if maximum number of CUDA functions reached */
		if (frm_emu_max_functions && frm_emu->grid_count >= 
				frm_emu_max_functions)
			esim_finish = esim_finish_frm_max_functions;

		/* Remove grid from finished list and free */
		list_remove(frm_emu->finished_grids, grid);
		frm_grid_free(grid);
	}

	/* Continue emulation */
	return TRUE;
}


void frm_emu_dump_summary(FILE *f)
{
}
