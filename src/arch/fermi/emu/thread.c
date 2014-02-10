/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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


#include "emu.h"
#include "grid.h"
#include "thread.h"
#include "thread-block.h"
#include "warp.h"


enum
{
	SR_LANEID = 0,
	SR_CLOCK = 1,
	SR_VIRTCFG = 2,
	SR_VIRTID = 3,
	SR_PM0 = 4,
	SR_PM1 = 5,
	SR_PM2 = 6,
	SR_PM3 = 7,
	SR_PM4 = 8,
	SR_PM5 = 9,
	SR_PM6 = 10,
	SR_PM7 = 11,
	SR_PRIM_TYPE = 16,
	SR_INVOCATION_ID = 17,
	SR_Y_DIRECTION = 18,
	SR_THREAD_KILL = 19,
	SR_SHADER_TYPE = 20,
	SR_MACHINE_ID_0 = 24,
	SR_MACHINE_ID_1 = 25,
	SR_MACHINE_ID_2 = 26,
	SR_MACHINE_ID_3 = 27,
	SR_AFFINITY = 28,
	SR_TID = 32,
	SR_TID_X = 33,
	SR_TID_Y = 34,
	SR_TID_Z = 35,
	SR_CTA_PARAM = 36,
	SR_CTAID_X = 37,
	SR_CTAID_Y = 38,
	SR_CTAID_Z = 39,
	SR_NTID = 40,
	SR_NTID_X = 41,
	SR_NTID_Y = 42,
	SR_NTID_Z = 43,
	SR_GRIDPARAM = 44,
	SR_NCTAID_X = 45,
	SR_NCTAID_Y = 46,
	SR_NCTAID_Z = 47,
	SR_SWINLO = 48,
	SR_SWINSZ = 49,
	SR_SMEMSZ = 50,
	SR_SMEMBANKS = 51,
	SR_LWINLO = 52,
	SR_LWINSZ = 53,
	SR_LMEMLOSZ = 54,
	SR_LMEMHIOFF = 55,
	SR_EQMASK = 56,
	SR_LTMASK = 57,
	SR_LEMASK = 58,
	SR_GTMASK = 59,
	SR_GEMASK = 60,
	SR_GLOBALERRORSTATUS = 64,
	SR_WARPERRORSTATUS = 66,
	SR_WARPERRORSTATUSCLEAR = 67,
	SR_CLOCKLO = 80,
	SR_CLOCKHI = 81
};

/*
 * Public Functions
 */

void FrmThreadCreate(FrmThread *self, int id, FrmWarp *warp)
{
	int i;

	/* Initialization */
	self->id = id + warp->thread_block->id * warp->thread_block->thread_count;
	self->id_in_warp = id % frm_emu_warp_size;
	self->warp = warp;
	self->thread_block = warp->thread_block;
	self->grid = warp->thread_block->grid;

	/* General purpose registers */
	for (i = 0; i < 64; ++i)
		self->gpr[i].u32 = 0;

	/* Special registers */
	for (i = 0; i < 82; ++i)
		self->sr[i].u32 = 0;
	self->sr[SR_LANEID].u32 = self->id_in_warp;
	self->sr[SR_TID_X].u32 = id % self->grid->thread_block_size3[0];
	self->sr[SR_TID_Y].u32 = (id / self->grid->thread_block_size3[0]) %
			self->grid->thread_block_size3[1];
	self->sr[SR_TID_Z].u32 = id / (self->grid->thread_block_size3[0] *
			self->grid->thread_block_size3[1]);
	self->sr[SR_CTAID_X].u32 = self->thread_block->id %
			self->grid->thread_block_count3[0];
	self->sr[SR_CTAID_Y].u32 = (self->thread_block->id /
			self->grid->thread_block_count3[0]) %
			self->grid->thread_block_count3[1];
	self->sr[SR_CTAID_Z].u32 = self->thread_block->id /
			(self->grid->thread_block_count3[0] *
					self->grid->thread_block_count3[1]);

	/* Predicate registers */
	for (i = 0; i < 7; ++i)
		self->pr[i] = 0;
	self->pr[7] = 1;

	/* Condition code registers */
	self->cc.sign = 0;
	self->cc.carry = 0;
	self->cc.zero = 0;
	self->cc.overflow = 0;

	/* Add thread to warp */
	warp->threads[self->id_in_warp] = self;
}

void FrmThreadDestroy(FrmThread *self)
{
}
