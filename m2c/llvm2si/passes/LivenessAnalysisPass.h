/*
 *  Multi2Sim
 *  Copyright (C) 2014  Ce Gao (gaoce@coe.neu.edu)
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

#ifndef M2C_LLVM2SI_PASSES_LIVENESS_ANALYSIS_PASS_H
#define M2C_LLVM2SI_PASSES_LIVENESS_ANALYSIS_PASS_H

#include <memory>
#include <unordered_map>

#include <m2c/common/Pass.h>
#include <m2c/common/PassInfo.h>
#include <m2c/llvm2si/Function.h>
#include <src/lib/cpp/Bitmap.h>
#include <src/lib/cpp/Misc.h>


namespace llvm2si
{

class BasicBlockLivenessAnalysisPassInfo : public comm::BasicBlockPassInfo
{
        // Hash tables mapping basic block id to the corresponding bitmaps for
	// vector register def, use, live_out, live_in sets
	std::unordered_map<int, std::unique_ptr<misc::Bitmap>> vector_def_map;
	std::unordered_map<int, std::unique_ptr<misc::Bitmap>> vector_use_map;
	std::unordered_map<int, std::unique_ptr<misc::Bitmap>> vector_out_map;
	std::unordered_map<int, std::unique_ptr<misc::Bitmap>> vector_in_map;

        // Hash tables mapping basic block id to the corresponding bitmaps for
	// scalar register def, use, live_out, live_in sets
	std::unordered_map<int, std::unique_ptr<misc::Bitmap>> scalar_def_map;
	std::unordered_map<int, std::unique_ptr<misc::Bitmap>> scalar_use_map;
	std::unordered_map<int, std::unique_ptr<misc::Bitmap>> scalar_out_map;
	std::unordered_map<int, std::unique_ptr<misc::Bitmap>> scalar_in_map;

public:

	/// Return vector def set associated with the basic block id, or a
	/// nullptr if the id is not found
	misc::Bitmap *getVectorDefSet(int basic_block_id) const
	{
		auto it = vector_def_map.find(basic_block_id);
		return it == vector_def_map.end() ? nullptr : it->second.get();
	}

	/// Return vector use set associated with the basic block id, or a
	/// nullptr if the id is not found
	misc::Bitmap *getVectorUseSet(int basic_block_id) const
	{
		auto it = vector_use_map.find(basic_block_id);
		return it == vector_use_map.end() ? nullptr : it->second.get();
	}

	/// Return vector out set associated with the basic block id, or a
	/// nullptr if the id is not found
	misc::Bitmap *getVectorOutSet(int basic_block_id) const
	{
		auto it = vector_out_map.find(basic_block_id);
		return it == vector_out_map.end() ? nullptr : it->second.get();
	}

	/// Return vector def set associated with the basic block id, or a
	/// nullptr if the id is not found
	misc::Bitmap *getVectorInSet(int basic_block_id) const
	{
		auto it = vector_in_map.find(basic_block_id);
		return it == vector_in_map.end() ? nullptr : it->second.get();
	}

	/// Return scalar def set associated with the basic block id, or a
	/// nullptr if the id is not found
	misc::Bitmap *getScalarDefSet(int basic_block_id) const
	{
		auto it = scalar_def_map.find(basic_block_id);
		return it == scalar_def_map.end() ? nullptr : it->second.get();
	}

	/// Return scalar use set associated with the basic block id, or a
	/// nullptr if the id is not found
	misc::Bitmap *getScalarUseSet(int basic_block_id) const
	{
		auto it = scalar_use_map.find(basic_block_id);
		return it == scalar_use_map.end() ? nullptr : it->second.get();
	}

	/// Return vector out set associated with the basic block id, or a
	/// nullptr if the id is not found
	misc::Bitmap *getScalarOutSet(int basic_block_id) const
	{
		auto it = scalar_out_map.find(basic_block_id);
		return it == scalar_out_map.end() ? nullptr : it->second.get();
	}

	/// Return scalar def set associated with the basic block id, or a
	/// nullptr if the id is not found
	misc::Bitmap *getScalarInSet(int basic_block_id) const
	{
		auto it = scalar_in_map.find(basic_block_id);
		return it == scalar_in_map.end() ? nullptr : it->second.get();
	}

	/// Reset the vector def set to 0 at all locations
	misc::Bitmap *resetVectorDefSet(int basic_block_id, int num_registers)
	{
		vector_def_map[basic_block_id] =
				misc::new_unique<misc::Bitmap>(num_registers);
		return vector_def_map[basic_block_id].get();
	}

	/// Reset the vector use set to 0 at all locations
	misc::Bitmap *resetVectorUseSet(int basic_block_id, int num_registers)
	{
		vector_use_map[basic_block_id] =
				misc::new_unique<misc::Bitmap>(num_registers);
		return vector_use_map[basic_block_id].get();
	}

	/// Reset the vector out set to 0 at all locations
	misc::Bitmap *resetVectorOutSet(int basic_block_id, int num_registers)
	{
		vector_out_map[basic_block_id] =
				misc::new_unique<misc::Bitmap>(num_registers);
		return vector_out_map[basic_block_id].get();
	}

	/// Reset the vector in set to 0 at all locations
	misc::Bitmap *resetVectorInSet(int basic_block_id, int num_registers)
	{
		vector_in_map[basic_block_id] =
				misc::new_unique<misc::Bitmap>(num_registers);
		return vector_in_map[basic_block_id].get();
	}

	/// Reset the scalar def set to 0 at all locations
	misc::Bitmap *resetScalarDefSet(int basic_block_id, int num_registers)
	{
		scalar_def_map[basic_block_id] =
				misc::new_unique<misc::Bitmap>(num_registers);
		return scalar_def_map[basic_block_id].get();
	}

	/// Reset the scalar use set to 0 at all locations
	misc::Bitmap *resetScalarUseSet(int basic_block_id, int num_registers)
	{
		scalar_use_map[basic_block_id] =
				misc::new_unique<misc::Bitmap>(num_registers);
		return scalar_use_map[basic_block_id].get();
	}

	/// Reset the scalar out set to 0 at all locations
	misc::Bitmap *resetScalarOutSet(int basic_block_id, int num_registers)
	{
		scalar_out_map[basic_block_id] =
				misc::new_unique<misc::Bitmap>(num_registers);
		return scalar_out_map[basic_block_id].get();
	}

	/// Reset the scalar in set to 0 at all locations
	misc::Bitmap *resetScalarInSet(int basic_block_id, int num_registers)
	{
		scalar_in_map[basic_block_id] =
				misc::new_unique<misc::Bitmap>(num_registers);
		return scalar_in_map[basic_block_id].get();
	}

	/// Destructor
	~BasicBlockLivenessAnalysisPassInfo() {}
};

class LivenessAnalysisPass : public comm::Pass
{
	// Function to run LivenessAnalysis on
	llvm2si::Function *function;
	
	// Pass id
	int id;

public:

	/// Constructor
	LivenessAnalysisPass(llvm2si::Function *function, int id) :
		function(function), id(id)
	{
	}

	/*
	bool hasInfo;
	setInfo;
	*/

	/// Return a pointer of BasicBlockLivenessAnalysisPassInfo
	template<typename ConcreteType> ConcreteType *getInfo(
			BasicBlock *basic_block)
	{
		return basic_block->getPassInfoPool()->get<ConcreteType>(id);
	}

	/// Run the Liveness Analysis Pass
	void run();

	/// Destructor
	~LivenessAnalysisPass() {}
};


}  // namespace llvm2si


#endif

