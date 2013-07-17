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

#include "for-blocks.h"

struct cl2llvm_for_blocks_t *cl2llvm_for_blocks_create(void)
{
	struct cl2llvm_for_blocks_t *for_blocks;

	for_blocks = xcalloc(1, sizeof(struct cl2llvm_for_blocks_t));

	return for_blocks;
}

void cl2llvm_for_blocks_free(struct cl2llvm_for_blocks_t *for_blocks)
{
	free(for_blocks);
}

