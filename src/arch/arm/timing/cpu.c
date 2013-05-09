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

#include <arch/arm/emu/emu.h>
#include <arch/common/arch.h>
#include <lib/esim/esim.h>
#include <lib/util/misc.h>

#include "cpu.h"


/*
 * Global Variables
 */


/* 
 * Public Functions
 */

void arm_cpu_read_config(void)
{
}


void arm_cpu_init(void)
{
}


void arm_cpu_done(void)
{
}


void arm_cpu_dump(FILE *f)
{
}


/* Run one iteration of timing simulation. Return TRUE if the timing simulation
 * is still running. */
int arm_cpu_run(void)
{
	return FALSE;
}


/* TODO: Implement the Arm cpu dump for the cycle accurate simulation */
void arm_cpu_dump_summary(FILE *f)
{
}
