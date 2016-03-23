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

	// Input physical registers
	int inputs[Uinst::MaxIDeps] = {};

	// Output physical registers
	int outputs[Uinst::MaxODeps] = {};

	// Old output physical registers
	int old_outputs[Uinst::MaxODeps] = {};

	// Total number of input dependencies
	int num_inputs = 0;

	// Total number of output dependencies
	int num_outputs = 0;

	// Number of integer input dependencies
	int num_integer_inputs = 0;

	// Number of integer output dependencies
	int num_integer_outputs = 0;

	// Number of floating-point input dependencies
	int num_floating_point_inputs = 0;

	// Number of floating-point output dependencies
	int num_floating_point_outputs = 0;

	// Number of XMM input dependencies
	int num_xmm_inputs = 0;

	// Number of XMM output dependencies
	int num_xmm_outputs = 0;

public:

	/// Constructor
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
	void Dump(std::ostream &os = std::cout) const;

	/// Alternative invocation to Dump()
	friend std::ostream &operator<<(std::ostream &os, const Uop &uop)
	{
		uop.Dump(os);
		return os;
	}

	/// Get thread that the uop belongs to
	Thread *getThread() const { return thread; }

	/// Get core that the uop belongs to
	Core *getCore() const { return core; }

	/// Return the micro-instruction associated with this uop.
	Uinst *getUinst() const { return uinst.get(); }

	/// Return the opcode of the associated micro-instruction
	Uinst::Opcode getOpcode() const { return uinst->getOpcode(); }

	/// Return a globally unique identifier for the uop
	long long getId() const { return id; }

	/// Return a unique identifier of the uop in the core
	long long getIdInCore() const { return id_in_core; }

	/// Get flags
	int getFlags() const { return flags; }

	/// Set input physical register dependency
	void setInput(int index, int physical_register)
	{
		assert(misc::inRange(index, 0, Uinst::MaxIDeps - 1));
		inputs[index] = physical_register;
	}

	/// Set output physical register dependency
	void setOutput(int index, int physical_register)
	{
		assert(misc::inRange(index, 0, Uinst::MaxODeps - 1));
		outputs[index] = physical_register;
	}

	/// Set old output physical register dependency
	void setOldOutput(int index, int physical_register)
	{
		assert(misc::inRange(index, 0, Uinst::MaxODeps - 1));
		old_outputs[index] = physical_register;
	}

	/// Get input physical register dependency
	int getInput(int index) const
	{
		assert(misc::inRange(index, 0, Uinst::MaxIDeps - 1));
		return inputs[index];
	}

	/// Get output physical register dependency
	int getOutput(int index) const
	{
		assert(misc::inRange(index, 0, Uinst::MaxODeps - 1));
		return outputs[index];
	}

	/// Get old output physical register dependency
	int getOldOutput(int index) const
	{
		assert(misc::inRange(index, 0, Uinst::MaxODeps - 1));
		return old_outputs[index];
	}

	/// Compare the current uop against another uop based on the completion
	/// time set in their `complete_when` field. If both uops have the
	/// same completion time, they are compared based on their identifier.
	///
	/// \param uop
	///	Uop to compare with
	///
	/// \return
	///	< 0: Current uop comes first
	///	> 0: Uop given in the argument comes first
	///
	int Compare(Uop *uop)
	{
		return complete_when != uop->complete_when ?
				complete_when - uop->complete_when :
				id - uop->id;
	}


	
	
	//
	// Dependencies
	//

	/// Return the number of integer input dependencies
	int getNumIntegerInputs() const { return num_integer_inputs; }

	/// Return the number of integer output dependencies
	int getNumIntegerOutputs() const { return num_integer_outputs; }

	/// Return the number of floating-point input dependencies
	int getNumFloatingPointInputs() const { return num_floating_point_inputs; }

	/// Return the number of floating-point output dependencies
	int getNumFloatingPointOutputs() const { return num_floating_point_outputs; }

	/// Return the number of XMM input dependencies
	int getNumXmmInputs() const { return num_xmm_inputs; }

	/// Return the number of XMM output dependencies
	int getNumXmmOutputs() const { return num_xmm_outputs; }





	//
	// Queues and iterators
	//

	/// True if the instruction is currently in the fetch queue
	bool in_fetch_queue = false;

	/// Position of the uop in the thread's fetch queue
	std::list<std::shared_ptr<Uop>>::iterator fetch_queue_iterator;

	/// True if the instruction is currently in the uop queue
	bool in_uop_queue = false;

	/// Position of the uop in the thread's uop queue
	std::list<std::shared_ptr<Uop>>::iterator uop_queue_iterator;

	/// True if the instruction is currently in the core's event queue
	bool in_event_queue = false;

	/// Position of the uop in the core's event queue, or past-the-end
	/// iterator to this queue if not present.
	std::list<std::shared_ptr<Uop>>::iterator event_queue_iterator;

	/// True if the instruction is currently present in the thread's
	/// reorder buffer
	bool in_reorder_buffer = false;

	/// Position of the uop in the reorder buffer, if present
	std::list<std::shared_ptr<Uop>>::iterator reorder_buffer_iterator;

	/// True if the instruction is currently present in the thread's
	/// instruction queue
	bool in_instruction_queue = false;

	/// Position of the uop in the thread's instruction queue, or past-the-
	/// end iterator to this queue if not present.
	std::list<std::shared_ptr<Uop>>::iterator instruction_queue_iterator;

	/// True if the instruction is currently present in the thread's
	/// load queue
	bool in_load_queue = false;

	/// Position of the uop in the thread's load queue, or past-the-end
	/// iterator to this queue if not present.
	std::list<std::shared_ptr<Uop>>::iterator load_queue_iterator;

	/// True if the instruction is currently present in the thread's
	/// store queue
	bool in_store_queue = false;

	/// Position of the uop in the thread's store queue, or past-the-end
	/// iterator to this queue if not present.
	std::list<std::shared_ptr<Uop>>::iterator store_queue_iterator;

	/// True if the instruction is currently present in the uop trace list
	/// of the CPU
	bool in_trace_list = false;

	/// Position of the uop in the CPU's trace list, if present
	std::list<std::shared_ptr<Uop>>::iterator trace_list_iterator;



	
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

	/// Flag indicating whether the uop is the first in a sequence of
	/// uops in speculative mode in the reorder buffer.
	bool first_speculative_mode = false;

	/// Flag indicating whether the uop was fetched from the trace cache
	bool from_trace_cache = false;
	
	/// Physical address that this uop was fetched from
	unsigned fetch_address = 0;

	// For memory uops, Physical address of memory access
	unsigned physical_address = 0;

	// For memory uops, unique identifier of memory access
	long long memory_access = 0;

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
	int two_level_bht_index = 0;

	/// Two-level branch predictor PHT row
	int two_level_pht_row = 0;

	/// Two-level branch predictor PHT column
	int two_level_pht_col = 0;

	/// Two-level branch prediction
	BranchPredictor::Prediction two_level_prediction = BranchPredictor::PredictionNotTaken;

	/// Choice index in the combined branch predictor
	int choice_index = 0;

	/// Prediction in the combined branch predictor
	BranchPredictor::Prediction choice_prediction = BranchPredictor::PredictionNotTaken;
	
	
	
	
	//
	// State
	//

	/// True if uop has been dipatched
	bool dispatched = false;

	/// Cycle when uop was dispatched, or 0 if not dispatched yet.
	long long dispatch_when = 0;

	/// True if uop is ready to be issued
	bool ready = false;

	/// Cycle when uop was made ready, or 0 if not ready yet
	long long ready_when = 0;

	/// True if uop was already issued
	bool issued = false;

	/// Cycle when instruction was issued, or 0 if not issued yet
	long long issue_when = 0;

	/// True if uop finished execution
	bool completed = false;

	/// Time when uop is set to finish execution
	long long complete_when = 0;

	/// First cycle when the uop first tried to reserve a functional unit
	long long first_alu_cycle = 0;
};

}

#endif

