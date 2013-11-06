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

#include <cassert>
#include <cstdarg>
#include <iostream>

#include <lib/cpp/ELFReader.h>
#include <lib/cpp/Misc.h>

#include "Asm.h"


namespace Kepler
{


#define KPL_ASM_OPCODE_A            	0
#define KPL_ASM_OPCODE_B            	1
#define KPL_ASM_OPCODE_C            	2

#define KPL_ASM_OPCODE_A_A          	0

#define KPL_ASM_OPCODE_B_A          	0
#define KPL_ASM_OPCODE_B_B          	1
#define KPL_ASM_OPCODE_B_C          	2
#define KPL_ASM_OPCODE_B_D          	3

#define KPL_ASM_OPCODE_B_C_A        	0
#define KPL_ASM_OPCODE_B_C_B        	1
#define KPL_ASM_OPCODE_B_C_C        	2
#define KPL_ASM_OPCODE_B_C_D        	3

#define KPL_ASM_OPCODE_B_C_D_A      	1
#define KPL_ASM_OPCODE_B_C_D_B      	2
#define KPL_ASM_OPCODE_B_C_D_C      	3

#define KPL_ASM_OPCODE_B_C_D_C_A    	1
#define KPL_ASM_OPCODE_B_C_D_C_B    	2
#define KPL_ASM_OPCODE_B_C_D_C_C    	3

#define KPL_ASM_OPCODE_B_D_A        	0

#define KPL_ASM_OPCODE_C_A		0
#define KPL_ASM_OPCODE_C_B_0		1
#define KPL_ASM_OPCODE_C_B_1		2
#define KPL_ASM_OPCODE_C_B_2		3

#define KPL_ASM_OPCODE_C_A_A		2
#define KPL_ASM_OPCODE_C_A_B		3

#define KPL_ASM_OPCODE_C_B_A_0		1
#define KPL_ASM_OPCODE_C_B_A_1		9
#define KPL_ASM_OPCODE_C_B_B_0		2
#define KPL_ASM_OPCODE_C_B_B_1		10
#define KPL_ASM_OPCODE_C_B_C_0		3
#define KPL_ASM_OPCODE_C_B_C_1		11
#define KPL_ASM_OPCODE_C_B_D		12
#define KPL_ASM_OPCODE_C_B_E_0  	0
#define KPL_ASM_OPCODE_C_B_E_1      	4
#define KPL_ASM_OPCODE_C_B_E_2      	5
#define KPL_ASM_OPCODE_C_B_E_3      	6
#define KPL_ASM_OPCODE_C_B_E_4      	7
#define KPL_ASM_OPCODE_C_B_E_5      	13
#define KPL_ASM_OPCODE_C_B_E_6      	14
#define KPL_ASM_OPCODE_C_B_E_7          15

#define KPL_ASM_OPCODE_C_B_C_A			1
#define KPL_ASM_OPCODE_C_B_C_B			2
#define KPL_ASM_OPCODE_C_B_C_C			3

#define KPL_ASM_OPCODE_C_B_C_C_A		1
#define KPL_ASM_OPCODE_C_B_C_C_B		2
#define KPL_ASM_OPCODE_C_B_C_C_C		3

#define KPL_ASM_OPCODE_C_B_E_A			1
#define KPL_ASM_OPCODE_C_B_E_B			2
#define KPL_ASM_OPCODE_C_B_E_C			3

#define KPL_ASM_OPCODE_C_B_E_A_A		6
#define KPL_ASM_OPCODE_C_B_E_A_B		7

#define KPL_ASM_OPCODE_C_B_E_A_A_A		4
#define KPL_ASM_OPCODE_C_B_E_A_A_B		5
#define KPL_ASM_OPCODE_C_B_E_A_A_C		6
#define KPL_ASM_OPCODE_C_B_E_A_A_D		7

#define KPL_ASM_OPCODE_C_B_E_A_A_C_A		1

#define KPL_ASM_OPCODE_C_B_E_A_B_A		0
#define KPL_ASM_OPCODE_C_B_E_A_B_B		1

#define KPL_ASM_OPCODE_C_B_E_A_B_A_A		0
#define KPL_ASM_OPCODE_C_B_E_A_B_A_B		1
#define KPL_ASM_OPCODE_C_B_E_A_B_A_C		2

#define KPL_ASM_OPCODE_C_B_E_B_A		0
#define KPL_ASM_OPCODE_C_B_E_B_B		1

#define KPL_ASM_OPCODE_C_B_E_B_A_A		1



void Asm::InitTable(InstOpcode opcode, const char *name,
		const char *fmt_str, int num_args, ...)
{
	va_list ap;

	int index;
	int i;

	/* Initialize instruction info table */
	inst_info[opcode].opcode = opcode;
	inst_info[opcode].name = name;
	inst_info[opcode].fmt_str = fmt_str;

	/* Initialize argument list */
	va_start(ap, num_args);
	InstDecodeInfo *table = dec_table;

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
	table[index].info = &inst_info[opcode];
}


Asm::Asm()
{
	/* Initiate values for the 'next_table', 'next_table_low' and 'next_table_high'
	 * fields of the tables */

	dec_table[KPL_ASM_OPCODE_A].next_table =
		dec_table_a;
	dec_table[KPL_ASM_OPCODE_A].next_table_low                            	= 61;
	dec_table[KPL_ASM_OPCODE_A].next_table_high                           	= 63;

	dec_table[KPL_ASM_OPCODE_B].next_table =
		dec_table_b;
	dec_table[KPL_ASM_OPCODE_B].next_table_low                            	= 62;
	dec_table[KPL_ASM_OPCODE_B].next_table_high                           	= 63;

	dec_table[KPL_ASM_OPCODE_C].next_table =
		dec_table_c;
	dec_table[KPL_ASM_OPCODE_C].next_table_low                            	= 62;
	dec_table[KPL_ASM_OPCODE_C].next_table_high                           	= 63;

	dec_table_a[KPL_ASM_OPCODE_A_A].next_table =
		dec_table_a_a;
	dec_table_a[KPL_ASM_OPCODE_A_A].next_table_low                     	= 55;
	dec_table_a[KPL_ASM_OPCODE_A_A].next_table_high                      	= 60;

	dec_table_b[KPL_ASM_OPCODE_B_A].next_table =
		dec_table_b_a;
	dec_table_b[KPL_ASM_OPCODE_B_A].next_table_low                        	= 61;
	dec_table_b[KPL_ASM_OPCODE_B_A].next_table_high                       	= 61;

	dec_table_b[KPL_ASM_OPCODE_B_B].next_table =
		dec_table_b_b;
	dec_table_b[KPL_ASM_OPCODE_B_B].next_table_low                        	= 60;
	dec_table_b[KPL_ASM_OPCODE_B_B].next_table_high                       	= 61;

	dec_table_b[KPL_ASM_OPCODE_B_C].next_table =
		dec_table_b_c;
	dec_table_b[KPL_ASM_OPCODE_B_C].next_table_low                         	= 60;
	dec_table_b[KPL_ASM_OPCODE_B_C].next_table_high                       	= 61;

	dec_table_b[KPL_ASM_OPCODE_B_D].next_table =
		dec_table_b_d;
	dec_table_b[KPL_ASM_OPCODE_B_D].next_table_low                        	= 60;
	dec_table_b[KPL_ASM_OPCODE_B_D].next_table_high                       	= 61;

	dec_table_b_c[KPL_ASM_OPCODE_B_C_A].next_table =
		dec_table_b_c_a;
	dec_table_b_c[KPL_ASM_OPCODE_B_C_A].next_table_low                   	= 58;
	dec_table_b_c[KPL_ASM_OPCODE_B_C_A].next_table_high                	= 58;

	dec_table_b_c[KPL_ASM_OPCODE_B_C_B].next_table =
		dec_table_b_c_b;
	dec_table_b_c[KPL_ASM_OPCODE_B_C_B].next_table_low                  	= 58;
	dec_table_b_c[KPL_ASM_OPCODE_B_C_B].next_table_high                 	= 58;

	dec_table_b_c[KPL_ASM_OPCODE_B_C_C].next_table =
		dec_table_b_c_c;
	dec_table_b_c[KPL_ASM_OPCODE_B_C_C].next_table_low                 	= 58;
	dec_table_b_c[KPL_ASM_OPCODE_B_C_C].next_table_high                  	= 58;

	dec_table_b_c[KPL_ASM_OPCODE_B_C_D].next_table =
		dec_table_b_c_d;
	dec_table_b_c[KPL_ASM_OPCODE_B_C_D].next_table_low                  	= 57;
	dec_table_b_c[KPL_ASM_OPCODE_B_C_D].next_table_high                 	= 58;

	dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_A].next_table =
		dec_table_b_c_d_a;
	dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_A].next_table_low              	= 55;
	dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_A].next_table_high              	= 56;

	dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_B].next_table =
		dec_table_b_c_d_b;
	dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_B].next_table_low               	= 55;
	dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_B].next_table_high              	= 56;

	dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_C].next_table =
		dec_table_b_c_d_c;
	dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_C].next_table_low              	= 55;
	dec_table_b_c_d[KPL_ASM_OPCODE_B_C_D_C].next_table_high               	= 56;

	dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_A].next_table =
		dec_table_b_c_d_c_a;
	dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_A].next_table_low          	= 54;
	dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_A].next_table_high           	= 54;

	dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_B].next_table =
		dec_table_b_c_d_c_b;
	dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_B].next_table_low        	= 54;
	dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_B].next_table_high            	= 54;

	dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_C].next_table =
		dec_table_b_c_d_c_c;
	dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_C].next_table_low          	= 54;
	dec_table_b_c_d_c[KPL_ASM_OPCODE_B_C_D_C_C].next_table_high         	= 54;

	dec_table_b_d[KPL_ASM_OPCODE_B_D_A].next_table =
		dec_table_b_d_a;
	dec_table_b_d[KPL_ASM_OPCODE_B_D_A].next_table_low              	= 54;
	dec_table_b_d[KPL_ASM_OPCODE_B_D_A].next_table_high                    	= 58;



	dec_table_c[KPL_ASM_OPCODE_C_A].next_table =
		dec_table_c_a;
	dec_table_c[KPL_ASM_OPCODE_C_A].next_table_low                        	= 60;
	dec_table_c[KPL_ASM_OPCODE_C_A].next_table_high                        	= 61;

	dec_table_c[KPL_ASM_OPCODE_C_B_0].next_table =
		dec_table_c_b;
	dec_table_c[KPL_ASM_OPCODE_C_B_0].next_table_low                      	= 59;
	dec_table_c[KPL_ASM_OPCODE_C_B_0].next_table_high                     	= 62;

	dec_table_c[KPL_ASM_OPCODE_C_B_1].next_table =
		dec_table_c_b;
	dec_table_c[KPL_ASM_OPCODE_C_B_1].next_table_low                     	= 59;
	dec_table_c[KPL_ASM_OPCODE_C_B_1].next_table_high                  	= 62;

	dec_table_c[KPL_ASM_OPCODE_C_B_2].next_table =
		dec_table_c_b;
	dec_table_c[KPL_ASM_OPCODE_C_B_2].next_table_low                      	= 59;
	dec_table_c[KPL_ASM_OPCODE_C_B_2].next_table_high                   	= 62;

	dec_table_c_a[KPL_ASM_OPCODE_C_A_A].next_table =
		dec_table_c_a_a;
	dec_table_c_a[KPL_ASM_OPCODE_C_A_A].next_table_low                 	= 59;
	dec_table_c_a[KPL_ASM_OPCODE_C_A_A].next_table_high               	= 59;

	dec_table_c_a[KPL_ASM_OPCODE_C_A_B].next_table =
		dec_table_c_a_b;
	dec_table_c_a[KPL_ASM_OPCODE_C_A_B].next_table_low                  	= 59;
	dec_table_c_a[KPL_ASM_OPCODE_C_A_B].next_table_high                   	= 59;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_A_0].next_table =
		dec_table_c_b_a;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_A_0].next_table_low                	= 58;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_A_0].next_table_high                 	= 58;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_A_1].next_table =
		dec_table_c_b_a;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_A_1].next_table_low                  	= 58;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_A_1].next_table_high                	= 58;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_B_0].next_table =
		dec_table_c_b_b;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_B_0].next_table_low                	= 58;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_B_0].next_table_high             	= 58;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_B_1].next_table =
		dec_table_c_b_b;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_B_1].next_table_low                	= 58;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_B_1].next_table_high               	= 58;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_C_0].next_table =
		dec_table_c_b_c;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_C_0].next_table_low                	= 57;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_C_0].next_table_high                 	= 58;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_C_1].next_table =
		dec_table_c_b_c;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_C_1].next_table_low                 	= 57;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_C_1].next_table_high               	= 58;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_D].next_table =
		dec_table_c_b_d;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_D].next_table_low                   	= 54;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_D].next_table_high                	= 58;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_0].next_table =
		dec_table_c_b_e;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_0].next_table_low                 	= 62;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_0].next_table_high                	= 63;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_1].next_table =
		dec_table_c_b_e;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_1].next_table_low                 	= 62;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_1].next_table_high               	= 63;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_2].next_table =
		dec_table_c_b_e;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_2].next_table_low                	= 62;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_2].next_table_high              	= 63;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_3].next_table =
		dec_table_c_b_e;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_3].next_table_low               	= 62;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_3].next_table_high                 	= 63;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_4].next_table =
		dec_table_c_b_e;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_4].next_table_low                	= 62;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_4].next_table_high                	= 63;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_5].next_table =
		dec_table_c_b_e;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_5].next_table_low                  	= 62;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_5].next_table_high                	= 63;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_6].next_table =
		dec_table_c_b_e;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_6].next_table_low                 	= 62;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_6].next_table_high               	= 63;

	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_7].next_table =
		dec_table_c_b_e;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_7].next_table_low                    = 62;
	dec_table_c_b[KPL_ASM_OPCODE_C_B_E_7].next_table_high                   = 63;

	dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_A].next_table =
		dec_table_c_b_c_a;
	dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_A].next_table_low                	= 55;
	dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_A].next_table_high               	= 56;

	dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_B].next_table =
		dec_table_c_b_c_b;
	dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_B].next_table_low                	= 55;
	dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_B].next_table_high                	= 56;

	dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_C].next_table =
		dec_table_c_b_c_c;
	dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_C].next_table_low                	= 56;
	dec_table_c_b_c[KPL_ASM_OPCODE_C_B_C_C].next_table_high               	= 55;

	dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_A].next_table =
		dec_table_c_b_c_c_a;
	dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_A].next_table_low             	= 54;
	dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_A].next_table_high            	= 54;

	dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_B].next_table =
		dec_table_c_b_c_c_b;
	dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_B].next_table_low            	= 54;
	dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_B].next_table_high           	= 54;

	dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_C].next_table =
		dec_table_c_b_c_c_c;
	dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_C].next_table_low          	= 54;
	dec_table_c_b_c_c[KPL_ASM_OPCODE_C_B_C_C_C].next_table_high         	= 54;

	dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_A].next_table =
		dec_table_c_b_e_a;
	dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_A].next_table_low               	= 59;
	dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_A].next_table_high             	= 61;

	dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_B].next_table =
		dec_table_c_b_e_b;
	dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_B].next_table_low            	= 61;
	dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_B].next_table_high             	= 61;

	dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_C].next_table =
		dec_table_c_b_e_c;
	dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_C].next_table_low               	= 59;
	dec_table_c_b_e[KPL_ASM_OPCODE_C_B_E_C].next_table_high             	= 59;

	dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_A].next_table =
		dec_table_c_b_e_a_a;
	dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_A].next_table_low            	= 56;
	dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_A].next_table_high          	= 58;

	dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_B].next_table =
		dec_table_c_b_e_a_b;
	dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_B].next_table_low           	= 58;
	dec_table_c_b_e_a[KPL_ASM_OPCODE_C_B_E_A_B].next_table_high         	= 58;

	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_A].next_table =
		dec_table_c_b_e_a_a_a;
	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_A].next_table_low        	= 55;
	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_A].next_table_high      	= 55;

	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_B].next_table =
		dec_table_c_b_e_a_a_b;
	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_B].next_table_low      	= 53;
	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_B].next_table_high      	= 55;

	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_C].next_table =
		dec_table_c_b_e_a_a_c;
	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_C].next_table_low       	= 55;
	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_C].next_table_high       	= 55;

	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_D].next_table =
		dec_table_c_b_e_a_a_d;
	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_D].next_table_low       	= 54;
	dec_table_c_b_e_a_a[KPL_ASM_OPCODE_C_B_E_A_A_D].next_table_high     	= 55;

	dec_table_c_b_e_a_a_c[KPL_ASM_OPCODE_C_B_E_A_A_C_A].next_table =
		dec_table_c_b_e_a_a_c_a;
	dec_table_c_b_e_a_a_c[KPL_ASM_OPCODE_C_B_E_A_A_C_A].next_table_low     	= 54;
	dec_table_c_b_e_a_a_c[KPL_ASM_OPCODE_C_B_E_A_A_C_A].next_table_high 	= 54;

	dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_A].next_table =
		dec_table_c_b_e_a_b_a;
	dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_A].next_table_low      	= 56;
	dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_A].next_table_high     	= 57;

	dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_B].next_table =
		dec_table_c_b_e_a_b_b;
	dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_B].next_table_low        	= 54;
	dec_table_c_b_e_a_b[KPL_ASM_OPCODE_C_B_E_A_B_B].next_table_high      	= 57;

	dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_A].next_table =
		dec_table_c_b_e_a_b_a_a;
	dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_A].next_table_low 	= 54;
	dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_A].next_table_high  	= 55;

	dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_B].next_table =
		dec_table_c_b_e_a_b_a_b;
	dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_B].next_table_low   	= 55;
	dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_B].next_table_high   	= 55;

	dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_C].next_table =
		dec_table_c_b_e_a_b_a_c;
	dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_C].next_table_low    	= 54;
	dec_table_c_b_e_a_b_a[KPL_ASM_OPCODE_C_B_E_A_B_A_C].next_table_high  	= 55;

	dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_A].next_table =
		dec_table_c_b_e_b_a;
	dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_A].next_table_low         	= 58;
	dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_A].next_table_high         	= 58;

	dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_B].next_table =
		dec_table_c_b_e_b_b;
	dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_B].next_table_low           	= 59;
	dec_table_c_b_e_b[KPL_ASM_OPCODE_C_B_E_B_B].next_table_high          	= 60;

	dec_table_c_b_e_b_a[KPL_ASM_OPCODE_C_B_E_B_A_A].next_table =
		dec_table_c_b_e_b_a_a;
	dec_table_c_b_e_b_a[KPL_ASM_OPCODE_C_B_E_B_A_A].next_table_low      	= 54;
	dec_table_c_b_e_b_a[KPL_ASM_OPCODE_C_B_E_B_A_A].next_table_high      	= 57;

#define DEFINST(_name, _fmt_str, ...) \
	InitTable(INST_##_name, #_name, _fmt_str, PP_NARG(__VA_ARGS__), __VA_ARGS__);

#include "asm.dat"
#undef DEFINST
}


void Asm::DisassembleBinary(const std::string &path)
{
	/* Initializations */
	ELFReader::File f(path);
	Inst inst(this);

	/* Read Sections */
	for (int i = 0; i < f.getNumSections(); i++)
	{
		/* Get section and skip it if it does not contain code */
		ELFReader::Section *section = f.getSection(i);
		if (!(section->getFlags() & SHF_EXECINSTR))
			continue;

		/* Set section name (get rid of .text.) */
		std::string title = section->getName();
		if (Misc::StringPrefix(title, ".text."))
			title.erase(0, 6);

		/* Title */
		std::cout << "\tcode for sm_35";
		std::cout << "\n\t\tFunction : " << title;

		/* Decode and dump instructions */
		for (size_t pos = 0; pos < section->getSize(); pos += 8)
		{
			/* Decode and dump */
			inst.Decode(section->getBuffer() + pos, section->getAddr() + pos);
			inst.DumpHex(std::cout);
			inst.Dump(std::cout);
		}

		/* End */
		std::cout << "\n\t\t" << std::string(33, '.') << "\n\n";
	}
	std::cout << '\n';
}


}  /* namespace Kepler */


/*
 * C Wrapper
 */

struct KplAsm *KplAsmCreate()
{
	Kepler::Asm *as = new Kepler::Asm();
	return (KplAsm *) as;
}

void KplAsmFree(struct KplAsm *self)
{
	Kepler::Asm *as = (Kepler::Asm *) self;
	delete as;
}

void KplAsmDisassembleBinary(struct KplAsm *self, const char *path)
{
	Kepler::Asm *as = (Kepler::Asm *) self;
	as->DisassembleBinary(path);
}



