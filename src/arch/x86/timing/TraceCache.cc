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


TraceCache::TraceCache(const std::string &trace_cache_name)
{
	// Initialize 
	name = trace_cache_name;
	this->entry = std::unique_ptr<TraceCacheEntry[]>(new TraceCacheEntry[num_sets * assoc]);
	this->temp = std::unique_ptr<TraceCacheEntry[]>(new TraceCacheEntry[1]);;

	// Initialize LRU counter 
	for (int set = 0; set < num_sets; set++)
	{
		for (int way = 0; way < assoc; way++)
		{
			entry[set * assoc + way].counter = way;
		}
	}

	// Initialize all the statistic parameter
	accesses = 0;
	hits = 0;
	num_fetched_uinst = 0;
	num_dispatched_uinst = 0;
	num_issued_uinst = 0;
	num_committed_uinst = 0;
	num_squashed_uinst = 0;
	trace_length_acc = 0;
	trace_length_count = 0;
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
		throw misc::Panic(misc::fmt("%s: 'Sets' must be a power of 2 greater than 0", section.c_str()));
	if ((assoc & (assoc - 1)) || !assoc)
		throw misc::Panic(misc::fmt("%s: 'Assoc' must be a power of 2 greater than 0", section.c_str()));
	if (!trace_size)
		throw misc::Panic(misc::fmt("%s: Invalid value for 'TraceSize'", section.c_str()));
	if (!branch_max)
		throw misc::Panic(misc::fmt("%s: Invalid value for 'BranchMax'", section.c_str()));
	if (branch_max > trace_size)
		throw misc::Panic(misc::fmt("%s: 'BranchMax' must be equal or less than 'TraceSize'", section.c_str()));
	if (branch_max > 31)
		throw misc::Panic(misc::fmt("%s: Maximum value for 'BranchMax' is 31", section.c_str()));
}


void TraceCache::DumpConfig(std::ostream &os)
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

}
