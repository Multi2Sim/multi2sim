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

#ifndef ARCH_EVERGREEN_EMU_EMU_H
#define ARCH_EVERGREEN_EMU_EMU_H

#include <stdio.h>

#include <arch/common/emu.h>


/*
 * Class 'EvgEmu'
 */

CLASS_BEGIN(EvgEmu, Emu)

	/* OpenCL objects */
	struct evg_opencl_repo_t *opencl_repo;
	struct evg_opencl_platform_t *opencl_platform;
	struct evg_opencl_device_t *opencl_device;

	/* List of ND-Ranges */
	struct evg_ndrange_t *ndrange_list_head;
	struct evg_ndrange_t *ndrange_list_tail;
	int ndrange_list_count;
	int ndrange_list_max;

	/* List of pending ND-Ranges */
	struct evg_ndrange_t *pending_ndrange_list_head;
	struct evg_ndrange_t *pending_ndrange_list_tail;
	int pending_ndrange_list_count;
	int pending_ndrange_list_max;

	/* List of running ND-Ranges */
	struct evg_ndrange_t *running_ndrange_list_head;
	struct evg_ndrange_t *running_ndrange_list_tail;
	int running_ndrange_list_count;
	int running_ndrange_list_max;

	/* List of finished ND-Ranges */
	struct evg_ndrange_t *finished_ndrange_list_head;
	struct evg_ndrange_t *finished_ndrange_list_tail;
	int finished_ndrange_list_count;
	int finished_ndrange_list_max;

	/* Constant memory (constant buffers)
	 * There are 15 constant buffers, referenced as CB0 to CB14.
	 * Each buffer can hold up to 1024 four-component vectors.
	 * These buffers will be represented as a memory object indexed as
	 *   buffer_id * 1024 * 4 * 4 + vector_id * 4 * 4 + elem_id * 4
	 */
	struct mem_t *const_mem;

	/* Flags indicating whether the first 9 vector positions of CB0
	 * are initialized. A warning will be issued by the simulator
	 * if an uninitialized element is used by the kernel. */
	int const_mem_cb0_init[9 * 4];

	/* Global memory */
	struct mem_t *global_mem;
	unsigned int global_mem_top;

	/* Stats */
	int ndrange_count;  /* Number of OpenCL kernels executed */

CLASS_END(EvgEmu)


void EvgEmuCreate(EvgEmu *self);
void EvgEmuDestroy(EvgEmu *self);

void EvgEmuDump(Object *self, FILE *f);
void EvgEmuDumpSummary(Emu *self, FILE *f);

/* Virtual function from class 'Emu' */
int EvgEmuRun(Emu *self);



/*
 * Non-Class Stuff
 */

extern long long evg_emu_max_cycles;
extern long long evg_emu_max_inst;
extern int evg_emu_max_kernels;

extern char *evg_emu_opencl_binary_name;
extern char *evg_emu_report_file_name;
extern FILE *evg_emu_report_file;

extern int evg_emu_wavefront_size;

extern char *evg_err_opencl_note;
extern char *evg_err_opencl_param_note;

extern EvgEmu *evg_emu;


void evg_emu_init(void);
void evg_emu_done(void);

void evg_emu_disasm(char *path);
void evg_emu_opengl_disasm(char *path, int opengl_shader_index);


#endif

