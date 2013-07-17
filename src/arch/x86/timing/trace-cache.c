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


#include <arch/x86/emu/context.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "core.h"
#include "cpu.h"
#include "trace-cache.h"
#include "uop.h"
#include "thread.h"



/*
 * Class 'X86Thread'
 */

void X86ThreadInitTraceCache(X86Thread *self)
{
	X86Core *core = self->core;
	char name[MAX_STRING_SIZE];

	/* Trace cache present */
	if (!x86_trace_cache_present)
		return;

	/* Initialization */
	snprintf(name, sizeof name,"Core[%d].Thread[%d].TraceCache",
			core->id, self->id_in_core);
	self->trace_cache = x86_trace_cache_create(name);
}


void X86ThreadFreeTraceCache(X86Thread *self)
{
	/* Trace cache present */
	if (!x86_trace_cache_present)
		return;
	
	/* Finalization */
	x86_trace_cache_free(self->trace_cache);
}


void X86ThreadDumpTraceCacheReport(X86Thread *self, FILE *f)
{
	struct x86_trace_cache_t *trace_cache = self->trace_cache;

	fprintf(f, "; Trace cache - parameters\n");
	fprintf(f, "TraceCache.Sets = %d\n", x86_trace_cache_num_sets);
	fprintf(f, "TraceCache.Assoc = %d\n", x86_trace_cache_assoc);
	fprintf(f, "TraceCache.TraceSize = %d\n", x86_trace_cache_trace_size);
	fprintf(f, "TraceCache.BranchMax = %d\n", x86_trace_cache_branch_max);
	fprintf(f, "TraceCache.QueueSize = %d\n", x86_trace_cache_queue_size);
	fprintf(f, "\n");

	fprintf(f, "; Trace cache - statistics\n");
	fprintf(f, "TraceCache.Accesses = %lld\n", trace_cache->accesses);
	fprintf(f, "TraceCache.Hits = %lld\n", trace_cache->hits);
	fprintf(f, "TraceCache.HitRatio = %.4g\n", trace_cache->accesses ? (double)
		trace_cache->hits / trace_cache->accesses : 0.0);
	fprintf(f, "TraceCache.Fetched = %lld\n", trace_cache->num_fetched_uinst);
	fprintf(f, "TraceCache.Dispatched = %lld\n", trace_cache->num_dispatched_uinst);
	fprintf(f, "TraceCache.Issued = %lld\n", trace_cache->num_issued_uinst);
	fprintf(f, "TraceCache.Committed = %lld\n", trace_cache->num_committed_uinst);
	fprintf(f, "TraceCache.Squashed = %lld\n", trace_cache->num_squashed_uinst);
	fprintf(f, "TraceCache.TraceLength = %.2f\n",
		trace_cache->trace_length_count ? (double) trace_cache->trace_length_acc /
			trace_cache->trace_length_count : 0);
	fprintf(f, "\n");
}


/* Dump branch prediction sequence */
static void x86_trace_cache_pred_dump(int pred, int num, FILE *f)
{
	int i;
	char *comma;

	comma = "";
	fprintf(f, "(");
	for (i = 0; i < num; i++)
	{
		fprintf(f, "%s%c", comma, pred & (1 << i) ? 'T' : 'n');
		comma = "-";
	}
	fprintf(f, ")");
}


/* Dump trace line */
static void X86ThraceDumpTraceCacheEntry(X86Thread *self,
	struct x86_trace_cache_entry_t *entry, FILE *f)
{
	struct mem_t *mem;
	char *comma;
	int i;

	/* Get memory object for the thread */
	assert(self->ctx && self->ctx->mem);
	mem = self->ctx->mem;

	/* Tag and number of branches */
	fprintf(f, "tag = 0x%x, ", entry->tag);

	/* Branch flags */
	fprintf(f, "branches = ");
	x86_trace_cache_pred_dump(entry->branch_flags, entry->branch_count, f);
	fprintf(f, "\n");

	/* Fall-through and target address */
	fprintf(f, "fall_through = 0x%x, ", entry->fall_through);
	fprintf(f, "target = 0x%x\n", entry->target);

	/* Macro-Instructions */
	fprintf(f, "uops = %d, ", entry->uop_count);
	fprintf(f, "mops = {");
	comma = "";
	for (i = 0; i < entry->mop_count; i++)
	{
		char mop_name[MAX_STRING_SIZE];
		char *mop_name_end;
		char mop_bytes[20];

		unsigned int addr;

		int mem_safe;
		int mop_name_length;

		struct x86_inst_t inst;

		/* Disable memory safe mode */
		mem_safe = mem->safe;
		mem->safe = 0;

		/* Read instruction */
		addr = entry->mop_array[i];
		mem_read(mem, addr, sizeof mop_bytes, mop_bytes);

		/* Disassemble */
		x86_inst_decode(&inst, addr, mop_bytes);

		/* Extract instruction name */
		mop_name_end = index(inst.format, '_');
		mop_name_length = mop_name_end ? mop_name_end - inst.format : strlen(inst.format);
		str_substr(mop_name, sizeof mop_name, inst.format, 0, mop_name_length);
		fprintf(f, "%s%s", comma, mop_name);
		comma = ", ";

		/* Restore safe mode */
		mem->safe = mem_safe;
	}
	fprintf(f, "}\n");
}


/* Flush temporary trace of committed instructions back into the trace cache */
static void X86ThreadFlushTraceCache(X86Thread *self)
{
	struct x86_trace_cache_t *trace_cache = self->trace_cache;
	struct x86_trace_cache_entry_t *entry;
	struct x86_trace_cache_entry_t *found_entry;
	struct x86_trace_cache_entry_t *trace = trace_cache->temp;

	int set;
	int way;
	int found_way;

	/* There must be something to commit */
	if (!trace->uop_count)
		return;

	/* If last instruction was a branch, remove it from the mask and flags fields,
	 * since this prediction does not affect the trace. Instead, the 'target'
	 * field of the trace cache line will be stored. */
	assert(trace->tag);
	if (trace->target)
	{
		assert(trace->branch_count);
		trace->branch_count--;
		trace->branch_mask &= ~(1 << trace->branch_count);
		trace->branch_flags &= ~(1 << trace->branch_count);
	}

	/* Allocate new line for the trace. If trace is already in the cache,
	 * do nothing. If there is any invalid entry, choose it. */
	found_entry = NULL;
	found_way = -1;
	set = trace->tag % x86_trace_cache_num_sets;
	for (way = 0; way < x86_trace_cache_assoc; way++)
	{
		/* Invalid entry found. Since an invalid entry should appear
		 * consecutively and at the end of the set, there is no hope
		 * that the trace will be in a later way. Stop here. */
		entry = X86_TRACE_CACHE_ENTRY(set, way);
		if (!entry->tag)
		{
			found_entry = entry;
			found_way = way;
			break;
		}

		/* Hit */
		if (entry->tag == trace->tag && entry->branch_mask == trace->branch_mask
			&& entry->branch_flags == trace->branch_flags)
		{
			found_entry = entry;
			found_way = way;
			break;
		}
	}

	/* If no invalid entry found, look for LRU. */
	if (!found_entry)
	{
		for (way = 0; way < x86_trace_cache_assoc; way++)
		{
			entry = X86_TRACE_CACHE_ENTRY(set, way);
			entry->counter--;
			if (entry->counter < 0)
			{
				entry->counter = x86_trace_cache_assoc - 1;
				found_entry = entry;
				found_way = way;
			}
		}
	}

	/* Flush temporary trace and reset it. When flushing, all fields are
	 * copied except for LRU counter. */
	assert(found_entry);
	assert(found_way >= 0);
	trace->counter = found_entry->counter;
	memcpy(found_entry, trace, X86_TRACE_CACHE_ENTRY_SIZE);
	memset(trace_cache->temp, 0, X86_TRACE_CACHE_ENTRY_SIZE);

	/* Debug */
	if (x86_trace_cache_debugging())
	{
		FILE *f;

		f = debug_file(x86_trace_cache_debug_category);
		fprintf(f, "** Commit trace **\n");
		fprintf(f, "Set = %d, Way = %d\n", set, found_way);
		X86ThraceDumpTraceCacheEntry(self, found_entry, f);
		fprintf(f, "\n");
	}

	/* Statistics */
	trace_cache->trace_length_acc += found_entry->uop_count;
	trace_cache->trace_length_count++;
}


void X86ThreadRecordUopInTraceCache(X86Thread *self, struct x86_uop_t *uop)
{
	struct x86_trace_cache_t *trace_cache = self->trace_cache;
	struct x86_trace_cache_entry_t *trace = trace_cache->temp;
	int taken;

	/* Only the first micro-instruction of a macro-instruction is inserted. */
	if (uop->mop_index)
		return;

	/* If there is not enough space for macro-instruction, commit trace. */
	assert(!uop->specmode);
	assert(uop->eip);
	assert(uop->id == uop->mop_id);
	if (trace->uop_count + uop->mop_count > x86_trace_cache_trace_size)
		X86ThreadFlushTraceCache(self);

	/* If even after flushing the current trace, the number of micro-instructions
	 * does not fit in a trace line, this macro-instruction cannot be stored. */
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
			X86ThreadFlushTraceCache(self);
	}
}


int X86ThreadLookupTraceCache(X86Thread *self, unsigned int eip, int pred,
	int *ptr_mop_count, unsigned int **ptr_mop_array, unsigned int *ptr_neip)
{
	struct x86_trace_cache_t *trace_cache = self->trace_cache;
	struct x86_trace_cache_entry_t *entry;
	struct x86_trace_cache_entry_t *found_entry;

	unsigned int neip;

	int set;
	int way;
	int taken;

	FILE *f;

	/* Debug */
	if (x86_trace_cache_debugging())
	{
		f = debug_file(x86_trace_cache_debug_category);
		fprintf(f, "** Lookup **\n");
		fprintf(f, "eip = 0x%x, pred = ", eip);
		x86_trace_cache_pred_dump(pred, x86_trace_cache_branch_max, f);
		fprintf(f, "\n");
	}

	/* Look for trace cache line */
	found_entry = NULL;
	set = eip % x86_trace_cache_num_sets;
	for (way = 0; way < x86_trace_cache_assoc; way++)
	{
		entry = X86_TRACE_CACHE_ENTRY(set, way);
		if (entry->tag == eip && ((pred & entry->branch_mask) == entry->branch_flags))
		{
			found_entry = entry;
			break;
		}
	}

	/* Statistics */
	trace_cache->accesses++;
	if (found_entry)
		trace_cache->hits++;

	/* Miss */
	if (!found_entry)
	{
		x86_trace_cache_debug("Miss\n");
		x86_trace_cache_debug("\n");
		return 0;
	}
	
	/* Calculate address of the next instruction to fetch after this trace.
	 * The 'neip' value will be the trace 'target' if the last instruction in
	 * the trace is a branch and 'pred' predicts it taken. */
	taken = found_entry->target && (pred & (1 << found_entry->branch_count));
	neip = taken ? found_entry->target : found_entry->fall_through;

	/* Debug */
	if (x86_trace_cache_debugging())
	{
		f = debug_file(x86_trace_cache_debug_category);
		fprintf(f, "Hit - Set = %d, Way = %d\n", set, way);
		X86ThraceDumpTraceCacheEntry(self, found_entry, f);
		fprintf(f, "Next trace prediction = %c\n", taken ? 'T' : 'n');
		fprintf(f, "Next fetch address = 0x%x\n", neip);
		fprintf(f, "\n");
	}

	/* Return fields. */
	PTR_ASSIGN(ptr_mop_count, found_entry->mop_count);
	PTR_ASSIGN(ptr_mop_array, found_entry->mop_array);
	PTR_ASSIGN(ptr_neip, neip);

	/* Hit */
	return 1;
}




/*
 * Object 'x86_trace_cache_t'
 */

struct x86_trace_cache_t *x86_trace_cache_create(char *name)
{
	struct x86_trace_cache_t *trace_cache;
	struct x86_trace_cache_entry_t *entry;

	int set;
	int way;

	/* Initialize */
	trace_cache = xcalloc(1, sizeof(struct x86_trace_cache_t));
	trace_cache->name = xstrdup(name);
	trace_cache->entry = xcalloc(x86_trace_cache_num_sets * x86_trace_cache_assoc,
		X86_TRACE_CACHE_ENTRY_SIZE);
	trace_cache->temp = xcalloc(1, X86_TRACE_CACHE_ENTRY_SIZE);

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
	free(trace_cache->name);
	free(trace_cache->entry);
	free(trace_cache->temp);
	free(trace_cache);
}




/*
 * Public
 */

/* Debug */
int x86_trace_cache_debug_category;

/* Parameters */
int x86_trace_cache_present;  /* Use trace cache */
int x86_trace_cache_num_sets;  /* Number of sets */
int x86_trace_cache_assoc;  /* Number of ways */
int x86_trace_cache_trace_size;  /* Maximum number of uops in a trace */
int x86_trace_cache_branch_max;  /* Maximum number of branches in a trace */
int x86_trace_cache_queue_size;  /* Fetch queue for pre-decoded uops */


void X86ReadTraceCacheConfig(struct config_t *config)
{
	char *section;
	char *file_name;

	/* Section in configuration file */
	section = "TraceCache";
	file_name = config_get_file_name(config);

	/* Read variables */
	x86_trace_cache_present = config_read_bool(config, section, "Present", 0);
	x86_trace_cache_num_sets = config_read_int(config, section, "Sets", 64);
	x86_trace_cache_assoc = config_read_int(config, section, "Assoc", 4);
	x86_trace_cache_trace_size = config_read_int(config, section, "TraceSize", 16);
	x86_trace_cache_branch_max = config_read_int(config, section, "BranchMax", 3);
	x86_trace_cache_queue_size = config_read_int(config, section, "QueueSize", 32);

	/* Integrity checks */
	if ((x86_trace_cache_num_sets & (x86_trace_cache_num_sets - 1)) || !x86_trace_cache_num_sets)
		fatal("%s: %s: 'Sets' must be a power of 2 greater than 0",
			file_name, section);
	if ((x86_trace_cache_assoc & (x86_trace_cache_assoc - 1)) || !x86_trace_cache_assoc)
		fatal("%s: %s: 'Assoc' must be a power of 2 greater than 0",
			file_name, section);
	if (!x86_trace_cache_trace_size)
		fatal("%s: %s: Invalid value for 'TraceSize'",
			file_name, section);
	if (!x86_trace_cache_branch_max)
		fatal("%s: %s: Invalid value for 'BranchMax'",
			file_name, section);
	if (x86_trace_cache_branch_max > x86_trace_cache_trace_size)
		fatal("%s: %s: 'BranchMax' must be equal or less than 'TraceSize'",
			file_name, section);
	if (x86_trace_cache_branch_max > 31)
		fatal("%s: %s: Maximum value for 'BranchMax' is 31",
			file_name, section);
}
