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

#ifndef ARCH_X86_TIMING_UOP_H
#define ARCH_X86_TIMING_UOP_H

#include <deque>

#include <arch/x86/emulator/Uinst.h>
#include <arch/x86/emulator/Context.h>
#include <lib/cpp/Misc.h>

#include "BranchPredictor.h"


namespace x86
{

// Forward declarations
class Core;
class Thread;


// Class Uop
class Uop
{
	//
	// Static fields
	//

	// Counter used to assign unique global uop identifiers
	static long long id_counter;




	//
	// Class members
	//

	// Count the uop dependencies based on information extracted from the
	// associated emulator micro-instruction.
	void CountDependencies();

	// Globally unique uop identifier, initialized in constructor
	long long id;

	// Unique identifier of uop in core, initialized in constructor
	long long id_in_core;

	// Thread that the uop belongs to, initialized in constructor
	Thread *thread;

	// Core that the uop belongs to, initialized in constructor
	Core *core;

	// Emulator context that this micro-instruction belongs to, assigned
	// in the constructor.
	Context *context;

	// Emulator micro-instruction associated with this uop
	std::shared_ptr<Uinst> uinst;

	// Uop flags, taken from the associated micro-instruction. This field
	// is assigned in the constructor.
	int flags;




	//
	// Dependencies
	//

	// Input dependency count
	int idep_count = 0;

	// Output dependency count
	int odep_count = 0;

	// Number of output dependencies on physical integer registers
	int phy_int_odep_count = 0;

	// Number of output dependencies on physical floating-point registers
	int phy_fp_odep_count = 0;

	// Number of output dependencies on physical XMM registers
	int phy_xmm_odep_count = 0;

	// Number of input dependencies on physical integer registers
	int phy_int_idep_count = 0;

	// Number of input dependencies on physical floating-point registers
	int phy_fp_idep_count = 0;

	// Number of input dependences on physical XMM registers
	int phy_xmm_idep_count = 0;

	// Input dependency physical register table
	int phy_idep[Uinst::MaxIDeps] = {};

	// Output dependency physical register table
	int phy_odep[Uinst::MaxODeps] = {};

	// Old output dependency physical register table
	int phy_oodep[Uinst::MaxODeps] = {};

public:

	/// Constructor.
	///
	/// \param thread
	///	Hardware thread that this uop belongs to.
	///
	/// \param context
	///	Emulator context that this uop is associated with.
	///
	/// \param uinst
	///	Emulator micro-instruction that this uop is associated with.
	///
	Uop(Thread *thread,
			Context *context,
			std::shared_ptr<Uinst> uinst);

	/// Dump uop information
	void Dump();

	/// Get thread that the uop belongs to
	Thread *getThread() const { return thread; }

	/// Get core that the uop belongs to
	Core *getCore() const { return core; }

	/// Return the micro-instruction associated with this uop.
	Uinst *getUinst() { return uinst.get(); }

	/// Return a globally unique identifier for the uop
	long long getId() const { return id; }

	/// Return a unique identifier of the uop in the core
	long long getIdInCore() const { return id_in_core; }

	/// Get flags
	int getFlags() const { return flags; }

	/// Set physical register in the input dependency table
	void setPhyRegIdep(int index, int reg_no)
	{
		assert(misc::inRange(index, 0, Uinst::MaxIDeps - 1));
		phy_idep[index] = reg_no;
	}

	/// Set physical register in the output dependency table
	void setPhyRegOdep(int index, int reg_no)
	{
		assert(misc::inRange(index, 0, Uinst::MaxODeps - 1));
		phy_odep[index] = reg_no;
	}

	/// Set physical register in the old output dependency table
	void setPhyRegOOdep(int index, int reg_no)
	{
		assert(misc::inRange(index, 0, Uinst::MaxODeps - 1));
		phy_oodep[index] = reg_no;
	}

	/// Get physical register in the input dependency table
	int getPhyRegIdep(int index) const
	{
		assert(misc::inRange(index, 0, Uinst::MaxIDeps - 1));
		return phy_idep[index];
	}

	/// Get physical register in the output dependency table
	int getPhyRegOdep(int index) const
	{
		assert(misc::inRange(index, 0, Uinst::MaxODeps - 1));
		return phy_odep[index];
	}

	/// Get physical register in the old output dependency table
	int getPhyRegOOdep(int index) const
	{
		assert(misc::inRange(index, 0, Uinst::MaxODeps - 1));
		return phy_oodep[index];
	}

	/// Uop comparison based on ready time or unique ID
	///
	/// \param uop
	///	Uop to compare with
	///
	/// \return
	///	< 0: Current uop comes first
	///	> 0: Uop given in the argument comes first
	///
	int Compare(Uop *uop);


	
	
	//
	// Dependencies
	//

	/// Return the number of output dependencies on physical integer
	/// registers.
	int getPhyIntOdepCount() const { return phy_int_odep_count; }

	/// Return the number of output dependencies on physical floating-point
	/// registers.
	int getPhyFpOdepCount() const { return phy_fp_odep_count; }

	/// Return the number of output dependencies on physical XMM registers.
	int getPhyXmmOdepCount() const { return phy_xmm_odep_count; }

	/// Return the number of input dependencies on physical integer
	/// registers.
	int getPhyIntIdepCount() const { return phy_int_idep_count; }

	/// Return the number of input dependencies on physical floating-point
	/// registers.
	int getPhyFpIdepCount() const { return phy_fp_idep_count; }

	/// Return the number of input dependences on physical XMM registers.
	int getPhyXmmIdepCount() const { return phy_xmm_idep_count; }





	//
	// Queues and iterators
	//

	/// True if the instruction is currently in the fetch queue
	bool in_fetch_queue = false;

	/// Position of the uop in the thread's fetch queue
	std::deque<std::shared_ptr<Uop>>::iterator fetch_queue_iterator;

	/// True if the instruction is currently in the uop queue
	bool in_uop_queue = false;

	/// Position of the uop in the thread's uop queue
	std::deque<std::shared_ptr<Uop>>::iterator uop_queue_iterator;

	/// True if the instruction is currently in the core's event queue
	bool in_event_queue = false;

	/// Position of the uop in the core's event queue, or past-the-end
	/// iterator to this queue if not present.
	std::list<std::shared_ptr<Uop>>::iterator event_queue_iterator;

	/// True if the instruction is currently present in the thread's
	/// reorder buffer
	bool in_reorder_buffer = false;

	/// True if the instruction is currently present in the thread's
	/// instruction queue
	bool in_instruction_queue = false;

	/// True if the instruction is currently present in the thread's
	/// load queue
	bool in_load_queue = false;

	/// True if the instruction is currently present in the thread's
	/// store queue
	bool in_store_queue = false;



	
	//
	// Macro-instruction info
	//

	/// Index of uop within macro-instruction
	int mop_index = 0;

	/// Number of uops within macro-instruction
	int mop_count = 0;

	/// Corresponding macro-instruction size
	int mop_size = 0;

	/// Sequence number of macro-instruction
	long long mop_id = 0;




	//
	// Fetch info
	//

	/// Address of x86 macro-instruction
	unsigned int eip = 0;

	/// Address of next non-speculative x86 macro-instruction
	unsigned int neip = 0;

	/// Flag telling if micro-operation is in speculative mode
	bool speculative_mode = false;

	/// Flag indicating whether the uop was fetched from the trace cache
	bool from_trace_cache = false;
	
	/// Physical address that this uop was fetched from
	unsigned fetch_address = 0;

	// Physical address for memory uops
	unsigned physical_address = 0;

	/// Access identifier for instruction fetch
	long long fetch_access = 0;




	//
	// Branch prediction
	//
	
	/// Address of next predicted x86 macro-instruction (for branches)
	unsigned int predicted_neip = 0;

	/// Address of target x86 macro-instruction assuming branch taken (for
	/// branches)
	unsigned int target_neip = 0;

	/// Global prediction
	BranchPredictor::Prediction prediction = BranchPredictor::PredictionNotTaken;

	/// Bimodal predictor index
	int bimod_index = 0;

	/// Prediction from bimodal branch predictor
	BranchPredictor::Prediction bimod_prediction = BranchPredictor::PredictionNotTaken;

	/// Two-level branch predictor BHT index
	int twolevel_bht_index = 0;

	/// Two-level branch predictor PHT row
	int twolevel_pht_row = 0;

	/// Two-level branch predictor PHT column
	int twolevel_pht_col = 0;

	/// Two-level branch prediction
	BranchPredictor::Prediction twolevel_prediction = BranchPredictor::PredictionNotTaken;

	/// Choice index in the combined branch predictor
	int choice_index = 0;

	/// Prediction in the combined branch predictor
	BranchPredictor::Prediction choice_prediction = BranchPredictor::PredictionNotTaken;
	
	
	
	
	//
	// State
	//

	// True if uop is ready to be issued
	bool ready = false;

	// Cycle when uop was made ready, or 0 if not ready yet
	long long ready_when = 0;

	// True if uop was already issued
	bool issued = false;

	// Cycle when instruction was issued, or 0 if not issued yet
	long long issue_when = 0;

	// True if uop finished execution
	bool completed = false;

	// First cycle when functional unit is tried to be reserved
	long long first_cycle_try_reserve = 0;
};

}

#endif

