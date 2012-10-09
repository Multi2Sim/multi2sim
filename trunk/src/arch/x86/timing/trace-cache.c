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

#include "x86-timing.h"


/* Parameters */

int x86_trace_cache_present;  /* Use trace cache */
int x86_trace_cache_num_sets;  /* Number of sets */
int x86_trace_cache_assoc;  /* Number of ways */
int x86_trace_cache_trace_size;  /* Maximum number of uops in a trace */
int x86_trace_cache_branch_max;  /* Maximum number of branches in a trace */
int x86_trace_cache_queue_size;  /* Fetch queue for pre-decoded uops */


void x86_trace_cache_init()
{
	int core;
	int thread;

	/* Trace cache present */
	if (!x86_trace_cache_present)
		return;

	/* FIXME: trace cache disabled.
	 * New mechanism of micro-instruction generation is not supported by
	 * old trace cache implementation.
	 */
	fatal("trace cache temporarily not supported.\n"
		"\tAs of SVN Rev. 330, a new mechanism has been implemented for x86 CISC\n"
		"\tinstructions decoding. This is a more accurate mechanism that is not\n"
		"\tcompatible with the old trace cache model. To request this update, please\n"
		"\temail development@multi2sim.org, or use the Multi2Sim forum.\n");

	/* Integrity */
	if ((x86_trace_cache_num_sets & (x86_trace_cache_num_sets - 1)) || !x86_trace_cache_num_sets)
		fatal("trace cache sets must be power of 2 and > 0");
	if ((x86_trace_cache_assoc & (x86_trace_cache_assoc - 1)) || !x86_trace_cache_assoc)
		fatal("trace cache associativity must be power of 2 and > 0");
	if (!x86_trace_cache_trace_size || !x86_trace_cache_branch_max)
		fatal("trace cache: max number of branches and micro-instructions must be > 0");
	if (x86_trace_cache_branch_max > x86_trace_cache_trace_size)
		fatal("max branches in trace cache cannot be greater than trace size");
	if (x86_trace_cache_branch_max > 31)
		fatal("max branches must be less than 32");
	
	/* Initialization */
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		X86_THREAD.trace_cache = x86_trace_cache_create();
		sprintf(X86_THREAD.trace_cache->name, "c%dt%d.trace_cache", core, thread);
	}
}


void x86_trace_cache_done()
{
	int core;
	int thread;

	/* Trace cache present */
	if (!x86_trace_cache_present)
		return;
	
	/* Finalization */
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
		x86_trace_cache_free(X86_THREAD.trace_cache);
}


struct x86_trace_cache_t *x86_trace_cache_create()
{
	struct x86_trace_cache_t *trace_cache;
	struct x86_trace_cache_entry_t *entry;

	int set;
	int way;

	/* Create trace cache */
	trace_cache = calloc(1, sizeof(struct x86_trace_cache_t));
	if (!trace_cache)
		fatal("%s: out of memory", __FUNCTION__);

	/* Entries */
	trace_cache->entry = calloc(x86_trace_cache_num_sets * x86_trace_cache_assoc, X86_TRACE_CACHE_ENTRY_SIZE);
	if (!trace_cache->entry)
		fatal("%s: out of memory", __FUNCTION__);

	/* Temporary trace */
	trace_cache->temp = calloc(1, X86_TRACE_CACHE_ENTRY_SIZE);
	if (!trace_cache->temp)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize LRU counter */
	for (set = 0; set < x86_trace_cache_num_sets; set++)
	{
		for (way = 0; way < x86_trace_cache_assoc; way++)
		{
			entry = X86_TRACE_CACHE_ENTRY(set, way);
			entry->counter = way;
		}
	}

	/* Return */
	return trace_cache;
}


void x86_trace_cache_free(struct x86_trace_cache_t *trace_cache)
{
	free(trace_cache->entry);
	free(trace_cache->temp);
	free(trace_cache);
}


void x86_trace_cache_dump_report(struct x86_trace_cache_t *trace_cache, FILE *f)
{
	fprintf(f, "# Trace cache - parameters\n");
	fprintf(f, "TraceCache.Sets = %d\n", x86_trace_cache_num_sets);
	fprintf(f, "TraceCache.Assoc = %d\n", x86_trace_cache_assoc);
	fprintf(f, "TraceCache.TraceSize = %d\n", x86_trace_cache_trace_size);
	fprintf(f, "TraceCache.BranchMax = %d\n", x86_trace_cache_branch_max);
	fprintf(f, "TraceCache.QueueSize = %d\n", x86_trace_cache_queue_size);
	fprintf(f, "\n");

	fprintf(f, "# Trace cache - statistics\n");
	fprintf(f, "TraceCache.Accesses = %lld\n", trace_cache->accesses);
	fprintf(f, "TraceCache.Hits = %lld\n", trace_cache->hits);
	fprintf(f, "TraceCache.HitRatio = %.4g\n", trace_cache->accesses ? (double)
		trace_cache->hits / trace_cache->accesses : 0.0);
	fprintf(f, "TraceCache.Committed = %lld\n", trace_cache->committed);
	fprintf(f, "TraceCache.Squashed = %lld\n", trace_cache->squashed);
	fprintf(f, "TraceCache.TraceLength = %.2g\n",
		trace_cache->trace_length_count ? (double) trace_cache->trace_length_acc /
			trace_cache->trace_length_count : 0);
	fprintf(f, "\n");
}


/* Flush temporary trace of committed instructions back into the trace cache */
static void x86_trace_cache_flush_trace(struct x86_trace_cache_t *trace_cache)
{
	struct x86_trace_cache_entry_t *entry, *found = NULL;
	struct x86_trace_cache_entry_t *trace = trace_cache->temp;

	int set;
	int way;

	/* There must be something to commit */
	if (!trace->uop_count)
		return;
	assert(trace->tag);

	/* If last instruction was a branch, remove it from the mask and flags fields,
	 * since this prediction does not affect the trace. Instead, the 'target'
	 * field of the trace cache line will be stored. */
	if (trace->target)
	{
		assert(trace->branch_count);
		trace->branch_count--;
		trace->branch_mask &= ~(1 << trace->branch_count);
		trace->branch_flags &= ~(1 << trace->branch_count);
	}

	/* Allocate new line for the trace. If trace is already in the cache,
	 * do nothing. If there is any invalid entry, choose it. */
	set = trace->tag % x86_trace_cache_num_sets;
	for (way = 0; way < x86_trace_cache_assoc; way++)
	{
		entry = X86_TRACE_CACHE_ENTRY(set, way);
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
	if (!found)
	{
		for (way = 0; way < x86_trace_cache_assoc; way++)
		{
			entry = X86_TRACE_CACHE_ENTRY(set, way);
			entry->counter--;
			if (entry->counter < 0) {
				entry->counter = x86_trace_cache_assoc - 1;
				found = entry;
			}
		}
	}

	/* Flush temporary trace and reset it. When flushing, all fields are
	 * copied except for LRU counter. */
	assert(found);
	trace_cache->trace_length_acc += trace->uop_count;
	trace_cache->trace_length_count++;
	trace->counter = found->counter;
	memcpy(found, trace, X86_TRACE_CACHE_ENTRY_SIZE);
	memset(trace_cache->temp, 0, X86_TRACE_CACHE_ENTRY_SIZE);
}


void x86_trace_cache_new_uop(struct x86_trace_cache_t *trace_cache, struct x86_uop_t *uop)
{
	struct x86_trace_cache_entry_t *trace = trace_cache->temp;
	int taken;

	/* Only uops heading the macroinst are inserted in the trace for simulation. */
	/* FIXME: wrong. Control instructions and string operations generate intermediate
	 * branch instructions. This is not handled properly.
	 * FIXME: trace cache option disabled for now. */
	if (uop->mop_index)
		return;

	/* If there is not enough space for macroinst, commit trace.
	 * If macroinst does not fit in line, it cannot be included in the trace. */
	assert(!uop->specmode);
	assert(uop->eip);
	assert(uop->id == uop->mop_id);
	if (trace->uop_count + uop->mop_count > x86_trace_cache_trace_size)
		x86_trace_cache_flush_trace(trace_cache);
	if (uop->mop_count > x86_trace_cache_trace_size)
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
	if (uop->flags & X86_UINST_CTRL)
	{
		taken = uop->neip != uop->eip + uop->mop_size;
		trace->branch_mask |= 1 << trace->branch_count;
		trace->branch_flags |= taken << trace->branch_count;
		trace->branch_count++;
		trace->target = uop->target_neip;
		if (trace->branch_count == x86_trace_cache_branch_max)
			x86_trace_cache_flush_trace(trace_cache);
	}
}


int x86_trace_cache_lookup(struct x86_trace_cache_t *trace_cache, uint32_t eip, int pred,
	int *ptr_mop_count, uint32_t **ptr_mop_array, uint32_t *ptr_neip)
{
	struct x86_trace_cache_entry_t *entry = NULL, *found = NULL;
	int set, way;
	uint32_t neip;

	/* Look for trace cache line */
	set = eip % x86_trace_cache_num_sets;
	for (way = 0; way < x86_trace_cache_assoc; way++)
	{
		entry = X86_TRACE_CACHE_ENTRY(set, way);
		if (entry->tag == eip && ((pred & entry->branch_mask) == entry->branch_flags))
		{
			found = entry;
			break;
		}
	}

	/* If there was a miss, do nothing else */
	trace_cache->accesses++;
	if (!found)
		return 0;
	trace_cache->hits++;
	
	/* Trace cache hit. Return fields. The next address to fetch is 'target' if
	 * the last instruction in the trace is a branch, and 'pred' tells us it is taken. */
	neip = entry->target && (pred & (1 << entry->branch_count)) ?
		entry->target : entry->fall_through;
	PTR_ASSIGN(ptr_mop_count, entry->mop_count);
	PTR_ASSIGN(ptr_mop_array, entry->mop_array);
	PTR_ASSIGN(ptr_neip, neip);
	return 1;
}
