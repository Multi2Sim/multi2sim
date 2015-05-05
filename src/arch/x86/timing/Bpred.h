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

#ifndef X86_ARCH_TIMING_BPRED_H
#define X86_ARCH_TIMING_BPRED_H

#include "Uop.h"

namespace x86
{

// Forward declarations
class Uop;

// BTB Entry 
struct BtbEntry
{
	unsigned int source;  // eip 
	unsigned int target;  // neip 
	int counter;  // LRU counter 
};


// Branch Predictor Structure 
class Bpred
{
private:

	// Unique instance of the singleton
	static std::unique_ptr<Bpred> instance;

	char *name;

	// RAS 
	unsigned int *ras;
	int ras_index;

	// BTB - array of x86_bpred_btb_sets*x86_bpred_btb_assoc entries of
	// type x86_bpred_btb_entry_t. 
	BtbEntry *btb;

	// bimod - array of bimodal counters indexed by PC
	//   0,1 - Branch not taken.
	//   2,3 - Branch taken.
	char *bimod;

	// Two-level adaptive branch predictor. It contains a
	// BHT (branch history table) and PHT (pattern history table).
	unsigned int *twolevel_bht;  // array of level1_size branch history registers 
	char *twolevel_pht;  // array of level2_size*2^hist_size 2-bit counters 

	// choice - array of bimodal counters indexed by PC
	//   0,1 - Use bimodal predictor.
	//   2,3 - Use two-level adaptive predictor
	char *choice;

	// Stats 
	long long accesses;
	long long hits;

public:

	/// Constructor
	Bpred(char *name);

	/// Destructor
	~Bpred();

	/// Return unique instance of the branch predictor
	static Bpred *getInstance();

};

}

#endif // X86_ARCH_TIMING_BPRED_H
