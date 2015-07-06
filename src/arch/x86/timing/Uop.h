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

#include <lib/cpp/Misc.h>

#include <arch/x86/emulator/UInst.h>
#include <arch/x86/emulator/Context.h>

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

	// Globally unique uop identifier, initialized in constructor
	long long id;

	// Unique identifier of uop in core, initialized in constructor
	long long id_in_core;

	// Thread that the uop belongs to, initialized in constructor
	Thread *thread;

	// Core that the uop belongs to, initialized in constructor
	Core *core;

	// Micro-instruction
	Uinst *uinst = nullptr;

	// Context
	Context *context = nullptr;

	// Uop flags
	int flags = 0;




	//
	// Fields associated with macroinstruction 
	//

	// Index of uop within macroinstruction
	int mop_index = 0;

	// Number of uops within macroinstruction
	int mop_count = 0;

	// Corresponding macroinstruction size
	int mop_size = 0;

	// Sequence number of macroinstruction
	long long mop_id = 0;




	//
	// Logical dependencies
	//

	// Input dependency count
	int idep_count = 0;

	// Output dependency count
	int odep_count = 0;




	//
	// Physical mappings
	//

	// Output dependency count for Physical INT/FP/XMM registers
	int phy_int_odep_count = 0;
	int phy_fp_odep_count = 0;
	int phy_xmm_odep_count = 0;

	// Input dependency physical register table
	int phy_idep[UInstMaxIDeps];

	// Output dependency physical register table
	int phy_odep[UInstMaxODeps];

	// Old output dependency physical register table
	int phy_oodep[UInstMaxODeps];




	//
	// Instruction status
	//

	// Is Uop ready
	bool ready = false;

	// Is Uop issued
	bool issued = false;

	// Is Uop completed
	bool completed = false;

	// Physical address for memory uops
	unsigned int phy_addr = 0;




	//
	// Cycles 
	//

	// cycle when ready
	long long ready_when = 0;

	// first cycle when functional unit is tried to be reserved
	long long first_cycle_try_reserve = 0;

	// cycle when issued
	long long issue_when = 0;




	// Global prediction (0=not taken, 1=taken)
	BranchPredictor::Prediction pred = BranchPredictor::PredictionNotTaken;

	// Bimodal index
	int bimod_index = 0;

	// Bimodal Branch prediction
	BranchPredictor::Prediction bimod_pred = BranchPredictor::PredictionNotTaken;

	// Two level branch predictor BHT and PHT indices
	int twolevel_bht_index = 0;
	int twolevel_pht_row = 0;
	int twolevel_pht_col = 0;

	// Twolevel Branch prediction
	BranchPredictor::Prediction twolevel_pred = BranchPredictor::PredictionNotTaken;

	// Choice index in the combined branch predictor
	int choice_index = 0;

	// Combined Branch prediction
	BranchPredictor::Prediction choice_pred = BranchPredictor::PredictionNotTaken;

public:

	/// Constructor for Unit test
	Uop();

	/// Constructor
	Uop(Thread *thread);

	/// Dump Uop information
	void Dump();

	/// Count the dependencies of the current Uop
	void CountDependencies();



	//
	// Setters
	//

	/// Set micro-instruction associated with this Uop
	void setUInst(Uinst *uinst) {this->uinst = uinst; }

	/// Set flags
	void setFlags(int flags) { this->flags = flags; }

	/// Set global prediction
	void setPrediction(BranchPredictor::Prediction pred)
	{
		this->pred = pred;
	}

	/// Set next EIP
	void setNeip(unsigned int neip) { this->neip = neip; }

	/// Set EIP
	void setEip(unsigned int eip) { this->eip = eip; }

	/// Set Macro-Operation size
	void setMopSize(int mop_size) { this->mop_size = mop_size; }

	/// Set bimodal index
	void setBimodIndex(int bimod_index) { this->bimod_index = bimod_index; }

	/// Set bimadal prediction
	void setBimodPrediction(BranchPredictor::Prediction pred)
	{
		this->bimod_pred = pred;
	}

	/// Set two level BHT index
	void setTwolevelBHTIndex(int index) { this->twolevel_bht_index = index; }

	/// Set two level PHT row
	void setTwolevelPHTRow(int row) { this->twolevel_pht_row = row; }

	/// Set two level PHT colomn
	void setTwolevelPHTCol(int col) { this->twolevel_pht_col = col; }

	/// Set Two level prediction
	void setTwolevelPrediction(BranchPredictor::Prediction pred)
	{
		this->twolevel_pred = pred;
	}

	/// Set choice index of combined branch predictor
	void setChoiceIndex(int choice_index) { this->choice_index = choice_index; }

	/// Set combined branch predictor predictor
	void setChoicePrediction(BranchPredictor::Prediction pred)
	{
		this->choice_pred = pred;
	}

	/// Set physical register in the input dependency table
	void setPhyRegIdep(int index, int reg_no) { phy_idep[index] = reg_no; }

	/// Set physical register in the output dependency table
	void setPhyRegOdep(int index, int reg_no) { phy_odep[index] = reg_no; }

	/// Set physical register in the old output dependency table
	void setPhyRegOOdep(int index, int reg_no) { phy_oodep[index] = reg_no; }

	/// Set first cycle when functional unit is tried to be reserved
	void setFirstCycleTryReserve(long long first_cycle_try_reserve)
	{
		this->first_cycle_try_reserve = first_cycle_try_reserve;
	}




	//
	// Getters
	//

	/// Get thread that the uop belongs to
	Thread *getThread() const { return thread; }

	/// Get core that the uop belongs to
	Core *getCore() const { return core; }

	/// Get micro-instruction associated with this Uop
	Uinst *getUinst() { return uinst; }

	/// Return a globally unique identifier for the uop
	int getId() const { return id; }

	/// Return a unique identifier of the uop in the core
	int getIdInCore() const { return id_in_core; }

	/// Get flags
	int getFlags() const { return flags; }

	/// Get global prediction
	BranchPredictor::Prediction getPrediction() const { return pred; }

	/// Get next EIP
	unsigned int getNeip() const { return neip; }

	/// Get EIP
	unsigned int getEip() const { return eip; }

	/// Get Target EIP
	unsigned int getTargetNeip() const { return target_neip; }

	/// Get Macro-Operation size
	int getMopSize() const { return mop_size; }

	/// Get Uop counts of a particular Macro-operation
	int getMopCount() const { return mop_count; }

	/// Get Macro-Operation index
	int getMopIndex() const { return mop_index; }

	/// Get Macro-Operation ID
	long long getMopId() const { return mop_id; }

	/// Get bimodal index
	int getBimodIndex() const { return bimod_index; }

	/// Get bimadal prediction
	BranchPredictor::Prediction getBimodPrediction() const { return bimod_pred; }

	/// Get two level BHT index
	int getTwolevelBHTIndex() const { return twolevel_bht_index; }

	/// Get two level PHT row
	int getTwolevelPHTRow() const { return twolevel_pht_row; }

	/// Get two level PHT column
	int getTwolevelPHTCol() const { return twolevel_pht_col; }

	/// Get Two level prediction
	BranchPredictor::Prediction getTwolevelPrediction() const { return twolevel_pred; }

	/// Get choice index of combined branch predictor
	int getChoiceIndex() const { return choice_index; }

	/// Get combined branch predictor predictor
	BranchPredictor::Prediction getChoicePrediction() const { return choice_pred; }

	/// Get speculative mode
	bool getSpeculativeMode() const { return speculative_mode; }

	/// Get output dependency count of INT physical register
	int getPhyRegIntOdepCount() const { return phy_int_odep_count; }

	/// Get output dependency count of FP physical register
	int getPhyRegFpOdepCount() const { return phy_fp_odep_count; }

	/// Get output dependency count of XMM physical register
	int getPhyRegXmmOdepCount() const { return phy_xmm_odep_count; }

	/// Get physical register in the input dependency table
	int getPhyRegIdep(int index) { return phy_idep[index]; }

	/// Get physical register in the output dependency table
	int getPhyRegOdep(int index) { return phy_odep[index]; }

	/// Get physical register in the old output dependency table
	int getPhyRegOOdep(int index) { return phy_oodep[index]; }

	/// Get first cycle when functional unit is tried to be reserved
	long long getFirstCycleTryReserve() { return first_cycle_try_reserve; }

	/// Get the cycle when the uop is ready
	long long getReadyWhen() { return ready_when; }

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
	// Queues and iterators
	//

	// True is the instruction is currently in the fetch queue
	bool in_fetch_queue = false;

	// Position of the uop in the core's fetch queue, or past-the-end
	// iterator if not present.
	std::list<std::shared_ptr<Uop>>::iterator fetch_queue_iterator;

	// True if the instruction is currently in the core's event queue
	bool in_event_queue = false;

	// Position of the uop in the core's event queue, or past-the-end
	// iterator to this queue if not present.
	std::list<std::shared_ptr<Uop>>::iterator event_queue_iterator;



	
	//
	// Fetch info
	//

	// Address of x86 macro-instruction
	unsigned int eip = 0;

	// Address of next non-speculative x86 macro-instruction
	unsigned int neip = 0;

	// Address of next predicted x86 macro-instruction (for branches)
	unsigned int predicted_neip = 0;

	// Address of target x86 macro-instruction assuming branch taken (for branches)
	unsigned int target_neip = 0;

	// Flag telling if micro-operation is in speculative mode
	bool speculative_mode = false;




};

}

#endif

