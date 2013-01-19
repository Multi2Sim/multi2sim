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
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "isa.h"


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
        frm_cuda_object_list = linked_list_create();
        frm_cuda_device_create();
}


void frm_emu_done(void)
{
	/* Free CUDA object list */
	frm_cuda_object_free_all();
	linked_list_free(frm_cuda_object_list);

	frm_isa_done();

        mem_free(frm_emu->const_mem);
        mem_free(frm_emu->global_mem);
        free(frm_emu);
}

