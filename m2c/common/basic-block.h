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

#include <stdio.h>

#include <lib/util/class.h>

/* Forward declarations */
CLASS_FORWARD_DECLARATION(Node);


/*
 * Class 'BasicBlock'
 */

CLASS_BEGIN(BasicBlock, Object)

	/* Node associated in control tree */
	Node *node;

CLASS_END(BasicBlock)


void BasicBlockCreate(BasicBlock *self, Node *node);
void BasicBlockDestroy(BasicBlock *self);


#endif
