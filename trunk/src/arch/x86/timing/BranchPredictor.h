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

#ifndef X86_ARCH_TIMING_BRANCH_PREDICTOR_H
#define X86_ARCH_TIMING_BRANCH_PREDICTOR_H

#include <string>

#include <lib/cpp/IniFile.h>

#include "Thread.h"

namespace x86
{

// Enumeration of branch predictor type
enum BranchPredictorType
{
	BranchPredictorTypePerfect = 0,
	BranchPredictorTypeTaken,
	BranchPredictorTypeNottaken,
	BranchPredictorTypeBimod,
	BranchPredictorTypeTwolevel,
	BranchPredictorTypeComb
};

// Branch Predictor class
class BranchPredictor
{
private:

	// The defined name
	std::string name;

	// Return address stack
	std::unique_ptr<int[]> ras;
	int ras_index;

	// BTB Entry
	struct BTBEntry
	{
		unsigned int source;  // eip
		unsigned int target;  // neip
		int counter;  // LRU counter
	};

	// BTB - array of x86_bpred_btb_sets*x86_bpred_btb_assoc entries of
	// type BTBEntry.
	std::unique_ptr<BTBEntry[]> btb;

	// bimod - array of bimodal counters indexed by PC
	//   0,1 - Branch not taken.
	//   2,3 - Branch taken.
	std::unique_ptr<char[]> bimod;

	// Two-level adaptive branch predictor. It contains a
	// BHT (branch history table) and PHT (pattern history table).
	std::unique_ptr<unsigned int[]> twolevel_bht;  // array of level1_size branch history registers
	std::unique_ptr<char[]> twolevel_pht;  // array of level2_size*2^hist_size 2-bit counters

	// choice - array of bimodal counters indexed by PC
	//   0,1 - Use bimodal predictor.
	//   2,3 - Use two-level adaptive predictor
	std::unique_ptr<char[]> choice;

	// Stats 
	long long accesses;
	long long hits;

	// Branch predictor parameter
	static int btb_sets;
	static int btb_assoc;
	static int ras_size;
	static int bimod_size;
	static int choice_size;
	static int twolevel_l1size;
	static int twolevel_l2size;
	static int twolevel_hist_size;
	static int twolevel_l2height;

public:

	/// Constructor
	BranchPredictor(std::string &name);

	/// Read branch predictor configuration from configuration file
	static void ReadBranchPredictorConfig(std::string &config);
};

}

#endif // X86_ARCH_TIMING_BRANCH_PREDICTOR_H
