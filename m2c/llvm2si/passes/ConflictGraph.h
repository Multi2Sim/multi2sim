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
#include <iostream>

#include "LivenessAnalysisPass.h"

namespace llvm2si
{
// Class that defines the attributes of a node
class GraphNode
{
	// id of the node/symbolic register
	// if it is a large node, then stores the id of lowest register
	int id;

	// Defines the physical register the variable is assigned to
	int color = -1;

	// Number of neighbors
	unsigned int degree = 0;

	// Size of the node
	int size = 0;

	// List of adjacent nodes
	std::vector<int> adj_node_list;

	// List of neighboring nodes that are removed. Used during graph coloring
	std::vector<int> rem_node_list;

	public:
		// Constructor
		GraphNode (int id, int size):
			id(id), size(size)
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

		// Return the size of the node
		int GetNodeSize()
		{
			return this->size;
		}

		// Dump the adjacent/removed node list
		void DumpAdjList(std::ofstream *f);
	};

// Class that defines the information collected at Function level
/*class FunctionConflictGraphPassInfo : public comm::FunctionPassInfo
{
public:
	// friend class ConflictGraphPass;
	std::list <GraphNode *> *vector_adj_graph;
	std::list <GraphNode *> *scalar_adj_graph;

	std::vector<std::vector<bool> > *vector_adj_matrix;
	std::vector<std::vector<bool> > *scalar_adj_matrix;

	void SetVectorGraph(std::list <GraphNode *> *v_adj_graph)
	{
		vector_adj_graph->resize(v_adj_graph->size());
		*vector_adj_graph = *v_adj_graph;
	}

	std::list <GraphNode *>* GetVectorGraph()
	{
		return vector_adj_graph;
	}

	void SetVectorAdjMatrix(std::vector<std::vector<bool> > *v_adj_matrix)
	{
		vector_adj_matrix->resize(v_adj_matrix->size());
		*vector_adj_matrix = *v_adj_matrix;
	}

	std::vector<std::vector<bool> > GetVectorAdjGraph()
	{
		return *vector_adj_matrix;
	}
}; */

class ConflictGraphPass : public comm::Pass
{
	llvm2si::Function *cgp_function;
	std::list <GraphNode *> v_adj_graph;
	std::vector<std::vector<bool> > v_adj_matrix;

	std::list <GraphNode *> s_adj_graph;
	std::vector<std::vector<bool> > s_adj_matrix;

public:
		/// constructor
		ConflictGraphPass(llvm2si::Function *function) :
			cgp_function(function)
		{
			// std::cout << "I am at the CG constructor\n";
		}

		/// Return a pointer of BasicBlockLivenessAnalysisPassInfo
		template<typename ConcreteType> ConcreteType* getInfo(BasicBlock *basic_block)
		{
				// std::cout << " CG id : " << getId() << "\n";
				return basic_block->getPassInfoPool()->get<ConcreteType>(getId()-1);
		}

		/// Return a pointer of BasicBlockLivenessAnalysisPassInfo
		template<typename ConcreteType> ConcreteType* getInfo(Function *func)
		{
			// std::cout << " CG id : " << getId() << "\n";
			return func->getPassInfoPool()->get<ConcreteType>(getId());
		}
		/// run the pass and create the conflict graph
		void run();

		void runScalar();

		void runVector();

		/// dump the graph info
		void dump();

		~ConflictGraphPass() {}
	};

#endif
} // namespace llvm2si
