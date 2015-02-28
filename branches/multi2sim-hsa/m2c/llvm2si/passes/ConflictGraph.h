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

#ifndef M2C_LLVM2SI_PASSES_CONFLICT_GRAPH_H
#define M2C_LLVM2SI_PASSES_CONFLICT_GRAPH_H

#include <vector>
#include <string>
#include <list>

#include <m2c/common/Pass.h>
#include <m2c/common/PassInfo.h>
#include <m2c/llvm2si/Function.h>
#include <src/lib/cpp/Bitmap.h>
#include <src/lib/cpp/Misc.h>

#include "LivenessAnalysisPass.h"

namespace llvm2si
{
class GraphNode
{
	// id of the node/symbolic register
	int id;

	// Defines the physical register the variable is assigned to
	int color = -1;

	// Number of neighbors
	unsigned int degree = 0;

	// List of adjacent nodes
	// std::vector<graphNode *> adj_node_list;
	std::vector<int> adj_node_list;

	// List of neighboring nodes that are removed. Used during graph coloring
	// std::vector<graphNode *> rem_node_list;
	std::vector<int> rem_node_list;

	public:
		// Constructor
		GraphNode (int id):
			id(id)
		{
		}

		// Function to add conflict edge between two nodes if their live ranges interfere.
		void AddNeighbor(GraphNode *adj_node);

		// Function to remove conflict between two nodes. Used during graph coloring.
		void RemoveNeighbor(GraphNode *adj_node);

		// Check if a particular node is already present in the list or not.
		// Returns true if the node is found in the list.
		bool InAdjList(int id)
		{
			return std::find(adj_node_list.begin(), adj_node_list.end(), id) != adj_node_list.end();
		}

		// Returns the number of neighbors (degree) of the node
		int GetDegree()
		{
			return this->degree;
		}

		// Return name of the variable
		int GetId()
		{
			return this->id;
		}

		// Dump the adjacent/removed node list
		void DumpAdjList();
	};

	class ConflictGraphPass : public comm::Pass
	{
		llvm2si::Function *cgp_function;

	public:

		/// constructor
		ConflictGraphPass(llvm2si::Function *function) :
			cgp_function(function)
		{
			std::cout << "I am at the CG constructor\n";
		}

		/// Return a pointer of BasicBlockLivenessAnalysisPassInfo
		template<typename ConcreteType> ConcreteType* getInfo(BasicBlock *basic_block)
		{
				std::cout << " CG id : " << getId() << "\n";
				return basic_block->getPassInfoPool()->get<ConcreteType>(getId()-1);
		}

		/// run the pass and create the conflict graph
		void run();

		/// dump the graph info
		void dump(std::ostream &os);

		~ConflictGraphPass() {}

	};

#endif
}
