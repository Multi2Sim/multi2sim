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

#include <assert.h>
#include <stdarg.h>

#include <lib/class/array.h>
#include <lib/class/elf-reader.h>
#include <lib/class/string.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>

#include "asm.h"




/*
 * Class 'KplAsm'
 */

static void KplAsmInitTable(KplAsm *self, KplInstOpcode opcode, char *name,
		char *fmt_str, int num_args, ...)
{
	KplInstTableEntry *table;
	va_list ap;

	int index;
	int i;

	/* Initialize instruction info table */
	self->inst_info[opcode].opcode = opcode;
	self->inst_info[opcode].name = name;
	self->inst_info[opcode].fmt_str = fmt_str;

	/* Initialize argument list */
	va_start(ap, num_args);
	table = self->dec_table;

	/* Traverse argument list */
	for (i = 0; i < num_args - 1; i++)
	{
		/* Obtain index to access in this iteration */
		index = va_arg(ap, int);

		/* Sanity: no instruction, but next table */
		assert(!table[index].info);
		assert(table[index].next_table);

		/* Go to next table */
		table = table[index].next_table;
	}

	/* Get index for last table */
	index = va_arg(ap, int);
	assert(!table[index].next_table);

	/* Set final instruction info */
	table[index].info = &self->inst_info[opcode];
}


void KplAsmCreate(KplAsm *self)
{
	/* Allocate storage for the instruction tables */
	self->dec_table =		xcalloc(4, sizeof(KplInstTableEntry));

	self->dec_table_a =		xcalloc(8, sizeof(KplInstTableEntry));
	self->dec_table_b =		xcalloc(4, sizeof(KplInstTableEntry));
	self->dec_table_c =		xcalloc(4, sizeof(KplInstTableEntry));

	self->dec_table_a_a =		xcalloc(64, sizeof(KplInstTableEntry));

	self->dec_table_b_a =		xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_b_b =		xcalloc(4, sizeof(KplInstTableEntry));
	self->dec_table_b_c =		xcalloc(4, sizeof(KplInstTableEntry));
	self->dec_table_b_d =		xcalloc(4, sizeof(KplInstTableEntry));

	self->dec_table_b_c_a =		xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_b_c_b =		xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_b_c_c =		xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_b_c_d =		xcalloc(4, sizeof(KplInstTableEntry));

	self->dec_table_b_c_d_a =	xcalloc(4, sizeof(KplInstTableEntry));
	self->dec_table_b_c_d_b =	xcalloc(4, sizeof(KplInstTableEntry));
	self->dec_table_b_c_d_c =	xcalloc(4, sizeof(KplInstTableEntry));

	self->dec_table_b_c_d_c_a =	xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_b_c_d_c_b =	xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_b_c_d_c_c =	xcalloc(2, sizeof(KplInstTableEntry));

	self->dec_table_b_d_a =		xcalloc(32, sizeof(KplInstTableEntry));

	self->dec_table_c_a =		xcalloc(4, sizeof(KplInstTableEntry));
	self->dec_table_c_b =		xcalloc(16, sizeof(KplInstTableEntry));

	self->dec_table_c_a_a =		xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_c_a_b =		xcalloc(2, sizeof(KplInstTableEntry));

	self->dec_table_c_b_a =		xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_c_b_b =		xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_c_b_c =		xcalloc(4, sizeof(KplInstTableEntry));
	self->dec_table_c_b_d =		xcalloc(32, sizeof(KplInstTableEntry));
	self->dec_table_c_b_e =		xcalloc(4, sizeof(KplInstTableEntry));

	self->dec_table_c_b_c_a =	xcalloc(4, sizeof(KplInstTableEntry));
	self->dec_table_c_b_c_b =	xcalloc(4, sizeof(KplInstTableEntry));
	self->dec_table_c_b_c_c =	xcalloc(4, sizeof(KplInstTableEntry));

	self->dec_table_c_b_c_c_a =	xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_c_b_c_c_b =	xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_c_b_c_c_c =	xcalloc(2, sizeof(KplInstTableEntry));

	self->dec_table_c_b_e_a =	xcalloc(8, sizeof(KplInstTableEntry));
	self->dec_table_c_b_e_b =	xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_c_b_e_c =	xcalloc(2, sizeof(KplInstTableEntry));

	self->dec_table_c_b_e_a_a =	xcalloc(8, sizeof(KplInstTableEntry));
	self->dec_table_c_b_e_a_b =	xcalloc(2, sizeof(KplInstTableEntry));

	self->dec_table_c_b_e_a_a_a =	xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_c_b_e_a_a_b =	xcalloc(8, sizeof(KplInstTableEntry));
	self->dec_table_c_b_e_a_a_c =	xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_c_b_e_a_a_d =	xcalloc(4, sizeof(KplInstTableEntry));

	self->dec_table_c_b_e_a_a_c_a =	xcalloc(2, sizeof(KplInstTableEntry));

	self->dec_table_c_b_e_a_b_a =	xcalloc(4, sizeof(KplInstTableEntry));
	self->dec_table_c_b_e_a_b_b =	xcalloc(16, sizeof(KplInstTableEntry));

	self->dec_table_c_b_e_a_b_a_a =	xcalloc(4, sizeof(KplInstTableEntry));
	self->dec_table_c_b_e_a_b_a_b =	xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_c_b_e_a_b_a_c =	xcalloc(4, sizeof(KplInstTableEntry));

	self->dec_table_c_b_e_b_a =	xcalloc(2, sizeof(KplInstTableEntry));
	self->dec_table_c_b_e_b_b =	xcalloc(4, sizeof(KplInstTableEntry));

	self->dec_table_c_b_e_b_a_a =	xcalloc(16, sizeof(KplInstTableEntry));



	/* Initiate values for the 'next_table', 'next_table_low' and 'next_table_high'
	 * fields of the tables */


	self->dec_table[KPL_ASM_OPCODE_A].next_table =
		self->dec_table_a;
	self->dec_table[KPL_ASM_OPCODE_A].next_table_low                            	= 61;
	self->dec_table[KPL_ASM_OPCODE_A].next_table_high                           	= 63;

	self->dec_table[KPL_ASM_OPCODE_B].next_table =
		self->dec_table_b;
	self->dec_table[KPL_ASM_OPCODE_B].next_table_low                            	= 62;
	self->dec_table[KPL_ASM_OPCODE_B].next_table_high                           	= 63;

	self->dec_table[KPL_ASM_OPCODE_C].next_table =
		self->dec_table_c;
	self->dec_table[KPL_ASM_OPCODE_C].next_table_low                            	= 62;
	self->dec_table[KPL_ASM_OPCODE_C].next_table_high                           	= 63;

	self->dec_table_a[KPL_ASM_OPCODE_A_A].next_table =
		self->dec_table_a_a;
	self->dec_table_a[KPL_ASM_OPCODE_A_A].next_table_low                     	= 55;
	self->dec_table_a[KPL_ASM_OPCODE_A_A].next_table_high                      	= 60;

	self->dec_table_b[KPL_ASM_OPCODE_B_A].next_table =
		self->dec_table_b_a;
	self->dec_table_b[KPL_ASM_OPCODE_B_A].next_table_low                        	= 61;
	self->dec_table_b[KPL_ASM_OPCODE_B_A].next_table_high                       	= 61;

	self->dec_table_b[KPL_ASM_OPCODE_B_B].next_table =
		self->dec_table_b_b;
	self->dec_table_b[KPL_ASM_OPCODE_B_B].next_table_low                        	= 60;
	self->dec_table_b[KPL_ASM_OPCODE_B_B].next_table_high                       	= 61;

	self->dec_table_b[KPL_ASM_OPCODE_B_C].next_table =
		self->dec_table_b_c;
	self->dec_table_b[KPL_ASM_OPCODE_B_C].next_table_low                         	= 60;
	self->dec_table_b[KPL_ASM_OPCODE_B_C].next_table_high                       	= 61;

	self->dec_table_b[KPL_ASM_OPCODE_B_D].next_table =
		self->dec_table_b_d;
	self->dec_table_b[KPL_ASM_OPCODE_B_D].next_table_low                        	= 60;
	self->dec_table_b[KPL_ASM_OPCODE_B_D].next_table_high                       	= 61;

	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_A].next_table =
		self->dec_table_b_c_a;
	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_A].next_table_low                   	= 58;
	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_A].next_table_high                	= 58;

	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_B].next_table =
		self->dec_table_b_c_b;
	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_B].next_table_low                  	= 58;
	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_B].next_table_high                 	= 58;

	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_C].next_table =
		self->dec_table_b_c_c;
	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_C].next_table_low                 	= 58;
	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_C].next_table_high                  	= 58;

	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_D].next_table =
		self->dec_table_b_c_d;
	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_D].next_table_low                  	= 57;
	self->dec_table_b_c[KPL_ASM_OPCODE_B_C_D].next_table_high                 	= 58;

	self->dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_A].next_table =
		self->dec_table_b_c_d_a;
	self->dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_A].next_table_low              	= 55;
	self->dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_A].next_table_high              	= 56;

	self->dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_B].next_table =
		self->dec_table_b_c_d_b;
	self->dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_B].next_table_low               	= 55;
	self->dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_B].next_table_high              	= 56;

	self->dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_C].next_table =
		self->dec_table_b_c_d_c;
	self->dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_C].next_table_low              	= 55;
	self->dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_C].next_table_high               	= 56;

	self->dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_A].next_table =
		self->dec_table_b_c_d_c_a;
	self->dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_A].next_table_low          	= 54;
	self->dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_A].next_table_high           	= 54;

	self->dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_B].next_table =
		self->dec_table_b_c_d_c_b;
	self->dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_B].next_table_low        	= 54;
	self->dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_B].next_table_high            	= 54;

	self->dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_C].next_table =
		self->dec_table_b_c_d_c_c;
	self->dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_C].next_table_low          	= 54;
	self->dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_C].next_table_high         	= 54;

	self->dec_table_b_d[KPL_ASM_OPCODE_B_D_A].next_table =
		self->dec_table_b_d_a;
	self->dec_table_b_d[KPL_ASM_OPCODE_B_D_A].next_table_low              	= 54;
	self->dec_table_b_d[KPL_ASM_OPCODE_B_D_A].next_table_high                    	= 58;



	self->dec_table_c[KPL_ASM_OPCODE_C_A].next_table =
		self->dec_table_c_a;
	self->dec_table_c[KPL_ASM_OPCODE_C_A].next_table_low                        	= 60;
	self->dec_table_c[KPL_ASM_OPCODE_C_A].next_table_high                        	= 61;

	self->dec_table_c[KPL_ASM_OPCODE_C_B_0].next_table =
		self->dec_table_c_b;
	self->dec_table_c[KPL_ASM_OPCODE_C_B_0].next_table_low                      	= 59;
	self->dec_table_c[KPL_ASM_OPCODE_C_B_0].next_table_high                     	= 62;

	self->dec_table_c[KPL_ASM_OPCODE_C_B_1].next_table =
		self->dec_table_c_b;
	self->dec_table_c[KPL_ASM_OPCODE_C_B_1].next_table_low                     	= 59;
	self->dec_table_c[KPL_ASM_OPCODE_C_B_1].next_table_high                  	= 62;

	self->dec_table_c[KPL_ASM_OPCODE_C_B_2].next_table =
		self->dec_table_c_b;
	self->dec_table_c[KPL_ASM_OPCODE_C_B_2].next_table_low                      	= 59;
	self->dec_table_c[KPL_ASM_OPCODE_C_B_2].next_table_high                   	= 62;

	self->dec_table_c_a[KPL_ASM_OPCODE_C_A_A].next_table =
		self->dec_table_c_a_a;
	self->dec_table_c_a[KPL_ASM_OPCODE_C_A_A].next_table_low                 	= 59;
	self->dec_table_c_a[KPL_ASM_OPCODE_C_A_A].next_table_high               	= 59;

	self->dec_table_c_a[KPL_ASM_OPCODE_C_A_B].next_table =
		self->dec_table_c_a_b;
	self->dec_table_c_a[KPL_ASM_OPCODE_C_A_B].next_table_low                  	= 59;
	self->dec_table_c_a[KPL_ASM_OPCODE_C_A_B].next_table_high                   	= 59;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_A_0].next_table =
		self->dec_table_c_b_a;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_A_0].next_table_low                	= 58;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_A_0].next_table_high                 	= 58;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_A_1].next_table =
		self->dec_table_c_b_a;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_A_1].next_table_low                  	= 58;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_A_1].next_table_high                	= 58;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_B_0].next_table =
		self->dec_table_c_b_b;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_B_0].next_table_low                	= 58;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_B_0].next_table_high             	= 58;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_B_1].next_table =
		self->dec_table_c_b_b;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_B_1].next_table_low                	= 58;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_B_1].next_table_high               	= 58;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_C_0].next_table =
		self->dec_table_c_b_c;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_C_0].next_table_low                	= 57;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_C_0].next_table_high                 	= 58;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_C_1].next_table =
		self->dec_table_c_b_c;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_C_1].next_table_low                 	= 57;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_C_1].next_table_high               	= 58;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_D].next_table =
		self->dec_table_c_b_d;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_D].next_table_low                   	= 54;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_D].next_table_high                	= 58;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_0].next_table =
		self->dec_table_c_b_e;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_0].next_table_low                 	= 62;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_0].next_table_high                	= 63;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_1].next_table =
		self->dec_table_c_b_e;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_1].next_table_low                 	= 62;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_1].next_table_high               	= 63;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_2].next_table =
		self->dec_table_c_b_e;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_2].next_table_low                	= 62;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_2].next_table_high              	= 63;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_3].next_table =
		self->dec_table_c_b_e;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_3].next_table_low               	= 62;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_3].next_table_high                 	= 63;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_4].next_table =
		self->dec_table_c_b_e;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_4].next_table_low                	= 62;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_4].next_table_high                	= 63;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_5].next_table =
		self->dec_table_c_b_e;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_5].next_table_low                  	= 62;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_5].next_table_high                	= 63;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_6].next_table =
		self->dec_table_c_b_e;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_6].next_table_low                 	= 62;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_6].next_table_high               	= 63;

	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_7].next_table =
		self->dec_table_c_b_e;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_7].next_table_low                    = 62;
	self->dec_table_c_b[KPL_ASM_OPCODE_C_B_E_7].next_table_high                   = 63;

	self->dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_A].next_table =
		self->dec_table_c_b_c_a;
	self->dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_A].next_table_low                	= 55;
	self->dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_A].next_table_high               	= 56;

	self->dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_B].next_table =
		self->dec_table_c_b_c_b;
	self->dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_B].next_table_low                	= 55;
	self->dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_B].next_table_high                	= 56;

	self->dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_C].next_table =
		self->dec_table_c_b_c_c;
	self->dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_C].next_table_low                	= 56;
	self->dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_C].next_table_high               	= 55;

	self->dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_A].next_table =
		self->dec_table_c_b_c_c_a;
	self->dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_A].next_table_low             	= 54;
	self->dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_A].next_table_high            	= 54;

	self->dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_B].next_table =
		self->dec_table_c_b_c_c_b;
	self->dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_B].next_table_low            	= 54;
	self->dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_B].next_table_high           	= 54;

	self->dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_C].next_table =
		self->dec_table_c_b_c_c_c;
	self->dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_C].next_table_low          	= 54;
	self->dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_C].next_table_high         	= 54;

	self->dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_A].next_table =
		self->dec_table_c_b_e_a;
	self->dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_A].next_table_low               	= 59;
	self->dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_A].next_table_high             	= 61;

	self->dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_B].next_table =
		self->dec_table_c_b_e_b;
	self->dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_B].next_table_low            	= 61;
	self->dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_B].next_table_high             	= 61;

	self->dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_C].next_table =
		self->dec_table_c_b_e_c;
	self->dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_C].next_table_low               	= 59;
	self->dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_C].next_table_high             	= 59;

	self->dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_A].next_table =
		self->dec_table_c_b_e_a_a;
	self->dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_A].next_table_low            	= 56;
	self->dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_A].next_table_high          	= 58;

	self->dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_B].next_table =
		self->dec_table_c_b_e_a_b;
	self->dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_B].next_table_low           	= 58;
	self->dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_B].next_table_high         	= 58;

	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_A].next_table =
		self->dec_table_c_b_e_a_a_a;
	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_A].next_table_low        	= 55;
	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_A].next_table_high      	= 55;

	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_B].next_table =
		self->dec_table_c_b_e_a_a_b;
	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_B].next_table_low      	= 53;
	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_B].next_table_high      	= 55;

	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_C].next_table =
		self->dec_table_c_b_e_a_a_c;
	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_C].next_table_low       	= 55;
	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_C].next_table_high       	= 55;

	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_D].next_table =
		self->dec_table_c_b_e_a_a_d;
	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_D].next_table_low       	= 54;
	self->dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_D].next_table_high     	= 55;

	self->dec_table_c_b_e_a_a_c[KPL_ASM_OPCODE_C_B_E_A_A_C_A].next_table =
		self->dec_table_c_b_e_a_a_c_a;
	self->dec_table_c_b_e_a_a_c[KPL_ASM_OPCODE_C_B_E_A_A_C_A].next_table_low     	= 54;
	self->dec_table_c_b_e_a_a_c[KPL_ASM_OPCODE_C_B_E_A_A_C_A].next_table_high 	= 54;

	self->dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_A].next_table =
		self->dec_table_c_b_e_a_b_a;
	self->dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_A].next_table_low      	= 56;
	self->dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_A].next_table_high     	= 57;

	self->dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_B].next_table =
		self->dec_table_c_b_e_a_b_b;
	self->dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_B].next_table_low        	= 54;
	self->dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_B].next_table_high      	= 57;

	self->dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_A].next_table =
		self->dec_table_c_b_e_a_b_a_a;
	self->dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_A].next_table_low 	= 54;
	self->dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_A].next_table_high  	= 55;

	self->dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_B].next_table =
		self->dec_table_c_b_e_a_b_a_b;
	self->dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_B].next_table_low   	= 55;
	self->dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_B].next_table_high   	= 55;

	self->dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_C].next_table =
		self->dec_table_c_b_e_a_b_a_c;
	self->dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_C].next_table_low    	= 54;
	self->dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_C].next_table_high  	= 55;

	self->dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_A].next_table =
		self->dec_table_c_b_e_b_a;
	self->dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_A].next_table_low         	= 58;
	self->dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_A].next_table_high         	= 58;

	self->dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_B].next_table =
		self->dec_table_c_b_e_b_b;
	self->dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_B].next_table_low           	= 59;
	self->dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_B].next_table_high          	= 60;

	self->dec_table_c_b_e_b_a[KPL_ASM_OPCODE_C_B_E_B_A_A].next_table =
		self->dec_table_c_b_e_b_a_a;
	self->dec_table_c_b_e_b_a[KPL_ASM_OPCODE_C_B_E_B_A_A].next_table_low      	= 54;
	self->dec_table_c_b_e_b_a[KPL_ASM_OPCODE_C_B_E_B_A_A].next_table_high      	= 57;

#define DEFINST(_name, _fmt_str, ...) \
	KplAsmInitTable(self, KPL_INST_##_name, #_name, _fmt_str, PP_NARG(__VA_ARGS__), __VA_ARGS__);

#include "asm.dat"
#undef DEFINST
}


void KplAsmDestroy(KplAsm *self)
{
	free(self->dec_table);
	free(self->dec_table_a);
	free(self->dec_table_b);
	free(self->dec_table_c);

	free(self->dec_table_a_a);

	free(self->dec_table_b_a);
	free(self->dec_table_b_b);
	free(self->dec_table_b_c);
	free(self->dec_table_b_d);

	free(self->dec_table_b_c_a);
	free(self->dec_table_b_c_b);
	free(self->dec_table_b_c_c);
	free(self->dec_table_b_c_d);

	free(self->dec_table_b_c_d_a);
	free(self->dec_table_b_c_d_b);
	free(self->dec_table_b_c_d_c);

	free(self->dec_table_b_c_d_c_a);
	free(self->dec_table_b_c_d_c_b);
	free(self->dec_table_b_c_d_c_c);

	free(self->dec_table_b_d_a);

	free(self->dec_table_c_a);
	free(self->dec_table_c_b);

	free(self->dec_table_c_a_a);
	free(self->dec_table_c_a_b);

	free(self->dec_table_c_b_a);
	free(self->dec_table_c_b_b);
	free(self->dec_table_c_b_c);
	free(self->dec_table_c_b_d);
	free(self->dec_table_c_b_e);

	free(self->dec_table_c_b_c_a);
	free(self->dec_table_c_b_c_b);
	free(self->dec_table_c_b_c_c);

	free(self->dec_table_c_b_c_c_a);
	free(self->dec_table_c_b_c_c_b);
	free(self->dec_table_c_b_c_c_c);

	free(self->dec_table_c_b_e_a);
	free(self->dec_table_c_b_e_b);
	free(self->dec_table_c_b_e_c);

	free(self->dec_table_c_b_e_a_a);
	free(self->dec_table_c_b_e_a_b);

	free(self->dec_table_c_b_e_a_a_a);
	free(self->dec_table_c_b_e_a_a_b);
	free(self->dec_table_c_b_e_a_a_c);
	free(self->dec_table_c_b_e_a_a_d);

	free(self->dec_table_c_b_e_a_a_c_a);

	free(self->dec_table_c_b_e_a_b_a);
	free(self->dec_table_c_b_e_a_b_b);

	free(self->dec_table_c_b_e_a_b_a_a);
	free(self->dec_table_c_b_e_a_b_a_b);
	free(self->dec_table_c_b_e_a_b_a_c);

	free(self->dec_table_c_b_e_b_a);
	free(self->dec_table_c_b_e_b_b);

	free(self->dec_table_c_b_e_b_a_a);
}


void KplAsmDisassembleBinary(KplAsm *self, char *path)
{
	KplInst *inst;

	ELFReader *reader;
	ELFSection *section;

	void *ptr;
	char *title;


	/* Initializations */
	inst = new(KplInst, self);
	reader = new(ELFReader, path);

	/* Read Sections */
	ArrayForEach(reader->section_array, section, ELFSection)
	{
		/* Get section and skip it if it does not contain code */
		if (!(section->header->sh_flags & SHF_EXECINSTR))
			continue;

		/* Set section name (get rid of .text.) */
		title = section->name->length >= 6 ? section->name->text + 6
				: section->name->text;

		/* Title */
		printf("\tcode for sm_35");
		printf("\n\t\tFunction : %s", title);

		/* Decode and dump instructions */
		for (ptr = section->buffer->ptr; ptr < section->buffer->ptr +
				section->buffer->size; ptr += 8)
		{
			/* Decode and dump */
			KplInstDecode(inst, ptr, section->header->sh_addr + section->buffer->pos);
			KplInstDumpHex(inst, stdout);
			KplInstDump(inst, stdout);

			/* Move to next instruction */
			section->buffer->pos += 8;
		}
		printf("\n\t\t.................................\n\n");
	}
	printf("\n");
	delete(reader);
	delete(inst);
}
