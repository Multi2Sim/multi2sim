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

#include <arch/x86/emu/UInst.h>


namespace x86
{

// Forward declaration
class Uop;

// Global prediction
enum BranchPredictorPred
{
	BranchPredictorPredNotTaken = 0,
	BranchPredictorPredTaken
};

// Branch Predictor class
class BranchPredictor
{
public:

	/// Enumeration of branch predictor Kind
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

	/// string map of branch predictor kind
	static misc::StringMap KindMap;

private:

	// The defined name
	std::string name;

	// Return address stack
	std::unique_ptr<int[]> ras;
	int ras_index = 0;

	// BTB Entry (Branch Target Buffer)
	struct BTBEntry
	{
		unsigned int source;  // eip
		unsigned int target;  // neip
		int counter;  // LRU counter
	};

	// BTB - array of btb_sets*btb_assoc entries of
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
	long long accesses = 0;
	long long hits = 0;

	// Branch predictor parameter
	static Kind kind;
	static int btb_sets;
	static int btb_assoc;
	static int ras_size;
	static int bimod_size;
	static int choice_size;
	static int twolevel_l1size;
	static int twolevel_l2size;
	static int twolevel_history_size;
	static int twolevel_l2height;

public:

	/// Constructor
	BranchPredictor(const std::string &name = "");

	/// Read branch predictor configuration from configuration file
	static void ParseConfiguration(const std::string &section,
			misc::IniFile &config);

	/// Dump configuration
	void DumpConfiguration(std::ostream &os = std::cout);

	/// Configuration getters
	static Kind getKind() { return kind; }
	static int getBTBSets() { return btb_sets; }
	static int getBTBAssociativity() { return btb_assoc; }
	static int getRasSize() { return ras_size; }
	static int getBimodSize() { return bimod_size; }
	static int getChoiceSize() { return choice_size; }
	static int getTwolevelL1size() { return twolevel_l1size; }
	static int getTwolevelL2size() { return twolevel_l2size; }
	static int getTwolevelHistorySize() { return twolevel_history_size; }
	static int getTwolevelL2hight() { return twolevel_l2height; }

	/// Getters
	char getBimodStatus(int index) const { return bimod[index]; }

	/// Return prediction for an address (0=not taken, 1=taken)
	///
	/// \param uop
	/// 	Micro-opertion including all the information regarding a micro instruction
	///
	/// \return
	/// 	Global prediction result
	BranchPredictorPred LookupBranchPrediction(Uop &uop);

	/// Return multiple predictions for an address. This can only be done for two-level
	/// adaptive predictors, since they use global history. The prediction of the
	/// primary branch is stored in the least significant bit (bit 0), whereas the prediction
	/// of the last branch is stored in bit 'count-1'.
	///
	/// \param eip
	/// 	The instruction address
	///
	/// \param count
	/// 	Maximum number of branches in a trace
	///
	/// \return
	/// 	Global prediction result
	int LookupBranchPredictionMultiple(unsigned int eip, int count);

	/// Update the parameter inside branch predictor
	///
	/// \param uop
	/// 	Micro-opertion including all the information regarding a micro instruction
	///
	/// \return
	/// 	No value is returned
	void UpdateBranchPredictor(Uop &uop);

	/// Lookup BTB. If it contains the uop address, return target. The BTB also contains
	/// information about the type of branch, i.e., jump, call, ret, or conditional. If
	/// instruction is call or ret, access RAS instead of BTB.
	///
	/// \param uop
	/// 	Micro-opertion including all the information regarding a micro instruction
	///
	/// \return
	/// 	Target address
	unsigned int LookupBTB(Uop &uop);

	/// Update the BTB
	///
	/// \param uop
	/// 	Micro-opertion including all the information regarding a micro instruction
	///
	/// \return
	/// 	No value is returned
	void UpdateBTB(Uop &uop);

	/// Find address of next branch after eip within current block.
	/// This is useful for accessing the trace
	/// cache. At that point, the uop is not ready to call \c LookupBTB(), since
	/// functional simulation has not happened yet.
	/// \param eip
	/// 	The instruction address
	///
	/// \param block_size
	/// 	The block size of instruction cache
	///
	/// \return
	/// 	Next branch address
	unsigned int GetNextBranch(unsigned int eip, unsigned int block_size);
};

}

#endif // X86_ARCH_TIMING_BRANCH_PREDICTOR_H
