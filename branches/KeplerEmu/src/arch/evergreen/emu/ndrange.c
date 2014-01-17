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


#include <arch/evergreen/asm/bin-file.h>
#include <driver/opencl-old/evergreen/command-queue.h>
#include <driver/opencl-old/evergreen/kernel.h>
#include <driver/opencl-old/evergreen/mem.h>
#include <driver/opencl-old/evergreen/opencl.h>
#include <driver/opencl-old/evergreen/repo.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/timer.h>

#include "emu.h"
#include "isa.h"
#include "ndrange.h"
#include "work-item.h"
#include "work-group.h"


/*
 * Class 'EvgNDRange'
 */

void EvgNDRangeCreate(EvgNDRange *self, EvgEmu *emu, OpenclOldDriver *driver,
		struct evg_opencl_kernel_t *kernel)
{
	/* Insert in ND-Range list of Evergreen emulator */
	DOUBLE_LINKED_LIST_INSERT_TAIL(emu, ndrange, self);

	/* Initialize */
	self->emu = emu;
	self->driver = driver;
	self->name = xstrdup(kernel->name);
	self->kernel = kernel;
	self->local_mem_top = kernel->func_mem_local;
	self->id = emu->ndrange_count++;

	/* Instruction histogram */
	if (evg_emu_report_file)
		self->inst_histogram = xcalloc(EvgInstOpcodeCount, sizeof(unsigned int));
}


void EvgNDRangeDestroy(EvgNDRange *self)
{
	EvgEmu *emu = self->emu;
	int i;

	/* Clear task from command queue */
	if (self->command_queue && self->command)
	{
		evg_opencl_command_queue_complete(self->command_queue, self->command);
		evg_opencl_command_free(self->command);
	}

	/* Clear all states that affect lists. */
	EvgNDRangeClearState(self, EvgNDRangePending);
	EvgNDRangeClearState(self, EvgNDRangeRunning);
	EvgNDRangeClearState(self, EvgNDRangeFinished);

	/* Extract from ND-Range list in Evergreen emulator */
	assert(DOUBLE_LINKED_LIST_MEMBER(emu, ndrange, self));
	DOUBLE_LINKED_LIST_REMOVE(emu, ndrange, self);

	/* Free work-groups */
	for (i = 0; i < self->work_group_count; i++)
		delete(self->work_groups[i]);
	free(self->work_groups);

	/* Free wavefronts */
	for (i = 0; i < self->wavefront_count; i++)
		delete(self->wavefronts[i]);
	free(self->wavefronts);

	/* Free work-items */
	for (i = 0; i < self->work_item_count; i++)
		delete(self->work_items[i]);
	free(self->work_items);

	/* Free instruction histogram */
	if (self->inst_histogram)
		free(self->inst_histogram);

	/* Rest */
	free(self->name);
}


void EvgNDRangeDump(EvgNDRange *self, FILE *f)
{
	EvgEmu *emu = self->emu;
	EvgAsm *as = emu->as;

	EvgWorkGroup *work_group;

	int work_group_id;
	int work_item_id, last_work_item_id;
	int branch_digest_count;
	int i;

	unsigned int branch_digest;
	unsigned int last_branch_digest;

	if (!f)
		return;
	
	fprintf(f, "[ NDRange[%d] ]\n\n", self->id);
	fprintf(f, "Name = %s\n", self->name);
	fprintf(f, "WorkGroupFirst = %d\n", self->work_group_id_first);
	fprintf(f, "WorkGroupLast = %d\n", self->work_group_id_last);
	fprintf(f, "WorkGroupCount = %d\n", self->work_group_count);
	fprintf(f, "WaveFrontFirst = %d\n", self->wavefront_id_first);
	fprintf(f, "WaveFrontLast = %d\n", self->wavefront_id_last);
	fprintf(f, "WaveFrontCount = %d\n", self->wavefront_count);
	fprintf(f, "WorkItemFirst = %d\n", self->work_item_id_first);
	fprintf(f, "WorkItemLast = %d\n", self->work_item_id_last);
	fprintf(f, "WorkItemCount = %d\n", self->work_item_count);

	/* Branch digests */
	assert(self->work_item_count);
	branch_digest_count = 0;
	last_work_item_id = 0;
	last_branch_digest = self->work_items[0]->branch_digest;
	for (work_item_id = 1; work_item_id <= self->work_item_count; work_item_id++)
	{
		branch_digest = work_item_id < self->work_item_count ? self->work_items[work_item_id]->branch_digest : 0;
		if (work_item_id == self->work_item_count || branch_digest != last_branch_digest)
		{
			fprintf(f, "BranchDigest[%d] = %d %d %08x\n", branch_digest_count,
				last_work_item_id, work_item_id - 1, last_branch_digest);
			last_work_item_id = work_item_id;
			last_branch_digest = branch_digest;
			branch_digest_count++;
		}
	}
	fprintf(f, "BranchDigestCount = %d\n", branch_digest_count);
	fprintf(f, "\n");

	/* Instruction histogram */
	if (self->inst_histogram)
	{
		for (i = 0; i < EvgInstOpcodeCount; i++)
			if (self->inst_histogram[i])
				fprintf(f, "InstHistogram[%s] = %u\n",
						as->inst_info[i].name,
						self->inst_histogram[i]);
		fprintf(f, "\n");
	}

	/* Work-groups */
	EVG_FOR_EACH_WORK_GROUP_IN_NDRANGE(self, work_group_id)
	{
		work_group = self->work_groups[work_group_id];
		EvgWorkGroupDump(work_group, f);
	}
}


int EvgNDRangeGetState(EvgNDRange *self, EvgNDRangeState state)
{
	return (self->state & state) > 0;
}


void EvgNDRangeSetState(EvgNDRange *self, EvgNDRangeState state)
{
	EvgEmu *emu = self->emu;

	/* Get only the new bits */
	state &= ~self->state;

	/* Add ND-Range to lists */
	if (state & EvgNDRangePending)
		DOUBLE_LINKED_LIST_INSERT_TAIL(emu, pending_ndrange, self);
	if (state & EvgNDRangeRunning)
		DOUBLE_LINKED_LIST_INSERT_TAIL(emu, running_ndrange, self);
	if (state & EvgNDRangeFinished)
		DOUBLE_LINKED_LIST_INSERT_TAIL(emu, finished_ndrange, self);

	/* Start/stop Evergreen timer depending on ND-Range states */
	if (emu->running_ndrange_list_count)
		m2s_timer_start(asEmu(emu)->timer);
	else
		m2s_timer_stop(asEmu(emu)->timer);

	/* Update it */
	self->state |= state;
}


void EvgNDRangeClearState(EvgNDRange *self, EvgNDRangeState state)
{
	EvgEmu *emu = self->emu;

	/* Get only the bits that are set */
	state &= self->state;

	/* Remove ND-Range from lists */
	if (state & EvgNDRangePending)
		DOUBLE_LINKED_LIST_REMOVE(emu, pending_ndrange, self);
	if (state & EvgNDRangeRunning)
		DOUBLE_LINKED_LIST_REMOVE(emu, running_ndrange, self);
	if (state & EvgNDRangeFinished)
		DOUBLE_LINKED_LIST_REMOVE(emu, finished_ndrange, self);

	/* Update state */
	self->state &= ~state;
}


void EvgNDRangeSetupWorkItems(EvgNDRange *self)
{
	struct evg_opencl_kernel_t *kernel = self->kernel;

	EvgWorkGroup *work_group;
	EvgWavefront *wavefront;
	EvgWorkItem *work_item;

	int gidx, gidy, gidz;  /* 3D work-group ID iterators */
	int lidx, lidy, lidz;  /* 3D work-item local ID iterators */

	int tid;  /* Global ID iterator */
	int gid;  /* Group ID iterator */
	int wid;  /* Wavefront ID iterator */
	int lid;  /* Local ID iterator */

	char name[MAX_STRING_SIZE];

	/* Create array of work-groups */
	self->work_group_count = kernel->group_count;
	self->work_group_id_first = 0;
	self->work_group_id_last = self->work_group_count - 1;
	self->work_groups = xcalloc(self->work_group_count, sizeof(void *));

	/* Create work-groups */
	for (gid = 0; gid < kernel->group_count; gid++)
	{
		self->work_groups[gid] = new(EvgWorkGroup, self);
		work_group = self->work_groups[gid];
	}
	
	/* Array of wavefronts */
	self->wavefronts_per_work_group = (kernel->local_size + evg_emu_wavefront_size - 1) / evg_emu_wavefront_size;
	self->wavefront_count = self->wavefronts_per_work_group * self->work_group_count;
	self->wavefront_id_first = 0;
	self->wavefront_id_last = self->wavefront_count - 1;
	assert(self->wavefronts_per_work_group > 0 && self->wavefront_count > 0);
	self->wavefronts = xcalloc(self->wavefront_count, sizeof(void *));
	for (wid = 0; wid < self->wavefront_count; wid++)
	{
		gid = wid / self->wavefronts_per_work_group;
		work_group = self->work_groups[gid];
		self->wavefronts[wid] = new(EvgWavefront, work_group);
		wavefront = self->wavefronts[wid];

		wavefront->id = wid;
		wavefront->id_in_work_group = wid % self->wavefronts_per_work_group;
		wavefront->ndrange = self;
		wavefront->work_group = work_group;
		DOUBLE_LINKED_LIST_INSERT_TAIL(work_group, running, wavefront);
	}
	
	/* Array of work-items */
	self->work_item_count = kernel->global_size;
	self->work_item_id_first = 0;
	self->work_item_id_last = self->work_item_count - 1;
	self->work_items = xcalloc(self->work_item_count, sizeof(void *));

	/* Create work-items */
	tid = 0;
	gid = 0;
	for (gidz = 0; gidz < kernel->group_count3[2]; gidz++)
	{
		for (gidy = 0; gidy < kernel->group_count3[1]; gidy++)
		{
			for (gidx = 0; gidx < kernel->group_count3[0]; gidx++)
			{
				/* Assign work-group ID */
				work_group = self->work_groups[gid];
				work_group->ndrange = self;
				work_group->id_3d[0] = gidx;
				work_group->id_3d[1] = gidy;
				work_group->id_3d[2] = gidz;
				work_group->id = gid;
				EvgWorkGroupSetState(work_group, EvgWorkGroupPending);

				/* First, last, and number of work-items in work-group */
				work_group->work_item_id_first = tid;
				work_group->work_item_id_last = tid + kernel->local_size - 1;
				work_group->work_item_count = kernel->local_size;
				work_group->work_items = &self->work_items[tid];
				snprintf(name, sizeof name, "work-group[i%d-i%d]",
					work_group->work_item_id_first, work_group->work_item_id_last);
				EvgWorkGroupSetName(work_group, name);

				/* First ,last, and number of wavefronts in work-group */
				work_group->wavefront_id_first = gid * self->wavefronts_per_work_group;
				work_group->wavefront_id_last = work_group->wavefront_id_first + self->wavefronts_per_work_group - 1;
				work_group->wavefront_count = self->wavefronts_per_work_group;
				work_group->wavefronts = &self->wavefronts[work_group->wavefront_id_first];

				/* Iterate through work-items */
				lid = 0;
				for (lidz = 0; lidz < kernel->local_size3[2]; lidz++)
				{
					for (lidy = 0; lidy < kernel->local_size3[1]; lidy++)
					{
						for (lidx = 0; lidx < kernel->local_size3[0]; lidx++)
						{
							/* Wavefront ID */
							wid = gid * self->wavefronts_per_work_group +
								lid / evg_emu_wavefront_size;
							assert(wid < self->wavefront_count);
							wavefront = self->wavefronts[wid];
							
							/* Create work-item */
							self->work_items[tid] = new(EvgWorkItem, wavefront);
							work_item = self->work_items[tid];
							work_item->ndrange = self;

							/* Global IDs */
							work_item->id_3d[0] = gidx * kernel->local_size3[0] + lidx;
							work_item->id_3d[1] = gidy * kernel->local_size3[1] + lidy;
							work_item->id_3d[2] = gidz * kernel->local_size3[2] + lidz;
							work_item->id = tid;

							/* Local IDs */
							work_item->id_in_work_group_3d[0] = lidx;
							work_item->id_in_work_group_3d[1] = lidy;
							work_item->id_in_work_group_3d[2] = lidz;
							work_item->id_in_work_group = lid;

							/* Other */
							work_item->id_in_wavefront = work_item->id_in_work_group %
									evg_emu_wavefront_size;
							work_item->work_group = self->work_groups[gid];
							work_item->wavefront = self->wavefronts[wid];

							/* First, last, and number of work-items in wavefront */
							if (!wavefront->work_item_count) {
								wavefront->work_item_id_first = tid;
								wavefront->work_items = &self->work_items[tid];
							}
							wavefront->work_item_count++;
							wavefront->work_item_id_last = tid;
							bit_map_set(wavefront->active_stack, work_item->id_in_wavefront, 1, 1);

							/* Save local IDs in register R0 */
							work_item->gpr[0].elem[0] = lidx;  /* R0.x */
							work_item->gpr[0].elem[1] = lidy;  /* R0.y */
							work_item->gpr[0].elem[2] = lidz;  /* R0.z */

							/* Save work-group IDs in register R1 */
							work_item->gpr[1].elem[0] = gidx;  /* R1.x */
							work_item->gpr[1].elem[1] = gidy;  /* R1.y */
							work_item->gpr[1].elem[2] = gidz;  /* R1.z */

							/* Next work-item */
							tid++;
							lid++;
						}
					}
				}

				/* Next work-group */
				gid++;
			}
		}
	}

	/* Assign names to wavefronts */
	for (wid = 0; wid < self->wavefront_count; wid++)
	{
		/* Set name */
		wavefront = self->wavefronts[wid];
		snprintf(name, sizeof name, "wavefront[i%d-i%d]",
			wavefront->work_item_id_first, wavefront->work_item_id_last);
		EvgWavefrontSetName(wavefront, name);

		/* Initialize wavefront program counter */
		if (!kernel->bin_file->enc_dict_entry_evergreen->sec_text_buffer.size)
			fatal("%s: cannot load kernel code", __FUNCTION__);
		wavefront->cf_buf_start = kernel->bin_file->enc_dict_entry_evergreen->sec_text_buffer.ptr;
		wavefront->cf_buf = wavefront->cf_buf_start;
		wavefront->clause_kind = EvgInstClauseCF;
	}

	/* Debug */
	evg_isa_debug("local_size = %d (%d,%d,%d)\n", kernel->local_size, kernel->local_size3[0],
		kernel->local_size3[1], kernel->local_size3[2]);
	evg_isa_debug("global_size = %d (%d,%d,%d)\n", kernel->global_size, kernel->global_size3[0],
		kernel->global_size3[1], kernel->global_size3[2]);
	evg_isa_debug("group_count = %d (%d,%d,%d)\n", kernel->group_count, kernel->group_count3[0],
		kernel->group_count3[1], kernel->group_count3[2]);
	evg_isa_debug("wavefront_count = %d\n", self->wavefront_count);
	evg_isa_debug("wavefronts_per_work_group = %d\n", self->wavefronts_per_work_group);
	evg_isa_debug(" tid tid2 tid1 tid0   gid gid2 gid1 gid0   "
			"lid lid2 lid1 lid0  wavefront            work-group\n");
	for (tid = 0; tid < self->work_item_count; tid++)
	{
		work_item = self->work_items[tid];
		wavefront = work_item->wavefront;
		work_group = work_item->work_group;
		evg_isa_debug("%4d %4d %4d %4d  ", work_item->id, work_item->id_3d[2],
			work_item->id_3d[1], work_item->id_3d[0]);
		evg_isa_debug("%4d %4d %4d %4d  ", work_group->id, work_group->id_3d[2],
			work_group->id_3d[1], work_group->id_3d[0]);
		evg_isa_debug("%4d %4d %4d %4d  ", work_item->id_in_work_group, work_item->id_in_work_group_3d[2],
			work_item->id_in_work_group_3d[1], work_item->id_in_work_group_3d[0]);
		evg_isa_debug("%20s.%-4d  ", wavefront->name, work_item->id_in_wavefront);
		evg_isa_debug("%20s.%-4d\n", work_group->name, work_item->id_in_work_group);
	}

}


/* Write initial values in constant buffer 0 (CB0) */
/* FIXME: constant memory should be member of 'emu' or 'ndrange'? */
void EvgNDRangeSetupConstantMemory(EvgNDRange *self)
{
	EvgEmu *emu = self->emu;
	struct evg_opencl_kernel_t *kernel = self->kernel;
	unsigned int zero = 0;
	float f;

	/* CB0[0]
	 * x,y,z: global work size for the {x,y,z} dimensions.
	 * w: number of work dimensions.  */
	EvgEmuConstMemWrite(emu, 0, 0, 0, &kernel->global_size3[0], 4);
	EvgEmuConstMemWrite(emu, 0, 0, 1, &kernel->global_size3[1], 4);
	EvgEmuConstMemWrite(emu, 0, 0, 2, &kernel->global_size3[2], 4);
	EvgEmuConstMemWrite(emu, 0, 0, 3, &kernel->work_dim, 4);

	/* CB0[1]
	 * x,y,z: local work size for the {x,y,z} dimensions.
	 * w: 0  */
	EvgEmuConstMemWrite(emu, 0, 1, 0, &kernel->local_size3[0], 4);
	EvgEmuConstMemWrite(emu, 0, 1, 1, &kernel->local_size3[1], 4);
	EvgEmuConstMemWrite(emu, 0, 1, 2, &kernel->local_size3[2], 4);
	EvgEmuConstMemWrite(emu, 0, 1, 3, &zero, 4);

	/* CB0[2]
	 * x,y,z: global work size {x,y,z} / local work size {x,y,z}
	 * w: 0  */
	EvgEmuConstMemWrite(emu, 0, 2, 0, &kernel->group_count3[0], 4);
	EvgEmuConstMemWrite(emu, 0, 2, 1, &kernel->group_count3[1], 4);
	EvgEmuConstMemWrite(emu, 0, 2, 2, &kernel->group_count3[2], 4);
	EvgEmuConstMemWrite(emu, 0, 2, 3, &zero, 4);

	/* CB0[3]
	 * x: Offset to private memory ring (0 if private memory is not emulated).
	 * y: Private memory allocated per work_item.
	 * z,w: 0  */
	/* FIXME */
	
	/* CB0[4]
	 * x: Offset to local memory ring (0 if local memory is not emulated).
	 * y: Local memory allocated per group.
	 * z: 0
	 * w: Pointer to location in global buffer where math library tables start. */
	/* FIXME */

	/* CB[5]
	 * x: 0.0 as IEEE-32bit float - required for math library.
	 * y: 0.5 as IEEE-32bit float - required for math library.
	 * z: 1.0 as IEEE-32bit float - required for math library.
	 * w: 2.0 as IEEE-32bit float - required for math library. */
	f = 0.0f;
	EvgEmuConstMemWrite(emu, 0, 5, 0, &f, 4);
	f = 0.5f;
	EvgEmuConstMemWrite(emu, 0, 5, 1, &f, 4);
	f = 1.0f;
	EvgEmuConstMemWrite(emu, 0, 5, 2, &f, 4);
	f = 2.0f;
	EvgEmuConstMemWrite(emu, 0, 5, 3, &f, 4);

	/* CB0[6]
	 * x,y,z: Global offset for the {x,y,z} dimension of the work_item spawn.
	 * z: Global single dimension flat offset: x * y * z. */
	EvgEmuConstMemWrite(emu, 0, 6, 0, &zero, 4);
	EvgEmuConstMemWrite(emu, 0, 6, 1, &zero, 4);
	EvgEmuConstMemWrite(emu, 0, 6, 2, &zero, 4);
	EvgEmuConstMemWrite(emu, 0, 6, 3, &zero, 4);

	/* CB0[7]
	 * x,y,z: Group offset for the {x,y,z} dimensions of the work_item spawn.
	 * w: Group single dimension flat offset, x * y * z.  */
	EvgEmuConstMemWrite(emu, 0, 7, 0, &zero, 4);
	EvgEmuConstMemWrite(emu, 0, 7, 1, &zero, 4);
	EvgEmuConstMemWrite(emu, 0, 7, 2, &zero, 4);
	EvgEmuConstMemWrite(emu, 0, 7, 3, &zero, 4);

	/* CB0[8]
	 * x: Offset in the global buffer where data segment exists.
	 * y: Offset in buffer for printf support.
	 * z: Size of the printf buffer. */
}


void EvgNDRangeSetupArguments(EvgNDRange *self)
{
	EvgEmu *emu = self->emu;
	OpenclOldDriver *driver = self->driver;

	struct evg_opencl_kernel_t *kernel = self->kernel;
	struct evg_opencl_kernel_arg_t *arg;
	int i;

	/* Kernel arguments */
	for (i = 0; i < list_count(kernel->arg_list); i++)
	{
		arg = list_get(kernel->arg_list, i);
		assert(arg);

		/* Check that argument was set */
		if (!arg->set)
			fatal("kernel '%s': argument '%s' has not been assigned with 'clKernelSetArg'.",
				kernel->name, arg->name);

		/* Process argument depending on its type */
		switch (arg->kind)
		{

		case EVG_OPENCL_KERNEL_ARG_KIND_VALUE:
		{
			/* Value copied directly into device constant memory */
			EvgEmuConstMemWrite(emu, arg->constant_buffer_num, 
				arg->constant_offset/16, 0, &arg->value, 
				arg->size);
			evg_opencl_debug("    arg %d: value loaded into CB1[%d] (size %d)\n", 
				i, arg->constant_offset/16, 
				arg->size);
			break;
		}

		case EVG_OPENCL_KERNEL_ARG_KIND_IMAGE:

			switch (arg->mem_scope) 
			{

			case EVG_OPENCL_MEM_SCOPE_GLOBAL:
			{
				struct evg_opencl_mem_t *mem;

				/* Image type */
				mem = evg_opencl_repo_get_object(
					driver->opencl_repo,
					evg_opencl_object_mem, arg->value[0]);

				EvgEmuConstMemWrite(emu, 
					arg->constant_buffer_num, 
					arg->constant_offset/16, 0, 
					&mem->device_ptr, arg->size);
				evg_opencl_debug("    arg %d: opencl_mem id 0x%x loaded into CB1[%d]," 
						" device_ptr=0x%x\n", i, 
						arg->value[0], 
						arg->constant_offset/16,
						mem->device_ptr);
				break;
			}

			default:
				fatal("%s: argument in memory scope %d not supported",
					__FUNCTION__, arg->mem_scope);
			}
			break;

		case EVG_OPENCL_KERNEL_ARG_KIND_POINTER:
		{
			switch (arg->mem_scope)
			{

			case EVG_OPENCL_MEM_SCOPE_CONSTANT:
			case EVG_OPENCL_MEM_SCOPE_GLOBAL:
			{
				struct evg_opencl_mem_t *mem;

				/* Pointer in __global scope.
				 * Argument value is a pointer to an 'opencl_mem' object.
				 * It is translated first into a device memory pointer. */
				mem = evg_opencl_repo_get_object(
					driver->opencl_repo,
					evg_opencl_object_mem, arg->value[0]);

				EvgEmuConstMemWrite(emu, 
					arg->constant_buffer_num, 
					arg->constant_offset/16, 0, 
					&mem->device_ptr, 4);
				evg_opencl_debug("    arg %d: opencl_mem id 0x%x loaded into CB1[%d]," 
					" device_ptr=0x%x\n", i, arg->value[0], 
					arg->constant_offset/16,
						mem->device_ptr);
				break;
			}

			case EVG_OPENCL_MEM_SCOPE_LOCAL:
			{
				/* Pointer in __local scope.
				 * Argument value is always NULL, 
				 * just assign space for it. */
				EvgEmuConstMemWrite(emu, 
					arg->constant_buffer_num, 
					arg->constant_offset/16, 0, 
					&self->local_mem_top, 4);
				evg_opencl_debug("    arg %d: %d bytes reserved in local memory at 0x%x\n",
					i, arg->size, self->local_mem_top);
					self->local_mem_top += arg->size;
				break;
			}

			default:
				fatal("%s: argument in memory scope %d not supported",
					__FUNCTION__, arg->mem_scope);
			}
			break;
		}

		case EVG_OPENCL_KERNEL_ARG_KIND_SAMPLER:
		{
			evg_opencl_debug("    arg %d: sampler at CB1[%d].x\n", 
				i, arg->constant_offset/16);
			break;
		}

		default:
			fatal("%s: argument type not recognized", __FUNCTION__);
		}
	}
}

