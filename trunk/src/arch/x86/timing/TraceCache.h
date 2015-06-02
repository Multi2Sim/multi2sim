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

#include <lib/cpp/IniFile.h>

#include "Uop.h"

namespace x86
{

// Trace cache class
class TraceCache
{
private:

	// Name of the trace cache
	std::string name;

	// structure of trace cache entry
	struct TraceCacheEntry
	{
		// LRU counter 
		int counter;

		// Address of the first instruction in the trace 
		unsigned int tag;

		// Number of micro- and macro-instructions in the cache line 
		int uop_count;
		int mop_count;

		// Number of branches in the trace 
		int branch_count;

		// Bit mask of 'branch_count' bits, with all bits set to one. 
		int branch_mask;

		// Bit mask of 'branch_count' bits. A bit set to one represents a
		// taken branch. The MSB corresponds to the last branch in the trace.
		// The LSB corresponds to the first branch in the trace.
		int branch_flags;

		// Flag that indicates whether the last instruction is a branch or not
		bool is_last_instruction_branch = false;

		// Address of the instruction following the last instruction in the
		// trace.
		unsigned int fall_through;

		// Address of the target address of the last branch in the trace 
		unsigned int target;

		// This field has to be the last in the structure.
		// It is a list composed of 'trace_size' elements.
		// Each element contains the address of the micro-instructions in the trace.
		// Only if each single micro-instructions comes from a different macro-
		// instruction can this array be full.
		std::vector<unsigned int> mop_array;
	};

	// Trace cache lines ('sets' * 'assoc' elements) 
	std::unique_ptr<TraceCacheEntry[]> entry;

	// Temporary trace, progressively filled up in the commit stage,
	// and dumped into the trace cache when full.
	std::unique_ptr<TraceCacheEntry> temp;

	// Statistics 
	long long accesses = 0;
	long long hits = 0;
	long long num_fetched_uinst = 0;
	long long num_dispatched_uinst = 0;
	long long num_issued_uinst = 0;
	long long num_committed_uinst = 0;
	long long num_squashed_uinst = 0;
	long long trace_length_acc = 0;
	long long trace_length_count = 0;

	// Trace cache parameters
	static int present;
	static int num_sets;
	static int assoc;
	static int trace_size;
	static int branch_max;
	static int queue_size;

public:

	/// Exception for X86 trace cache
	class Error : public misc::Error
	{
	public:

		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("X86 trace cache");
		}
	};

	/// Constructor
	TraceCache(const std::string &name = "");

	/// Read trace cache configuration from configuration file
	static void ParseConfiguration(misc::IniFile *ini_file);

	/// Dump configuration
	void DumpConfiguration(std::ostream &os = std::cout);

	/// Configuration getters
	static bool getPresent() { return present; }
	static int getNumSets() { return num_sets; }
	static int getAssoc() { return assoc; }
	static int getTraceSize() { return trace_size; }
	static int getMaxNumBranch() { return branch_max; }
	static int getQueuesize() { return queue_size; }

	/// Statistic setters


	/// Statistic getters


	/// Dump functions
	/// Dump the trace cache report
	void DumpReport(std::ostream &os = std::cout);

	/// Record the Uop in trace cache
	void RecordUop(Uop &uop);

	/// Look up cache entry in the trace cache
	///
	/// \param eip
	/// 	The instruction address
	///
	/// \param pred
	/// 	The prediction of this branch instruction
	///
	/// \param return_entry
	/// 	The found cache entry
	///
	/// \param neip
	/// 	The next instruction address
	///
	/// \return
	/// 	Whether or not the entry is found
	bool Lookup(unsigned int eip, int pred,
			TraceCacheEntry &return_entry, unsigned int &neip);

	/// Flush temporary trace of committed instructions back into the trace cache
	void Flush();

	/// Debugger files
	static std::string debug_file;

	/// Debugger for trace cache
	static misc::Debug debug;

};

}

#endif // X86_ARCH_TIMING_TRACE_CACHE_H
