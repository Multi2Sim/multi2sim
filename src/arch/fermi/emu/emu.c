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
#include <arch/common/arch-list.h>
#include <arch/fermi/asm/asm.h>
#include <driver/cuda/device.h>
#include <driver/cuda/object.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
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
struct arch_t *frm_emu_arch;

long long frm_emu_max_cycles = 0;
long long frm_emu_max_inst = 0;
int frm_emu_max_functions = 0;

enum arch_sim_kind_t frm_emu_sim_kind = arch_sim_kind_functional;

char *frm_emu_cuda_binary_name = "";
char *frm_emu_report_file_name = "";
FILE *frm_emu_report_file = NULL;

int frm_emu_warp_size = 32;




void frm_emu_init(void)
{
	/* Register architecture */
	frm_emu_arch = arch_list_register("Fermi", "frm");
	frm_emu_arch->sim_kind = frm_emu_sim_kind;

        /* Allocate */
        frm_emu = xcalloc(1, sizeof(struct frm_emu_t));
        if (!frm_emu)
                fatal("%s: out of memory", __FUNCTION__);

        /* Initialize */
        frm_emu->const_mem = mem_create();
        frm_emu->const_mem->safe = 0;
        frm_emu->global_mem = mem_create();
        frm_emu->global_mem->safe = 0;
        frm_emu->total_global_mem_size = 1 << 31; /* 2GB */
        frm_emu->free_global_mem_size = frm_emu->total_global_mem_size;
        frm_emu->global_mem_top = 0;

	frm_disasm_init();
	frm_isa_init();

        /* Create device */
        cuda_object_list = linked_list_create();  /* FIXME - Should be in driver implementation */
        cuda_device_create();  /* FIXME - should just call cuda_init() or similar */
}


void frm_emu_done(void)
{
	/* Free grid */
	while (frm_emu->grid_list_count)
		frm_grid_free(frm_emu->grid_list_head);

	/* Free CUDA object list */
	cuda_object_free_all();
	linked_list_free(cuda_object_list);

	frm_isa_done();

        mem_free(frm_emu->const_mem);
        mem_free(frm_emu->global_mem);
        free(frm_emu);
}


void frm_emu_run(void)
{
	struct frm_grid_t *grid;
	struct frm_grid_t *grid_next;

	struct frm_thread_block_t *thread_block;
	struct frm_thread_block_t *thread_block_next;

	struct frm_warp_t *warp;
	struct frm_warp_t *warp_next;

	unsigned long long int cycle = 0;

	grid = frm_emu->pending_grid_list_head;

	/* Set all ready thread_blocks to running */
	while ((thread_block = grid->pending_list_head))
	{
		frm_thread_block_clear_status(thread_block, frm_thread_block_pending);
		frm_thread_block_set_status(thread_block, frm_thread_block_running);
	}
	/* Set is in state 'running' */
	frm_grid_clear_status(grid, frm_grid_pending);
	frm_grid_set_status(grid, frm_grid_running);


	/* Execution loop */
	while (grid->running_list_head)
	{
		/* Stop if maximum number of GPU cycles exceeded */
		if (frm_emu_max_cycles && cycle >= frm_emu_max_cycles)
			esim_finish = esim_finish_frm_max_cycles;

		/* Stop if maximum number of GPU instructions exceeded */
		if (frm_emu_max_inst && frm_emu->inst_count >= frm_emu_max_inst)
			esim_finish = esim_finish_frm_max_inst;

		/* Stop if any reason met */
		if (esim_finish)
			break;

		/* Next cycle */
		cycle++;

		/* Execute an instruction from each work-group */
		for (thread_block = grid->running_list_head; thread_block; thread_block = thread_block_next)
		{
			/* Save next running work-group */
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

	/* Dump stats */
	frm_grid_dump(grid, stdout);

	/* Stop if maximum number of functions reached */
	//if (frm_emu_max_functions && frm_emu->grid_count >= frm_emu_max_functions)
	//	x86_emu_finish = x86_emu_finish_max_gpu_functions;

}

