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

namespace llvm2si
{
	class graphNode
	{
		/// Name of the node
		std::string name;

		/// Defines the physical register the variable is assigned to
		int color = -1;

		/// Number of neighbors
		unsigned int degree = 0;

		/// List of adjacent nodes
		std::list<graphNode *> adj_node_list;

		/// List of neighboring nodes that are removed. Used during graph coloring
		std::list<graphNode *> rem_node_list;

	public:
		/// Constructor
		graphNode(const std::string &name) :
			name(name)
		{
		}

		/// Function to add conflict edge between two nodes if their live ranges interfere.
		void AddConflict(graphNode *Node);

		/// Function to remove conflict between two nodes. Used during graph coloring.
		void RemoveConflict(graphNode *Node);

		/// Check if a particular node is already present in the list or not. Returns true if the node is found in the list.
		bool InList(std::list<Node *> &list);

		/// Dump the adjacent/removed node list
		void DumpList(std::list<Node *> &list);
	};

#endif
}
