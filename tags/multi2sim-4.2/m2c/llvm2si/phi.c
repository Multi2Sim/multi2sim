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

#include <lib/mhandle/mhandle.h>

#include "phi.h"



/*
 * Class 'Llvm2siPhi'
 */

CLASS_IMPLEMENTATION(Llvm2siPhi);

void Llvm2siPhiCreate(Llvm2siPhi *self, Node *src_node,
		LLVMValueRef src_value, struct si2bin_arg_t *dest_value)
{
	self->src_node = src_node;
	self->src_value = src_value;
	self->dest_value = dest_value;
}


void Llvm2siPhiDestroy(Llvm2siPhi *self)
{
}

