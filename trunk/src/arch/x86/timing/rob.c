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


#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "core.h"
#include "cpu.h"
#include "rob.h"
#include "uop.h"
#include "thread.h"



/* Global variables */

char *x86_rob_kind_map[] = { "Private", "Shared" };
enum x86_rob_kind_t x86_rob_kind;
int x86_rob_size;




/* Private variables */

static int x86_rob_total_size = 0;



/*
 * Class 'X86Core'
 */


void X86CoreInitROB(X86Core *self)
{
	X86Thread *thread;
	int i;

	switch (x86_rob_kind)
	{

	case x86_rob_kind_private:

		/* Initialization */
		for (i = 0; i < x86_cpu_num_threads; i++)
		{
			thread = self->threads[i];
			thread->rob_left_bound = i * x86_rob_size;
			thread->rob_right_bound = (i + 1) * x86_rob_size - 1;
			thread->rob_head = thread->rob_left_bound;
			thread->rob_tail = thread->rob_left_bound;
		}

		break;
	
	case x86_rob_kind_shared:
		break;
	}

	/* Create ROBs */
	x86_rob_total_size = x86_rob_size * x86_cpu_num_threads;
	self->rob = list_create_with_size(x86_rob_total_size);
	for (i = 0; i < x86_rob_total_size; i++)
		list_add(self->rob, NULL);
}


void X86CoreFreeROB(X86Core *self)
{
	int i;
	struct x86_uop_t *uop;

	assert(list_count(self->rob) == x86_rob_total_size);
	for (i = 0; i < x86_rob_total_size; i++)
	{
		uop = list_get(self->rob, i);
		if (uop)
		{
			uop->in_rob = 0;
			x86_uop_free_if_not_queued(uop);
		}
	}
	list_free(self->rob);
}


static void X86CoreTrimROB(X86Core *self)
{
	int idx;
	struct x86_uop_t *uop;

	/* Trim head */
	while (self->rob_count)
	{
		uop = list_get(self->rob, self->rob_head);
		if (uop)
			break;
		self->rob_head = self->rob_head == x86_rob_total_size - 1 ?
			0 : self->rob_head + 1;
		self->rob_count--;
	}

	/* Trim tail */
	while (self->rob_count)
	{
		idx = self->rob_tail ? self->rob_tail - 1 : x86_rob_total_size - 1;
		uop = list_get(self->rob, idx);
		if (uop)
			break;
		self->rob_tail = idx;
		self->rob_count--;
	}
}


void X86CoreDumpROB(X86Core *self, FILE *f)
{
	X86Thread *thread;
	int i, j;
	struct x86_uop_t *uop;

	switch (x86_rob_kind)
	{

	case x86_rob_kind_private:
	{
		for (i = 0; i < x86_cpu_num_threads; i++)
		{
			thread = self->threads[i];
			fprintf(f, "  rob for thread %s (entries %d to %d), count=%d, size=%d\n",
				thread->name, thread->rob_left_bound, thread->rob_right_bound,
				thread->rob_count, x86_rob_size);
			for (j = thread->rob_left_bound; j <= thread->rob_right_bound; j++)
			{
				uop = list_get(self->rob, j);
				fprintf(f, "   %c%c ",
					j == thread->rob_head ? 'H' : ' ',
					j == thread->rob_tail ? 'T' : ' ');
				if (uop)
				{
					x86_uinst_dump(uop->uinst, f);
					fprintf(f, "\n");
				}
				else
				{
					fprintf(f, "-\n");
				}
			}
		}
		break;
	}
	
	case x86_rob_kind_shared:
	{
		X86CoreTrimROB(self);
		for (j = 0; j < x86_rob_total_size; j++)
		{
			uop = list_get(self->rob, j);
			fprintf(f, " %c%c ",
				j == self->rob_head ? 'H' : ' ',
				j == self->rob_tail ? 'T' : ' ');
			if (uop)
			{
				x86_uinst_dump(uop->uinst, f);
				fprintf(f, "\n");
			}
			else
			{
				fprintf(f, "-\n");
			}
		}
		break;
	}

	default:
		panic("%s: invalid ROB type", __FUNCTION__);
	}
}




/*
 * Class 'X86Thread'
 */

int X86ThreadCanDequeueFromROB(X86Thread *self)
{
	X86Core *core = self->core;
	struct x86_uop_t *uop;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:

		if (self->rob_count > 0)
			return 1;

		break;
	
	case x86_rob_kind_shared:

		X86CoreTrimROB(core);
		if (!core->rob_count)
			return 0;

		uop = list_get(core->rob, core->rob_head);
		assert(x86_uop_exists(uop));
		if (uop->thread == self)
			return 1;
		break;
	}
	return 0;
}


struct x86_uop_t *X86ThreadGetROBHead(X86Thread *self)
{
	X86Core *core = self->core;
	struct x86_uop_t *uop;
	int idx;
	int i;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:

		if (self->rob_count > 0)
		{
			uop = list_get(core->rob, self->rob_head);
			return uop;
		}
		break;
	
	case x86_rob_kind_shared:

		X86CoreTrimROB(core);
		if (!self->rob_count)
			return NULL;

		for (i = 0; i < core->rob_count; i++)
		{
			idx = (core->rob_head + i) % x86_rob_total_size;
			uop = list_get(core->rob, idx);
			if (uop && uop->thread == self)
				return uop;
		}
		panic("%s: no head found", __FUNCTION__);
		break;
	}
	return NULL;
}


void X86ThreadRemoveROBHead(X86Thread *self)
{
	X86Core *core = self->core;
	struct x86_uop_t *uop = NULL;
	int idx, i;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:

		assert(self->rob_count > 0);
		uop = list_get(core->rob, self->rob_head);
		assert(x86_uop_exists(uop));
		assert(uop->thread == self);
		list_set(core->rob, self->rob_head, NULL);
		self->rob_head = self->rob_head == self->rob_right_bound ?
			self->rob_left_bound : self->rob_head + 1;
		self->rob_count--;
		break;
	
	case x86_rob_kind_shared:
		X86CoreTrimROB(core);
		assert(self->rob_count);
		for (i = 0; i < core->rob_count; i++)
		{
			idx = (core->rob_head + i) % x86_rob_total_size;
			uop = list_get(core->rob, idx);
			if (uop && uop->thread == self)
			{
				list_set(core->rob, idx, NULL);
				self->rob_count--;
				break;
			}
		}
		break;
	}

	/* Free instruction */
	uop->in_rob = 0;
	x86_uop_free_if_not_queued(uop);
}


struct x86_uop_t *X86ThreadGetROBTail(X86Thread *self)
{
	X86Core *core = self->core;
	struct x86_uop_t *uop;
	int idx, i;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:

		if (self->rob_count > 0)
		{
			idx = self->rob_tail == self->rob_left_bound ?
				self->rob_right_bound : self->rob_tail - 1;
			uop = list_get(core->rob, idx);
			return uop;
		}
		break;
	
	case x86_rob_kind_shared:

		X86CoreTrimROB(core);
		if (!self->rob_count)
			return NULL;
		for (i = core->rob_count - 1; i >= 0; i--)
		{
			idx = (core->rob_head + i) % x86_rob_total_size;
			uop = list_get(core->rob, idx);
			if (uop && uop->thread == self)
				return uop;
		}
		panic("rob_tail: no tail found");
		break;
	}
	return NULL;
}


struct x86_uop_t *X86GetROBEntry(X86Thread *self, int index)
{
	X86Core *core = self->core;
	struct x86_uop_t *uop;

	/* Check that index is in bounds */
	if (index < 0 || index >= self->rob_count)
		return NULL;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:
		index += self->rob_head;
		if (index > self->rob_right_bound)
			index = index - self->rob_right_bound + self->rob_left_bound - 1;
		uop = list_get(core->rob, index);
		assert(uop);
		return uop;
	
	case x86_rob_kind_shared:
		X86CoreTrimROB(core);
		index = (core->rob_head + index) % x86_rob_total_size;
		uop = list_get(core->rob, index);
		assert(uop);
		return uop;
	}
	return NULL;
}


void X86ThreadRemoveROBTail(X86Thread *self)
{
	X86Core *core = self->core;
	struct x86_uop_t *uop = NULL;
	int idx, i;

	switch (x86_rob_kind)
	{

	case x86_rob_kind_private:

		assert(self->rob_count > 0);
		idx = self->rob_tail == self->rob_left_bound ?
			self->rob_right_bound : self->rob_tail - 1;
		uop = list_get(core->rob, idx);
		assert(x86_uop_exists(uop));
		assert(uop->thread == self);
		list_set(core->rob, idx, NULL);
		self->rob_tail = idx;
		self->rob_count--;
		break;
	
	case x86_rob_kind_shared:

		X86CoreTrimROB(core);
		assert(self->rob_count);
		for (i = core->rob_count - 1; i >= 0; i--)
		{
			idx = (core->rob_head + i) % x86_rob_total_size;
			uop = list_get(core->rob, idx);
			if (uop && uop->thread == self)
			{
				list_set(core->rob, idx, NULL);
				self->rob_count--;
				break;
			}
		}
		break;
	}

	/* Free instruction */
	uop->in_rob = 0;
	x86_uop_free_if_not_queued(uop);
}




/*
 * Public
 */

int X86CoreCanEnqueueInROB(X86Core *self, struct x86_uop_t *uop)
{
	X86Thread *thread;

	switch (x86_rob_kind)
	{

	case x86_rob_kind_private:

		thread = uop->thread;
		if (thread->rob_count < x86_rob_size)
			return 1;
		break;
	
	case x86_rob_kind_shared:

		X86CoreTrimROB(self);
		if (self->rob_count < x86_rob_total_size)
			return 1;
		break;
	}

	return 0;
}


void X86CoreEnqueueInROB(X86Core *self, struct x86_uop_t *uop)
{
	X86Thread *thread;

	thread = uop->thread;
	switch (x86_rob_kind)
	{

	case x86_rob_kind_private:

		assert(thread->rob_count < x86_rob_size);
		assert(!list_get(self->rob, thread->rob_tail));
		list_set(self->rob, thread->rob_tail, uop);
		thread->rob_tail = thread->rob_tail == thread->rob_right_bound ?
				thread->rob_left_bound : thread->rob_tail + 1;
		thread->rob_count++;
		break;
	
	case x86_rob_kind_shared:

		X86CoreTrimROB(self);
		assert(self->rob_count < x86_rob_total_size);
		assert(!list_get(self->rob, self->rob_tail));
		list_set(self->rob, self->rob_tail, uop);
		self->rob_tail = self->rob_tail == x86_rob_total_size - 1 ?
			0 : self->rob_tail + 1;
		self->rob_count++;
		thread->rob_count++;
		break;
	}

	/* Instruction is in the ROB */
	uop->in_rob = 1;
}



