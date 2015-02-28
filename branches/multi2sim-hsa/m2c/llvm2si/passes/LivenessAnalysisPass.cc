/*
 *  Multi2Sim
 *  Copyright (C) 2014  Charu Kalra (ckalra@ece.neu.edu)
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
#include "LivenessAnalysisPass.h"

namespace llvm2si
{

// Liveness Analysis consists of two phases. Phase 1 evaluates the Use and Def
// sets for every basic block. Phase 2 evaluates the Live In and Live Out of
// every basic block using information from Phase 1
void LivenessAnalysisPass::run()
{
	// Create a basic block queue
	std::vector<llvm2si::BasicBlock *> basic_block_queue;

	// Create Def and Use bitmaps of given size for vector
	// and scalar registers. Used in Phase 1.
	misc::Bitmap v_def(lap_function->getNumVectorRegisters());
	misc::Bitmap v_use(lap_function->getNumVectorRegisters());
	misc::Bitmap s_def(lap_function->getNumScalarRegisters());
	misc::Bitmap s_use(lap_function->getNumScalarRegisters());

	// Fetch basic blocks from the function and
	// push them in to the queue
	for (auto &it : *(lap_function->getBasicBlocks()))
		basic_block_queue.push_back(it.get());

	// Traverse the basic blocks in the function in reverse order.
	for (auto basic_block = basic_block_queue.rbegin();
			basic_block != basic_block_queue.rend();
			basic_block++)
	{
		// Reset all bitmaps
		v_def.Reset();
		v_use.Reset();
		s_def.Reset();
		s_use.Reset();

		auto *basic_block_pass_info =
						getInfo<BasicBlockLivenessAnalysisPassInfo>(*basic_block);

		// Reset all vector sets associated with all basic blocks
		basic_block_pass_info->ResetVectorDefInfo(lap_function->getNumVectorRegisters());
		basic_block_pass_info->ResetVectorUseInfo(lap_function->getNumVectorRegisters());

		basic_block_pass_info->ResetVectorInInfo(lap_function->getNumVectorRegisters());
		basic_block_pass_info->ResetVectorOutInfo(lap_function->getNumVectorRegisters());

		// Reset all scalar sets associated with all basic blocks
		basic_block_pass_info->ResetScalarDefInfo(lap_function->getNumScalarRegisters());
		basic_block_pass_info->ResetScalarUseInfo(lap_function->getNumScalarRegisters());

		basic_block_pass_info->ResetScalarInInfo(lap_function->getNumScalarRegisters());
		basic_block_pass_info->ResetScalarOutInfo(lap_function->getNumScalarRegisters());

		// Create pointers to vector and scalar registers
		si2bin::ArgVectorRegister *vector_register;
		si2bin::ArgScalarRegister *scalar_register;

		std::vector<si2bin::Instruction *> instruction_queue;

		for (auto &rit : (*basic_block)->getInstructions())
				instruction_queue.push_back(rit.get());

		// Traverse through all the instructions in the basic block
		for (auto instruction = instruction_queue.rbegin();
				instruction != instruction_queue.rend();
							instruction++)
		{
			// Get arguments from the instruction
			for (auto &argument : (*instruction)->getArguments())
			{
				switch (argument->getToken()->getDirection())
				{
				case si2bin::TokenDirectionDst:
					switch (argument->getType())
					{
					case si2bin::Argument::TypeVectorRegister:
						vector_register = misc::cast<si2bin::ArgVectorRegister*>(argument.get());
						v_use.Set(vector_register->getId(), false);
						v_def.Set(vector_register->getId(), true);
						break;
					case si2bin::Argument::TypeScalarRegister:
						scalar_register = misc::cast<si2bin::ArgScalarRegister*>(argument.get());
						s_use.Set(scalar_register->getId(), false);
						s_def.Set(scalar_register->getId(), true);
						break;
					default:
						break;
					}
				break;

				case si2bin::TokenDirectionSrc:
					switch (argument->getType())
					{
					case si2bin::Argument::TypeVectorRegister:
						vector_register = misc::cast<si2bin::ArgVectorRegister*>(argument.get());
						v_use.Set(vector_register->getId(), true);
						v_def.Set(vector_register->getId(), false);
						break;
					case si2bin::Argument::TypeScalarRegister:
						scalar_register = misc::cast<si2bin::ArgScalarRegister*>(argument.get());
						s_use.Set(scalar_register->getId(), true);
						s_def.Set(scalar_register->getId(), false);
						break;
					default:
						break;
					}
					break;
				default:
					break;
				}
			}
		}
		basic_block_pass_info->SetVectorDefInfo(&v_def);
		basic_block_pass_info->SetVectorUseInfo(&v_use);
		basic_block_pass_info->SetScalarDefInfo(&s_def);
		basic_block_pass_info->SetScalarUseInfo(&s_use);
	}// basic block traversing

	//----------------- Phase 2 -------------------- //

	// Declare Live In and Live Out sets for vector and scalar registers.
	misc::Bitmap v_in(lap_function->getNumVectorRegisters());
	misc::Bitmap v_out(lap_function->getNumVectorRegisters());
	misc::Bitmap s_in(lap_function->getNumVectorRegisters());
	misc::Bitmap s_out(lap_function->getNumVectorRegisters());

	// Declare bitmaps to save the current state of every block
	misc::Bitmap v_in_current(lap_function->getNumVectorRegisters());
	misc::Bitmap v_out_current(lap_function->getNumVectorRegisters());
	misc::Bitmap s_in_current(lap_function->getNumScalarRegisters());
	misc::Bitmap s_out_current(lap_function->getNumScalarRegisters());

	// Declare bitmaps for live_in sets for the successors of current
	// blocks
	misc::Bitmap v_in_successor(lap_function->getNumVectorRegisters());
	misc::Bitmap s_in_successor(lap_function->getNumVectorRegisters());

	// Declare pointers to current basic block and its successor
	llvm2si::BasicBlock *basic_block_successor;

	// Vector that keeps track of the change in liveness of every block
	std::vector<bool> bb_change;
	bool changed = true;

	// Iterate through basic_block_queue until no basic blocks remain
	while(changed == true)
	{
		changed = false;
		for (auto basic_block_current = basic_block_queue.rbegin();
					basic_block_current != basic_block_queue.rend();
					++basic_block_current)
		{
			// Get a pointer to PassInfo obj associated with the basic block
			auto *basic_block_pass_info_current =
					getInfo<BasicBlockLivenessAnalysisPassInfo>
					(*basic_block_current);

			// Get live-in and live-out sets for vector and scalar
			// registers
			v_in  = basic_block_pass_info_current->GetVectorInInfo();
			v_out = basic_block_pass_info_current->GetVectorOutInfo();
			s_in  = basic_block_pass_info_current->GetScalarInInfo();
			s_out = basic_block_pass_info_current->GetScalarOutInfo();

			v_def = basic_block_pass_info_current->GetVectorDefInfo();
			v_use = basic_block_pass_info_current->GetVectorUseInfo();
			s_def = basic_block_pass_info_current->GetScalarDefInfo();
			s_use = basic_block_pass_info_current->GetScalarUseInfo();

			// Save current status of live-in and live-out sets for
			// vector and scalar registers
			v_in_current  = v_in;
			v_out_current = v_out;
			s_in_current  = s_in;
			s_out_current = s_out;

			// Get successors of current basic block
			for (auto &node : (*basic_block_current)->getNode()->getBackupSuccList())
			{
				// Assert the node is leaf node
				assert(node->getKind() == comm::Node::KindLeaf);

				// Get the basic block that this LeafNode corresponds to
				basic_block_successor = misc::cast<llvm2si::BasicBlock*>
						(misc::cast<comm::LeafNode*>(node)->getBasicBlock());

				// Get a pointer to PassInfo obj associated with the basic block
				auto *basic_block_pass_info_successor =
						getInfo<BasicBlockLivenessAnalysisPassInfo>
						(basic_block_successor);

				// Get live-in sets for vector and scalar registers of
				// the succeeding basic block
				v_in_successor = basic_block_pass_info_successor->GetVectorInInfo();
				s_in_successor = basic_block_pass_info_successor->GetScalarInInfo();

				v_out |= v_in_successor;
				s_out |= s_in_successor;
			}

			// Update live_in sets for vector and scalar registers
			v_in = (v_out & ~v_def) | v_use;
			s_in = (s_out & ~s_def) | s_use;

			// The following statements may be redundant. Will be removed after
			// testing a few programs
			basic_block_pass_info_current->SetVectorInInfo(&v_in);
			basic_block_pass_info_current->SetVectorOutInfo(&v_out);
			basic_block_pass_info_current->SetScalarInInfo(&s_in);
			basic_block_pass_info_current->SetScalarOutInfo(&s_out);

			// The check must continue until the live in and live out of every
			// basic block does not change anymore
			if (v_in == v_in_current && s_in == s_in_current &&
				v_out == v_out_current && s_out == s_out_current)
				bb_change.push_back(false);
			else
				bb_change.push_back(true);
		}

		// check if the live in and live out of every basic block
		// has changed or not
		while(!bb_change.empty())
		{
			changed |= bb_change.back();
			bb_change.pop_back();
		}
	}
}

void LivenessAnalysisPass::dump()
{
	// open a new file to dump the information
	std::ofstream file("LivenessInfo.txt", std::ofstream::out);

	 std::vector<llvm2si::BasicBlock *> basic_block_queue;

	 file << "--------Liveness Debug Info--------\n";
	 file << "\nTotal number of basic blocks in the function: "
			 << lap_function->getBasicBlocks()->size() << "\n";

	 // print the actual flow in the graph
	 file << "header -> uavs -> args -> body";

	 for (auto &it : *(lap_function->getBasicBlocks()))
	 	basic_block_queue.push_back(it.get());

	 // Traverse the basic blocks in the function in reverse order
	 for (auto basic_block = basic_block_queue.begin();
			basic_block != basic_block_queue.end();
			basic_block++)
	 {
	 	// Get a pointer to PassInfo obj associated with the basic block
		auto *basic_block_pass_info = getInfo<BasicBlockLivenessAnalysisPassInfo>
	 						(*basic_block);


	 	file << "\n\nBasic Block: " << (*basic_block)->getNode()->getName();
	 	file << "\nv_live_in registers: " << (basic_block_pass_info->GetVectorInInfo()).CountOnes() << "\t";

	 	// traverse every bit in the bitmap from left to right
	 	for(size_t i = 0; i < (basic_block_pass_info->GetVectorInInfo()).getSize(); i++)
	 	{
	 		 // if that bit is one, print the register id
	 		 if((basic_block_pass_info->GetVectorInInfo()).Test(i)== 1)
	 		 file << "v" << i << ", ";
	 	}

	 	file << "\nv_live_out registers: " << (basic_block_pass_info->GetVectorOutInfo()).CountOnes() << "\t";

	 	// traverse every bit in the bitmap from left to right
	 	for(size_t i = 0; i < (basic_block_pass_info->GetVectorOutInfo()).getSize(); i++)
	 	{
	 		// if that bit is one, print the register id
	 		if((basic_block_pass_info->GetVectorOutInfo()).Test(i)== 1)
	 			file << "v" << i << ", ";
	 	}

	 	file << "\ns_live_in registers: " << (basic_block_pass_info->GetScalarInInfo()).CountOnes() << "\t";

	 	// traverse every bit in the bitmap from left to right
	 	for(size_t i = 0; i < (basic_block_pass_info->GetScalarInInfo()).getSize(); i++)
	 	{
	 		// if that bit is one, print the register id
	 		if((basic_block_pass_info->GetScalarInInfo()).Test(i)== 1)
	 			file << "s" << i << ", ";
	 	}

	 	file << "\n" << "s_live_out registers: " << (basic_block_pass_info->GetScalarOutInfo()).CountOnes() << "\t";
	 	for(size_t i = 0; i < (basic_block_pass_info->GetScalarOutInfo()).getSize(); i++)
	 	{
	 		// if that bit is one, print the register id
	 		if((basic_block_pass_info->GetScalarOutInfo()).Test(i)== 1)
	 		 file << "s" << i << ", ";
	 	}
	}
}
}
