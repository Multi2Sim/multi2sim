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




/* Instruction Fetch Queue */

uint32_t fetchq_size = 64;


void fetchq_reg_options()
{
	opt_reg_uint32("-fetchq_size", "fetch queue size in bytes", &fetchq_size);
}


void fetchq_init()
{
	int core, thread;
	FOREACH_CORE FOREACH_THREAD
		THREAD.fetchq = list_create(fetchq_size);
}


void fetchq_done()
{
	int core, thread;
	struct list_t *fetchq;
	struct uop_t *uop;

	FOREACH_CORE FOREACH_THREAD {
		fetchq = THREAD.fetchq;
		while (list_count(fetchq)) {
			uop = list_remove_at(fetchq, 0);
			uop->in_fetchq = 0;
			uop_free_if_not_queued(uop);
		}
		list_free(fetchq);
	}
}


struct uop_t *fetchq_remove(int core, int thread, int index)
{
	struct list_t *fetchq = THREAD.fetchq;
	struct uop_t *uop;
	assert(index >= 0 && index < list_count(fetchq));
	uop = list_remove_at(fetchq, index);
	uop->in_fetchq = 0;
	if (!uop->fetch_tcache && !uop->mop_index) {
		THREAD.fetchq_occ -= uop->mop_size;
		assert(THREAD.fetchq_occ >= 0);
	}
	if (uop->fetch_tcache) {
		THREAD.tcacheq_occ--;
		assert(THREAD.tcacheq_occ >= 0);
	}
	if (!list_count(fetchq)) {
		assert(!THREAD.fetchq_occ);
		assert(!THREAD.tcacheq_occ);
	}
	return uop;
}


void fetchq_recover(int core, int thread)
{
	struct list_t *fetchq = THREAD.fetchq;
	struct uop_t *uop;

	while (list_count(fetchq)) {
		uop = list_get(fetchq, list_count(fetchq) - 1);
		assert(uop->thread == thread);
		if (!uop->specmode)
			break;
		uop = fetchq_remove(core, thread, list_count(fetchq) - 1);
		uop_free_if_not_queued(uop);
	}
}




/* Uop Queue */

uint32_t uopq_size = 32;


void uopq_reg_options()
{
	opt_reg_uint32("-uopq_size", "uop queue size in microinstructions", &uopq_size);
}


void uopq_init()
{
	int core, thread;
	FOREACH_CORE FOREACH_THREAD
		THREAD.uopq = list_create(uopq_size);
}


void uopq_done()
{
	int core, thread;
	struct list_t *uopq;
	struct uop_t *uop;

	FOREACH_CORE FOREACH_THREAD {
		uopq = THREAD.uopq;
		while (list_count(uopq)) {
			uop = list_remove_at(uopq, 0);
			uop->in_uopq = 0;
			uop_free_if_not_queued(uop);
		}
		list_free(uopq);
	}
}


void uopq_recover(int core, int thread)
{
	struct list_t *uopq = THREAD.uopq;
	struct uop_t *uop;

	while (list_count(uopq)) {
		uop = list_get(uopq, list_count(uopq) - 1);
		assert(uop->thread == thread);
		if (!uop->specmode)
			break;
		list_remove_at(uopq, list_count(uopq) - 1);
		uop->in_uopq = 0;
		uop_free_if_not_queued(uop);
	}
}




/* Instruction Queue */

uint32_t iq_size = 40;
enum iq_kind_enum iq_kind = iq_kind_private;


void iq_reg_options()
{
	static char *iq_kind_map[] = { "shared", "private" };
	opt_reg_enum("-iq_kind", "instruction queue kind {shared|private}",
		(int *) &iq_kind, iq_kind_map, 2);
	opt_reg_uint32("-iq_size", "instruction queue size per thread",
		&iq_size);
}


void iq_init()
{
	int core, thread;
	FOREACH_CORE FOREACH_THREAD
		THREAD.iq = lnlist_create();
}


void iq_done()
{
	struct lnlist_t *iq;
	struct uop_t *uop;
	int core, thread;
	FOREACH_CORE FOREACH_THREAD {
		iq = THREAD.iq;
		lnlist_head(iq);
		while (lnlist_count(iq)) {
			uop = lnlist_get(iq);
			uop->in_iq = 0;
			lnlist_remove(iq);
			uop_free_if_not_queued(uop);
		}
		lnlist_free(iq);
	}
}


int iq_can_insert(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	int count, size;

	size = iq_kind == iq_kind_private ? iq_size : iq_size * p_threads;
	count = iq_kind == iq_kind_private ? THREAD.iq_count : CORE.iq_count;
	return count < size;
}


/* Insert a uop into the corresponding IQ. Since this is a non-FIFO queue,
 * the insertion position doesn't matter. */
void iq_insert(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	struct lnlist_t *iq = THREAD.iq;

	assert(!uop->in_iq);
	lnlist_out(iq);
	lnlist_insert(iq, uop);
	uop->in_iq = 1;

	CORE.iq_count++;
	THREAD.iq_count++;
}


/* Remove the uop in the current position of the linked list representing
 * the IQ of the specified thread. */
void iq_remove(int core, int thread)
{
	struct lnlist_t *iq = THREAD.iq;
	struct uop_t *uop;

	uop = lnlist_get(iq);
	assert(uop_exists(uop));
	lnlist_remove(iq);
	uop->in_iq = 0;

	assert(CORE.iq_count && THREAD.iq_count);
	CORE.iq_count--;
	THREAD.iq_count--;
}


/* Remove all speculative uops from the current thread */
void iq_recover(int core, int thread)
{
	struct lnlist_t *iq = THREAD.iq;
	struct uop_t *uop;

	lnlist_head(iq);
	while (!lnlist_eol(iq)) {
		uop = lnlist_get(iq);
		if (uop->specmode) {
			iq_remove(core, thread);
			uop_free_if_not_queued(uop);
			continue;
		}
		lnlist_next(iq);
	}
}




/* Load/Store Queue */

uint32_t lsq_size = 20;
enum lsq_kind_enum lsq_kind = lsq_kind_private;


void lsq_reg_options()
{
	static char *lsq_kind_map[] = { "shared", "private" };
	opt_reg_enum("-lsq_kind", "load/store queue kind {shared|private}",
		(int *) &lsq_kind, lsq_kind_map, 2);
	opt_reg_uint32("-lsq_size", "load/store queue size per thread",
		&lsq_size);
}


void lsq_init()
{
	int core, thread;
	FOREACH_CORE FOREACH_THREAD {
		THREAD.lq = lnlist_create();
		THREAD.sq = lnlist_create();
	}
}


void lsq_done()
{
	struct lnlist_t *lq, *sq;
	struct uop_t *uop;
	int core, thread;

	/* Load queue */
	FOREACH_CORE FOREACH_THREAD {
		lq = THREAD.lq;
		lnlist_head(lq);
		while (lnlist_count(lq)) {
			uop = lnlist_get(lq);
			uop->in_lq = 0;
			lnlist_remove(lq);
			uop_free_if_not_queued(uop);
		}
		lnlist_free(lq);
	}

	/* Store queue */
	FOREACH_CORE FOREACH_THREAD {
		sq = THREAD.sq;
		lnlist_head(sq);
		while (lnlist_count(sq)) {
			uop = lnlist_get(sq);
			uop->in_sq = 0;
			lnlist_remove(sq);
			uop_free_if_not_queued(uop);
		}
		lnlist_free(sq);
	}
}


int lsq_can_insert(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	int count, size;

	size = lsq_kind == lsq_kind_private ? lsq_size : lsq_size * p_threads;
	count = lsq_kind == lsq_kind_private ? THREAD.lsq_count : CORE.lsq_count;
	return count < size;
}


/* Insert uop into corresponding load/store queue */
void lsq_insert(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	struct lnlist_t *lq = THREAD.lq;
	struct lnlist_t *sq = THREAD.sq;

	assert(!uop->in_lq && !uop->in_sq);
	assert((uop->flags & FLOAD) || (uop->flags & FSTORE));
	if (uop->flags & FLOAD) {
		lnlist_out(lq);
		lnlist_insert(lq, uop);
		uop->in_lq = 1;
	} else {
		lnlist_out(sq);
		lnlist_insert(sq, uop);
		uop->in_sq = 1;
	}
	CORE.lsq_count++;
	THREAD.lsq_count++;
}


/* Remove all speculative uops from a given load/store queue in the
 * given thread. */
void lsq_recover(int core, int thread)
{
	struct lnlist_t *lq = THREAD.lq;
	struct lnlist_t *sq = THREAD.sq;
	struct uop_t *uop;

	/* Recover load queue */
	lnlist_head(lq);
	while (!lnlist_eol(lq)) {
		uop = lnlist_get(lq);
		if (uop->specmode) {
			lq_remove(core, thread);
			uop_free_if_not_queued(uop);
			continue;
		}
		lnlist_next(lq);
	}

	/* Recover store queue */
	lnlist_head(sq);
	while (!lnlist_eol(sq)) {
		uop = lnlist_get(sq);
		if (uop->specmode) {
			sq_remove(core, thread);
			uop_free_if_not_queued(uop);
			continue;
		}
		lnlist_next(sq);
	}
}


/* Remove the uop in the current position of the linked list representing
 * the load queue of the specified thread. */
void lq_remove(int core, int thread)
{
	struct lnlist_t *lq = THREAD.lq;
	struct uop_t *uop;

	uop = lnlist_get(lq);
	assert(uop_exists(uop));
	lnlist_remove(lq);
	uop->in_lq = 0;

	assert(CORE.lsq_count && THREAD.lsq_count);
	CORE.lsq_count--;
	THREAD.lsq_count--;
}


/* Remove an uop in the current position of the store queue */
void sq_remove(int core, int thread)
{
	struct lnlist_t *sq = THREAD.sq;
	struct uop_t *uop;

	uop = lnlist_get(sq);
	assert(uop_exists(uop));
	assert(uop->in_sq);
	lnlist_remove(sq);
	uop->in_sq = 0;

	assert(CORE.lsq_count && THREAD.lsq_count);
	CORE.lsq_count--;
	THREAD.lsq_count--;
}




/* Event Queue */

void eventq_init()
{
	int core;
	FOREACH_CORE
		CORE.eventq = lnlist_create();
}


void eventq_done()
{
	int core;
	FOREACH_CORE {
		while (lnlist_count(CORE.eventq))
			uop_free_if_not_queued(eventq_extract(CORE.eventq));
		lnlist_free(CORE.eventq);
	}
}


static int eventq_compare(const void *item1, const void *item2)
{
	const struct uop_t *uop1 = item1;
	const struct uop_t *uop2 = item2;
	return uop1->when != uop2->when ? uop1->when - uop2->when
		: uop1->seq - uop2->seq;
}


int eventq_longlat(int core, int thread)
{
	struct lnlist_t *eventq = CORE.eventq;
	struct uop_t *uop;
	
	for (lnlist_head(eventq); !lnlist_eol(eventq); lnlist_next(eventq)) {
		uop = lnlist_get(eventq);
		if (uop->thread != thread)
			continue;
		if (sim_cycle - uop->issue_when > 20)
			return 1;
	}
	return 0;
}


int eventq_cachemiss(int core, int thread)
{
	struct lnlist_t *eventq = CORE.eventq;
	struct uop_t *uop;

	for (lnlist_head(eventq); !lnlist_eol(eventq); lnlist_next(eventq)) {
		uop = lnlist_get(eventq);
		if (uop->thread != thread || !(uop->flags & FLOAD))
			continue;
		if (sim_cycle - uop->issue_when > 5)
			return 1;
	}
	return 0;
}


void eventq_insert(struct lnlist_t *eventq, struct uop_t *uop)
{
	struct uop_t *item;
	assert(!uop->in_eventq);
	lnlist_head(eventq);
	for (;;) {
		item = lnlist_get(eventq);
		if (!item || eventq_compare(uop, item) < 0)
			break;
		lnlist_next(eventq);
	}
	lnlist_insert(eventq, uop);
	uop->in_eventq = 1;
}


struct uop_t *eventq_extract(struct lnlist_t *eventq)
{
	struct uop_t *uop;
	if (!lnlist_count(eventq))
		return NULL;
	lnlist_head(eventq);
	uop = lnlist_get(eventq);
	assert(uop_exists(uop));
	assert(uop->in_eventq);
	lnlist_remove(eventq);
	uop->in_eventq = 0;
	return uop;
}


void eventq_recover(int core, int thread)
{
	struct lnlist_t *eventq = CORE.eventq;
	struct uop_t *uop;

	lnlist_head(eventq);
	while (!lnlist_eol(eventq)) {
		uop = lnlist_get(eventq);
		if (uop->thread == thread && uop->specmode) {
			lnlist_remove(eventq);
			uop->in_eventq = 0;
			uop_free_if_not_queued(uop);
			continue;
		}
		lnlist_next(eventq);
	}
}

