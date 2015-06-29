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

#include <iostream>

#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "ConflictGraph.h"
#include "LivenessAnalysisPass.h"

namespace llvm2si
{

void GraphNode::AddNeighbor(GraphNode *adj_node)
{
	assert(!adj_node->InAdjList(this->id));
	assert(!InAdjList(adj_node->id));
	this->adj_node_list.push_back(adj_node->id);
	adj_node->adj_node_list.push_back(this->id);
	this->degree++;
	adj_node->degree++;
}

void GraphNode::RemoveNeighbor(GraphNode *adj_node)
{
	auto it1 = std::find(adj_node_list.begin(), adj_node_list.end(), adj_node->id);
	auto it2 = std::find(adj_node->adj_node_list.begin(),
				adj_node->adj_node_list.end(), this->id);
	if (it1 == adj_node_list.end() || it2 == adj_node->adj_node_list.end())
	misc::panic("%s: invalid connection between conflict graph nodes",
					__FUNCTION__);

	// Remove it
	this->adj_node_list.erase(it1);
	adj_node->adj_node_list.erase(it2);
}

void GraphNode::DumpAdjList(std::ofstream *fp)
{
	for (auto it = adj_node_list.begin(); it != adj_node_list.end(); it++)
		*fp << *it << "  ";

	*fp << "\n\n";
}

// Run the Conflict Graph Pass - first for scalar registers
// and then for vector registers
void ConflictGraphPass::run()
{
	if(getPassType() == 1)
		runScalar();
	else if (getPassType() == 0)
		runVector();
}

void ConflictGraphPass::runScalar()
{
	// Build both adjacency matrix and adjacency graph for efficiency
// std::list <GraphNode *> s_adj_graph;
// std::vector<std::vector<bool> > s_adj_matrix;

	int num_sregs = cgp_function->getNumScalarRegisters();

	// Resize the scalar adjacency matrix in one dimension
	s_adj_matrix.resize(num_sregs);

	// Resize the matrix in other dimension
	for (int i = 0; i < num_sregs; i++)
		s_adj_matrix[i].resize(num_sregs);

	// Create a queue of basic blocks
	std::vector<llvm2si::BasicBlock *> basic_block_queue;

	// Push all basic blocks in the queue
	for (auto &it : *(cgp_function->getBasicBlocks()))
		basic_block_queue.push_back(it.get());

	// Create a node for every argument in the graph
	for (auto basic_block = basic_block_queue.rbegin();
				basic_block != basic_block_queue.rend();
				basic_block++)
	{
		for (auto &instruction : (*basic_block)->getInstructions())
		{
			// Get arguments from the instruction
			for (auto &argument : instruction->getArguments())
			{
				switch(argument->getType())
				{
					case si2bin::Argument::TypeScalarRegister:
					{
						si2bin::ArgScalarRegister *s_register = misc::cast<si2bin::ArgScalarRegister*>(argument.get());
						// Make sure that the node created for every variable is unique.
						// If node already exists, then found = true
						bool found = false;
						for(auto &it: s_adj_graph)
						{
							if(it->GetId() == s_register->getId())
								found = true;
						}
						if(found == false)
							s_adj_graph.push_back(new GraphNode(s_register->getId(), 1));
						break;
					}
					case si2bin::Argument::TypeScalarRegisterSeries:
					{
						si2bin::ArgScalarRegisterSeries *s_register = misc::cast<si2bin::ArgScalarRegisterSeries*>(argument.get());
						int low = s_register->getLow();
						int high = s_register->getHigh();
						// Make sure that the node created for every variable is unique.
						// Create a big node for the series and set size as the offset between
						// high and low
						bool found = false;
						for(auto &it: s_adj_graph)
						{
							if(it->GetId() == low)
								found = true;
						}
						if(found == false)
							s_adj_graph.push_back(new GraphNode(low, high - low + 1));
						break;
					}
					default:
						break;
				}
			}
		}
	}

	for (auto basic_block = basic_block_queue.rbegin();
					basic_block != basic_block_queue.rend();
					basic_block++)
	{
		auto *basic_block_pass_info = getInfo<BasicBlockLivenessAnalysisPassInfo>(*basic_block);
		std::vector<si2bin::Instruction *> instruction_queue;

		misc::Bitmap LiveNow = basic_block_pass_info->GetScalarOutInfo();
		instruction_queue.clear();

		// Create a queue to traverse instructions in reverse direction later
		for (auto &instruction : (*basic_block)->getInstructions())
			instruction_queue.push_back(instruction.get());

		std::vector<int> s_dest_args;
		std::vector<int> s_src_args;

		si2bin::ArgScalarRegister *scalar_register;

		for (auto instruction = instruction_queue.rbegin();
							instruction != instruction_queue.rend();
							instruction++)
		{
			SI::Instruction::Opcode opcode = (*instruction)->getOpcode();
			// Phi and Mov instructions don't introduce interference
			if(opcode != SI::Instruction::Opcode_PHI || opcode != SI::Instruction::Opcode_S_MOV_B32 || opcode != SI::Instruction::Opcode_S_MOV_B64)
			{
				// Get arguments from the instruction
				for (auto &argument : (*instruction)->getArguments())
				{
					switch (argument->getToken()->getDirection())
					{
						case si2bin::TokenDirectionDst:
						{
							switch (argument->getType())
							{
								case si2bin::Argument::TypeScalarRegister:
								{
									scalar_register = misc::cast<si2bin::ArgScalarRegister*>(argument.get());
									s_dest_args.push_back(scalar_register->getId());
									break;
								}
								case si2bin::Argument::TypeScalarRegisterSeries:
								{
									si2bin::ArgScalarRegisterSeries *s_register = misc::cast<si2bin::ArgScalarRegisterSeries*>(argument.get());
									// get the register range
									int low = s_register->getLow();
									int high = s_register->getHigh();
									while (low <= high)
									{
										s_dest_args.push_back(low);
										low++;
									}
									break;
								}
								default:
									break;
							}
							break;
						}
						case si2bin::TokenDirectionSrc:
						{
							switch (argument->getType())
							{
								case si2bin::Argument::TypeScalarRegister:
								{
									scalar_register = misc::cast<si2bin::ArgScalarRegister*>(argument.get());
									s_src_args.push_back(scalar_register->getId());
									break;
								}
								case si2bin::Argument::TypeScalarRegisterSeries:
								{
									si2bin::ArgScalarRegisterSeries *s_register = misc::cast<si2bin::ArgScalarRegisterSeries*>(argument.get());
									// get the register range
									int low = s_register->getLow();
									int high = s_register->getHigh();
									while (low <= high)
									{
										s_src_args.push_back(low);
										low++;
									}
									break;
								}
								default:
									break;
							}
							break;
						}
						default:
							break;
					}
				}
			}

			// Add conflict between interfering LiveRanges in the matrix
			// All 'defined' LiveRanges interfere with members of the LiveNow
			for(unsigned int i = 0; i <  s_dest_args.size(); i++)
			{
				unsigned int row = s_dest_args[i]; //->getId();
				for(unsigned int j = 0; j < LiveNow.getSize(); j++)
				{
					// int col = LiveNow[j]; //->getId();
					// Build a lower triangular matrix
					// Diagonal must be zero
					if(LiveNow[j] == true)
					{
						if(row == j)
							s_adj_matrix[row][j] = false;
						else if(row > j)
							s_adj_matrix[row][j] = true;
						else
							s_adj_matrix[j][row] = true;
					}
					// If the src arg is not in LiveNow, means this is the last use of that argument
					// set the LiveNow of this arg to true as its liveness has just started
				//	else
				//		LiveNow.Set(col, true);
				}
				// Remove the destination arg from live now as it's no longer going to be live
			//	LiveNow.Reset(row);
			}

			for (unsigned int i = 0; i < s_dest_args.size(); i++)
				LiveNow.Reset(s_dest_args[i]);

			for (unsigned int i = 0; i < s_src_args.size(); i++)
				LiveNow.Set(s_src_args[i], true);
		} // instruction traversing
	}

	// phase 2: Add edges to conflict graph
	for(int i = 0; i < num_sregs; i++)
	{
		for(int j = 0; j < num_sregs; j++)
		{
			if(s_adj_matrix[i][j] == true)
			{
				for(auto &it1: s_adj_graph)
				{
					if(it1->GetId() == i)
					{
						for(auto &it2: s_adj_graph)
						{
							if(it2->GetId() == j)
								it1->AddNeighbor(it2);
						}
					}
				}
			}
		}
	}
}

void ConflictGraphPass::runVector()
{
	// Build both adjacency matrix and adjacency graph for efficiency
//	std::list <GraphNode *> v_adj_graph;
//	std::vector<std::vector<bool> > v_adj_matrix;

	int num_vregs = cgp_function->getNumVectorRegisters();

	// Resize the vector adjacency matrix in both dimensions
	v_adj_matrix.resize(num_vregs);

	for (int i = 0; i < num_vregs; i++)
		v_adj_matrix[i].resize(num_vregs);

	// Create a queue of basic blocks
	std::vector<llvm2si::BasicBlock *> basic_block_queue;

	// Push all basic blocks in the queue
	for (auto &it : *(cgp_function->getBasicBlocks()))
		basic_block_queue.push_back(it.get());

	// Create a node for every argument in the graph
	for (auto basic_block = basic_block_queue.rbegin();
				basic_block != basic_block_queue.rend();
				basic_block++)
	{
		for (auto &instruction : (*basic_block)->getInstructions())
		{
			// Get arguments from the instruction
			for (auto &argument : instruction->getArguments())
			{
				switch(argument->getType())
				{
					case si2bin::Argument::TypeVectorRegister:
					{
						si2bin::ArgVectorRegister *v_register = misc::cast<si2bin::ArgVectorRegister*>(argument.get());

						// Make sure that the node created for every variable is unique.
						// If node already exists, then found = true
						bool found = false;
						for(auto &it: v_adj_graph)
						{
							if(it->GetId() == v_register->getId())
								found = true;
						}
						if(found == false)
							v_adj_graph.push_back(new GraphNode(v_register->getId(), 1));
						break;
					}
					case si2bin::Argument::TypeVectorRegisterSeries:
					{
						si2bin::ArgVectorRegisterSeries *v_register = misc::cast<si2bin::ArgVectorRegisterSeries*>(argument.get());
						int low = v_register->getLow();
						int high = v_register->getHigh();
						// Make sure that the node created for every variable is unique.
						// If node already exists, then found = true
						//	while (low <= high)
						//{
							bool found = false;
							for(auto &it: v_adj_graph)
							{
								if(it->GetId() == low)
									found = true;
							}
							if(found == false)
								v_adj_graph.push_back(new GraphNode(low, high - low + 1));
						//	low++;
					//	}
						break;
					}
					default:
						break;
				}
			}
		}
	}

	for (auto basic_block = basic_block_queue.rbegin();
					basic_block != basic_block_queue.rend();
					basic_block++)
	{
		auto *basic_block_pass_info = getInfo<BasicBlockLivenessAnalysisPassInfo>(*basic_block);
		std::vector<si2bin::Instruction *> instruction_queue;

		misc::Bitmap LiveNow = basic_block_pass_info->GetVectorOutInfo();
		instruction_queue.clear();

		// Create a queue to traverse instructions in reverse direction later
		for (auto &instruction : (*basic_block)->getInstructions())
				instruction_queue.push_back(instruction.get());

		std::vector<int> v_dest_args;
		std::vector<int> v_src_args;
		si2bin::ArgVectorRegister *vector_register;

		for (auto instruction = instruction_queue.rbegin();
							instruction != instruction_queue.rend();
							instruction++)
		{
			SI::Instruction::Opcode opcode = (*instruction)->getOpcode();
			// Phi and Mov instructions don't introduce interference
			if(opcode != SI::Instruction::Opcode_PHI || opcode != SI::Instruction::Opcode_V_MOV_B32)
			{
				// Get arguments from the instruction
				for (auto &argument : (*instruction)->getArguments())
				{
					switch (argument->getToken()->getDirection())
					{
						case si2bin::TokenDirectionDst:
						{
							switch (argument->getType())
							{
								case si2bin::Argument::TypeVectorRegister:
								{
									vector_register = misc::cast<si2bin::ArgVectorRegister*>(argument.get());
									v_dest_args.push_back(vector_register->getId());
									break;
								}
								case si2bin::Argument::TypeVectorRegisterSeries:
								{
									si2bin::ArgVectorRegisterSeries *v_register = misc::cast<si2bin::ArgVectorRegisterSeries*>(argument.get());
									// get the register range
									int low = v_register->getLow();
									int high = v_register->getHigh();
									while (low <= high)
									{
										v_dest_args.push_back(low);
										low++;
									}
								break;
								}
								default:
									break;
							}
							break;
						}
						case si2bin::TokenDirectionSrc:
						{
							switch (argument->getType())
							{
								case si2bin::Argument::TypeVectorRegister:
								{
									vector_register = misc::cast<si2bin::ArgVectorRegister*>(argument.get());
									v_src_args.push_back(vector_register->getId());
									break;
								}
								case si2bin::Argument::TypeVectorRegisterSeries:
								{
									si2bin::ArgVectorRegisterSeries *v_register = misc::cast<si2bin::ArgVectorRegisterSeries*>(argument.get());
									// get the register range
									int low = v_register->getLow();
									int high = v_register->getHigh();
									while (low <= high)
									{
										v_src_args.push_back(low);
										low++;
									}
									break;
								}
								default:
									break;
							}
							break;
						}
						default:
							break;
					}
				}
			}

			for(unsigned int i = 0; i <  v_dest_args.size(); i++)
			{
				unsigned int row = v_dest_args[i];
				for(unsigned int j = 0; j < LiveNow.getSize(); j++)
				{	// Build a lower triangular matrix
					// Diagonal must be zero
					if(LiveNow[j] == true)
					{
						if(row == j)
							v_adj_matrix[row][j] = false;
						else if(row > j)
							v_adj_matrix[row][j] = true;
						else
							v_adj_matrix[j][row] = true;
					}
				}
			}

			for (unsigned int i = 0; i < v_dest_args.size(); i++)
				LiveNow.Reset(v_dest_args[i]);

			for (unsigned int i = 0; i < v_src_args.size(); i++)
				LiveNow.Set(v_src_args[i], true);

		}
	}

	// phase 2: Add edges to conflict graph
	for(int i = 0; i < num_vregs; i++)
	{
		for(int j = 0; j < num_vregs; j++)
		{
			if(v_adj_matrix[i][j] == true)
			{
				for(auto &it1: v_adj_graph)
				{
					if(it1->GetId() == i)
					{
						for(auto &it2: v_adj_graph)
						{
							if(it2->GetId() == j)
								it1->AddNeighbor(it2);
						}
					}
				}
			}
		}
	}
}

void ConflictGraphPass::dump()
{
	// open a new file to dump the information
	std::ofstream file("ConflictGraphInfo.txt", std::ofstream::out);

	file << "*****Conflict Graph Debug Info*****\n\n";

	file << "-----Scalar Interference graph-----\n";
	for (auto &it: s_adj_graph)
	{
		file << "Id: s" << it->GetId() << ",\t";
		file << "Size: " << it->GetNodeSize() << "\t";
		file << "Degree: " << it->GetDegree() << "\n";
		file << "Adjacent Nodes: s";
		it->DumpAdjList(&file);
	}

	file << "----- Vector Interference graph-----\n";
	for (auto &it: v_adj_graph)
	{
		file << "Id: v" << it->GetId() << ",\t";
		file << "Degree: " << it->GetDegree() << "\n";
		file << "Adjacent Nodes: v";
		it->DumpAdjList(&file);
	}
}
} //namespace llvm2si



