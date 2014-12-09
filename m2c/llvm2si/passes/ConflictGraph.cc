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

	void graphNode::AddConflict(graphNode *adj_node)
	{
		assert(!adj_node->InList(this->adj_node_list));
		assert(!InList(adj_node->adj_node_list));
		this->adj_node_list.push_back(adj_node);
		adj_node->adj_node_list.push_back(this);
		this->degree++;
		adj_node->degree++;
	}

	void graphNode::RemoveConflict(graphNode *adj_node)
	{
		auto it1 = std::find(adj_node_list.begin(), adj_node_list.end(), adj_node);
		auto it2 = std::find(adj_node->adj_node_list.begin(),
					adj_node->adj_node_list.end(), this);
		if (it1 == adj_node_list.end() || it2 == adj_node->adj_node_list.end())
		misc::panic("%s: invalid connection between conflict graph nodes",
						__FUNCTION__);

		/// Remove it
		this->adj_node_list.erase(it1);
		adj_node->adj_node_list.erase(it2);
	}

	bool graphNode::InList(std::list<Node *> &list)
	{
		return std::find(list.begin(), list.end(), this) != list.end();
	}

}

