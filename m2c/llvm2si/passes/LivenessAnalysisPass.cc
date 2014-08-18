/*
 *  Multi2Sim
 *  Copyright (C) 2014  Nathan Lilienthal (nathan@nixpulvis.com)
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

void LivenessAnalysisPass::run()
{
	// Declare pointers to vector def, use, live-out, live-in sets
	misc::Bitmap *v_def_ptr, *v_use_ptr, *v_out_ptr, *v_in_ptr;

	// Declare pointers to scalar def, use, live-out, live-in sets
	misc::Bitmap *s_def_ptr, *s_use_ptr, *s_out_ptr, *s_in_ptr;

	// Fill a queue of basic blocks in the function
	// TODO: Is this the correct order? It depends on the implementation of
	// getBasicBlocks()
	std::vector<llvm2si::BasicBlock *> basic_block_queue;
	for (auto &it : *function->getBasicBlocks())
		basic_block_queue.push_back(it.get());

	// Traverse the basic blocks in the function in reverse order
	for (auto basic_block = basic_block_queue.rbegin();
			basic_block != basic_block_queue.rend();
			basic_block++)
	{
		// Get a pointer to PassInfo obj associated with the basic block
		auto *basic_block_pass_info =
				getInfo<BasicBlockLivenessAnalysisPassInfo>
				(*basic_block);

		// Assigns blank bitmaps of size num_vregs to each of the
		// basic block's bitmap fields
		v_def_ptr = basic_block_pass_info->resetVectorDefSet(
				(*basic_block)->getId(),
				function->getNumVectorRegisters());
		v_use_ptr = basic_block_pass_info->resetVectorUseSet(
				(*basic_block)->getId(),
				function->getNumVectorRegisters());

		// Assigns blank bitmaps of size num_sregs to each of the
		// basic block's bitmap fields
		s_def_ptr = basic_block_pass_info->resetScalarDefSet(
				(*basic_block)->getId(),
				function->getNumScalarRegisters());
		s_use_ptr = basic_block_pass_info->resetScalarUseSet(
				(*basic_block)->getId(),
				function->getNumScalarRegisters());

		si2bin::ArgVectorRegister *vector_register;
		si2bin::ArgScalarRegister *scalar_register;
		// Read each line of basic block
		for (auto &instruction : (*basic_block)->getInstructions())
		{
			// Get each argument in the line
			for (auto &argument : instruction->getArguments())
			{
				// Determine register direction
				switch (argument->getToken()->getDirection())
				{
					case si2bin::TokenDirectionDst:
					{
						switch (argument->getType())
						{
							// Determine register type: scalar or vector
							case si2bin::Argument::TypeVectorRegister:

								// Cast the argument to vector register
								vector_register = misc::cast<si2bin::ArgVectorRegister*>
										(argument.get());
								// Set true for destination
								// register in def set
								v_def_ptr->Set(vector_register->getId(), true);
								break;

							case si2bin::Argument::TypeScalarRegister:

								// Cast argument to scalar register
								scalar_register = misc::cast<si2bin::ArgScalarRegister*>
										(argument.get());
								s_def_ptr->Set(scalar_register->getId(), true);
								break;

							default:
								break;
						}
					}
					case si2bin::TokenDirectionSrc:
					{
						switch (argument->getType())
						{
							case si2bin::Argument::TypeVectorRegister:

								// Cast the argument to vector register
								vector_register = misc::cast<si2bin::ArgVectorRegister*>
												(argument.get());
								// Set true for source register
								// in use set, and false in def
								// set
								v_use_ptr->Set(vector_register->getId(),true);
								v_def_ptr->Set(vector_register->getId(),false);
								break;

							case si2bin::Argument::TypeScalarRegister:

								// Cast argument to scalar register
								scalar_register = misc::cast<si2bin::ArgScalarRegister*>
												(argument.get());
								// Set true for source register
								// in use set, and false in def
								// set
								s_use_ptr->Set(scalar_register->getId(), true);
								s_def_ptr->Set(scalar_register->getId(), false);
								break;

							default:
								break;
						} // end switch
					} // end source case
					default:
						break;
				} // end register direction
			} // end arguments
		} //end basic block
	}

	// Declare pointers to current basic block and its successor
	llvm2si::BasicBlock *basic_block_current, *basic_block_successor;

	// Declare pointers to live_out and live_in sets for current vector
	misc::Bitmap v_in_current(function->getNumVectorRegisters());
	misc::Bitmap v_out_current(function->getNumVectorRegisters());

	// Declare pointers to live_out and live_in sets for current scalar
	misc::Bitmap s_in_current(function->getNumScalarRegisters());
	misc::Bitmap s_out_current(function->getNumScalarRegisters());

	// Declare pointers to live_in sets for the successors of current
	// vector and scalar registers
	misc::Bitmap v_in_successor(function->getNumVectorRegisters());
	misc::Bitmap s_in_successor(function->getNumScalarRegisters());

	// Iterates through basic_block_queue until no basic blocks remain
	while (basic_block_queue.size() != 0)
	{
		basic_block_current = basic_block_queue.back();
		basic_block_queue.pop_back();

		// Get a pointer to PassInfo obj associated with the basic block
		auto *basic_block_pass_info_current =
				getInfo<BasicBlockLivenessAnalysisPassInfo>
				(basic_block_current);

		// Get live-in and live-out sets for vector and scalar
		// registers
		v_in_ptr  = basic_block_pass_info_current->getVectorInSet(
					basic_block_current->getId());
		v_out_ptr = basic_block_pass_info_current->getVectorOutSet(
					basic_block_current->getId());
		s_in_ptr  = basic_block_pass_info_current->getScalarInSet(
					basic_block_current->getId());
		s_out_ptr = basic_block_pass_info_current->getScalarOutSet(
					basic_block_current->getId());

		// Clones live-in and live-out sets for vector and scalar
		// registers to save the current status
		v_in_current   = *v_in_ptr;
		v_out_current  = *v_out_ptr;
		s_in_current   = *s_in_ptr;
		s_out_current  = *s_out_ptr;

		// Get succesors of current basic block
		for (auto &node : basic_block_current->getNode()->getSuccList())
		{
			// Assert the node is leaf node
			assert(node->getKind() == comm::Node::KindLeaf);

			// Get the basic block that this LeafNode corresponds to
			basic_block_successor = misc::cast<llvm2si::BasicBlock*>
					(misc::cast<comm::LeafNode*>(node)
					->getBasicBlock());

			// Get a pointer to PassInfo obj associated with the basic block
			auto *basic_block_pass_info_successor =
					getInfo<BasicBlockLivenessAnalysisPassInfo>
					(basic_block_successor);

			// Get live-in sets for vector and scalar registers of
			// the succeeding basic block
			v_in_successor = *basic_block_pass_info_successor
					->getVectorInSet(basic_block_successor->getId());
			s_in_successor = *basic_block_pass_info_successor
					->getScalarInSet(basic_block_successor->getId());
			*v_out_ptr |= v_in_successor;
			*s_out_ptr |= s_in_successor;
		}

		// Update live_in sets for vector and scalar registers
		*v_in_ptr = (*v_out_ptr - *v_def_ptr) & *v_use_ptr;
		*s_in_ptr = (*s_out_ptr - *s_def_ptr) & *s_use_ptr;

		// The register liveness in the basic block has changed
		if (*v_out_ptr == v_out_current
				&& *v_in_ptr  == v_in_current
				&& *s_out_ptr == s_out_current
				&& *s_in_ptr  == s_in_current)
			basic_block_queue.insert(basic_block_queue.begin(),
					basic_block_current);

	} // While loop to go through basic_block_queue
}

}  // namespace llvm2si

