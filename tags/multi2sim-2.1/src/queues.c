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
	opt_reg_uint32("-fetchq_size", "fetch queue size (in CISC inst)", &fetchq_size);
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


int fetchq_can_insert(int core, int thread)
{
	struct list_t *fetchq = THREAD.fetchq;
	return list_count(fetchq) < fetchq_size;
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
		uop_pdg_recover(uop);
		list_remove_at(fetchq, list_count(fetchq) - 1);
		ptrace_end_uop(uop);
		uop->in_fetchq = FALSE;
		uop_free_if_not_queued(uop);
	}
}




/* Instruction Queue */

uint32_t iq_size = 96;
enum iq_kind_enum iq_kind = iq_kind_private;


void iq_reg_options()
{
	static char *iq_kind_map[] = { "shared", "private" };
	opt_reg_enum("-iq_kind", "instruction queue kind {shared|private}",
		(int *) &iq_kind, iq_kind_map, 2);
	opt_reg_uint32("-iq_size", "instruction queue size",
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

	/* dcra */
	if (p_fetch_policy == p_fetch_policy_dcra && iq_kind == iq_kind_shared) {
		THREAD.dcra_active[dcra_resource_iq] = DCRA_ACTIVE_MAX;
		return THREAD.iq_count < THREAD.dcra_limit[dcra_resource_iq];
	}

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
			uop_pdg_recover(uop);
			iq_remove(core, thread);
			uop_free_if_not_queued(uop);
			continue;
		}
		lnlist_next(iq);
	}
}




/* Load Queue */

uint32_t lq_size = 96;
enum lq_kind_enum lq_kind = lq_kind_private;


void lq_reg_options()
{
	static char *lq_kind_map[] = { "shared", "private" };
	opt_reg_enum("-lq_kind", "load queue kind {shared|private}",
		(int *) &lq_kind, lq_kind_map, 2);
	opt_reg_uint32("-lq_size", "load queue size",
		&lq_size);
}


void lq_init()
{
	int core, thread;
	FOREACH_CORE FOREACH_THREAD
		THREAD.lq = lnlist_create();
}


void lq_done()
{
	struct lnlist_t *lq;
	struct uop_t *uop;
	int core, thread;
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
}


int lq_can_insert(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	int count, size;

	/* dcra */
	if (p_fetch_policy == p_fetch_policy_dcra && lq_kind == lq_kind_shared) {
		THREAD.dcra_active[dcra_resource_lq] = DCRA_ACTIVE_MAX;
		return THREAD.lq_count < THREAD.dcra_limit[dcra_resource_lq];
	}

	size = lq_kind == lq_kind_private ? lq_size : lq_size * p_threads;
	count = lq_kind == lq_kind_private ? THREAD.lq_count : CORE.lq_count;
	return count < size;
}


/* Insert a uop into the corresponding load queue. Since this is a non-FIFO queue,
 * the insertion position doesn't matter. */
void lq_insert(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	struct lnlist_t *lq = THREAD.lq;

	assert(!uop->in_lq);
	lnlist_out(lq);
	lnlist_insert(lq, uop);
	uop->in_lq = 1;

	CORE.lq_count++;
	THREAD.lq_count++;
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

	assert(CORE.lq_count && THREAD.lq_count);
	CORE.lq_count--;
	THREAD.lq_count--;
}


/* Remove all speculative uops from the current thread */
void lq_recover(int core, int thread)
{
	struct lnlist_t *lq = THREAD.lq;
	struct uop_t *uop;

	lnlist_head(lq);
	while (!lnlist_eol(lq)) {
		uop = lnlist_get(lq);
		if (uop->specmode) {
			uop_pdg_recover(uop);
			lq_remove(core, thread);
			uop_free_if_not_queued(uop);
			continue;
		}
		lnlist_next(lq);
	}
}




/* Store Queue */

uint32_t sq_size = 32;


void sq_reg_options()
{
	opt_reg_uint32("-sq_size", "load queue size",
		&sq_size);
}


void sq_init()
{
	int core, thread;
	FOREACH_CORE FOREACH_THREAD
		THREAD.sq = lnlist_create();
}


void sq_done()
{
	struct lnlist_t *sq;
	struct uop_t *uop;
	int core, thread;
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


/* Insert a uop into the store queue tail */
void sq_insert(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	struct lnlist_t *sq = THREAD.sq;

	assert(!uop->in_sq);
	lnlist_out(sq);
	lnlist_insert(sq, uop);
	uop->in_sq = 1;

	CORE.sq_count++;
	THREAD.sq_count++;
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

	assert(CORE.sq_count && THREAD.sq_count);
	CORE.sq_count--;
	THREAD.sq_count--;
}


int sq_can_insert(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;

	/* dcra */
	if (p_fetch_policy == p_fetch_policy_dcra && lq_kind == lq_kind_shared) {
		THREAD.dcra_active[dcra_resource_sq] = DCRA_ACTIVE_MAX;
		return THREAD.sq_count < THREAD.dcra_limit[dcra_resource_sq];
	}

	return THREAD.sq_count < sq_size;
}


/* Remove all speculative uops from the current thread */
void sq_recover(int core, int thread)
{
	struct lnlist_t *sq = THREAD.sq;
	struct uop_t *uop;

	lnlist_head(sq);
	while (!lnlist_eol(sq)) {
		uop = lnlist_get(sq);
		if (uop->specmode) {
			uop_pdg_recover(uop);
			sq_remove(core, thread);
			uop_free_if_not_queued(uop);
			continue;
		}
		lnlist_next(sq);
	}
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
			return TRUE;
	}
	return FALSE;
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
			return TRUE;
	}
	return FALSE;
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
	uop->in_eventq = TRUE;
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
	uop->in_eventq = FALSE;
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
			uop_pdg_recover(uop);
			lnlist_remove(eventq);
			uop->in_eventq = 0;
			uop_free_if_not_queued(uop);
			continue;
		}
		lnlist_next(eventq);
	}
}

