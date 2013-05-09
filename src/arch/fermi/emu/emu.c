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
#include <driver/cuda/object.h>
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
			fatal("%s: cannot open report for Fermi "
				"emulator", frm_emu_report_file_name);
		}
	}

        /* Initialize */
        frm_emu = xcalloc(1, sizeof(struct frm_emu_t));
        frm_emu->global_mem = mem_create();
        frm_emu->global_mem->safe = 0;
        frm_emu->total_global_mem_size = 1 << 31; /* 2GB */
        frm_emu->free_global_mem_size = 1 << 31; /* 2GB */
        frm_emu->global_mem_top = 0;
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

	/* Free grid */
	while (frm_emu->grid_list_count)
		frm_grid_free(frm_emu->grid_list_head);

	/* Free CUDA object list */
	cuda_object_free_all();
	if (cuda_object_list)
		linked_list_free(cuda_object_list);

	/* Finalize disassembler */
	frm_disasm_done();

	/* Finalize ISA */
	frm_isa_done();

	/* Finalize GPU kernel */
        mem_free(frm_emu->const_mem);
        mem_free(frm_emu->global_mem);
        free(frm_emu);
}


void frm_emu_dump(FILE *f)
{
}


/* One iteration of emulator. Return TRUE if emulation is still running. */
int frm_emu_run(void)
{
	struct frm_grid_t *grid;
	struct frm_grid_t *grid_next;

	struct frm_thread_block_t *thread_block;
	struct frm_thread_block_t *thread_block_next;

	struct frm_warp_t *warp;
	struct frm_warp_t *warp_next;

	/* For efficiency when no emulation is selected, 
	 * exit here if the list of existing grid is empty. */
	if (!frm_emu->grid_list_count)
		return FALSE;

	/* Start any grid in state 'pending' */
	while ((grid = frm_emu->pending_grid_list_head))
	{
		/* Set all ready thread blocks to running */
		while ((thread_block = grid->pending_list_head))
		{
			frm_thread_block_clear_status(thread_block, frm_thread_block_pending);
			frm_thread_block_set_status(thread_block, frm_thread_block_running);
		}

		/* Set is in state 'running' */
		frm_grid_clear_status(grid, frm_grid_pending);
		frm_grid_set_status(grid, frm_grid_running);
	}

	/* Run one instruction of each warp in each thread block of each
	 * grid that is in status 'running'. */
	for (grid = frm_emu->running_grid_list_head; grid; 
		grid = grid_next)
	{
		/* Save next grid in state 'running'. This is done because 
		 * the state might change during the execution of the 
		 * grid. */
		grid_next = grid->running_grid_list_next;

		/* Execute an instruction from each thread block */
		for (thread_block = grid->running_list_head; thread_block; thread_block = thread_block_next)
		{
			/* Save next running thread block */
			thread_block_next = thread_block->running_list_next;

			/* Run an instruction from each warp */
			for (warp = thread_block->running_list_head; warp; warp = warp_next)
			{
				/* Save next running warp */
				warp_next = warp->running_list_next;

				/* Execute instruction in warp */
				frm_warp_execute(warp);
			}
		}
	}

	/* Free grid that finished */
	while ((grid = frm_emu->finished_grid_list_head))
	{
		/* Dump grid report */
		frm_grid_dump(grid, frm_emu_report_file);

		/* Stop if maximum number of kernels reached */
		if (frm_emu_max_functions && frm_emu->grid_count >= 
				frm_emu_max_functions)
			esim_finish = esim_finish_frm_max_functions;

		/* Extract from list of finished grid and free */
		frm_grid_free(grid);
	}

	/* Still emulating */
	return TRUE;
}


void frm_emu_dump_summary(FILE *f)
{
}
