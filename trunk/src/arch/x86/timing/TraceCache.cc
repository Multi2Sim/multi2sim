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
#include "CPU.h"
#include "TraceCache.h"
#include "Uop.h"
#include "Thread.h"

namespace x86
{

int TraceCache::present;
int TraceCache::num_sets;
int TraceCache::assoc;
int TraceCache::trace_size;
int TraceCache::branch_max;
int TraceCache::queue_size;

// Debug file
std::string TraceCache::debug_file;

// Debuggers
misc::Debug TraceCache::debug;


TraceCache::TraceCache(const std::string &name)
	:
	name(name)
{
	// Initialize 
	this->entry = misc::new_unique_array<TraceCacheEntry>(num_sets * assoc);
	this->temp = misc::new_unique<TraceCacheEntry>();

	// Initialize LRU counter 
	for (int set = 0; set < num_sets; set++)
	{
		for (int way = 0; way < assoc; way++)
		{
			entry[set * assoc + way].counter = way;
		}
	}
}


void TraceCache::ParseConfiguration(const std::string &section,
				misc::IniFile &config)
{
	// Read variables
	present = config.ReadBool(section, "Present", 0);
	num_sets = config.ReadInt(section, "Sets", 64);
	assoc = config.ReadInt(section, "Assoc", 4);
	trace_size = config.ReadInt(section, "TraceSize", 16);
	branch_max = config.ReadInt(section, "BranchMax", 3);
	queue_size = config.ReadInt(section, "QueueSize", 32);

	// Integrity checks
	if ((num_sets & (num_sets - 1)) || !num_sets)
		misc::fatal("%s: 'Sets' must be a power of 2 greater than 0", section.c_str());
	if ((assoc & (assoc - 1)) || !assoc)
		misc::fatal("%s: 'Assoc' must be a power of 2 greater than 0", section.c_str());
	if (!trace_size)
		misc::fatal("%s: Invalid value for 'TraceSize'", section.c_str());
	if (!branch_max)
		misc::fatal("%s: Invalid value for 'BranchMax'", section.c_str());
	if (branch_max > trace_size)
		misc::fatal("%s: 'BranchMax' must be equal or less than 'TraceSize'", section.c_str());
	if (branch_max > 31)
		misc::fatal("%s: Maximum value for 'BranchMax' is 31", section.c_str());
}


void TraceCache::DumpConfiguration(std::ostream &os)
{
	// Dump network information
	os << misc::fmt("\n***** TraceCache *****\n");
	os << misc::fmt("\tPresent: %d\n", present);
	os << misc::fmt("\tSets: %d\n", num_sets);
	os << misc::fmt("\tAssoc: %d\n", assoc);
	os << misc::fmt("\tTraceSize: %d\n", trace_size);
	os << misc::fmt("\tBranchMax: %d\n", branch_max);
	os << misc::fmt("\tQueueSize: %d\n", queue_size);
}


void TraceCache::RecordUop(Uop &uop)
{
	// Local variable
	bool taken = false;

	// Only the first micro-instruction of a macro-instruction is inserted.
	if (uop.getMopIndex())
		return;

	// If there is not enough space for macro-instruction, commit trace.
	assert(!uop.getSpeculativeMode());
	assert(uop.getEip());
	assert(uop.getID() == uop.getMopId());
	if (temp->uop_count + uop.getMopCount() > trace_size)
		Flush();

	// If even after flushing the current trace, the number of micro-instructions
	// does not fit in a trace line, this macro-instruction cannot be stored.
	if (uop.getMopCount() > trace_size)
		return;

	// First instruction. Store trace tag.
	if (!temp->uop_count)
		temp->tag = uop.getEip();

	// Add eip to list
	temp->mop_array[temp->mop_count] = uop.getEip();
	temp->mop_count++;
	temp->uop_count += uop.getMopCount();
	temp->target = 0;
	temp->fall_through = uop.getEip() + uop.getMopSize();

	// Instruction is branch. If maximum number of branches is reached,
	// commit trace.
	if (uop.getFlags() & UInstFlagCtrl)
	{
		taken = uop.getNeip() != uop.getEip() + uop.getMopSize();
		temp->branch_mask |= 1 << temp->branch_count;
		temp->branch_flags |= taken << temp->branch_count;
		temp->branch_count++;
		temp->target = uop.getTargetNeip();
		temp->branch = true;
		if (temp->branch_count == branch_max)
			Flush();
	}
}


bool TraceCache::Lookup(unsigned int eip, int pred,
		TraceCacheEntry &return_entry, unsigned int &neip)
{
	// Local variable declaration
	TraceCacheEntry *entry_ptr;
	TraceCacheEntry *found_entry = nullptr;
	int set, way;
	bool taken;

	// Debug
	debug << misc::fmt("** Lookup **\n");
	debug << misc::fmt("eip = 0x%x, pred = ", eip);
	debug << misc::fmt("\n");

	// Look for trace cache line
	set = eip % num_sets;
	for (way = 0; way < assoc; way++)
	{
		entry_ptr = &entry[set * assoc + way];
		if (entry_ptr->tag == eip && ((pred & entry_ptr->branch_mask) == entry_ptr->branch_flags))
		{
			found_entry = entry_ptr;
			break;
		}
	}

	// Statistics
	accesses++;
	if (found_entry)
		hits++;

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
	taken = found_entry->target && (pred & (1 << found_entry->branch_count));
	neip = taken ? found_entry->target : found_entry->fall_through;

	// Debug
	debug << misc::fmt("Hit - Set = %d, Way = %d\n", set, way);
	debug << misc::fmt("Next trace prediction = %c\n", taken ? 'T' : 'N');
	debug << misc::fmt("Next fetch address = 0x%x\n", neip);
	debug << misc::fmt("\n");


	// Return entry
	return_entry = *found_entry;

	// Hit
	return true;
}


void TraceCache::Flush()
{
	// Local variable declaration
	TraceCacheEntry *entry_ptr;
	TraceCacheEntry *found_entry = nullptr;
	int found_way = -1;

	// There must be something to commit
	if (!temp->uop_count)
		return;

	// If last instruction was a branch, remove it from the mask and flags fields,
	// since this prediction does not affect the trace. Instead, the 'target'
	// field of the trace cache line will be stored.
	assert(temp->tag);
	if (temp->branch)
	{
		assert(temp->branch_count);
		temp->branch_count--;
		temp->branch_mask &= ~(1 << temp->branch_count);
		temp->branch_flags &= ~(1 << temp->branch_count);
	}

	// Allocate new line for the trace. If trace is already in the cache,
	// do nothing. If there is any invalid entry, choose it.
	int set = temp->tag % num_sets;
	for (int way = 0; way < assoc; way++)
	{
		// Invalid entry found. Since an invalid entry should appear
		// consecutively and at the end of the set, there is no hope
		// that the trace will be in a later way. Stop here.
		TraceCacheEntry *entry_ptr = &entry[set * assoc + way];
		if (!entry_ptr->tag)
		{
			found_entry = entry_ptr;
			found_way = way;
			break;
		}

		// Hit
		if (entry_ptr->tag == temp->tag && entry_ptr->branch_mask == temp->branch_mask
				&& entry_ptr->branch_flags == temp->branch_flags)
		{
			found_entry = entry_ptr;
			found_way = way;
			break;
		}
	}

	// If no invalid entry found, look for LRU.
	if (!found_entry)
	{
		for (int way = 0; way < assoc; way++)
		{
			entry_ptr = &entry[set * assoc + way];
			entry_ptr->counter--;
			if (entry_ptr->counter < 0)
			{
				entry_ptr->counter = assoc - 1;
				found_entry = entry_ptr;
				found_way = way;
			}
		}
	}

	// Flush temporary trace and reset it. When flushing, all fields are
	// copied except for LRU counter.
	assert(found_entry);
	assert(found_way >= 0);
	temp->counter = found_entry->counter;
	TraceCacheEntry *temp_ptr = temp.get();
	memcpy(found_entry, temp_ptr, sizeof(TraceCacheEntry));
	memset(temp_ptr, 0, sizeof(TraceCacheEntry));

	// Debug
	debug << misc::fmt("** Commit trace **\n");
	debug << misc::fmt("Set = %d, Way = %d\n", set, found_way);
	debug << misc::fmt("\n");

	// Statistics
	trace_length_acc += found_entry->uop_count;
	trace_length_count++;
}

}
