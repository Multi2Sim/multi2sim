/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include "m2skernel.h"

struct regs_t *regs_create()
{
	struct regs_t *regs;
	regs = calloc(1, sizeof(struct regs_t));
	return regs;
}


void regs_free(struct regs_t *regs)
{
	free(regs);
}


void regs_copy(struct regs_t *dst, struct regs_t *src)
{
	memcpy(dst, src, sizeof(struct regs_t));
}


void regs_fpu_stack_dump(struct regs_t *regs, FILE *f)
{
	int index, i;
	double value;
	char *comma = "";

	fprintf(f, "{");
	index = (regs->fpu_top + 7) % 8;
	for (i = 0; i < 8; i++) {
		if (regs->fpu_stack[index].valid) {
			value = isa_extended_to_double(regs->fpu_stack[index].value);
			fprintf(f, "%s%g", comma, value);
			comma = ", ";
		}
		index = (index + 7) % 8;
	}
	fprintf(f, "}\n");
}


void regs_dump(struct regs_t *regs, FILE *f)
{
	/* Integer registers */
	fprintf(f, "  eax=%08x  ecx=%08x  edx=%08x  ebx=%08x\n",
		regs->eax, regs->ecx, regs->edx, regs->ebx);
	fprintf(f, "  esp=%08x  ebp=%08x  esi=%08x  edi=%08x\n",
		regs->esp, regs->ebp, regs->esi, regs->edi);
	fprintf(f, "  es=%x, cs=%x, ss=%x, ds=%x, fs=%x, gs=%x\n",
		regs->es, regs->cs, regs->ss, regs->ds, regs->fs, regs->gs);
	fprintf(f, "  eip=%08x\n", regs->eip);
	fprintf(f, "  flags=%04x (cf=%d  pf=%d  af=%d  zf=%d  sf=%d  df=%d  of=%d)\n",
		regs->eflags,
		(regs->eflags & (1 << flag_cf)) > 0,
		(regs->eflags & (1 << flag_pf)) > 0,
		(regs->eflags & (1 << flag_af)) > 0,
		(regs->eflags & (1 << flag_zf)) > 0,
		(regs->eflags & (1 << flag_sf)) > 0,
		(regs->eflags & (1 << flag_df)) > 0,
		(regs->eflags & (1 << flag_of)) > 0);
	
	/* Floating-point stack */
	fprintf(f, "  fpu_stack (last=top): ");
	regs_fpu_stack_dump(regs, f);

	/* Floating point code (part from status register) */
	fprintf(f, "  fpu_code (C3-C2-C1-C0): %d-%d-%d-%d\n",
		GETBIT32(regs->fpu_code, 3) > 0, GETBIT32(regs->fpu_code, 2) > 0,
		GETBIT32(regs->fpu_code, 1) > 0, GETBIT32(regs->fpu_code, 0) > 0);
	fprintf(f, "  fpu_ctrl=%04x\n",
		regs->fpu_ctrl);
}

