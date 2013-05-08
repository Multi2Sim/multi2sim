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

#include <arch/common/arch.h>
#include <arch/mips/emu/emu.h>
#include <lib/esim/esim.h>

#include "cpu.h"


/*
 * Public Functions
 */

void mips_cpu_read_config(void)
{
}


void mips_cpu_init(void)
{
}


void mips_cpu_done(void)
{
}

void mips_cpu_dump(FILE *f)
{
}

/* Run one iteration of timing simulation. Return values are:
 *  - arch_sim_kind_invalid - no timing simulation. 
 *  - arch_sim_kind_detailed - still simulating. */
enum arch_sim_kind_t mips_cpu_run(void)
{
	return arch_sim_kind_invalid;
}

/* TODO: Implement the Mips cpu dump for the cycle accurate simulation */
void mips_cpu_dump_summary(FILE *f)
{
}
