/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <m2s.h>



/* Variables */

uint32_t rob_size = 64;
enum rob_kind_enum rob_kind = rob_kind_private;
int total_rob_size = 0;




/* Private Functions */

static void rob_trim(int core)
{
	int idx;
	struct uop_t *uop;

	/* Trim head */
	while (CORE.rob_count) {
		uop = list_get(CORE.rob, CORE.rob_head);
		if (uop)
			break;
		CORE.rob_head = CORE.rob_head == total_rob_size - 1 ?
			0 : CORE.rob_head + 1;
		CORE.rob_count--;
	}

	/* Trim tail */
	while (CORE.rob_count) {
		idx = CORE.rob_tail ? CORE.rob_tail - 1 : total_rob_size - 1;
		uop = list_get(CORE.rob, idx);
		if (uop)
			break;
		CORE.rob_tail = idx;
		CORE.rob_count--;
	}
}




/* Public Functions */

void rob_reg_options()
{
	static char *rob_kind_map[] = { "private", "shared" };
	opt_reg_enum("-rob_kind", "reorder buffer sharing {private|shared}",
		(int *) &rob_kind, rob_kind_map, 2);
	opt_reg_uint32("-rob_size", "reorder buffer size per thread",
		&rob_size);
}


void rob_init()
{
	int core, thread;
	int i;

	switch (rob_kind) {

	case rob_kind_private:

		/* Initialization */
		FOREACH_CORE FOREACH_THREAD {
			THREAD.rob_left_bound = thread * rob_size;
			THREAD.rob_right_bound = (thread + 1) * rob_size - 1;
			THREAD.rob_head = THREAD.rob_left_bound;
			THREAD.rob_tail = THREAD.rob_left_bound;
		}

		break;
	
	case rob_kind_shared:
		break;
	}

	/* Create ROBs */
	total_rob_size = rob_size * p_threads;
	FOREACH_CORE {
		CORE.rob = list_create(total_rob_size);
		for (i = 0; i < total_rob_size; i++)
			list_add(CORE.rob, NULL);
	}
}


void rob_done()
{
	int core, i;
	struct uop_t *uop;

	switch (rob_kind) {
	case rob_kind_private:
		break;
	
	case rob_kind_shared:
		break;
	}

	FOREACH_CORE {
		assert(list_count(CORE.rob) == total_rob_size);
		for (i = 0; i < total_rob_size; i++) {
			uop = list_get(CORE.rob, i);
			if (uop) {
				uop->in_rob = 0;
				uop_free_if_not_queued(uop);
			}
		}
		list_free(CORE.rob);
	}
}


int rob_can_enqueue(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;

	switch (rob_kind) {
	case rob_kind_private:
		if (THREAD.rob_count < rob_size)
			return 1;
		break;
	
	case rob_kind_shared:
		rob_trim(core);
		if (CORE.rob_count < total_rob_size)
			return 1;
		break;
	}
	return 0;
}


void rob_enqueue(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;

	switch (rob_kind) {
	case rob_kind_private:
		assert(THREAD.rob_count < rob_size);
		assert(!list_get(CORE.rob, THREAD.rob_tail));
		list_set(CORE.rob, THREAD.rob_tail, uop);
		THREAD.rob_tail = THREAD.rob_tail == THREAD.rob_right_bound ?
			THREAD.rob_left_bound : THREAD.rob_tail + 1;
		THREAD.rob_count++;
		break;
	
	case rob_kind_shared:
		rob_trim(core);
		assert(CORE.rob_count < total_rob_size);
		assert(!list_get(CORE.rob, CORE.rob_tail));
		list_set(CORE.rob, CORE.rob_tail, uop);
		CORE.rob_tail = CORE.rob_tail == total_rob_size - 1 ?
			0 : CORE.rob_tail + 1;
		CORE.rob_count++;
		THREAD.rob_count++;
		break;
	}

	/* Instruction is in the ROB */
	uop->in_rob = 1;
}


int rob_can_dequeue(int core, int thread)
{
	struct uop_t *uop;

	switch (rob_kind) {
	case rob_kind_private:
		if (THREAD.rob_count > 0)
			return 1;
		break;
	
	case rob_kind_shared:
		rob_trim(core);
		if (!CORE.rob_count)
			return 0;
		uop = list_get(CORE.rob, CORE.rob_head);
		assert(uop_exists(uop));
		assert(uop->core == core);
		if (uop->thread == thread)
			return 1;
		break;
	}
	return 0;
}


struct uop_t *rob_head(int core, int thread)
{
	struct uop_t *uop;
	int idx, i;

	switch (rob_kind) {
	case rob_kind_private:
		if (THREAD.rob_count > 0) {
			uop = list_get(CORE.rob, THREAD.rob_head);
			return uop;
		}
		break;
	
	case rob_kind_shared:
		rob_trim(core);
		if (!THREAD.rob_count)
			return NULL;
		for (i = 0; i < CORE.rob_count; i++) {
			idx = (CORE.rob_head + i) % total_rob_size;
			uop = list_get(CORE.rob, idx);
			if (uop && uop->thread == thread)
				return uop;
		}
		panic("rob_head: no head found");
		break;
	}
	return NULL;
}


void rob_remove_head(int core, int thread)
{
	struct uop_t *uop = NULL;
	int idx, i;

	switch (rob_kind) {
	case rob_kind_private:
		assert(THREAD.rob_count > 0);
		uop = list_get(CORE.rob, THREAD.rob_head);
		assert(uop_exists(uop));
		assert(uop->core == core && uop->thread == thread);
		list_set(CORE.rob, THREAD.rob_head, NULL);
		THREAD.rob_head = THREAD.rob_head == THREAD.rob_right_bound ?
			THREAD.rob_left_bound : THREAD.rob_head + 1;
		THREAD.rob_count--;
		break;
	
	case rob_kind_shared:
		rob_trim(core);
		assert(THREAD.rob_count);
		for (i = 0; i < CORE.rob_count; i++) {
			idx = (CORE.rob_head + i) % total_rob_size;
			uop = list_get(CORE.rob, idx);
			if (uop && uop->thread == thread) {
				list_set(CORE.rob, idx, NULL);
				THREAD.rob_count--;
				break;
			}
		}
		break;
	}

	/* Free instruction */
	uop->in_rob = 0;
	uop_free_if_not_queued(uop);
}


struct uop_t *rob_tail(int core, int thread)
{
	struct uop_t *uop;
	int idx, i;

	switch (rob_kind) {
	case rob_kind_private:
		if (THREAD.rob_count > 0) {
			idx = THREAD.rob_tail == THREAD.rob_left_bound ?
				THREAD.rob_right_bound : THREAD.rob_tail - 1;
			uop = list_get(CORE.rob, idx);
			return uop;
		}
		break;
	
	case rob_kind_shared:
		rob_trim(core);
		if (!THREAD.rob_count)
			return NULL;
		for (i = CORE.rob_count - 1; i >= 0; i--) {
			idx = (CORE.rob_head + i) % total_rob_size;
			uop = list_get(CORE.rob, idx);
			if (uop && uop->thread == thread)
				return uop;
		}
		panic("rob_tail: no tail found");
		break;
	}
	return NULL;
}


struct uop_t *rob_get(int core, int thread, int index)
{
	struct uop_t *uop;

	/* Check that index is in bounds */
	if (index < 0 || index >= THREAD.rob_count)
		return NULL;

	switch (rob_kind) {
	case rob_kind_private:
		index += THREAD.rob_head;
		if (index > THREAD.rob_right_bound)
			index = index - THREAD.rob_right_bound + THREAD.rob_left_bound - 1;
		uop = list_get(CORE.rob, index);
		assert(uop);
		return uop;
	
	case rob_kind_shared:
		rob_trim(core);
		index = (CORE.rob_head + index) % total_rob_size;
		uop = list_get(CORE.rob, index);
		assert(uop);
		return uop;
	}
	return NULL;
}


void rob_remove_tail(int core, int thread)
{
	struct uop_t *uop = NULL;
	int idx, i;

	switch (rob_kind) {
	case rob_kind_private:
		assert(THREAD.rob_count > 0);
		idx = THREAD.rob_tail == THREAD.rob_left_bound ?
			THREAD.rob_right_bound : THREAD.rob_tail - 1;
		uop = list_get(CORE.rob, idx);
		assert(uop_exists(uop));
		assert(uop->core == core && uop->thread == thread);
		list_set(CORE.rob, idx, NULL);
		THREAD.rob_tail = idx;
		THREAD.rob_count--;
		break;
	
	case rob_kind_shared:
		rob_trim(core);
		assert(THREAD.rob_count);
		for (i = CORE.rob_count - 1; i >= 0; i--) {
			idx = (CORE.rob_head + i) % total_rob_size;
			uop = list_get(CORE.rob, idx);
			if (uop && uop->thread == thread) {
				list_set(CORE.rob, idx, NULL);
				THREAD.rob_count--;
				break;
			}
		}
		break;
	}

	/* Free instruction */
	uop->in_rob = 0;
	uop_free_if_not_queued(uop);
}


void rob_dump(int core, FILE *f)
{
	int thread, i;
	struct uop_t *uop;

	switch (rob_kind) {
	case rob_kind_private:
		FOREACH_THREAD {
			fprintf(f, "  rob for thread %d (entries %d to %d), count=%d, size=%d\n",
				thread, THREAD.rob_left_bound, THREAD.rob_right_bound,
				THREAD.rob_count, rob_size);
			for (i = THREAD.rob_left_bound; i <= THREAD.rob_right_bound; i++) {
				uop = list_get(CORE.rob, i);
				fprintf(f, "   %c%c ",
					i == THREAD.rob_head ? 'H' : ' ',
					i == THREAD.rob_tail ? 'T' : ' ');
				if (uop)
					uop_dump(uop, f), fprintf(f, "\n");
				else
					fprintf(f, "-\n");
			}
		}
		break;
	
	case rob_kind_shared:
		rob_trim(core);
		for (i = 0; i < total_rob_size; i++) {
			uop = list_get(CORE.rob, i);
			fprintf(f, " %c%c ",
				i == CORE.rob_head ? 'H' : ' ',
				i == CORE.rob_tail ? 'T' : ' ');
			if (uop)
				uop_dump(uop, f), fprintf(f, "\n");
			else
				fprintf(f, "-\n");
		}
		break;
	}
}

