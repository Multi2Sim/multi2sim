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

#include <llvm-c/Core.h>

#include <lib/util/class.h>


/* Forward declarations */
CLASS_FORWARD_DECLARATION(Node);


/*
 * Class 'Llvm2siPhi'
 * This class represents an entry in the list of elements of a 'phi'
 * instruction in the LLVM code.
 */

CLASS_BEGIN(Llvm2siPhi, Object)
	
	/* Source node in the control tree to take the source value from. This
	 * node is extracted from the basic block argument in an entry of the
	 * 'phi' LLVM instruction. */
	Node *src_node;

	/* Value to take from the source basic block, as it appears in one of
	 * the arguments of the 'phi' LLVM instruction. */
	LLVMValueRef src_value;
	
	/* Destination value (in destination basic block) to write this value
	 * into. This object must be created by the caller, but will be freed
	 * internally. */
	struct si2bin_arg_t *dest_value;

CLASS_END(Llvm2siPhi);


void Llvm2siPhiCreate(Llvm2siPhi *self, Node *src_node,
		LLVMValueRef src_value, struct si2bin_arg_t *dest_value);

void Llvm2siPhiDestroy(Llvm2siPhi *self);


#endif

