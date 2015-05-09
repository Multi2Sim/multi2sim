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

// Branch Predictor class
class BranchPredictor
{
public:

	// Enumeration of branch predictor Kind
	enum Kind
	{
		KindInvalid = 0,
		KindPerfect,
		KindTaken,
		KindNottaken,
		KindBimod,
		KindTwolevel,
		KindCombined
	};

	// string map of branch predictor kind
	misc::StringMap KindMap
	{
		{ "Perfect", KindPerfect},
		{ "Taken", KindTaken },
		{"NotTaken", KindNottaken},
		{"Bimodal", KindBimod},
		{"TwoLevel", KindTwolevel},
		{"Combined", KindCombined}
	};

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
	static Kind kind;
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
	//BranchPredictor();

	/// Create branch preditor instance
	void Create(std::string &branch_predictor_name);

	/// Read branch predictor configuration from configuration file
	void ParseConfiguration(const std::string &section,
			misc::IniFile &config);

	/// Dump configuration
	void DumpConfig(std::ostream &os = std::cout);
};

}

#endif // X86_ARCH_TIMING_BRANCH_PREDICTOR_H
