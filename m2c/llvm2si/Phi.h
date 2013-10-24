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

#ifndef M2C_LLVM2SI_PHI_H
#define M2C_LLVM2SI_PHI_H

#include <llvm/Value.h>
#include <m2c/common/Node.h>
#include <m2c/si2bin/Arg.h>


namespace llvm2si
{

/*
 * This class represents an entry in the list of elements of a 'phi'
 * instruction in the LLVM code.
 */

class Phi
{
	/* Source node in the control tree to take the source value from. This
	 * node is extracted from the basic block argument in an entry of the
	 * 'phi' LLVM instruction. */
	Common::LeafNode *src_node;

	/* Value to take from the source basic block, as it appears in one of
	 * the arguments of the 'phi' LLVM instruction. */
	llvm::Value *src_value;
	
	/* Destination value (in destination basic block) to write this value
	 * into. This object must be created by the caller, but will be freed
	 * internally. */
	si2bin::Arg *dest_value;

public:

	/* Constructor */
	Phi(Common::LeafNode *src_node, llvm::Value *src_value,
			si2bin::Arg *dest_value) {
		this->src_node = src_node;
		this->src_value = src_value;
		this->dest_value = dest_value;
	}

	/* Getters */
	Common::LeafNode *GetSrcNode() { return src_node; }
	llvm::Value *GetSrcValue() { return src_value; }
	si2bin::Arg *GetDestValue() { return dest_value; }
};


}  /* namespace llvm2si */

#endif

