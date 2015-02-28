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

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <memory/memory.h>

#include "ndrange.h"
#include "wavefront.h"
#include "work-group.h"



/*
 * Class 'EvgWorkGroup'
 */

void EvgWorkGroupCreate(EvgWorkGroup *self, EvgNDRange *ndrange)
{
	/* Initialize */
	self->ndrange = ndrange;
	self->local_mem = mem_create();
	self->local_mem->safe = 0;
}


void EvgWorkGroupDestroy(EvgWorkGroup *self)
{
	mem_free(self->local_mem);
	str_free(self->name);
}


int EvgWorkGroupGetState(EvgWorkGroup *self, EvgWorkGroupState state)
{
	return (self->status & state) > 0;
}


void EvgWorkGroupSetState(EvgWorkGroup *self, EvgWorkGroupState state)
{
	EvgNDRange *ndrange = self->ndrange;

	/* Get only the new bits */
	state &= ~self->status;

	/* Add work-group to lists */
	if (state & EvgWorkGroupPending)
		DOUBLE_LINKED_LIST_INSERT_TAIL(ndrange, pending, self);
	if (state & EvgWorkGroupRunning)
		DOUBLE_LINKED_LIST_INSERT_TAIL(ndrange, running, self);
	if (state & EvgWorkGroupFinished)
		DOUBLE_LINKED_LIST_INSERT_TAIL(ndrange, finished, self);

	/* Update it */
	self->status |= state;
}


void EvgWorkGroupClearState(EvgWorkGroup *self, EvgWorkGroupState state)
{
	EvgNDRange *ndrange = self->ndrange;

	/* Get only the bits that are set */
	state &= self->status;

	/* Remove work-group from lists */
	if (state & EvgWorkGroupPending)
		DOUBLE_LINKED_LIST_REMOVE(ndrange, pending, self);
	if (state & EvgWorkGroupRunning)
		DOUBLE_LINKED_LIST_REMOVE(ndrange, running, self);
	if (state & EvgWorkGroupFinished)
		DOUBLE_LINKED_LIST_REMOVE(ndrange, finished, self);
	
	/* Update status */
	self->status &= ~state;
}


void EvgWorkGroupDump(EvgWorkGroup *self, FILE *f)
{
	EvgNDRange *ndrange = self->ndrange;
	EvgWavefront *wavefront;
	int wavefront_id;

	if (!f)
		return;
	
	fprintf(f, "[ NDRange[%d].WorkGroup[%d] ]\n\n", ndrange->id, self->id);
	fprintf(f, "Name = %s\n", self->name);
	fprintf(f, "WaveFrontFirst = %d\n", self->wavefront_id_first);
	fprintf(f, "WaveFrontLast = %d\n", self->wavefront_id_last);
	fprintf(f, "WaveFrontCount = %d\n", self->wavefront_count);
	fprintf(f, "WorkItemFirst = %d\n", self->work_item_id_first);
	fprintf(f, "WorkItemLast = %d\n", self->work_item_id_last);
	fprintf(f, "WorkItemCount = %d\n", self->work_item_count);
	fprintf(f, "\n");

	/* Dump wavefronts */
	EVG_FOREACH_WAVEFRONT_IN_WORK_GROUP(self, wavefront_id)
	{
		wavefront = ndrange->wavefronts[wavefront_id];
		EvgWavefrontDump(wavefront, f);
	}
}


void EvgWorkGroupSetName(EvgWorkGroup *self, char *name)
{
	self->name = str_set(self->name, name);
}
