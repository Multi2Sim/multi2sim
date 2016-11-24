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

#ifndef ARCH_X86_TIMING_TRACE_CACHE_H
#define ARCH_X86_TIMING_TRACE_CACHE_H

#include <lib/cpp/IniFile.h>

#include "Uop.h"

namespace x86
{

// Trace cache class
class TraceCache
{
public:

	// Trace cache entry
	class Entry
	{
		// The trace cache can freely access the entry
		friend class TraceCache;

		// LRU counter 
		int counter;

		// Address of the first instruction in the trace 
		unsigned int tag;

		// Number of micro-instructions in the cache line 
		int uop_count;

		// Number of branches in the trace 
		int branch_count;

		// Bit mask of 'branch_count' bits, with all bits set to one. 
		int branch_mask;

		// Bit mask of 'branch_count' bits. A bit set to one represents
		// taken branch. The MSB corresponds to the last branch in the
		// trace. The LSB corresponds to the first branch in the trace.
		int branch_flags;

		// Flag that indicates whether the last instruction is a branch
		// or not.
		bool is_last_instruction_branch = false;

		// Address of the instruction following the last instruction in the
		// trace.
		unsigned int fall_through;

		// Address of the target address of the last branch in the trace 
		unsigned int target;

		// It is a list composed of at most 'trace_size' elements. Each
		// element contains the address of the micro-instructions in the
		// trace. Only if each single micro-instructions comes from a
		// different macro-instruction can this array be full.
		std::vector<unsigned int> macro_instructions;
	
	public:

		/// Return the number of macro-instructions in the trace
		int getNumMacroInstructions() const
		{
			return macro_instructions.size();
		}

		/// Return the address of the macro-instruction with the given
		/// index.
		unsigned getMacroInstruction(int index) const
		{
			assert(misc::inRange(index, 0, macro_instructions.size() - 1));
			return macro_instructions[index];
		}
	};

private:

	//
	// Static fields
	//

	// Flag indicating whether trace cache is present
	static bool present;

	// Number of sets in trace cache
	static int num_sets;

	// Trace cache associativity
	static int num_ways;

	// Maximum size of the trace
	static int trace_size;

	// Maximum number of branches per trace
	static int max_branches;

	// Trace queue size
	static int queue_size;



	//
	// Class members
	//

	// Name of the trace cache
	std::string name;

	// Trace cache lines (num_sets * num_ways elements) 
	std::unique_ptr<Entry[]> entries;

	// Temporary trace, progressively filled up in the commit stage,
	// and dumped into the trace cache when full.
	std::unique_ptr<Entry> temp;




	//
	// Statistics 
	//

	// Total number of accesses to the trace cache
	long long num_accesses = 0;

	// Total number of hits
	long long num_hits = 0;

	// Number of fetched micro-instructions
	long long num_fetched_uinsts = 0;

	// Number of dispatched micro-instructions coming from the trace cache
	long long num_dispatched_uinsts = 0;

	// Number of issued micro-instructions coming from the trace cache
	long long num_issued_uinsts = 0;

	// Number of committed micro-instructions coming from the trace cache
	long long num_committed_uinsts = 0;

	// Num of squashed speculative micro-instructions coming from the trace
	// cache
	long long num_squashed_uinsts = 0;

	long long trace_length_acc = 0;

	long long trace_length_count = 0;

public:

	//
	// Error class
	//

	/// Exception for X86 trace cache
	class Error : public misc::Error
	{
	public:

		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("X86 trace cache");
		}
	};



	//
	// Static members
	//

	/// Read trace cache configuration from configuration file
	static void ParseConfiguration(misc::IniFile *ini_file);

	/// Dump configuration
	static void DumpConfiguration(std::ostream &os = std::cout);

	/// Return whether the trace cache was configured as present
	static bool isPresent() { return present; }

	/// Return the number of sets, as configured by the user
	static int getNumSets() { return num_sets; }

	/// Return the number of ways, as configured by the user
	static int getNumWays() { return num_ways; }

	/// Return the trace size, as configured by the user
	static int getTraceSize() { return trace_size; }
	
	/// Return the maximum number of branches in a trace, as configured
	/// by the user
	static int getMaxBranches() { return max_branches; }

	/// Return the trace queue size
	static int getQueueSize() { return queue_size; }
	
	/// Debugger files
	static std::string debug_file;

	/// Debugger for trace cache
	static misc::Debug debug;





	//
	// Class members
	//

	/// Constructor
	TraceCache(const std::string &name = "");

	/// Dump the trace cache report
	void DumpReport(std::ostream &os = std::cout);

	/// Record the uop in trace cache
	void RecordUop(Uop *uop);

	/// Look up cache entry in the trace cache
	///
	/// \param eip
	/// 	The instruction address
	///
	/// \param pred
	/// 	The prediction of this branch instruction
	///
	/// \param entry
	/// 	The found cache entry if there was a hit, or nullptr if there
	///	was a miss.
	///
	/// \param neip
	/// 	The next instruction address
	///
	/// \return
	/// 	Whether or not the entry is found
	///
	bool Lookup(unsigned int eip,
			int pred,
			Entry *&entry,
			unsigned int &neip);

	/// Flush temporary trace of committed instructions back into the trace
	/// cache.
	void Flush();
	
	
	

	//
	// Statistics
	//

	/// Increment number of fetched micro-instructions in trace cache
	void incNumFetchedUinsts() { num_fetched_uinsts++; }

	/// Increment number of dispatched micro-instructions coming from the
	/// trace cache
	void incNumDispatchedUinsts() { num_dispatched_uinsts++; }

	/// Increment the number of issued micor-instructions coming from the
	/// trace cache
	void incNumIssuedUinsts() { num_issued_uinsts++; }

	/// Increment the number of squashed micro-instructions coming from the
	/// trace cache
	void incNumSquashedUinsts() { num_squashed_uinsts++; }

	/// Increment the number of committed micro-instructions coming from the
	/// trace cache
	void incNumCommittedUinsts() { num_committed_uinsts++; }
};

}

#endif

