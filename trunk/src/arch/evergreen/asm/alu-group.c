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

#include <lib/class/class.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>

#include "alu-group.h"


/*
 * Class 'EvgALUGroup'
 */

void EvgALUGroupCreate(EvgALUGroup *self, EvgAsm *as)
{
	int i;

	/* Initialize */
	self->as = as;

	/* Create instructions */
	for (i = 0; i < EVG_ALU_GROUP_SIZE; i++)
		new_static(&self->inst[i], EvgInst, as);
}


void EvgALUGroupDestroy(EvgALUGroup *self)
{
	int i;

	/* Destroy instructions */
	for (i = 0; i < EVG_ALU_GROUP_SIZE; i++)
		delete_static(&self->inst[i]);
}


void EvgALUGroupClear(EvgALUGroup *self)
{
	int i;

	self->id = 0;
	self->inst_count = 0;
	self->literal_count = 0;
	for (i = 0; i < EVG_ALU_GROUP_MAX_LITERALS; i++)
		self->literal[i].as_uint = 0;
	for (i = 0; i < EVG_ALU_GROUP_SIZE; i++)
		EvgInstClear(&self->inst[i]);
}


void *EvgALUGroupDecode(EvgALUGroup *self, void *buf, int group_id)
{
	int dest_chan;
	int chan;
	int last;
	int i;

	EvgInst *inst;
	EvgInstAlu alu;

	int alu_busy[EvgInstAluCount];

	/* Reset group */
	EvgALUGroupClear(self);
	self->id = group_id;
	for (i = 0; i < EvgInstAluCount; i++)
		alu_busy[i] = 0;

	/* Decode instructions */
	do
	{
		/* Decode instruction */
		assert(self->inst_count < 5);
		inst = &self->inst[self->inst_count];
		buf = EvgInstDecodeALU(inst, buf);
		inst->alu_group = self;
		last = inst->words[0].alu_word0.last;

		/* Count associated literals.
		 * There is at least one literal slot if there is any destination element CHAN_
		 * There are two literal slots if there is any destination element CHAN_Z or CHAN_W. */
		if (inst->words[0].alu_word0.src0_sel == 253)  /* ALU_SRC_LITERAL */
		{
			chan = inst->words[0].alu_word0.src0_chan;
			self->literal_count = MAX(self->literal_count, (chan + 2) / 2);
		}
		if (inst->words[0].alu_word0.src1_sel == 253)
		{
			chan = inst->words[0].alu_word0.src1_chan;
			self->literal_count = MAX(self->literal_count, (chan + 2) / 2);
		}
		if (inst->info->fmt[1] == EvgInstFormatAluWord1Op3 &&
				inst->words[1].alu_word1_op3.src2_sel == 253)
		{
			chan = inst->words[1].alu_word1_op3.src2_chan;
			self->literal_count = MAX(self->literal_count, (chan + 2) / 2);
		}

		/* Allocate instruction to ALU
		 * Page 4-5 Evergreen Manual - FIXME */

		/* Initially, set ALU as indicated by the destination operand. For both OP2 and OP3 formats, field
		 * 'dest_chan' is located at the same bit position. */
		dest_chan = inst->words[1].alu_word1_op2.dst_chan;
		alu = inst->info->flags & EvgInstFlagTransOnly ? EvgInstAluTrans : dest_chan;
		if (alu_busy[alu])
			alu = EvgInstAluTrans;
		if (alu_busy[alu])
			fatal("group_id=%d, inst_id=%d: cannot allocate ALU", group_id, self->inst_count);
		alu_busy[alu] = 1;
		inst->alu = alu;

		/* One more instruction */
		self->inst_count++;

	} while (!last);


	/* Decode literals */
	memcpy(self->literal, buf, self->literal_count * 8);
	buf += self->literal_count * 8;

	/* Return new buffer pointer */
	return buf;
}


void EvgALUGroupDump(EvgALUGroup *self, int shift, FILE *f)
{
	EvgInst *inst;
	int i;

	for (i = 0; i < self->inst_count; i++)
	{
		inst = &self->inst[i];
		EvgInstSlotDump(inst, i ? -1 : self->id, shift, inst->alu, f);
	}
}


void EvgALUGroupDumpBuf(EvgALUGroup *self, char *buf, int size)
{
	EvgInst *inst;

	char str[MAX_LONG_STRING_SIZE];
	char str_trimmed[MAX_LONG_STRING_SIZE];

	char *space;

	int i;

	/* Add individual VLIW bundles */
	space = "";
	for (i = 0; i < self->inst_count; i++)
	{
		/* Get instruction dump */
		inst = &self->inst[i];
		EvgInstDumpBuf(inst, -1, 0, str, sizeof str);
		str_single_spaces(str_trimmed, sizeof str_trimmed, str);

		/* Copy to output buffer */
		str_printf(&buf, &size, "%s%s=\"%s\"", space,
			str_map_value(&evg_inst_alu_map, inst->alu), str_trimmed);
		space = " ";
	}
}
