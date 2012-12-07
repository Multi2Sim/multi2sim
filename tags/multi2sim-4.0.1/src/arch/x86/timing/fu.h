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

#ifndef X86_ARCH_TIMING_FU_H
#define X86_ARCH_TIMING_FU_H

#include "uop.h"


#define X86_FU_RES_MAX  10

enum x86_fu_class_t
{
	x86_fu_none = 0,

	x86_fu_intadd,
	x86_fu_intmult,
	x86_fu_intdiv,
	x86_fu_effaddr,
	x86_fu_logic,

	x86_fu_fpsimple,
	x86_fu_fpadd,
	x86_fu_fpmult,
	x86_fu_fpdiv,
	x86_fu_fpcomplex,

	x86_fu_xmm_int,
	x86_fu_xmm_float,
	x86_fu_xmm_logic,

	x86_fu_count
};

struct x86_fu_t
{
	long long cycle_when_free[x86_fu_count][X86_FU_RES_MAX];
	long long accesses[x86_fu_count];
	long long denied[x86_fu_count];
	long long waiting_time[x86_fu_count];
};

struct x86_fu_res_t
{
	int count;
	int oplat;
	int issuelat;
	char *name;
};

extern struct x86_fu_res_t x86_fu_res_pool[x86_fu_count];

void x86_fu_init(void);
void x86_fu_done(void);

int x86_fu_reserve(struct x86_uop_t *uop);
void x86_fu_release(int core);


#endif

