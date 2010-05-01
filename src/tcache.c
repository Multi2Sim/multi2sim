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


/* Parameters */

int tcache_present = 0;
uint32_t tcache_trace_size = 16;
uint32_t tcache_branch_max = 3;
uint32_t tcache_queue_size = 32;
static char *tcache_topo = "64:4";
static uint32_t tcache_sets;
static uint32_t tcache_assoc;


void tcache_reg_options()
{
	opt_reg_bool("-tcache", "Use trace cache {t|f}", &tcache_present);
	opt_reg_string("-tcache:topo", "Trace cache topology (<sets>:<assoc>)",
		&tcache_topo);
	opt_reg_uint32("-tcache:trace_size", "Maximum number of uops in a trace",
		&tcache_trace_size);
	opt_reg_uint32("-tcache:branch_max", "Maximum number of branches in a trace",
		&tcache_branch_max);
	opt_reg_uint32("-tcache:queue_size", "Fetch queue for predecoded uops",
		&tcache_queue_size);
}


void tcache_init()
{
	int core, thread;

	/* Trace cache present */
	if (!tcache_present)
		return;

	/* Obtain sets and assoc */
	if (sscanf(tcache_topo, "%d:%d", &tcache_sets, &tcache_assoc) != 2)
		fatal("invalid tcache:topo format");

	/* Integrity */
	if ((tcache_sets & (tcache_sets - 1)) || !tcache_sets)
		fatal("trace cache sets must be power of 2 and > 0");
	if ((tcache_assoc & (tcache_assoc - 1)) || !tcache_assoc)
		fatal("trace cache associativity must be power of 2 and > 0");
	if (!tcache_trace_size || !tcache_branch_max)
		fatal("trace cache: max number of branches and microinst must be > 0");
	if (tcache_branch_max > tcache_trace_size)
		fatal("tcache:branch_max cannot be greater than tcache:trace_size");
	if (tcache_branch_max > 31)
		fatal("tcache:branch_max cannot be greater than 31");
	
	/* Initialization */
	FOREACH_CORE FOREACH_THREAD {
		THREAD.tcache = tcache_create();
		sprintf(THREAD.tcache->name, "c%dt%d.tcache", core, thread);
	}
}


void tcache_done()
{
	int core, thread;

	/* Trace cache present */
	if (!tcache_present)
		return;
	
	/* Finalization */
	FOREACH_CORE FOREACH_THREAD
		tcache_free(THREAD.tcache);
}


struct tcache_t *tcache_create()
{
	struct tcache_t *tcache;
	struct tcache_entry_t *entry;
	int set, way;

	/* Create tcache */
	tcache = calloc(1, sizeof(struct tcache_t));
	tcache->entry = calloc(tcache_sets * tcache_assoc, TCACHE_ENTRY_SIZE);
	tcache->temp = calloc(1, TCACHE_ENTRY_SIZE);

	/* Init lru counter */
	for (set = 0; set < tcache_sets; set++) {
		for (way = 0; way < tcache_assoc; way++) {
			entry = TCACHE_ENTRY(set, way);
			entry->counter = way;
		}
	}

	/* Return */
	return tcache;
}


void tcache_free(struct tcache_t *tcache)
{
	free(tcache->entry);
	free(tcache->temp);
	free(tcache);
}


void tcache_dump_report(struct tcache_t *tcache, FILE *f)
{
	/* Print stats */
	fprintf(f, "# Trace cache\n");
	fprintf(f, "TraceCache.Accesses = %lld\n", (long long) tcache->accesses);
	fprintf(f, "TraceCache.Hits = %lld\n", (long long) tcache->hits);
	fprintf(f, "TraceCache.HitRatio = %.4g\n", tcache->accesses ? (double)
		tcache->hits / tcache->accesses : 0.0);
	fprintf(f, "TraceCache.Committed = %lld\n", (long long) tcache->committed);
	fprintf(f, "TraceCache.Squashed = %lld\n", (long long) tcache->squashed);
	fprintf(f, "TraceCache.TraceLength = %.2g\n",
		tcache->trace_length_count ? (double) tcache->trace_length_acc /
			tcache->trace_length_count : 0);
	fprintf(f, "\n");
}


/* Flush temporary trace of committed instructions back into the trace cache */
static void tcache_flush_trace(struct tcache_t *tcache)
{
	struct tcache_entry_t *entry, *found = NULL, *trace = tcache->temp;
	int set, way;

	/* There must be something to commit */
	if (!trace->uop_count)
		return;
	assert(trace->tag);

	/* If last instruction was a branch, remove it from the mask and flags fields,
	 * since this prediction does not affect the trace. Instead, the 'target'
	 * field of the trace cache line will be stored. */
	if (trace->target) {
		assert(trace->branch_count);
		trace->branch_count--;
		trace->branch_mask &= ~(1 << trace->branch_count);
		trace->branch_flags &= ~(1 << trace->branch_count);
	}

	/* Allocate new line for the trace. If trace is already in the cache,
	 * do nothing. If there is any invalid entry, choose it. */
	set = trace->tag % tcache_sets;
	for (way = 0; way < tcache_assoc; way++) {
		entry = TCACHE_ENTRY(set, way);
		if (entry->tag == trace->tag && entry->branch_mask == trace->branch_mask
			&& entry->branch_flags == trace->branch_flags)
		{
			found = entry;
			break;
		}
		if (!entry->tag) {
			found = entry;
			break;
		}
	}

	/* If no invalid entry found, look for LRU. */
	if (!found) {
		for (way = 0; way < tcache_assoc; way++) {
			entry = TCACHE_ENTRY(set, way);
			entry->counter--;
			if (entry->counter < 0) {
				entry->counter = tcache_assoc - 1;
				found = entry;
			}
		}
	}

	/* Flush temporary trace and reset it. When flushing, all fields are
	 * copied except for LRU counter. */
	assert(found);
	tcache->trace_length_acc += trace->uop_count;
	tcache->trace_length_count++;
	trace->counter = found->counter;
	memcpy(found, trace, TCACHE_ENTRY_SIZE);
	memset(tcache->temp, 0, TCACHE_ENTRY_SIZE);
}


void tcache_new_uop(struct tcache_t *tcache, struct uop_t *uop)
{
	struct tcache_entry_t *trace = tcache->temp;
	int taken;

	/* Only uops heading the macroinst are inserted in the trace for simulation. */
	if (uop->mop_index)
		return;

	/* If there is not enough space for macroinst, commit trace.
	 * If macroinst does not fit in line, it cannot be included in the trace. */
	assert(!uop->specmode);
	assert(uop->eip);
	assert(uop->seq == uop->mop_seq);
	if (trace->uop_count + uop->mop_count > tcache_trace_size)
		tcache_flush_trace(tcache);
	if (uop->mop_count > tcache_trace_size)
		return;

	/* First instruction. Store trace tag. */
	if (!trace->uop_count)
		trace->tag = uop->eip;
	
	/* Add eip to list */
	trace->mop_array[trace->mop_count] = uop->eip;
	trace->mop_count++;
	trace->uop_count += uop->mop_count;
	trace->target = 0;
	trace->fall_through = uop->eip + uop->mop_size;

	/* Instruction is branch. If maximum number of branches is reached,
	 * commit trace. */
	if (uop->flags & FCTRL) {
		taken = uop->neip != uop->eip + uop->mop_size;
		trace->branch_mask |= 1 << trace->branch_count;
		trace->branch_flags |= taken << trace->branch_count;
		trace->branch_count++;
		trace->target = uop->target_neip;
		if (trace->branch_count == tcache_branch_max)
			tcache_flush_trace(tcache);
	}
}


int tcache_lookup(struct tcache_t *tcache, uint32_t eip, int pred,
	int *ptr_mop_count, uint32_t **ptr_mop_array, uint32_t *ptr_neip)
{
	struct tcache_entry_t *entry = NULL, *found = NULL;
	int set, way;
	uint32_t neip;

	/* Look for trace cache line */
	set = eip % tcache_sets;
	for (way = 0; way < tcache_assoc; way++) {
		entry = TCACHE_ENTRY(set, way);
		if (entry->tag == eip && ((pred & entry->branch_mask) == entry->branch_flags)) {
			found = entry;
			break;
		}
	}

	/* If there was a miss, do nothing else */
	tcache->accesses++;
	if (!found)
		return 0;
	tcache->hits++;
	
	/* Trace cache hit. Return fields. The next address to fetch is 'target' if
	 * the last instruction in the trace is a branch, and 'pred' tells us it is taken. */
	neip = entry->target && (pred & (1 << entry->branch_count)) ?
		entry->target : entry->fall_through;
	PTR_ASSIGN(ptr_mop_count, entry->mop_count);
	PTR_ASSIGN(ptr_mop_array, entry->mop_array);
	PTR_ASSIGN(ptr_neip, neip);
	return 1;
}

