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

#ifndef EVERGREEN_ASM_ALU_GROUP_H
#define EVERGREEN_ASM_ALU_GROUP_H

#include "inst.h"



/*
 * Class 'EvgALUGroup'
 */

#define EVG_ALU_GROUP_SIZE  5
#define EVG_ALU_GROUP_MAX_LITERALS  4

CLASS_BEGIN(EvgALUGroup, Object)

	/* Disassembler */
	EvgAsm *as;

	int id;

	/* Number of instruction slots (max. 5) */
	int inst_count;

	/* Number of literal constant slots (max. 2) */
	int literal_count;

	/* Instructions */
	EvgInst inst[EVG_ALU_GROUP_SIZE];

	/* Literals for X, Y, Z, and W elements */
	union
	{
		unsigned int as_uint;
		float as_float;
	} literal[EVG_ALU_GROUP_MAX_LITERALS];

CLASS_END(EvgALUGroup)


void EvgALUGroupCreate(EvgALUGroup *self, EvgAsm *as);
void EvgALUGroupDestroy(EvgALUGroup *self);

void EvgALUGroupClear(EvgALUGroup *self);

/* Decode one ALU group from 'buf', and assign it the ALU group ID given in
 * 'group_id'. The function returns the next position in the buffer after
 * having read the ALU group. */
void *EvgALUGroupDecode(EvgALUGroup *self, void *buf, int group_id);

void EvgALUGroupDump(EvgALUGroup *self, int shift, FILE *f);
void EvgALUGroupDumpBuf(EvgALUGroup *self, char *buf, int size);

#endif

