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

#ifndef ARCH_EVERGREEN_EMU_NDRANGE_H
#define ARCH_EVERGREEN_EMU_NDRANGE_H

#include <stdio.h>

#include <lib/class/class.h>


/*
 * Class 'EvgNDRange'
 */

typedef enum
{
	EvgNDRangePending = 0x01,
	EvgNDRangeRunning = 0x02,
	EvgNDRangeFinished = 0x04
} EvgNDRangeState;


CLASS_BEGIN(EvgNDRange, Object)

	/* Emulator and driver */
	EvgEmu *emu;
	OpenclOldDriver *driver;

	/* ID */
	char *name;
	int id;  /* Sequential ndrange ID (given by evg_emu->ndrange_count counter) */

	/* Status */
	EvgNDRangeState state;

	/* OpenCL kernel associated */
	struct evg_opencl_kernel_t *kernel;

	/* Command queue and command queue task associated */
	struct evg_opencl_command_queue_t *command_queue;
	struct evg_opencl_command_t *command;

	/* Pointers to work-groups, wavefronts, and work_items */
	EvgWorkGroup **work_groups;
	EvgWavefront **wavefronts;
	EvgWorkItem **work_items;

	/* IDs of work-items contained */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* IDs of wavefronts contained */
	int wavefront_id_first;
	int wavefront_id_last;
	int wavefront_count;

	/* IDs of work-groups contained */
	int work_group_id_first;
	int work_group_id_last;
	int work_group_count;
	
	/* Size of work-groups */
	int wavefronts_per_work_group;  /* = ceil(local_size / evg_emu_wavefront_size) */

	/* List of ND-Ranges */
	EvgNDRange *ndrange_list_prev;
	EvgNDRange *ndrange_list_next;
	EvgNDRange *pending_ndrange_list_prev;
	EvgNDRange *pending_ndrange_list_next;
	EvgNDRange *running_ndrange_list_prev;
	EvgNDRange *running_ndrange_list_next;
	EvgNDRange *finished_ndrange_list_prev;
	EvgNDRange *finished_ndrange_list_next;

	/* List of pending work-groups */
	EvgWorkGroup *pending_list_head;
	EvgWorkGroup *pending_list_tail;
	int pending_list_count;
	int pending_list_max;

	/* List of running work-groups */
	EvgWorkGroup *running_list_head;
	EvgWorkGroup *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of finished work-groups */
	EvgWorkGroup *finished_list_head;
	EvgWorkGroup *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Local memory top to assign to local arguments.
	 * Initially it is equal to the size of local variables in kernel function. */
	unsigned int local_mem_top;


	/* Statistics */

	/* Histogram of executed instructions. Only allocated if the kernel report
	 * option is active. */
	unsigned int *inst_histogram;

CLASS_END(EvgNDRange)


void EvgNDRangeCreate(EvgNDRange *self, EvgEmu *emu, OpenclOldDriver *driver,
		struct evg_opencl_kernel_t *kernel);
void EvgNDRangeDestroy(EvgNDRange *self);

void EvgNDRangeDump(EvgNDRange *self, FILE *f);

int EvgNDRangeGetState(EvgNDRange *self, EvgNDRangeState state);
void EvgNDRangeSetState(EvgNDRange *self, EvgNDRangeState state);
void EvgNDRangeClearState(EvgNDRange *self, EvgNDRangeState state);

void EvgNDRangeSetupWorkItems(EvgNDRange *self);
void EvgNDRangeSetupConstantMemory(EvgNDRange *self);
void EvgNDRangeSetupArguments(EvgNDRange *self);


#endif

