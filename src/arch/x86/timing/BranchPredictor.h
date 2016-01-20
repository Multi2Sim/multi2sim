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

#ifndef ARCH_X86_TIMING_BRANCH_PREDICTOR_H
#define ARCH_X86_TIMING_BRANCH_PREDICTOR_H

#include <string>

#include <arch/x86/emulator/Uinst.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/IniFile.h>


namespace x86
{

// Forward declaration
class Uop;

// Branch Predictor class
class BranchPredictor
{
public:

	/// Global prediction enumeration
	enum Prediction
	{
		PredictionNotTaken = 0,
		PredictionTaken
	};

	/// Enumeration of branch predictor Kind
	enum Kind
	{
		KindInvalid = 0,
		KindPerfect,
		KindTaken,
		KindNottaken,
		KindBimod,
		KindTwoLevel,
		KindCombined
	};

	/// string map of branch predictor kind
	static misc::StringMap KindMap;

private:

	//
	// Static fields
	//

	// Branch predictor kind
	static Kind kind;

	// Number of sets in the BTB
	static int btb_num_sets;

	// Associativity of the BTB
	static int btb_num_ways;

	// Size of the return address stack
	static int ras_size;

	// Size of the bimodal predictor
	static int bimod_size;

	// Size of the choice predictor
	static int choice_size;

	// Size of the level 1 table of the two-level predictor
	static int two_level_l1_size;

	// Size of the level 2 table of the two-level predictor
	static int two_level_l2_size;

	// Prediction history size
	static int two_level_history_size;

	// Height of the level 2 table of the two-level predictor
	static int two_level_l2_height;




	//
	// Class members
	//

	// The defined name
	std::string name;

	// Return address stack
	std::unique_ptr<int[]> ras;

	// Top of the return address stack
	int ras_index = 0;

	// Branch Target Buffer entry
	struct BtbEntry
	{
		unsigned int source;  // eip
		unsigned int target;  // neip
		int counter;  // LRU counter
	};

	// BTB - array of btb_sets * btb_assoc entries of type BtbEntry.
	std::unique_ptr<BtbEntry[]> btb;

	// bimod - array of bimodal counters indexed by PC
	//   0,1 - Branch not taken.
	//   2,3 - Branch taken.
	std::unique_ptr<char[]> bimod;

	// Two-level adaptive branch predictor. It contains a
	// BHT (branch history table) and PHT (pattern history table).
	std::unique_ptr<unsigned int[]> two_level_bht;  // array of level1_size branch history registers
	std::unique_ptr<char[]> two_level_pht;  // array of level2_size*2^hist_size 2-bit counters

	// choice - array of bimodal counters indexed by PC
	//   0,1 - Use bimodal predictor.
	//   2,3 - Use two-level adaptive predictor
	std::unique_ptr<char[]> choice;

	// Stats 
	long long accesses = 0;
	long long hits = 0;

public:

	//
	// Class Error
	//

	/// Exception for X86 branch predictor
	class Error : public misc::Error
	{
	public:

		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("X86 branch predictor");
		}
	};



	//
	// Static functions
	//

	static Kind getKind() { return kind; }

	static int getBtbNumSets() { return btb_num_sets; }

	static int getBtbNumWays() { return btb_num_ways; }

	static int getRasSize() { return ras_size; }

	static int getBimodSize() { return bimod_size; }

	static int getChoiceSize() { return choice_size; }

	static int getTwoLevelL1Size() { return two_level_l1_size; }

	static int getTwoLevelL2Size() { return two_level_l2_size; }

	static int getTwoLevelHistorySize() { return two_level_history_size; }

	static int getTwoLevelL2Height() { return two_level_l2_height; }




	//
	// Class members
	//

	/// Constructor
	BranchPredictor(const std::string &name = "");

	/// Read branch predictor configuration from configuration file
	static void ParseConfiguration(misc::IniFile *ini_file);

	/// Dump configuration
	void DumpConfiguration(std::ostream &os = std::cout);

	char getBimodStatus(int index) const { return bimod[index]; }

	int getTwoLevelBhtStatus(int index) const { return two_level_bht[index]; }

	char getTwoLevelPhtStatus(int row, int col) const
	{
		return two_level_pht[row * two_level_l2_size + col];
	}

	int getChoiceStatus(int index) const { return choice[index]; }

	/// Return prediction for an address (0=not taken, 1=taken)
	///
	/// \param uop
	/// 	Micro-instruction with information used to read the branch
	///	predictor.
	///
	/// \return
	/// 	Global prediction result
	///
	Prediction Lookup(Uop *uop);

	/// Return multiple predictions for an address. This can only be done
	/// for two-level adaptive predictors, since they use global history.
	/// The prediction of the primary branch is stored in the least
	/// significant bit (bit 0), whereas the prediction of the last branch
	/// is stored in bit 'count - 1'.
	///
	/// \param eip
	/// 	The instruction address
	///
	/// \param count
	/// 	Maximum number of branches in a trace
	///
	/// \return
	/// 	Global prediction result
	///
	int LookupMultiple(unsigned int eip, int count);

	/// Update the parameter inside branch predictor
	///
	/// \param uop
	/// 	Micro-instruction with branch prediction information.
	///
	void Update(Uop *uop);

	/// Lookup BTB. If it contains the uop address, return target. The BTB
	/// also contains information about the type of branch, i.e., jump,
	/// call, ret, or conditional. If instruction is call or ret, access RAS
	/// instead of BTB.
	///
	/// \param uop
	/// 	Micro-instruction capturing all the information related with the
	///	branch prediction.
	///
	/// \return
	/// 	Target address
	///
	unsigned int LookupBtb(Uop *uop);

	/// Update the BTB
	///
	/// \param uop
	/// 	Micro-instruction including all the information related with
	///	its branch prediction.
	///
	void UpdateBtb(Uop *uop);

	/// Find address of next branch after eip within current block.
	/// This is useful for accessing the trace cache. At that point, the
	/// uop is not ready to call \c LookupBtb(), since functional simulation
	/// has not happened yet.
	///
	/// \param eip
	/// 	The instruction address
	///
	/// \param block_size
	/// 	The block size of instruction cache
	///
	/// \return
	/// 	Next branch address
	///
	unsigned int getNextBranch(unsigned int eip, unsigned int block_size);
};

}

#endif // ARCH_X86_TIMING_BRANCH_PREDICTOR_H
