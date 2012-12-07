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

#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "cpu.h"
#include "rob.h"
#include "uop.h"



/* Global variables */

char *x86_rob_kind_map[] = { "Private", "Shared" };
enum x86_rob_kind_t x86_rob_kind;
int x86_rob_size;




/* Private variables */

static int x86_rob_total_size = 0;




/*
 * Private Functions
 */

static void x86_rob_trim(int core)
{
	int idx;
	struct x86_uop_t *uop;

	/* Trim head */
	while (X86_CORE.rob_count)
	{
		uop = list_get(X86_CORE.rob, X86_CORE.rob_head);
		if (uop)
			break;
		X86_CORE.rob_head = X86_CORE.rob_head == x86_rob_total_size - 1 ?
			0 : X86_CORE.rob_head + 1;
		X86_CORE.rob_count--;
	}

	/* Trim tail */
	while (X86_CORE.rob_count)
	{
		idx = X86_CORE.rob_tail ? X86_CORE.rob_tail - 1 : x86_rob_total_size - 1;
		uop = list_get(X86_CORE.rob, idx);
		if (uop)
			break;
		X86_CORE.rob_tail = idx;
		X86_CORE.rob_count--;
	}
}




/*
 * Public Functions
 */

void x86_rob_init()
{
	int core;
	int thread;
	int i;

	switch (x86_rob_kind)
	{

	case x86_rob_kind_private:

		/* Initialization */
		X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
		{
			X86_THREAD.rob_left_bound = thread * x86_rob_size;
			X86_THREAD.rob_right_bound = (thread + 1) * x86_rob_size - 1;
			X86_THREAD.rob_head = X86_THREAD.rob_left_bound;
			X86_THREAD.rob_tail = X86_THREAD.rob_left_bound;
		}

		break;
	
	case x86_rob_kind_shared:
		break;
	}

	/* Create ROBs */
	x86_rob_total_size = x86_rob_size * x86_cpu_num_threads;
	X86_CORE_FOR_EACH
	{
		X86_CORE.rob = list_create_with_size(x86_rob_total_size);
		for (i = 0; i < x86_rob_total_size; i++)
			list_add(X86_CORE.rob, NULL);
	}
}


void x86_rob_done()
{
	int core, i;
	struct x86_uop_t *uop;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:
		break;
	
	case x86_rob_kind_shared:
		break;
	}

	X86_CORE_FOR_EACH
	{
		assert(list_count(X86_CORE.rob) == x86_rob_total_size);
		for (i = 0; i < x86_rob_total_size; i++)
		{
			uop = list_get(X86_CORE.rob, i);
			if (uop)
			{
				uop->in_rob = 0;
				x86_uop_free_if_not_queued(uop);
			}
		}
		list_free(X86_CORE.rob);
	}
}


int x86_rob_can_enqueue(struct x86_uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:
		if (X86_THREAD.rob_count < x86_rob_size)
			return 1;
		break;
	
	case x86_rob_kind_shared:
		x86_rob_trim(core);
		if (X86_CORE.rob_count < x86_rob_total_size)
			return 1;
		break;
	}
	return 0;
}


void x86_rob_enqueue(struct x86_uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:
		assert(X86_THREAD.rob_count < x86_rob_size);
		assert(!list_get(X86_CORE.rob, X86_THREAD.rob_tail));
		list_set(X86_CORE.rob, X86_THREAD.rob_tail, uop);
		X86_THREAD.rob_tail = X86_THREAD.rob_tail == X86_THREAD.rob_right_bound ?
			X86_THREAD.rob_left_bound : X86_THREAD.rob_tail + 1;
		X86_THREAD.rob_count++;
		break;
	
	case x86_rob_kind_shared:
		x86_rob_trim(core);
		assert(X86_CORE.rob_count < x86_rob_total_size);
		assert(!list_get(X86_CORE.rob, X86_CORE.rob_tail));
		list_set(X86_CORE.rob, X86_CORE.rob_tail, uop);
		X86_CORE.rob_tail = X86_CORE.rob_tail == x86_rob_total_size - 1 ?
			0 : X86_CORE.rob_tail + 1;
		X86_CORE.rob_count++;
		X86_THREAD.rob_count++;
		break;
	}

	/* Instruction is in the ROB */
	uop->in_rob = 1;
}


int x86_rob_can_dequeue(int core, int thread)
{
	struct x86_uop_t *uop;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:
		if (X86_THREAD.rob_count > 0)
			return 1;
		break;
	
	case x86_rob_kind_shared:
		x86_rob_trim(core);
		if (!X86_CORE.rob_count)
			return 0;
		uop = list_get(X86_CORE.rob, X86_CORE.rob_head);
		assert(x86_uop_exists(uop));
		assert(uop->core == core);
		if (uop->thread == thread)
			return 1;
		break;
	}
	return 0;
}


struct x86_uop_t *x86_rob_head(int core, int thread)
{
	struct x86_uop_t *uop;
	int idx, i;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:
		if (X86_THREAD.rob_count > 0)
		{
			uop = list_get(X86_CORE.rob, X86_THREAD.rob_head);
			return uop;
		}
		break;
	
	case x86_rob_kind_shared:
		x86_rob_trim(core);
		if (!X86_THREAD.rob_count)
			return NULL;
		for (i = 0; i < X86_CORE.rob_count; i++)
		{
			idx = (X86_CORE.rob_head + i) % x86_rob_total_size;
			uop = list_get(X86_CORE.rob, idx);
			if (uop && uop->thread == thread)
				return uop;
		}
		panic("%s: no head found", __FUNCTION__);
		break;
	}
	return NULL;
}


void x86_rob_remove_head(int core, int thread)
{
	struct x86_uop_t *uop = NULL;
	int idx, i;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:
		assert(X86_THREAD.rob_count > 0);
		uop = list_get(X86_CORE.rob, X86_THREAD.rob_head);
		assert(x86_uop_exists(uop));
		assert(uop->core == core && uop->thread == thread);
		list_set(X86_CORE.rob, X86_THREAD.rob_head, NULL);
		X86_THREAD.rob_head = X86_THREAD.rob_head == X86_THREAD.rob_right_bound ?
			X86_THREAD.rob_left_bound : X86_THREAD.rob_head + 1;
		X86_THREAD.rob_count--;
		break;
	
	case x86_rob_kind_shared:
		x86_rob_trim(core);
		assert(X86_THREAD.rob_count);
		for (i = 0; i < X86_CORE.rob_count; i++)
		{
			idx = (X86_CORE.rob_head + i) % x86_rob_total_size;
			uop = list_get(X86_CORE.rob, idx);
			if (uop && uop->thread == thread)
			{
				list_set(X86_CORE.rob, idx, NULL);
				X86_THREAD.rob_count--;
				break;
			}
		}
		break;
	}

	/* Free instruction */
	uop->in_rob = 0;
	x86_uop_free_if_not_queued(uop);
}


struct x86_uop_t *x86_rob_tail(int core, int thread)
{
	struct x86_uop_t *uop;
	int idx, i;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:
		if (X86_THREAD.rob_count > 0) {
			idx = X86_THREAD.rob_tail == X86_THREAD.rob_left_bound ?
				X86_THREAD.rob_right_bound : X86_THREAD.rob_tail - 1;
			uop = list_get(X86_CORE.rob, idx);
			return uop;
		}
		break;
	
	case x86_rob_kind_shared:
		x86_rob_trim(core);
		if (!X86_THREAD.rob_count)
			return NULL;
		for (i = X86_CORE.rob_count - 1; i >= 0; i--)
		{
			idx = (X86_CORE.rob_head + i) % x86_rob_total_size;
			uop = list_get(X86_CORE.rob, idx);
			if (uop && uop->thread == thread)
				return uop;
		}
		panic("rob_tail: no tail found");
		break;
	}
	return NULL;
}


struct x86_uop_t *x86_rob_get(int core, int thread, int index)
{
	struct x86_uop_t *uop;

	/* Check that index is in bounds */
	if (index < 0 || index >= X86_THREAD.rob_count)
		return NULL;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:
		index += X86_THREAD.rob_head;
		if (index > X86_THREAD.rob_right_bound)
			index = index - X86_THREAD.rob_right_bound + X86_THREAD.rob_left_bound - 1;
		uop = list_get(X86_CORE.rob, index);
		assert(uop);
		return uop;
	
	case x86_rob_kind_shared:
		x86_rob_trim(core);
		index = (X86_CORE.rob_head + index) % x86_rob_total_size;
		uop = list_get(X86_CORE.rob, index);
		assert(uop);
		return uop;
	}
	return NULL;
}


void x86_rob_remove_tail(int core, int thread)
{
	struct x86_uop_t *uop = NULL;
	int idx, i;

	switch (x86_rob_kind)
	{
	case x86_rob_kind_private:
		assert(X86_THREAD.rob_count > 0);
		idx = X86_THREAD.rob_tail == X86_THREAD.rob_left_bound ?
			X86_THREAD.rob_right_bound : X86_THREAD.rob_tail - 1;
		uop = list_get(X86_CORE.rob, idx);
		assert(x86_uop_exists(uop));
		assert(uop->core == core && uop->thread == thread);
		list_set(X86_CORE.rob, idx, NULL);
		X86_THREAD.rob_tail = idx;
		X86_THREAD.rob_count--;
		break;
	
	case x86_rob_kind_shared:
		x86_rob_trim(core);
		assert(X86_THREAD.rob_count);
		for (i = X86_CORE.rob_count - 1; i >= 0; i--)
		{
			idx = (X86_CORE.rob_head + i) % x86_rob_total_size;
			uop = list_get(X86_CORE.rob, idx);
			if (uop && uop->thread == thread)
			{
				list_set(X86_CORE.rob, idx, NULL);
				X86_THREAD.rob_count--;
				break;
			}
		}
		break;
	}

	/* Free instruction */
	uop->in_rob = 0;
	x86_uop_free_if_not_queued(uop);
}


void x86_rob_dump(int core, FILE *f)
{
	int thread, i;
	struct x86_uop_t *uop;

	switch (x86_rob_kind)
	{

	case x86_rob_kind_private:
	{
		X86_THREAD_FOR_EACH
		{
			fprintf(f, "  rob for thread %d (entries %d to %d), count=%d, size=%d\n",
				thread, X86_THREAD.rob_left_bound, X86_THREAD.rob_right_bound,
				X86_THREAD.rob_count, x86_rob_size);
			for (i = X86_THREAD.rob_left_bound; i <= X86_THREAD.rob_right_bound; i++)
			{
				uop = list_get(X86_CORE.rob, i);
				fprintf(f, "   %c%c ",
					i == X86_THREAD.rob_head ? 'H' : ' ',
					i == X86_THREAD.rob_tail ? 'T' : ' ');
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
		x86_rob_trim(core);
		for (i = 0; i < x86_rob_total_size; i++)
		{
			uop = list_get(X86_CORE.rob, i);
			fprintf(f, " %c%c ",
				i == X86_CORE.rob_head ? 'H' : ' ',
				i == X86_CORE.rob_tail ? 'T' : ' ');
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
