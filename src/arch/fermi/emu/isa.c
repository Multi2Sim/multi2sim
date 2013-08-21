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


#include <arch/fermi/asm/asm.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "isa.h"
#include "machine.h"



/* Debug */
int frm_isa_debug_category = 1;



/*
 * Global Memory
 */

void frm_isa_global_mem_write(unsigned int addr, void *pvalue)
{
	mem_write(frm_emu->global_mem, addr, sizeof(unsigned int), pvalue);
}


void frm_isa_global_mem_read(unsigned int addr, void *pvalue)
{
	mem_read(frm_emu->global_mem, addr, sizeof(unsigned int), pvalue);
}



/*
 * Constant Memory
 */

void frm_isa_const_mem_write(unsigned int addr, void *pvalue)
{
	/* Mark c[0][0..1c] as initialized */
	if (addr < 0x20)
		frm_emu->const_mem_init[addr] = 1;

	/* Write */
	mem_write(frm_emu->const_mem, addr, sizeof(unsigned int), pvalue);
}


void frm_isa_const_mem_read(unsigned int addr, void *pvalue)
{
	/* Warn if c[0][0..1c] is used uninitialized */
	if (addr < 0x20 && !frm_emu->const_mem_init[addr])
		warning("c [0] [0x%x] is used uninitialized", addr);
	
	/* Read */
	mem_read(frm_emu->const_mem, addr, sizeof(unsigned int), pvalue);
}



