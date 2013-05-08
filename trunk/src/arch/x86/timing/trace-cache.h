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

#ifndef X86_ARCH_TIMING_TRACE_CACHE_H
#define X86_ARCH_TIMING_TRACE_CACHE_H


/* Debug */
#define x86_trace_cache_debugging() debug_status(x86_trace_cache_debug_category)
#define x86_trace_cache_debug(...) debug(x86_trace_cache_debug_category, __VA_ARGS__)
extern int x86_trace_cache_debug_category;


#define X86_TRACE_CACHE_ENTRY_SIZE \
	(sizeof(struct x86_trace_cache_entry_t) + \
	sizeof(unsigned int) * x86_trace_cache_trace_size)
#define X86_TRACE_CACHE_ENTRY(SET, WAY) \
	((struct x86_trace_cache_entry_t *) (((unsigned char *) trace_cache->entry) + \
	X86_TRACE_CACHE_ENTRY_SIZE * ((SET) * x86_trace_cache_assoc + (WAY))))

struct x86_trace_cache_entry_t
{
	/* LRU counter */
	int counter;

	/* Address of the first instruction in the trace */
	unsigned int tag;

	/* Number of micro- and macro-instructions in the cache line */
	int uop_count;
	int mop_count;

	/*trace->branch_mask |= 1 << trace->branch_count;
	trace->branch_flags |= taken << trace->branch_count;*/

	/* Number of branches in the trace */
	int branch_count;

	/* Bit mask of 'branch_count' bits, with all bits set to one. */
	int branch_mask;

	/* Bit mask of 'branch_count' bits. A bit set to one represents a
	 * taken branch. The MSB corresponds to the last branch in the trace.
	 * The LSB corresponds to the first branch in the trace. */
	int branch_flags;

	/* Address of the instruction following the last instruction in the
	 * trace. */
	unsigned int fall_through;

	/* Address of the target address of the last branch in the trace */
	unsigned int target;

	/* This field has to be the last in the structure.
	 * It is a list composed of 'x86_trace_cache_trace_size' elements.
	 * Each element contains the address of the micro-instructions in the trace.
	 * Only if each single micro-instructions comes from a different macro-
	 * instruction can this array be full. */
	unsigned int mop_array[0];
};

struct x86_trace_cache_t
{
	char *name;

	/* Thread where it belongs */
	struct x86_thread_t *thread;

	/* Trace cache lines ('sets' * 'assoc' elements) */
	struct x86_trace_cache_entry_t *entry;

	/* Temporary trace, progressively filled up in the commit stage,
	 * and dumped into the trace cache when full. */
	struct x86_trace_cache_entry_t *temp;

	/* Statistics */
	long long accesses;
	long long hits;
	long long num_fetched_uinst;
	long long num_dispatched_uinst;
	long long num_issued_uinst;
	long long num_committed_uinst;
	long long num_squashed_uinst;
	long long trace_length_acc;
	long long trace_length_count;
};


extern int x86_trace_cache_present;
extern int x86_trace_cache_num_sets;
extern int x86_trace_cache_assoc;
extern int x86_trace_cache_trace_size;
extern int x86_trace_cache_branch_max;
extern int x86_trace_cache_queue_size;


struct config_t;
void x86_trace_cache_read_config(struct config_t *config);

void x86_trace_cache_init(void);
void x86_trace_cache_done(void);
void x86_trace_cache_dump_report(struct x86_trace_cache_t *trace_cache, FILE *f);

struct x86_trace_cache_t *x86_trace_cache_create(char *name, struct x86_thread_t *thread);
void x86_trace_cache_free(struct x86_trace_cache_t *trace_cache);

void x86_trace_cache_new_uop(struct x86_trace_cache_t *trace_cache, struct x86_uop_t *uop);
int x86_trace_cache_lookup(struct x86_trace_cache_t *trace_cache, unsigned int eip, int pred,
	int *ptr_mop_count, unsigned int **ptr_mop_array, unsigned int *ptr_neip);


#endif

