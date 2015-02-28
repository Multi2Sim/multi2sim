/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef M2C_COMMON_BASIC_BLOCK_H
#define M2C_COMMON_BASIC_BLOCK_H

#include "PassInfo.h"
#include "PassInfoPool.h"

namespace comm
{

// Forward declarations
class LeafNode;


/// This class is used as a base class representing a basic block of any
/// architecture. Each back-end should derive its own basic block class.
class BasicBlock
{
	// PassInfoPool containing the info per pass for this BasicBlock.
	PassInfoPool<BasicBlockPassInfo> pass_info_pool;

	// Node associated in control tree
	LeafNode *node;

public:

	/// Constructor
	///
	/// \param node
	///	Leaf node associated with the basic block
	BasicBlock(LeafNode *node);

	/// Getter for PassInfoPool
	PassInfoPool<BasicBlockPassInfo> *getPassInfoPool()
	{
		return &pass_info_pool;
	}

	/// Virtual destructor
	virtual ~BasicBlock() { }

	/// Return associated node in the control tree
	LeafNode *getNode() const { return node; }
};


}  // namespace comm

#endif

