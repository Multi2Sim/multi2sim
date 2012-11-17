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

#include <lib/util/linked-list.h>

#include "cpu.h"
#include "inst-queue.h"


char *x86_iq_kind_map[] = { "Shared", "Private" };
enum x86_iq_kind_t x86_iq_kind;
int x86_iq_size;


void x86_iq_init()
{
	int core;
	int thread;

	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
		X86_THREAD.iq = linked_list_create();
}


void x86_iq_done()
{
	struct linked_list_t *iq;
	struct x86_uop_t *uop;

	int core;
	int thread;

	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		iq = X86_THREAD.iq;
		linked_list_head(iq);
		while (linked_list_count(iq))
		{
			uop = linked_list_get(iq);
			uop->in_iq = 0;
			linked_list_remove(iq);
			x86_uop_free_if_not_queued(uop);
		}
		linked_list_free(iq);
	}
}


int x86_iq_can_insert(struct x86_uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	int count, size;

	size = x86_iq_kind == x86_iq_kind_private ? x86_iq_size : x86_iq_size * x86_cpu_num_threads;
	count = x86_iq_kind == x86_iq_kind_private ? X86_THREAD.iq_count : X86_CORE.iq_count;
	return count < size;
}


/* Insert a uop into the corresponding IQ. Since this is a non-FIFO queue,
 * the insertion position doesn't matter. */
void x86_iq_insert(struct x86_uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	struct linked_list_t *iq = X86_THREAD.iq;

	assert(!uop->in_iq);
	linked_list_out(iq);
	linked_list_insert(iq, uop);
	uop->in_iq = 1;

	X86_CORE.iq_count++;
	X86_THREAD.iq_count++;
}


/* Remove the uop in the current position of the linked list representing
 * the IQ of the specified thread. */
void x86_iq_remove(int core, int thread)
{
	struct linked_list_t *iq = X86_THREAD.iq;
	struct x86_uop_t *uop;

	uop = linked_list_get(iq);
	assert(x86_uop_exists(uop));
	linked_list_remove(iq);
	uop->in_iq = 0;

	assert(X86_CORE.iq_count && X86_THREAD.iq_count);
	X86_CORE.iq_count--;
	X86_THREAD.iq_count--;
}


/* Remove all speculative uops from the current thread */
void x86_iq_recover(int core, int thread)
{
	struct linked_list_t *iq = X86_THREAD.iq;
	struct x86_uop_t *uop;

	linked_list_head(iq);
	while (!linked_list_is_end(iq))
	{
		uop = linked_list_get(iq);
		if (uop->specmode) {
			x86_iq_remove(core, thread);
			x86_uop_free_if_not_queued(uop);
			continue;
		}
		linked_list_next(iq);
	}
}

