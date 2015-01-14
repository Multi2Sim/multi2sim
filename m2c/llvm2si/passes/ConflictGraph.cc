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
//void graphNode::AddNeighbor(int adj_node)
{
	assert(!adj_node->InAdjList(this->id));
	assert(!InAdjList(adj_node->id));
	this->adj_node_list.push_back(adj_node->id);
	adj_node->adj_node_list.push_back(this->id);
	this->degree++;
	adj_node->degree++;
}

void GraphNode::RemoveNeighbor(GraphNode *adj_node)
// void graphNode::RemoveNeighbor(int adj_node)
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

void GraphNode::DumpAdjList()
{
	for (auto it = adj_node_list.begin(); it != adj_node_list.end(); it++)
	{
		std::cout << *it << "\n";
	}
}

void ConflictGraphPass::run()
{
	std::list <GraphNode *> conflict_graph;
	std::vector<llvm2si::BasicBlock *> basic_block_queue;
	for (auto &it : *(cgp_function->getBasicBlocks()))
		basic_block_queue.push_back(it.get());

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
					for(auto &it: conflict_graph)
					{
						if(it->GetId() == v_register->getId())
							found = true;
					}
					if(found == false)
						conflict_graph.push_back(new GraphNode(v_register->getId()));

					break;
					}
					case si2bin::Argument::TypeScalarRegister:
					{
						break;
					}

					default:
						break;
					}
				}
			}
		}

	//misc::Bitmap *LiveNow;

	for (auto basic_block = basic_block_queue.rbegin();
					basic_block != basic_block_queue.rend();
					basic_block++)
	{
		auto *basic_block_pass_info = getInfo<BasicBlockLivenessAnalysisPassInfo>(*basic_block);

		std::cout << "I am here\n";
		misc::Bitmap LiveNow = basic_block_pass_info->GetVectorInInfo();

		std::cout << (*basic_block)->getNode()->getName() << "\t";
		std::cout << LiveNow.CountOnes() << "\n";

	//	assert(LiveNow.getSize() != 0);

		for (auto &instruction : (*basic_block)->getInstructions())
		{
			SI::InstOpcode opcode = instruction->getOpcode();
			if(opcode != SI::INST_PHI || opcode != SI::INST_S_MOV_B32 || opcode != SI::INST_V_MOV_B32)
			{

			}

		}
	}
		std::cout << conflict_graph.size() << "\n";
}
}
