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

#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>

#include "asm.h"


/* Global instruction info table */
struct kpl_inst_info_t kpl_inst_info[KPL_INST_COUNT];


/* Opcodes and secondary opcodes */
#define KPL_OPCODE_A            	0
#define KPL_OPCODE_B            	1
#define KPL_OPCODE_C            	2

#define KPL_OPCODE_A_A          	0 

#define KPL_OPCODE_B_A          	0
#define KPL_OPCODE_B_B          	1
#define KPL_OPCODE_B_C          	2
#define KPL_OPCODE_B_D          	3

#define KPL_OPCODE_B_C_A        	0
#define KPL_OPCODE_B_C_B        	1
#define KPL_OPCODE_B_C_C        	2
#define KPL_OPCODE_B_C_D        	3

#define KPL_OPCODE_B_C_D_A      	1
#define KPL_OPCODE_B_C_D_B      	2
#define KPL_OPCODE_B_C_D_C      	3

#define KPL_OPCODE_B_C_D_C_A    	1
#define KPL_OPCODE_B_C_D_C_B    	2
#define KPL_OPCODE_B_C_D_C_C    	3

#define KPL_OPCODE_B_D_A        	0

#define KPL_OPCODE_C_A			0
#define KPL_OPCODE_C_B_0		1	
#define KPL_OPCODE_C_B_1		2
#define KPL_OPCODE_C_B_2		3

#define KPL_OPCODE_C_A_A		2
#define KPL_OPCODE_C_A_B		3

#define KPL_OPCODE_C_B_A_0		1
#define KPL_OPCODE_C_B_A_1		9
#define KPL_OPCODE_C_B_B_0		2
#define KPL_OPCODE_C_B_B_1		10
#define KPL_OPCODE_C_B_C_0		3
#define KPL_OPCODE_C_B_C_1		11
#define KPL_OPCODE_C_B_D		12
#define KPL_OPCODE_C_B_E_0  		0
#define KPL_OPCODE_C_B_E_1      	4
#define KPL_OPCODE_C_B_E_2      	5
#define KPL_OPCODE_C_B_E_3      	6
#define KPL_OPCODE_C_B_E_4      	7
#define KPL_OPCODE_C_B_E_5      	13
#define KPL_OPCODE_C_B_E_6      	14	
#define KPL_OPCODE_C_B_E_7              15

#define KPL_OPCODE_C_B_C_A		1
#define KPL_OPCODE_C_B_C_B		2
#define KPL_OPCODE_C_B_C_C		3

#define KPL_OPCODE_C_B_C_C_A		1
#define KPL_OPCODE_C_B_C_C_B		2
#define KPL_OPCODE_C_B_C_C_C		3

#define KPL_OPCODE_C_B_E_A		1
#define KPL_OPCODE_C_B_E_B		2
#define KPL_OPCODE_C_B_E_C		3

#define KPL_OPCODE_C_B_E_A_A		6
#define KPL_OPCODE_C_B_E_A_B		7

#define KPL_OPCODE_C_B_E_A_A_A		4
#define KPL_OPCODE_C_B_E_A_A_B		5
#define KPL_OPCODE_C_B_E_A_A_C		6
#define KPL_OPCODE_C_B_E_A_A_D		7

#define KPL_OPCODE_C_B_E_A_A_C_A	1	

#define KPL_OPCODE_C_B_E_A_B_A		0
#define KPL_OPCODE_C_B_E_A_B_B		1

#define KPL_OPCODE_C_B_E_A_B_A_A	0	
#define KPL_OPCODE_C_B_E_A_B_A_B	1	
#define KPL_OPCODE_C_B_E_A_B_A_C	2

#define KPL_OPCODE_C_B_E_B_A		0
#define KPL_OPCODE_C_B_E_B_B		1

#define KPL_OPCODE_C_B_E_B_A_A		1


/* Pointers to the tables of instructions */

struct kpl_inst_table_entry_t *kpl_asm_table;
struct kpl_inst_table_entry_t *kpl_asm_table_a;
struct kpl_inst_table_entry_t *kpl_asm_table_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c;

struct kpl_inst_table_entry_t *kpl_asm_table_a_a;

struct kpl_inst_table_entry_t *kpl_asm_table_b_a;
struct kpl_inst_table_entry_t *kpl_asm_table_b_b;
struct kpl_inst_table_entry_t *kpl_asm_table_b_c;
struct kpl_inst_table_entry_t *kpl_asm_table_b_d;

struct kpl_inst_table_entry_t *kpl_asm_table_b_c_a;
struct kpl_inst_table_entry_t *kpl_asm_table_b_c_b;
struct kpl_inst_table_entry_t *kpl_asm_table_b_c_c;
struct kpl_inst_table_entry_t *kpl_asm_table_b_c_d;

struct kpl_inst_table_entry_t *kpl_asm_table_b_c_d_a;
struct kpl_inst_table_entry_t *kpl_asm_table_b_c_d_b;
struct kpl_inst_table_entry_t *kpl_asm_table_b_c_d_c;

struct kpl_inst_table_entry_t *kpl_asm_table_b_c_d_c_a;
struct kpl_inst_table_entry_t *kpl_asm_table_b_c_d_c_b;
struct kpl_inst_table_entry_t *kpl_asm_table_b_c_d_c_c;

struct kpl_inst_table_entry_t *kpl_asm_table_b_d_a;

struct kpl_inst_table_entry_t *kpl_asm_table_c_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b;

struct kpl_inst_table_entry_t *kpl_asm_table_c_a_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_a_b;

struct kpl_inst_table_entry_t *kpl_asm_table_c_b_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_c;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_d;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e;

struct kpl_inst_table_entry_t *kpl_asm_table_c_b_c_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_c_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_c_c;

struct kpl_inst_table_entry_t *kpl_asm_table_c_b_c_c_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_c_c_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_c_c_c;

struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_c;

struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_b;

struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_a_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_a_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_a_c;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_a_d;

struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_a_c_a;

struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_b_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_b_b;

struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_b_a_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_b_a_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_a_b_a_c;

struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_b_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_b_b;

struct kpl_inst_table_entry_t *kpl_asm_table_c_b_e_b_a_a;


static void kpl_asm_table_init(enum kpl_inst_opcode_t opcode, char *name,
		char *fmt_str, int num_args, ...)
{
	struct kpl_inst_table_entry_t *table;
	va_list ap;

	int index;
	int i;

	/* Initialize instruction info table */
	kpl_inst_info[opcode].opcode = opcode;
	kpl_inst_info[opcode].name = name;
	kpl_inst_info[opcode].fmt_str = fmt_str;

	/* Initialize argument list */
	va_start(ap, num_args);
	table = kpl_asm_table;

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
	table[index].info = &kpl_inst_info[opcode];
}

/*
 * Public Functions
 */


/* Build table of all the instructions */
void kpl_asm_init(void)
{
	/* Allocate storage for the instruction tables */
	kpl_asm_table =			xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_a =		xcalloc(8, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_a_a =		xcalloc(64, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_b_a =		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_b =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_d =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_b_c_a =		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_b =		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_c =		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_d =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_b_c_d_a =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_d_b =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_d_c =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_b_c_d_c_a =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_d_c_b =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_d_c_c =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_b_d_a =		xcalloc(32, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_a =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b =		xcalloc(16, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_a_a =		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_a_b =		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_b_a =		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_b =		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_c =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_d =		xcalloc(32, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_e =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_b_c_a =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_c_b =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_c_c =		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_b_c_c_a =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_c_c_b =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_c_c_c =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_b_e_a =		xcalloc(8, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_e_b =		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_e_c =		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_b_e_a_a =	xcalloc(8, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_e_a_b =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_b_e_a_a_a =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_e_a_a_b =	xcalloc(8, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_e_a_a_c =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_e_a_a_d =	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_b_e_a_a_c_a =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_b_e_a_b_a =	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_e_a_b_b =	xcalloc(16, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_b_e_a_b_a_a =	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_e_a_b_a_b =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_e_a_b_a_c =	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_b_e_b_a =	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b_e_b_b =	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_b_e_b_a_a =	xcalloc(16, sizeof(struct kpl_inst_table_entry_t));



	/* Initiate values for the 'next_table', 'next_table_low' and 'next_table_high'
	 * fields of the tables */


	kpl_asm_table[KPL_OPCODE_A].next_table =
		kpl_asm_table_a;
	kpl_asm_table[KPL_OPCODE_A].next_table_low                            	= 61;
	kpl_asm_table[KPL_OPCODE_A].next_table_high                           	= 63;

	kpl_asm_table[KPL_OPCODE_B].next_table =
		kpl_asm_table_b;
	kpl_asm_table[KPL_OPCODE_B].next_table_low                            	= 62;
	kpl_asm_table[KPL_OPCODE_B].next_table_high                           	= 63;

	kpl_asm_table[KPL_OPCODE_C].next_table =
		kpl_asm_table_c;
	kpl_asm_table[KPL_OPCODE_C].next_table_low                            	= 62;
	kpl_asm_table[KPL_OPCODE_C].next_table_high                           	= 63;

	kpl_asm_table_a[KPL_OPCODE_A_A].next_table =
		kpl_asm_table_a_a;
	kpl_asm_table_a[KPL_OPCODE_A_A].next_table_low                     	= 55;
	kpl_asm_table_a[KPL_OPCODE_A_A].next_table_high                      	= 60;

	kpl_asm_table_b[KPL_OPCODE_B_A].next_table =
		kpl_asm_table_b_a;
	kpl_asm_table_b[KPL_OPCODE_B_A].next_table_low                        	= 61;
	kpl_asm_table_b[KPL_OPCODE_B_A].next_table_high                       	= 61;

	kpl_asm_table_b[KPL_OPCODE_B_B].next_table =
		kpl_asm_table_b_b;
	kpl_asm_table_b[KPL_OPCODE_B_B].next_table_low                        	= 60;
	kpl_asm_table_b[KPL_OPCODE_B_B].next_table_high                       	= 61;

	kpl_asm_table_b[KPL_OPCODE_B_C].next_table =
		kpl_asm_table_b_c;
	kpl_asm_table_b[KPL_OPCODE_B_C].next_table_low                         	= 60;
	kpl_asm_table_b[KPL_OPCODE_B_C].next_table_high                       	= 61;

	kpl_asm_table_b[KPL_OPCODE_B_D].next_table =
		kpl_asm_table_b_d;
	kpl_asm_table_b[KPL_OPCODE_B_D].next_table_low                        	= 60;
	kpl_asm_table_b[KPL_OPCODE_B_D].next_table_high                       	= 61;

	kpl_asm_table_b_c[KPL_OPCODE_B_C_A].next_table =
		kpl_asm_table_b_c_a;
	kpl_asm_table_b_c[KPL_OPCODE_B_C_A].next_table_low                   	= 58;
	kpl_asm_table_b_c[KPL_OPCODE_B_C_A].next_table_high                	= 58;

	kpl_asm_table_b_c[KPL_OPCODE_B_C_B].next_table =
		kpl_asm_table_b_c_b;
	kpl_asm_table_b_c[KPL_OPCODE_B_C_B].next_table_low                  	= 58;
	kpl_asm_table_b_c[KPL_OPCODE_B_C_B].next_table_high                 	= 58;

	kpl_asm_table_b_c[KPL_OPCODE_B_C_C].next_table =
		kpl_asm_table_b_c_c;
	kpl_asm_table_b_c[KPL_OPCODE_B_C_C].next_table_low                 	= 58;
	kpl_asm_table_b_c[KPL_OPCODE_B_C_C].next_table_high                  	= 58;

	kpl_asm_table_b_c[KPL_OPCODE_B_C_D].next_table =
		kpl_asm_table_b_c_d;
	kpl_asm_table_b_c[KPL_OPCODE_B_C_D].next_table_low                  	= 57;
	kpl_asm_table_b_c[KPL_OPCODE_B_C_D].next_table_high                 	= 58;

	kpl_asm_table_b_c_d[KPL_OPCODE_B_C_D_A].next_table =
		kpl_asm_table_b_c_d_a;
	kpl_asm_table_b_c_d[KPL_OPCODE_B_C_D_A].next_table_low              	= 55;
	kpl_asm_table_b_c_d[KPL_OPCODE_B_C_D_A].next_table_high              	= 56;

	kpl_asm_table_b_c_d[KPL_OPCODE_B_C_D_B].next_table =
		kpl_asm_table_b_c_d_b;
	kpl_asm_table_b_c_d[KPL_OPCODE_B_C_D_B].next_table_low               	= 55;
	kpl_asm_table_b_c_d[KPL_OPCODE_B_C_D_B].next_table_high              	= 56;

	kpl_asm_table_b_c_d[KPL_OPCODE_B_C_D_C].next_table =
		kpl_asm_table_b_c_d_c;
	kpl_asm_table_b_c_d[KPL_OPCODE_B_C_D_C].next_table_low              	= 55;
	kpl_asm_table_b_c_d[KPL_OPCODE_B_C_D_C].next_table_high               	= 56;

	kpl_asm_table_b_c_d_c[KPL_OPCODE_B_C_D_C_A].next_table =
		kpl_asm_table_b_c_d_c_a;
	kpl_asm_table_b_c_d_c[KPL_OPCODE_B_C_D_C_A].next_table_low          	= 54;
	kpl_asm_table_b_c_d_c[KPL_OPCODE_B_C_D_C_A].next_table_high           	= 54;

	kpl_asm_table_b_c_d_c[KPL_OPCODE_B_C_D_C_B].next_table =
		kpl_asm_table_b_c_d_c_b;
	kpl_asm_table_b_c_d_c[KPL_OPCODE_B_C_D_C_B].next_table_low        	= 54;
	kpl_asm_table_b_c_d_c[KPL_OPCODE_B_C_D_C_B].next_table_high            	= 54;

	kpl_asm_table_b_c_d_c[KPL_OPCODE_B_C_D_C_C].next_table =
		kpl_asm_table_b_c_d_c_c;
	kpl_asm_table_b_c_d_c[KPL_OPCODE_B_C_D_C_C].next_table_low          	= 54;
	kpl_asm_table_b_c_d_c[KPL_OPCODE_B_C_D_C_C].next_table_high         	= 54;

	kpl_asm_table_b_d[KPL_OPCODE_B_D_A].next_table =
		kpl_asm_table_b_d_a;
	kpl_asm_table_b_d[KPL_OPCODE_B_D_A].next_table_low              	= 54;
	kpl_asm_table_b_d[KPL_OPCODE_B_D_A].next_table_high                    	= 58;



	kpl_asm_table_c[KPL_OPCODE_C_A].next_table =
		kpl_asm_table_c_a;
	kpl_asm_table_c[KPL_OPCODE_C_A].next_table_low                        	= 60;
	kpl_asm_table_c[KPL_OPCODE_C_A].next_table_high                        	= 61;

	kpl_asm_table_c[KPL_OPCODE_C_B_0].next_table =
		kpl_asm_table_c_b;
	kpl_asm_table_c[KPL_OPCODE_C_B_0].next_table_low                      	= 59;
	kpl_asm_table_c[KPL_OPCODE_C_B_0].next_table_high                     	= 62;

	kpl_asm_table_c[KPL_OPCODE_C_B_1].next_table =
		kpl_asm_table_c_b;
	kpl_asm_table_c[KPL_OPCODE_C_B_1].next_table_low                     	= 59;
	kpl_asm_table_c[KPL_OPCODE_C_B_1].next_table_high                  	= 62;

	kpl_asm_table_c[KPL_OPCODE_C_B_2].next_table =
		kpl_asm_table_c_b;
	kpl_asm_table_c[KPL_OPCODE_C_B_2].next_table_low                      	= 59;
	kpl_asm_table_c[KPL_OPCODE_C_B_2].next_table_high                   	= 62;

	kpl_asm_table_c_a[KPL_OPCODE_C_A_A].next_table =
		kpl_asm_table_c_a_a;
	kpl_asm_table_c_a[KPL_OPCODE_C_A_A].next_table_low                 	= 59;
	kpl_asm_table_c_a[KPL_OPCODE_C_A_A].next_table_high               	= 59;

	kpl_asm_table_c_a[KPL_OPCODE_C_A_B].next_table =
		kpl_asm_table_c_a_b;
	kpl_asm_table_c_a[KPL_OPCODE_C_A_B].next_table_low                  	= 59;
	kpl_asm_table_c_a[KPL_OPCODE_C_A_B].next_table_high                   	= 59;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_A_0].next_table =
		kpl_asm_table_c_b_a;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_A_0].next_table_low                	= 58;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_A_0].next_table_high                 	= 58;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_A_1].next_table =
		kpl_asm_table_c_b_a;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_A_1].next_table_low                  	= 58;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_A_1].next_table_high                	= 58;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_B_0].next_table =
		kpl_asm_table_c_b_b;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_B_0].next_table_low                	= 58;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_B_0].next_table_high             	= 58;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_B_1].next_table =
		kpl_asm_table_c_b_b;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_B_1].next_table_low                	= 58;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_B_1].next_table_high               	= 58;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_C_0].next_table =
		kpl_asm_table_c_b_c;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_C_0].next_table_low                	= 57;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_C_0].next_table_high                 	= 58;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_C_1].next_table =
		kpl_asm_table_c_b_c;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_C_1].next_table_low                 	= 57;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_C_1].next_table_high               	= 58;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_D].next_table =
		kpl_asm_table_c_b_d;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_D].next_table_low                   	= 54;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_D].next_table_high                	= 58;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_0].next_table =
		kpl_asm_table_c_b_e;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_0].next_table_low                 	= 62;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_0].next_table_high                	= 63;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_1].next_table =
		kpl_asm_table_c_b_e;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_1].next_table_low                 	= 62;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_1].next_table_high               	= 63;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_2].next_table =
		kpl_asm_table_c_b_e;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_2].next_table_low                	= 62;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_2].next_table_high              	= 63;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_3].next_table =
		kpl_asm_table_c_b_e;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_3].next_table_low               	= 62;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_3].next_table_high                 	= 63;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_4].next_table =
		kpl_asm_table_c_b_e;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_4].next_table_low                	= 62;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_4].next_table_high                	= 63;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_5].next_table =
		kpl_asm_table_c_b_e;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_5].next_table_low                  	= 62;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_5].next_table_high                	= 63;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_6].next_table =
		kpl_asm_table_c_b_e;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_6].next_table_low                 	= 62;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_6].next_table_high               	= 63;

	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_7].next_table =
		kpl_asm_table_c_b_e;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_7].next_table_low                    = 62;
	kpl_asm_table_c_b[KPL_OPCODE_C_B_E_7].next_table_high                   = 63;

	kpl_asm_table_c_b_c[KPL_OPCODE_C_B_C_A].next_table =
		kpl_asm_table_c_b_c_a;
	kpl_asm_table_c_b_c[KPL_OPCODE_C_B_C_A].next_table_low                	= 55;
	kpl_asm_table_c_b_c[KPL_OPCODE_C_B_C_A].next_table_high               	= 56;

	kpl_asm_table_c_b_c[KPL_OPCODE_C_B_C_B].next_table =
		kpl_asm_table_c_b_c_b;
	kpl_asm_table_c_b_c[KPL_OPCODE_C_B_C_B].next_table_low                	= 55;
	kpl_asm_table_c_b_c[KPL_OPCODE_C_B_C_B].next_table_high                	= 56;

	kpl_asm_table_c_b_c[KPL_OPCODE_C_B_C_C].next_table =
		kpl_asm_table_c_b_c_c;
	kpl_asm_table_c_b_c[KPL_OPCODE_C_B_C_C].next_table_low                	= 56;
	kpl_asm_table_c_b_c[KPL_OPCODE_C_B_C_C].next_table_high               	= 55;

	kpl_asm_table_c_b_c_c[KPL_OPCODE_C_B_C_C_A].next_table =
		kpl_asm_table_c_b_c_c_a;
	kpl_asm_table_c_b_c_c[KPL_OPCODE_C_B_C_C_A].next_table_low             	= 54;
	kpl_asm_table_c_b_c_c[KPL_OPCODE_C_B_C_C_A].next_table_high            	= 54;

	kpl_asm_table_c_b_c_c[KPL_OPCODE_C_B_C_C_B].next_table =
		kpl_asm_table_c_b_c_c_b;
	kpl_asm_table_c_b_c_c[KPL_OPCODE_C_B_C_C_B].next_table_low            	= 54;
	kpl_asm_table_c_b_c_c[KPL_OPCODE_C_B_C_C_B].next_table_high           	= 54;

	kpl_asm_table_c_b_c_c[KPL_OPCODE_C_B_C_C_C].next_table =
		kpl_asm_table_c_b_c_c_c;
	kpl_asm_table_c_b_c_c[KPL_OPCODE_C_B_C_C_C].next_table_low          	= 54;
	kpl_asm_table_c_b_c_c[KPL_OPCODE_C_B_C_C_C].next_table_high         	= 54;

	kpl_asm_table_c_b_e[KPL_OPCODE_C_B_E_A].next_table =
		kpl_asm_table_c_b_e_a;
	kpl_asm_table_c_b_e[KPL_OPCODE_C_B_E_A].next_table_low               	= 59;
	kpl_asm_table_c_b_e[KPL_OPCODE_C_B_E_A].next_table_high             	= 61;

	kpl_asm_table_c_b_e[KPL_OPCODE_C_B_E_B].next_table =
		kpl_asm_table_c_b_e_b;
	kpl_asm_table_c_b_e[KPL_OPCODE_C_B_E_B].next_table_low            	= 61;
	kpl_asm_table_c_b_e[KPL_OPCODE_C_B_E_B].next_table_high             	= 61;

	kpl_asm_table_c_b_e[KPL_OPCODE_C_B_E_C].next_table =
		kpl_asm_table_c_b_e_c;
	kpl_asm_table_c_b_e[KPL_OPCODE_C_B_E_C].next_table_low               	= 59;
	kpl_asm_table_c_b_e[KPL_OPCODE_C_B_E_C].next_table_high             	= 59;

	kpl_asm_table_c_b_e_a[KPL_OPCODE_C_B_E_A_A].next_table =
		kpl_asm_table_c_b_e_a_a;
	kpl_asm_table_c_b_e_a[KPL_OPCODE_C_B_E_A_A].next_table_low            	= 56;
	kpl_asm_table_c_b_e_a[KPL_OPCODE_C_B_E_A_A].next_table_high          	= 58;

	kpl_asm_table_c_b_e_a[KPL_OPCODE_C_B_E_A_B].next_table =
		kpl_asm_table_c_b_e_a_b;
	kpl_asm_table_c_b_e_a[KPL_OPCODE_C_B_E_A_B].next_table_low           	= 58;
	kpl_asm_table_c_b_e_a[KPL_OPCODE_C_B_E_A_B].next_table_high         	= 58;

	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_A].next_table =
		kpl_asm_table_c_b_e_a_a_a;
	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_A].next_table_low        	= 55;
	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_A].next_table_high      	= 55;

	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_B].next_table =
		kpl_asm_table_c_b_e_a_a_b;
	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_B].next_table_low      	= 53;
	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_B].next_table_high      	= 55;

	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_C].next_table =
		kpl_asm_table_c_b_e_a_a_c;
	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_C].next_table_low       	= 55;
	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_C].next_table_high       	= 55;

	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_D].next_table =
		kpl_asm_table_c_b_e_a_a_d;
	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_D].next_table_low       	= 54;
	kpl_asm_table_c_b_e_a_a[KPL_OPCODE_C_B_E_A_A_D].next_table_high     	= 55;

	kpl_asm_table_c_b_e_a_a_c[KPL_OPCODE_C_B_E_A_A_C_A].next_table =
		kpl_asm_table_c_b_e_a_a_c_a;
	kpl_asm_table_c_b_e_a_a_c[KPL_OPCODE_C_B_E_A_A_C_A].next_table_low     	= 54;
	kpl_asm_table_c_b_e_a_a_c[KPL_OPCODE_C_B_E_A_A_C_A].next_table_high 	= 54;

	kpl_asm_table_c_b_e_a_b[KPL_OPCODE_C_B_E_A_B_A].next_table =
		kpl_asm_table_c_b_e_a_b_a;
	kpl_asm_table_c_b_e_a_b[KPL_OPCODE_C_B_E_A_B_A].next_table_low      	= 56;
	kpl_asm_table_c_b_e_a_b[KPL_OPCODE_C_B_E_A_B_A].next_table_high     	= 57;

	kpl_asm_table_c_b_e_a_b[KPL_OPCODE_C_B_E_A_B_B].next_table =
		kpl_asm_table_c_b_e_a_b_b;
	kpl_asm_table_c_b_e_a_b[KPL_OPCODE_C_B_E_A_B_B].next_table_low        	= 54;
	kpl_asm_table_c_b_e_a_b[KPL_OPCODE_C_B_E_A_B_B].next_table_high      	= 57;

	kpl_asm_table_c_b_e_a_b_a[KPL_OPCODE_C_B_E_A_B_A_A].next_table =
		kpl_asm_table_c_b_e_a_b_a_a;
	kpl_asm_table_c_b_e_a_b_a[KPL_OPCODE_C_B_E_A_B_A_A].next_table_low 	= 54;
	kpl_asm_table_c_b_e_a_b_a[KPL_OPCODE_C_B_E_A_B_A_A].next_table_high  	= 55;

	kpl_asm_table_c_b_e_a_b_a[KPL_OPCODE_C_B_E_A_B_A_B].next_table =
		kpl_asm_table_c_b_e_a_b_a_b;
	kpl_asm_table_c_b_e_a_b_a[KPL_OPCODE_C_B_E_A_B_A_B].next_table_low   	= 55;
	kpl_asm_table_c_b_e_a_b_a[KPL_OPCODE_C_B_E_A_B_A_B].next_table_high   	= 55;

	kpl_asm_table_c_b_e_a_b_a[KPL_OPCODE_C_B_E_A_B_A_C].next_table =
		kpl_asm_table_c_b_e_a_b_a_c;
	kpl_asm_table_c_b_e_a_b_a[KPL_OPCODE_C_B_E_A_B_A_C].next_table_low    	= 54;
	kpl_asm_table_c_b_e_a_b_a[KPL_OPCODE_C_B_E_A_B_A_C].next_table_high  	= 55;

	kpl_asm_table_c_b_e_b[KPL_OPCODE_C_B_E_B_A].next_table =
		kpl_asm_table_c_b_e_b_a;
	kpl_asm_table_c_b_e_b[KPL_OPCODE_C_B_E_B_A].next_table_low         	= 58;
	kpl_asm_table_c_b_e_b[KPL_OPCODE_C_B_E_B_A].next_table_high         	= 58;

	kpl_asm_table_c_b_e_b[KPL_OPCODE_C_B_E_B_B].next_table =
		kpl_asm_table_c_b_e_b_b;
	kpl_asm_table_c_b_e_b[KPL_OPCODE_C_B_E_B_B].next_table_low           	= 59;
	kpl_asm_table_c_b_e_b[KPL_OPCODE_C_B_E_B_B].next_table_high          	= 60;

	kpl_asm_table_c_b_e_b_a[KPL_OPCODE_C_B_E_B_A_A].next_table =
		kpl_asm_table_c_b_e_b_a_a;
	kpl_asm_table_c_b_e_b_a[KPL_OPCODE_C_B_E_B_A_A].next_table_low      	= 54;
	kpl_asm_table_c_b_e_b_a[KPL_OPCODE_C_B_E_B_A_A].next_table_high      	= 57;


#define DEFINST(_name, _fmt_str, ...) \
	kpl_asm_table_init(KPL_INST_##_name, #_name, _fmt_str, PP_NARG(__VA_ARGS__), __VA_ARGS__);

#include "asm.dat" 
#undef DEFINST
}


void kpl_inst_decode(struct kpl_inst_t *inst)
{
	struct kpl_inst_table_entry_t *table;
	int index;
	int low;
	int high;

	/* Start with master table */
	table = kpl_asm_table;
	low = 0;
	high = 1;

	/* Traverse tables */
	while (1)
	{
		index = BITS64(inst->dword.as_dword, high, low);
		if (!table[index].next_table)
		{
			inst->info = table[index].info;
			return;
		}

		/* Go to next table */
		low = table[index].next_table_low;
		high = table[index].next_table_high;
		table = table[index].next_table;
	}
}


/*************************************************************************/
void kpl_inst_hex_dump(FILE *f, void *inst_ptr, unsigned int inst_addr)
{
	fprintf(f, "\n\t/*%04x*/     /*%08x%08x*/ \t", inst_addr,
			*(unsigned int *) inst_ptr,
			*(unsigned int *) (inst_ptr + 4));
}

/*************************************************************************/
#if 0
static int kpl_token_comp(char *fmt_str, char *token_str, int *token_len)
{
	*token_len = strlen(token_str);
	return !strncmp(fmt_str, token_str, *token_len) &&
		!isalnum(fmt_str[*token_len]);
	return 0;
}
#endif

/********************************************************************/
struct str_map_t kpl_inst_sat_map =
{
	2,
	{
		{ "", 0},
		{ ".SAT", 1}
	}
};

struct str_map_t kpl_inst_x_map =
{
	2,
	{
		{ "", 0},
		{ ".X", 1}
	}
};

struct str_map_t kpl_inst_cc_map =
{
	2,
	{
		{ "", 0},
		{ ".CC", 1}
	}
};

struct str_map_t kpl_inst_hi_map =
{
	2,
	{
		{ "", 0},
		{ ".HI", 1}
	}
};

struct str_map_t kpl_inst_keeprefcount_map =
{
	2,
	{
		{ "", 0},
		{ ".KEEPREFCOUNT", 1}
	}
};

struct str_map_t kpl_inst_s_map =
{
	2,
	{
		{ "", 0},
		{ ".S", 1}
	}
};

struct str_map_t kpl_inst_and_map =
{
	4,
	{
		{ ".AND", 0},
		{ ".OR", 1},
		{ ".XOR", 2},
		{ ".INVALIDBOP3", 3}
	}
};

struct str_map_t kpl_inst_cv_map =
{
	4,
	{
		{ "", 0},
		{ ".CG", 1},
		{ ".CS", 2},
		{ ".CV", 3}
	}
};

struct str_map_t kpl_inst_wt_map =
{
	4,
	{
		{ "", 0},
		{ ".CG", 1},
		{ ".CS", 2},
		{ ".WT", 3}
	}
};

struct str_map_t kpl_inst_po_map =
{
	2,
	{
		{ "", 0},
		{ ".PO", 1}
	}
};

struct str_map_t kpl_inst_u1_map =
{
	2,
	{
		{ ".U32.U32", 0},
		{ ".U32.S32", 1}
	}
};

struct str_map_t kpl_inst_f_map=
{
	8,
	{	
		{ ".F", 0},
		{ ".LT", 1},
		{ ".EQ", 2},
		{ ".LE", 3},
		{ ".GT", 4},
		{ ".NE", 5},
		{ ".GE", 6},
		{ ".T", 7}
	}
};

struct str_map_t kpl_inst_u_map =
{
	2,
	{
		{ "", 0},
		{ ".U", 1}
	}
};

struct str_map_t kpl_inst_lmt_map =
{
	2,
	{
		{ "", 0},
		{ ".LMT", 1}
	}
};

struct str_map_t kpl_inst_e_map =
{
	2,
	{
		{ "", 0},
		{ ".E", 1}
	}
};

struct str_map_t kpl_inst_u32_map =
{
	2,
	{
		{ ".U32", 0},
		{ "", 1}
	}
};

struct str_map_t kpl_inst_rm_map =
{
	4,
	{
		{ "", 0},
		{ ".RM", 1},
		{ ".RP", 2},
		{ ".RZ", 3}
	}
};

struct str_map_t kpl_inst_us_map =
{
	2,
	{
		{ ".U32", 0},
		{ ".S32", 1},
	}
};

/*************************************************************************************/

void kpl_inst_dump_PRED(struct kpl_inst_t *inst, char **str_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(inst->dword.as_dword, high, low);
	if (value != 7)
	{
		str_printf(str_ptr, size_ptr, "@");
		if (value >> 3)
			str_printf(str_ptr, size_ptr, "!");
		if (value == 15)
			str_printf(str_ptr, size_ptr, "PT ");
		else
			str_printf(str_ptr, size_ptr, "P%d ", value & 7);
	}
}

void kpl_inst_dump_REG(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(inst->dword.as_dword, high, low);
	if(value == 255)
		str_printf(str_ptr, size_ptr, "RZ");
	else
		str_printf(str_ptr, size_ptr, "R%d", value);
}

void kpl_inst_dump_S(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(inst->dword.as_dword, high, low);
	str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_s_map, value));
}

void kpl_inst_dump_F(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(inst->dword.as_dword, high, low);
	str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_f_map, value));
}

void kpl_inst_dump_AND(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(inst->dword.as_dword, high, low);
	str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_and_map, value));
}

void kpl_inst_dump_X(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(inst->dword.as_dword, high, low);
	str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_x_map, value));
}

void kpl_inst_dump_U32(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(inst->dword.as_dword, high, low);
	str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_u32_map, value));
}

void kpl_inst_dump_HI(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(inst->dword.as_dword, high, low);
	str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_hi_map, value));
}

void kpl_inst_dump_SAT(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(inst->dword.as_dword, high, low);
	str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_sat_map, value));
}

void kpl_inst_dump_PO(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(inst->dword.as_dword, high, low);
	str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_po_map, value));
}

void kpl_inst_dump_US(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high0, int low0, int high1, int low1)
{
	int value0;
	int value1; 

	value0 = BITS64(inst->dword.as_dword, high0, low0);
	value1 = BITS64(inst->dword.as_dword, high1, low1);

	if(value0 == 1 && value1 == 1)
		str_printf(str_ptr, size_ptr, "%s", "");
	else
	{
		str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_us_map, value1));
		str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_us_map, value0));
	}
}

void kpl_inst_dump_CC(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(inst->dword.as_dword, high, low);
        str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_cc_map, value));
}

void kpl_inst_dump_E(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(inst->dword.as_dword, high, low);
        str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_e_map, value));
}

void kpl_inst_dump_CV(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(inst->dword.as_dword, high, low);
        str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_cv_map, value));
}

void kpl_inst_dump_LMT(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(inst->dword.as_dword, high, low);
        str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_lmt_map, value));
}

void kpl_inst_dump_U(struct kpl_inst_t * inst, char **str_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(inst->dword.as_dword, high, low);
        str_printf(str_ptr, size_ptr, "%s", str_map_value(&kpl_inst_u_map, value));
}

/*************************************************************************/

void kpl_inst_dump_buf(struct kpl_inst_t *inst, char *str, int size)
{
	char *fmt_str;

	/* Decode the instruction */
	kpl_inst_decode(inst);

	/* Invalid instruction */
	if (!inst->info || !inst->info->fmt_str)
	{
		snprintf(str, size, "<unknown>");
		return;
	}

	/* Print entire format string temporarily */
	fmt_str = inst->info->fmt_str;
	while (*fmt_str)
	{
                if (str_prefix(fmt_str, "%_LMT"))
                {
                        // Extract corresponding fields of the instruction and print them as the predicate

                        kpl_inst_dump_LMT(inst, &str, &size, 8, 8);
                        fmt_str += 5;
                        continue;
                } 
         
	       if (str_prefix(fmt_str, "%_cg"))
                {
                        // Extract corresponding fields of the instruction and print them as the predicate

                        kpl_inst_dump_CV(inst, &str, &size, 50, 50);
                        fmt_str += 4;
                        continue;
                } 

                if (str_prefix(fmt_str, "%_e"))
                {
                        // Extract corresponding fields of the instruction and print them as the predicate

                        kpl_inst_dump_CC(inst, &str, &size, 60, 59);
                        fmt_str += 3;
                        continue;
                } 
                
		if (str_prefix(fmt_str, "%_cc"))
                {
                        // Extract corresponding fields of the instruction and print them as the predicate

                        kpl_inst_dump_CC(inst, &str, &size, 50, 50);
                        fmt_str += 4;
                        continue;
                } 

		if (str_prefix(fmt_str, "%_us"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_US(inst, &str, &size, 56, 56, 51, 51);
			fmt_str += 4;
			continue;
		}

		if (str_prefix(fmt_str, "%_po"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_PO(inst, &str, &size, 55, 55);
			fmt_str += 4;
			continue;
		}

		if (str_prefix(fmt_str, "%_sat"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_SAT(inst, &str, &size, 53, 53);
			fmt_str += 5;
			continue;
		}

		if (str_prefix(fmt_str, "%_hi"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_HI(inst, &str, &size, 57, 57);
			fmt_str += 4;
			continue;
		}

		if (str_prefix(fmt_str, "%_U32"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_U32(inst, &str, &size, 51, 51);
			fmt_str += 5;
			continue;
		}

		if (str_prefix(fmt_str, "%_x"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_X(inst, &str, &size, 46, 46);
			fmt_str += 3;
			continue;
		}

		if (str_prefix(fmt_str, "%_and"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_AND(inst, &str, &size, 49, 48);
			fmt_str += 5;
			continue;
		}

		if (str_prefix(fmt_str, "%_f"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_F(inst, &str, &size, 54, 52);
			fmt_str += 3;
			continue;
		}

		if (str_prefix(fmt_str, "%_s"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_S(inst, &str, &size, 22, 22);
			fmt_str += 3;
			continue;
		}                

                if (str_prefix(fmt_str, "%_u"))
                {
                        // Extract corresponding fields of the instruction and print them as the predicate

                        kpl_inst_dump_U(inst, &str, &size, 9, 9);
                        fmt_str += 3;
                        continue;
                }

		if (str_prefix(fmt_str, "%srcA"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_REG(inst, &str, &size, 17, 10);
			fmt_str += 5;
			continue;
		}                

		if (str_prefix(fmt_str, "%dst"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_REG(inst, &str, &size, 9, 2);
			fmt_str += 4;
			continue;
		}

		if (str_prefix(fmt_str, "%pred0"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_PRED(inst, &str, &size, 21, 18);
			fmt_str += 6;
			continue;
		}

		if (str_prefix(fmt_str, "%pred"))
		{
			// Extract corresponding fields of the instruction and print them as the predicate

			kpl_inst_dump_PRED(inst, &str, &size, 21, 18);
			//str_printf(&str, &size, "HELLO");
			fmt_str += 5;
			continue;
		}

		/* Print literal character */
		str_printf(&str, &size, "%c", *fmt_str);
		++fmt_str;
	}
}


/********************************************************************/


void kpl_disasm(char *path)
{
	void *inst_ptr;

	struct elf_section_t *section; 
	struct elf_file_t *elf_file;
	struct kpl_inst_t inst;

	char *title;
	char inst_str[MAX_STRING_SIZE];
	int i;


	/* Initializations - build tables of instructions */
	kpl_asm_init();

	/* Create an elf file from the executable */
	elf_file = elf_file_create_from_path(path);

	/* Read Sections */
	for (i=0; i < list_count(elf_file->section_list); i++)
	{
		/* Get section and skip it if it does not contain code */
		section = list_get(elf_file->section_list, i);
		if (!(section->header->sh_flags & SHF_EXECINSTR))
			continue;

		/* Set section name (get rid of .text.) */
		title = section->name;
		const int len = 22;
		const char* new_title = title +len - 16;

		/* Title */
		printf("\n\tcode for sm_35");
		printf("\n\t\tFunction : %s", new_title);

		/* Decode and dump instructions */
		for (inst_ptr = section->buffer.ptr; inst_ptr < section->buffer.ptr +
				section->buffer.size; inst_ptr += 8)
		{
			kpl_inst_hex_dump(stdout, inst_ptr, section->header->sh_addr + section->buffer.pos);

			inst.dword.as_dword = * (unsigned long long *) inst_ptr;
			kpl_inst_dump_buf(&inst, inst_str, sizeof inst_str);
			printf("%s", inst_str);

			/* Move to next instruction */
			section->buffer.pos += 8;
		}

		//kpl_disasm_buffer(stdout, sizeof(section));

	}
	printf("\n");
	elf_file_free(elf_file);
	kpl_asm_done();
}


/********************************************************************/
void kpl_asm_done(void)
{
	free(kpl_asm_table);
	free(kpl_asm_table_a);
	free(kpl_asm_table_b);
	free(kpl_asm_table_c);

	free(kpl_asm_table_a_a);

	free(kpl_asm_table_b_a);
	free(kpl_asm_table_b_b);
	free(kpl_asm_table_b_c);
	free(kpl_asm_table_b_d);

	free(kpl_asm_table_b_c_a);
	free(kpl_asm_table_b_c_b);
	free(kpl_asm_table_b_c_c);
	free(kpl_asm_table_b_c_d);

	free(kpl_asm_table_b_c_d_a);
	free(kpl_asm_table_b_c_d_b);
	free(kpl_asm_table_b_c_d_c);

	free(kpl_asm_table_b_c_d_c_a);
	free(kpl_asm_table_b_c_d_c_b);
	free(kpl_asm_table_b_c_d_c_c);

	free(kpl_asm_table_b_d_a);

	free(kpl_asm_table_c_a);
	free(kpl_asm_table_c_b);

	free(kpl_asm_table_c_a_a);
	free(kpl_asm_table_c_a_b);

	free(kpl_asm_table_c_b_a);
	free(kpl_asm_table_c_b_b);
	free(kpl_asm_table_c_b_c);
	free(kpl_asm_table_c_b_d);
	free(kpl_asm_table_c_b_e);

	free(kpl_asm_table_c_b_c_a);
	free(kpl_asm_table_c_b_c_b);
	free(kpl_asm_table_c_b_c_c);

	free(kpl_asm_table_c_b_c_c_a);
	free(kpl_asm_table_c_b_c_c_b);
	free(kpl_asm_table_c_b_c_c_c);

	free(kpl_asm_table_c_b_e_a);
	free(kpl_asm_table_c_b_e_b);
	free(kpl_asm_table_c_b_e_c);

	free(kpl_asm_table_c_b_e_a_a);
	free(kpl_asm_table_c_b_e_a_b);

	free(kpl_asm_table_c_b_e_a_a_a);
	free(kpl_asm_table_c_b_e_a_a_b);
	free(kpl_asm_table_c_b_e_a_a_c);
	free(kpl_asm_table_c_b_e_a_a_d);

	free(kpl_asm_table_c_b_e_a_a_c_a);

	free(kpl_asm_table_c_b_e_a_b_a);
	free(kpl_asm_table_c_b_e_a_b_b);

	free(kpl_asm_table_c_b_e_a_b_a_a);
	free(kpl_asm_table_c_b_e_a_b_a_b);
	free(kpl_asm_table_c_b_e_a_b_a_c);

	free(kpl_asm_table_c_b_e_b_a);
	free(kpl_asm_table_c_b_e_b_b);

	free(kpl_asm_table_c_b_e_b_a_a);
}

/*************************************************************************/

