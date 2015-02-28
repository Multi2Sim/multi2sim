/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FERMI_EMU_THREAD_H
#define FERMI_EMU_THREAD_H

#include <arch/fermi/asm/Wrapper.h>
#include <lib/class/class.h>


/*
 * Class 'FrmThread'
 */

typedef union
{
	unsigned u32;
	int s32;
	float f;
} FrmThreadReg;

typedef struct
{
	unsigned sign;
	unsigned carry;
	unsigned zero;
	unsigned overflow;
} FrmThreadCC;

CLASS_BEGIN(FrmThread, Object)

	/* IDs */
	int id;
	int id_in_warp;

	/* Warp, thread-block, and grid where it belongs */
	FrmWarp *warp;
	FrmThreadBlock *thread_block;
	FrmGrid *grid;

	/* Registers */
	FrmThreadReg gpr[64];  /* General purpose registers */
	FrmThreadReg sr[82];  /* Special registers */
	unsigned pr[8];  /* Predicate registers */
	FrmThreadCC cc;  /* Condition code registers */

	/* Fields below are used for architectural simulation only. */

CLASS_END(FrmThread)


void FrmThreadCreate(FrmThread *self, int id, FrmWarp *warp);
void FrmThreadDestroy(FrmThread *self);


#endif

