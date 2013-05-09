/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_FERMI_EMU_EMU_H
#define ARCH_FERMI_EMU_EMU_H

#include <stdio.h>


struct frm_emu_t
{
	/* List of ND-Ranges */
	struct frm_grid_t *grid_list_head;
	struct frm_grid_t *grid_list_tail;
	int grid_list_count;
	int grid_list_max;

	/* List of pending grids */
	struct frm_grid_t *pending_grid_list_head;
	struct frm_grid_t *pending_grid_list_tail;
	int pending_grid_list_count;
	int pending_grid_list_max;

	/* List of running grids */
	struct frm_grid_t *running_grid_list_head;
	struct frm_grid_t *running_grid_list_tail;
	int running_grid_list_count;
	int running_grid_list_max;

	/* List of finished grids */
	struct frm_grid_t *finished_grid_list_head;
	struct frm_grid_t *finished_grid_list_tail;
	int finished_grid_list_count;
	int finished_grid_list_max;

	/* Global memory */
	struct mem_t *global_mem;
	unsigned int global_mem_top;
	unsigned int free_global_mem_size;
	unsigned int total_global_mem_size;

	/* Constant memory, which is organized as 16 banks of 64KB each. */
	struct mem_t *const_mem;

	/* Flags indicating whether the first 32 bytes of constant memory
	 * are initialized. A warning will be issued by the simulator
	 * if an uninitialized element is used by the kernel. */
	int const_mem_init[32];

	/* Stats */
	int grid_count;  /* Number of CUDA functions executed */
	long long scalar_alu_inst_count;  /* Scalar ALU instructions executed */
	long long scalar_mem_inst_count;  /* Scalar mem instructions executed */
	long long branch_inst_count;  /* Branch instructions executed */
	long long vector_alu_inst_count;  /* Vector ALU instructions executed */
	long long lds_inst_count;  /* LDS instructions executed */
	long long vector_mem_inst_count;  /* Vector mem instructions executed */
	long long export_inst_count; /* Export instruction executed */
};

extern long long frm_emu_max_cycles;
extern long long frm_emu_max_inst;
extern int frm_emu_max_functions;

extern char *frm_emu_cuda_binary_name;
extern char *frm_emu_report_file_name;
extern FILE *frm_emu_report_file;

extern int frm_emu_warp_size;
extern char *err_frm_cuda_note;

extern struct frm_emu_t *frm_emu;

void frm_emu_init(void);
void frm_emu_done(void);
void frm_emu_dump(FILE *f);

int frm_emu_run(void);

void frm_emu_dump_summary(FILE *f);

#endif

