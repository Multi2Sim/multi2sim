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
class Thread;

// Class Uop
class Uop
{
private:

	//
	// IDs
	//

	// Unique ID
	long long id = 0;

	// Unique ID in core
	long long id_in_core = 0;




	// Thread the Uop belongs to
	Thread *thread = nullptr;

	// Micro-instruction
	UInst *uinst = nullptr;

	// Context
	Context *context = nullptr;

	// Uop flags
	int flags = 0;




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

	// Physical address of memory access to fetch this instruction
	unsigned int fetch_address = 0;

	// Access identifier to fetch this instruction
	long long fetch_access_id = 0;

	// Flag telling if uop came from trace cache
	bool is_from_trace_cache = false;




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

	// Input dependency count for Physical INT/FP/XMM registers
	int phy_int_idep_count = 0, phy_fp_idep_count = 0, phy_xmm_idep_count = 0;

	// Output dependency count for Physical INT/FP/XMM registers
	int phy_int_odep_count = 0, phy_fp_odep_count = 0, phy_xmm_odep_count = 0;

	// Input dependency physical register table
	int phy_idep[UInstMaxIDeps];

	// Output dependency physical register table
	int phy_odep[UInstMaxODeps];

	// Old output dependency physical register table
	int phy_oodep[UInstMaxODeps];




	//
	// Queues where instruction is
	//

	// Is in fetch queue
	bool in_fetch_queue = false;

	// Is in Uop queue
	bool in_uop_queue = false;

	// Is in instruction queue
	bool in_instruction_queue = false;

	// Is in load queue
	bool in_load_queue = false;

	// Is in store queue
	bool in_store_queue = false;

	// Is in event queue
	bool in_event_queue = false;

	// Is in reorder buffer
	bool in_reorder_buffer = false;

	// Is in Uop trace list
	bool in_uop_trace_list = false;




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
	int twolevel_bht_index = 0, twolevel_pht_row = 0, twolevel_pht_col = 0;

	// Twolevel Branch prediction
	BranchPredictor::Prediction twolevel_pred = BranchPredictor::PredictionNotTaken;

	// Choice index in the combined branch predictor
	int choice_index = 0;

	// Combined Branch prediction
	BranchPredictor::Prediction choice_pred = BranchPredictor::PredictionNotTaken;

public:

	/// Constructor for Unit test
	Uop();

	/// Override constructor
	Uop(int id, int id_in_core, Thread *thread);

	/// Dump Uop information
	void Dump();

	/// Count the dependencies of the current Uop
	void CountDependencies();



	//
	// Setters
	//

	/// Set micro-instruction associated with this Uop
	void setThread(Thread *thread) {this->thread = thread; }

	/// Set micro-instruction associated with this Uop
	void setUInst(UInst *uinst) {this->uinst = uinst; }

	/// Set micro-instruction associated with this Uop
	void setContext(Context *context) {this->context = context; }

	/// Set flags
	void setFlags(int flags) { this->flags = flags; }

	/// Set ID
	void setID(int id) { this->id = id; }

	/// set ID in core
	void setIdInCore(int id_in_core) { this->id_in_core = id_in_core; }

	/// Set global prediction
	void setPrediction(BranchPredictor::Prediction pred)
	{
		this->pred = pred;
	}

	/// Set next EIP
	void setNeip(unsigned int neip) { this->neip = neip; }

	/// Set EIP
	void setEip(unsigned int eip) { this->eip = eip; }

	/// Set predicted next EIP
	void setPredictedNeip(unsigned int addr) { this->predicted_neip = addr; }

	/// Set Target EIP
	void setTargetNeip(unsigned int addr) { this->target_neip = addr; }

	/// Set Macro-Operation size
	void setMopSize(int mop_size) { this->mop_size = mop_size; }

	/// Set Uop counts of a particular Macro-operation
	void setMopCount(int mop_count) { this->mop_count = mop_count; }

	/// Set Macro-Operation index
	void setMopIndex(int mop_index) { this->mop_index = mop_index; }

	/// Set Macro-Operation ID
	void setMopId(long long mop_id) { this->mop_id = mop_id;}

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

	/// Set speculative mode
	void setSpeculativeMode(bool speculative_mode) { this->speculative_mode = speculative_mode; }

	/// Set fetch address
	void setFetchAddress(int fetch_address) { this->fetch_address = fetch_address; }

	/// set fetch access identifier
	void setFetchAccessId(long long fetch_access_id) { this->fetch_access_id = fetch_access_id; }

	/// Set input dependency count of INT physical register
	void setPhyRegIntIdepCount(int phy_int_idep_count) { this->phy_int_idep_count = phy_int_idep_count; }

	/// Set output dependency count of INT physical register
	void setPhyRegIntOdepCount(int phy_int_odep_count) { this->phy_int_odep_count = phy_int_odep_count; }

	/// Set input dependency count of FP physical register
	void setPhyRegFpIdepCount(int phy_fp_idep_count) { this->phy_fp_idep_count = phy_fp_idep_count; }

	/// Set output dependency count of FP physical register
	void setPhyRegFpOdepCount(int phy_fp_odep_count) { this->phy_fp_odep_count = phy_fp_odep_count; }

	/// Set input dependency count of XMM physical register
	void setPhyRegXmmIdepCount(int phy_xmm_idep_count) { this->phy_xmm_idep_count = phy_xmm_idep_count; }

	/// Set output dependency count of XMM physical register
	void setPhyRegXmmOdepCount(int phy_xmm_odep_count) { this->phy_xmm_odep_count = phy_xmm_odep_count; }

	/// Set physical register in the input dependency table
	void setPhyRegIdep(int index, int reg_no) { phy_idep[index] = reg_no; }

	/// Set physical register in the output dependency table
	void setPhyRegOdep(int index, int reg_no) { phy_odep[index] = reg_no; }

	/// Set physical register in the old output dependency table
	void setPhyRegOOdep(int index, int reg_no) { phy_oodep[index] = reg_no; }

	/// Set flag of indicating whether uop is in reorder buffer
	void setInReorderBuffer(bool in_reorder_buffer) { this->in_reorder_buffer = in_reorder_buffer; }


	/// Set flag that indicates whether instruction is in fetch queue
	void setInFetchQueue(bool in_fetch_queue) { this->in_fetch_queue = in_fetch_queue; }

	/// Set flag that indicates whether instruction is from trace cache
	void setIsFromTraceCache(bool is_from_trace_cache)
	{
		this->is_from_trace_cache = is_from_trace_cache;
	}

	/// Set flag that indicates whether instruction is in instruction queue
	void setInInstructionQueue(bool in_instruction_queue)
	{
		this->in_instruction_queue = in_instruction_queue;
	}

	/// Set flag that indicates whether instruction is in load queue
	void setInLoadQueue(bool in_load_queue)
	{
		this->in_load_queue = in_load_queue;
	}

	/// Set flag that indicates whether instruction is in store queue
	void setInStoreQueue(bool in_store_queue)
	{
		this->in_store_queue = in_store_queue;
	}

	/// Set flag that indicates whether instruction is in uop queue
	void setInUopQueue(bool in_uop_queue)
	{
		this->in_uop_queue = in_uop_queue;
	}

	/// Set flag that indicates whether instruction is in event queue
	void setInEventQueue(bool in_event_queue)
	{
		this->in_event_queue = in_event_queue;
	}

	/// Set first cycle when functional unit is tried to be reserved
	void setFirstCycleTryReserve(long long first_cycle_try_reserve)
	{
		this->first_cycle_try_reserve = first_cycle_try_reserve;
	}

	/// Set the cycle when the uop is ready
	void setReadyWhen(long long ready_when)
	{
		this->ready_when = ready_when;
	}

	/// Set the cycle when the uop is issued
	void setIssueWhen(long long issue_when)
	{
		this->issue_when = issue_when;
	}

	/// Set the physical address of Uop
	void setPhyAddr(unsigned int phy_addr) { this->phy_addr = phy_addr; }




	//
	// Getters
	//

	/// Get associated thread
	Thread *getThread() const { return thread; }

	/// Get micro-instruction associated with this Uop
	UInst *getUinst() { return uinst; }

	/// Get the Uop ID
	int getID() const { return id; }

	/// Get flags
	int getFlags() const { return flags; }

	/// Get global prediction
	BranchPredictor::Prediction getPrediction() const { return pred; }

	/// Get next EIP
	unsigned int getNeip() const { return neip; }

	/// Get EIP
	unsigned int getEip() const { return eip; }

	/// Get predicted next EIP
	unsigned int getPredictedNeip() const { return predicted_neip; }

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

	/// Get fetch address
	int getFetchAddress() { return fetch_address; }

	/// Get fetch access identifier
	long long getFetchAccessId() { return fetch_access_id; }

	/// Get input dependency count of INT physical register
	int getPhyRegIntIdepCount() const { return phy_int_idep_count; }

	/// Get output dependency count of INT physical register
	int getPhyRegIntOdepCount() const { return phy_int_odep_count; }

	/// Get input dependency count of FP physical register
	int getPhyRegFpIdepCount() const { return phy_fp_idep_count; }

	/// Get output dependency count of FP physical register
	int getPhyRegFpOdepCount() const { return phy_fp_odep_count; }

	/// Get input dependency count of XMM physical register
	int getPhyRegXmmIdepCount() const { return phy_xmm_idep_count; }

	/// Get output dependency count of XMM physical register
	int getPhyRegXmmOdepCount() const { return phy_xmm_odep_count; }

	/// Get physical register in the input dependency table
	int getPhyRegIdep(int index) { return phy_idep[index]; }

	/// Get physical register in the output dependency table
	int getPhyRegOdep(int index) { return phy_odep[index]; }

	/// Get physical register in the old output dependency table
	int getPhyRegOOdep(int index) { return phy_oodep[index]; }

	/// Get flag of indicating whether uop is in reorder buffer
	bool getInReorderBuffer() { return in_reorder_buffer; }

	/// Get flag that indicates whether instruction is in fetch queue
	bool isInFetchQueue() { return in_fetch_queue; }

	/// Get flag that indicates whether instruction is from trace cache
	bool isFromTraceCache() { return is_from_trace_cache; }

	/// Get flag that indicates whether instruction is in instruction queue
	bool isInInstructionQueue() { return in_instruction_queue; }

	/// Get flag that indicates whether instruction is in load queue
	bool isInLoadQueue() { return in_load_queue; }

	/// Get flag that indicates whether instruction is in store queue
	bool isInStoreQueue() { return in_store_queue; }

	/// Get flag that indicates whether instruction is in uop queue
	bool isInUopQueue() { return in_uop_queue; }

	/// Get flag that indicates whether instruction is in event queue
	bool isInEventQueue() { return in_event_queue; }

	/// Get first cycle when functional unit is tried to be reserved
	long long getFirstCycleTryReserve() { return first_cycle_try_reserve; }

	/// Get the cycle when the uop is ready
	long long getReadyWhen() { return ready_when; }

	/// Get the cycle when the uop is issued
	long long getIssueWhen() { return issue_when; }

	/// Get the physical address of Uop
	unsigned int getPhyAddr() { return phy_addr; }




	/// Compare with other Uop based on its ready time or unique ID
	///
	/// \ param uop
	///	One other uop that is used to compare with current one
	///
	/// \return
	///	< 0: Current uop should be fetched in a queue before the other uop
	///	> 0: Current uop should be fetched in a queue after the other uop
	int Compare(Uop *uop);
};

}

#endif // ARCH_X86_TIMING_UOP_H
