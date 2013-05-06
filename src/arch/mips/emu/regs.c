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


#include <lib/mhandle/mhandle.h>

#include "regs.h"

char *gpr_name[32] = {
	"zero",	"at",	"v0",	"v1",	"a0",	"a1",	"a2",	"a3",
	"t0",	"t1",	"t2",	"t3",	"t4",	"t5",	"t6",	"t6",
	"s0",	"s1",	"s2",	"s3",	"s4",	"s5",	"s6",	"s7",
	"t8",	"t9",	"k0",	"k1",	"gp",	"sp",	"fp",	"ra"
};

char *fpr_name[32] = {
	"$f0",	"$f1",	"$f2",	"$f3",	"$f4",	"$f5",	"$f6",	"$f7",
	"$f8",	"$f9",	"$f10",	"$f11",	"$f12",	"$f13",	"$f14",	"$f15",
	"$f16",	"$f17",	"$f18",	"$f19",	"$f20",	"$f21",	"$f22",	"$f23",
	"$f24",	"$f25",	"$f26",	"$f27",	"$f28",	"$f29",	"$f30",	"$f31"
};


struct mips_regs_t *mips_regs_create()
{
	struct mips_regs_t *regs;

	regs = xcalloc(1, sizeof(struct mips_regs_t));

	//regs->cpsr.mode = MIPS_MODE_USER;
	return regs;
}


void mips_regs_free(struct mips_regs_t *regs)
{
	free(regs);
}


void mips_regs_copy(struct mips_regs_t *dst, struct mips_regs_t *src)
{
	memcpy(dst, src, sizeof(struct mips_regs_t));
}
