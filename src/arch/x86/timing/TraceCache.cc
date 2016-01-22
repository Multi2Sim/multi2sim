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

#include "Core.h"
#include "Cpu.h"
#include "TraceCache.h"
#include "Uop.h"
#include "Thread.h"

namespace x86
{

bool TraceCache::present;
int TraceCache::num_sets;
int TraceCache::num_ways;
int TraceCache::trace_size;
int TraceCache::max_branches;
int TraceCache::queue_size;

// Debug file
std::string TraceCache::debug_file;

// Debuggers
misc::Debug TraceCache::debug;


void TraceCache::ParseConfiguration(misc::IniFile *ini_file)
{
	// Section
	std::string section = "TraceCache";

	// Read variables
	present = ini_file->ReadBool(section, "Present", false);
	num_sets = ini_file->ReadInt(section, "Sets", 64);
	num_ways = ini_file->ReadInt(section, "Assoc", 4);
	trace_size = ini_file->ReadInt(section, "TraceSize", 16);
	max_branches = ini_file->ReadInt(section, "BranchMax", 3);
	queue_size = ini_file->ReadInt(section, "QueueSize", 32);

	// Integrity checks
	if ((num_sets & (num_sets - 1)) || !num_sets)
		throw Error(misc::fmt("%s: 'Sets' must be a power of 2 greater than 0", section.c_str()));
	if ((num_ways & (num_ways - 1)) || !num_ways)
		throw Error(misc::fmt("%s: 'Assoc' must be a power of 2 greater than 0", section.c_str()));
	if (!trace_size)
		throw Error(misc::fmt("%s: Invalid value for 'TraceSize'", section.c_str()));
	if (!max_branches)
		throw Error(misc::fmt("%s: Invalid value for 'BranchMax'", section.c_str()));
	if (max_branches > trace_size)
		throw Error(misc::fmt("%s: 'BranchMax' must be equal or less than 'TraceSize'", section.c_str()));
	if (max_branches > 31)
		throw Error(misc::fmt("%s: Maximum value for 'BranchMax' is 31", section.c_str()));
}


void TraceCache::DumpConfiguration(std::ostream &os)
{
	os << "; Trace cache - parameters\n";
	os << misc::fmt("TraceCache.Sets = %d\n", num_sets);
	os << misc::fmt("TraceCache.Assoc = %d\n", num_ways);
	os << misc::fmt("TraceCache.TraceSize = %d\n", trace_size);
	os << misc::fmt("TraceCache.BranchMax = %d\n", max_branches);
	os << misc::fmt("TraceCache.QueueSize = %d\n", queue_size);
	os << '\n';
}


TraceCache::TraceCache(const std::string &name) :
		name(name)
{
	// Initialize
	entries = misc::new_unique_array<Entry>(num_sets * num_ways);
	temp = misc::new_unique<Entry>();

	// Initialize LRU counter
	for (int set = 0; set < num_sets; set++)
		for (int way = 0; way < num_ways; way++)
			entries[set * num_ways + way].counter = way;
}


void TraceCache::DumpReport(std::ostream &os)
{
	// Dump the configuration
	DumpConfiguration(os);

	// Statistics
	os << "; Trace cache - statistics\n";
	os << misc::fmt("TraceCache.Accesses = %lld\n", num_accesses);
	os << misc::fmt("TraceCache.Hits = %lld\n", num_hits);
	os << misc::fmt("TraceCache.HitRatio = %.4g\n", num_accesses ?
			(double) num_hits / num_accesses : 0.0);
	os << misc::fmt("TraceCache.Fetched = %lld\n", num_fetched_uinsts);
	os << misc::fmt("TraceCache.Dispatched = %lld\n", num_dispatched_uinsts);
	os << misc::fmt("TraceCache.Issued = %lld\n", num_issued_uinsts);
	os << misc::fmt("TraceCache.Committed = %lld\n", num_committed_uinsts);
	os << misc::fmt("TraceCache.Squashed = %lld\n", num_squashed_uinsts);
	os << misc::fmt("TraceCache.TraceLength = %.2f\n", trace_length_count ?
			(double) trace_length_acc / trace_length_count : 0);

	// Done
	os << '\n';
}


void TraceCache::RecordUop(Uop *uop)
{
	// Local variable
	bool taken = false;

	// Only the first micro-instruction of a macro-instruction is inserted.
	if (uop->mop_index)
		return;

	// If there is not enough space for macro-instruction, commit trace.
	assert(!uop->speculative_mode);
	assert(uop->eip);
	assert(uop->getId() == uop->mop_id);
	if (temp->uop_count + uop->mop_count > trace_size)
		Flush();

	// If even after flushing the current trace, the number of micro-instructions
	// does not fit in a trace line, this macro-instruction cannot be stored.
	if (uop->mop_count > trace_size)
		return;

	// First instruction. Store trace tag.
	if (!temp->uop_count)
		temp->tag = uop->eip;

	// Add eip to list
	temp->macro_instructions.push_back(uop->eip);
	temp->uop_count += uop->mop_count;
	temp->is_last_instruction_branch = false;
	temp->fall_through = uop->eip + uop->mop_size;

	// Instruction is branch. If maximum number of branches is reached,
	// commit trace.
	if (uop->getFlags() & Uinst::FlagCtrl)
	{
		taken = uop->neip != uop->eip + uop->mop_size;
		temp->branch_mask |= 1 << temp->branch_count;
		temp->branch_flags |= taken << temp->branch_count;
		temp->branch_count++;
		temp->target = uop->target_neip;
		temp->is_last_instruction_branch = true;
		if (temp->branch_count == max_branches)
			Flush();
	}
}


bool TraceCache::Lookup(unsigned int eip,
		int pred,
		Entry *&found_entry,
		unsigned int &neip)
{
	// Debug
	debug << misc::fmt("** Lookup **\n");
	debug << misc::fmt("eip = 0x%x, pred = ", eip);
	debug << misc::fmt("\n");

	// Look for trace cache line
	int way;
	int set = eip % num_sets;
	found_entry = nullptr;
	for (way = 0; way < num_ways; way++)
	{
		Entry *entry = &entries[set * num_ways + way];
		if (entry->tag == eip && ((pred & entry->branch_mask) == entry->branch_flags))
		{
			found_entry = entry;
			break;
		}
	}

	// Statistics
	num_accesses++;
	if (found_entry)
		num_hits++;

	// Miss
	if (!found_entry)
	{
		debug << misc::fmt("Miss\n");
		debug << misc::fmt("\n");
		return false;
	}

	// Calculate address of the next instruction to fetch after this trace.
	// The 'neip' value will be the trace 'target' if the last instruction in
	// the trace is a branch and 'pred' predicts it taken.
	bool taken = found_entry->target && (pred & (1 << found_entry->branch_count));
	neip = taken ? found_entry->target : found_entry->fall_through;

	// Debug
	debug << misc::fmt("Hit - Set = %d, Way = %d\n", set, way);
	debug << misc::fmt("Next trace prediction = %c\n", taken ? 'T' : 'N');
	debug << misc::fmt("Next fetch address = 0x%x\n", neip);
	debug << misc::fmt("\n");

	// Hit
	return true;
}


void TraceCache::Flush()
{
	// There must be something to commit
	if (!temp->uop_count)
		return;

	// If last instruction was a branch, remove it from the mask and flags fields,
	// since this prediction does not affect the trace. Instead, the 'target'
	// field of the trace cache line will be stored.
	assert(temp->tag);
	if (temp->is_last_instruction_branch)
	{
		assert(temp->branch_count);
		temp->branch_count--;
		temp->branch_mask &= ~(1 << temp->branch_count);
		temp->branch_flags &= ~(1 << temp->branch_count);
	}

	// Allocate new line for the trace. If trace is already in the cache,
	// do nothing. If there is any invalid entry, choose it.
	int set = temp->tag % num_sets;
	Entry *found_entry = nullptr;
	int found_way = -1;
	for (int way = 0; way < num_ways; way++)
	{
		// Invalid entry found. Since an invalid entry should appear
		// consecutively and at the end of the set, there is no hope
		// that the trace will be in a later way. Stop here.
		Entry *entry = &entries[set * num_ways + way];
		if (!entry->tag)
		{
			found_entry = entry;
			found_way = way;
			break;
		}

		// Hit
		if (entry->tag == temp->tag && entry->branch_mask == temp->branch_mask
				&& entry->branch_flags == temp->branch_flags)
		{
			found_entry = entry;
			found_way = way;
			break;
		}
	}

	// If no invalid entry found, look for LRU.
	if (!found_entry)
	{
		for (int way = 0; way < num_ways; way++)
		{
			Entry *entry = &entries[set * num_ways + way];
			entry->counter--;
			if (entry->counter < 0)
			{
				entry->counter = num_ways - 1;
				found_entry = entry;
				found_way = way;
			}
		}
	}

	// Flush temporary trace and reset it. When flushing, all fields are
	// copied except for LRU counter.
	assert(found_entry);
	assert(found_way >= 0);
	temp->counter = found_entry->counter;
	Entry *temp_ptr = temp.get();
	memcpy(found_entry, temp_ptr, sizeof(Entry));
	memset(temp_ptr, 0, sizeof(Entry));

	// Debug
	debug << misc::fmt("** Commit trace **\n");
	debug << misc::fmt("Set = %d, Way = %d\n", set, found_way);
	debug << misc::fmt("\n");

	// Statistics
	trace_length_acc += found_entry->uop_count;
	trace_length_count++;
}

}
