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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_EMU_H
#define ARCH_SOUTHERN_ISLANDS_EMU_EMU_H

#include <stdio.h>


#define UAV_TABLE_START 0
#define UAV_TABLE_SIZE 1024
#define CONSTANT_MEMORY_START (UAV_TABLE_START + UAV_TABLE_SIZE)
#define CONSTANT_BUFFER_SIZE 1024
#define CONSTANT_BUFFERS 2
#define GLOBAL_MEMORY_START (CONSTANT_MEMORY_START + CONSTANT_BUFFERS * CONSTANT_BUFFER_SIZE)

struct si_emu_t
{
	/* Timer */
	struct m2s_timer_t *timer;

	/* OpenCL objects */
	struct si_opencl_repo_t *opencl_repo;
	struct si_opencl_platform_t *opencl_platform;
	struct si_opencl_device_t *opencl_device;

	/* List of ND-Ranges */
	struct si_ndrange_t *ndrange_list_head;
	struct si_ndrange_t *ndrange_list_tail;
	int ndrange_list_count;
	int ndrange_list_max;

	/* List of pending ND-Ranges */
	struct si_ndrange_t *pending_ndrange_list_head;
	struct si_ndrange_t *pending_ndrange_list_tail;
	int pending_ndrange_list_count;
	int pending_ndrange_list_max;

	/* List of running ND-Ranges */
	struct si_ndrange_t *running_ndrange_list_head;
	struct si_ndrange_t *running_ndrange_list_tail;
	int running_ndrange_list_count;
	int running_ndrange_list_max;

	/* List of finished ND-Ranges */
	struct si_ndrange_t *finished_ndrange_list_head;
	struct si_ndrange_t *finished_ndrange_list_tail;
	int finished_ndrange_list_count;
	int finished_ndrange_list_max;

	/* Global memory */
	struct mem_t *global_mem;
	unsigned int global_mem_top;

	/* Statistics */
	int ndrange_count;  /* Number of OpenCL kernels executed */
	long long inst_count;  /* Number of instructions executed by wavefronts */
	long long scalar_alu_inst_count;  /* Number of scalar ALU instructions executed */
	long long scalar_mem_inst_count;  /* Number of scalar memory instructions executed */
	long long branch_inst_count; /* Number of branch instructions executed */
	long long vector_alu_inst_count;  /* Number of vector ALU instructions executed */
	long long local_mem_inst_count; /* Number of LDS instructions executed */
	long long vector_mem_inst_count;  /* Number of scalar memory instructions executed */
};

/* Forward declaration */
struct x86_ctx_t;

extern enum arch_sim_kind_t si_emu_sim_kind;

extern long long si_emu_max_cycles;
extern long long si_emu_max_inst;
extern int si_emu_max_kernels;

extern char *si_emu_opencl_binary_name;
extern char *si_emu_report_file_name;
extern FILE *si_emu_report_file;

extern int si_emu_wavefront_size;

extern char *si_err_opencl_note;
extern char *si_err_opencl_param_note;

extern struct si_emu_t *si_emu;
extern struct arch_t *si_emu_arch;

void si_emu_init(void);
void si_emu_done(void);

void si_emu_dump_summary(FILE *f);

int si_emu_run(void);

void si_emu_disasm(char *path);
void si_emu_opengl_disasm(char *path, int opengl_shader_index);

#endif

