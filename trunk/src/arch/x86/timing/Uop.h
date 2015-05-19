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

#ifndef X86_ARCH_TIMING_UOP_H
#define X86_ARCH_TIMING_UOP_H

#include <lib/cpp/Misc.h>

#include <arch/x86/emu/UInst.h>

#include "BranchPredictor.h"

namespace x86
{

// Constant value
const unsigned int UopMagic = 0x10101010;

// Forward declarations
class UInst;
class Context;
class Thread;

// Class Uop
class Uop
{
private:

	// Micro-instruction
	UInst *uinst = nullptr;
	int flags = 0;

	// Name and sequence numbers 
	long long magic = UopMagic;  // Magic number for debugging
	long long id = 0;  // Unique ID
	long long id_in_core = 0;  // Unique ID in core

	// Software context and hardware thread where uop belongs 
	Context *ctx = nullptr;
	Thread *thread = nullptr;

	// Fetch info 
	unsigned int eip = 0;  // Address of x86 macro-instruction
	unsigned int neip = 0;  // Address of next non-speculative x86 macro-instruction
	unsigned int predicted_neip = 0; // Address of next predicted x86 macro-instruction (for branches)
	unsigned int target_neip = 0;  // Address of target x86 macro-instruction assuming branch taken (for branches)
	bool speculative_mode = false;
	unsigned int fetch_address = 0;  // Physical address of memory access to fetch this instruction
	long long fetch_access = 0;  // Access identifier to fetch this instruction
	bool trace_cache = false;  // Flag telling if uop came from trace cache

	// Fields associated with macroinstruction 
	int mop_index = 0;  // Index of uop within macroinstruction
	int mop_count = 0;  // Number of uops within macroinstruction
	int mop_size = 0;  // Corresponding macroinstruction size
	long long mop_id = 0;  // Sequence number of macroinstruction

	// Logical dependencies 
	int idep_count = 0;
	int odep_count = 0;

	// Physical mappings 
	int ph_int_idep_count = 0, ph_fp_idep_count = 0, ph_xmm_idep_count = 0;
	int ph_int_odep_count = 0, ph_fp_odep_count = 0, ph_xmm_odep_count = 0;
	int ph_idep[UInstMaxIDeps];
	int ph_odep[UInstMaxODeps];
	int ph_oodep[UInstMaxODeps];

	// Queues where instruction is 
	bool in_fetch_queue = false;
	bool in_uop_queue = false;
	bool in_iq = false;
	bool in_lq = false;
	bool in_sq = false;
	bool in_preq = false;
	bool in_event_queue = false;
	bool in_rob = false;
	bool in_uop_trace_list = false;

	// Instruction status 
	bool ready = false;
	bool issued = false;
	bool completed = false;

	// For memory uops 
	unsigned int phy_addr = 0;  // ... corresponding to 'uop->uinst->address'

	// Cycles 
	long long when = 0;  // cycle when ready
	long long issue_try_when = 0;  // first cycle when f.u. is tried to be reserved
	long long issue_when = 0;  // cycle when issued

	// Global prediction (0=not taken, 1=taken)
	BranchPredictorPred pred = BranchPredictorPredNotTaken;

	// Bimodal Branch prediction
	int bimod_index = 0;
	BranchPredictorPred bimod_pred = BranchPredictorPredNotTaken;

	// Twolevel Branch prediction
	int twolevel_bht_index = 0, twolevel_pht_row = 0, twolevel_pht_col = 0;
	BranchPredictorPred twolevel_pred = BranchPredictorPredNotTaken;

	// Combined Branch prediction
	int choice_index = 0;
	BranchPredictorPred choice_pred = BranchPredictorPredNotTaken;

public:

	void FreeIfNotQueued();
	void Dump();

	bool Exists();
	void CountDeps();

	/// Setters
	void setUInst(UInst *uinst) {this->uinst = uinst; }
	void setID(int id) { this->id = id; }
	void setFlags(int flags) { this->flags = flags; }
	void setPrediction(BranchPredictorPred pred)
	{
		this->pred = pred;
	}
	void setNeip(unsigned int neip) { this->neip = neip; }
	void setEip(unsigned int eip) { this->eip = eip; }
	void setPredictedNeip(unsigned int addr) { this->predicted_neip = addr; }
	void setTargetNeip(unsigned int addr) { this->target_neip = addr; }
	void setMopSize(int mop_size) { this->mop_size = mop_size; }
	void setMopCount(int mop_count) { this->mop_count = mop_count; }
	void setMopIndex(int mop_index) { this->mop_index = mop_index; }
	void setMopId(long long mop_id) { this->mop_id = mop_id;}
	void setBimodIndex(int bimod_index) { this->bimod_index = bimod_index; }
	void setBimodPrediction(BranchPredictorPred pred)
	{
		this->bimod_pred = pred;
	}
	void setTwolevelBHTIndex(int index) { this->twolevel_bht_index = index; }
	void setTwolevelPHTRow(int row) { this->twolevel_pht_row = row; }
	void setTwolevelPHTCol(int col) { this->twolevel_pht_col = col; }
	void setTwolevelPrediction(BranchPredictorPred pred)
	{
		this->twolevel_pred = pred;
	}
	void setChoiceIndex(int choice_index) { this->choice_index = choice_index; }
	void setChoicePrediction(BranchPredictorPred pred)
	{
		this->choice_pred = pred;
	}
	void setSpeculativeMode(bool speculative_mode) { this->speculative_mode = speculative_mode; }

	/// Getters
	UInst *getUinst() { return uinst; }
	int getID() const { return id; }
	int getFlags() const { return flags; }
	BranchPredictorPred getPrediction() const { return pred; }
	unsigned int getNeip() const { return neip; }
	unsigned int getEip() const { return eip; }
	unsigned int getPredictedNeip() const { return predicted_neip; }
	unsigned int getTargetNeip() const { return target_neip; }
	int getMopSize() const { return mop_size; }
	int getMopCount() const { return mop_count; }
	int getMopIndex() const { return mop_index; }
	long long getMopId() const { return mop_id; }
	int getBimodIndex() const { return bimod_index; }
	BranchPredictorPred getBimodPrediction() const { return bimod_pred; }
	int getTwolevelBHTIndex() const { return twolevel_bht_index; }
	int getTwolevelPHTRow() const { return twolevel_pht_row; }
	int getTwolevelPHTCol() const { return twolevel_pht_col; }
	BranchPredictorPred getTwolevelPrediction() const { return twolevel_pred; }
	int getChoiceIndex() const { return choice_index; }
	BranchPredictorPred getChoicePrediction() const { return choice_pred; }
	bool getSpeculativeMode() const { return speculative_mode; }
};

}

#endif // X86_ARCH_TIMING_UOP_H
