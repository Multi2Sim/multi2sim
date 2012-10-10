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



#define X86_TRACE_CACHE_ENTRY_SIZE \
	(sizeof(struct x86_trace_cache_entry_t) + \
	sizeof(uint32_t) * x86_trace_cache_trace_size)
#define X86_TRACE_CACHE_ENTRY(SET, WAY) \
	((struct x86_trace_cache_entry_t *) (((unsigned char *) trace_cache->entry) + \
	X86_TRACE_CACHE_ENTRY_SIZE * ((SET) * x86_trace_cache_assoc + (WAY))))

struct x86_trace_cache_entry_t
{
	int counter;  /* lru counter */
	unsigned int tag;
	int uop_count, mop_count;
	int branch_mask, branch_flags, branch_count;
	unsigned int fall_through;
	unsigned int target;

	/* Last field. This is a list of 'x86_trace_cache_trace_size' elements containing
	 * the addresses of the microinst located in the trace. Only in the case that
	 * all macroinst are decoded into just one uop can this array be filled up. */
	unsigned int mop_array[0];
};

struct x86_trace_cache_t
{
	/* Entries (sets * assoc) */
	struct x86_trace_cache_entry_t *entry;
	struct x86_trace_cache_entry_t *temp;  /* Temporary trace */

	/* Stats */
	char name[20];
	long long accesses;
	long long hits;
	long long committed;
	long long squashed;
	long long trace_length_acc;
	long long trace_length_count;
};


extern int x86_trace_cache_present;
extern int x86_trace_cache_num_sets;
extern int x86_trace_cache_assoc;
extern int x86_trace_cache_trace_size;
extern int x86_trace_cache_branch_max;
extern int x86_trace_cache_queue_size;

void x86_trace_cache_init(void);
void x86_trace_cache_done(void);
void x86_trace_cache_dump_report(struct x86_trace_cache_t *trace_cache, FILE *f);

struct x86_trace_cache_t *x86_trace_cache_create(void);
void x86_trace_cache_free(struct x86_trace_cache_t *trace_cache);

void x86_trace_cache_new_uop(struct x86_trace_cache_t *trace_cache, struct x86_uop_t *uop);
int x86_trace_cache_lookup(struct x86_trace_cache_t *trace_cache, uint32_t eip, int pred,
	int *ptr_mop_count, uint32_t **ptr_mop_array, uint32_t *ptr_neip);


#endif

