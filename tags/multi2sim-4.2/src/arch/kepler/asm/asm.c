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

#include "asm.h"


/* Global instruction info table */
struct kpl_inst_info_t kpl_inst_info[KPL_INST_COUNT];


/* Opcodes and secondary opcodes */
#define KPL_OPCODE_A		00
#define KPL_OPCODE_B		01
#define KPL_OPCODE_C  		10

#define KPL_OPCODE_A_A		000

#define KPL_OPCODE_B_A		00
#define KPL_OPCODE_B_B		01
#define KPL_OPCODE_B_C		10
#define KPL_OPCODE_B_D		11

#define KPL_OPCODE_B_C_A	00
#define KPL_OPCODE_B_C_B	01
#define KPL_OPCODE_B_C_C	10
#define KPL_OPCODE_B_C_D	11

#define KPL_OPCODE_B_C_D_A	01
#define KPL_OPCODE_B_C_D_B	10
#define KPL_OPCODE_B_C_D_C	11

#define KPL_OPCODE_B_C_D_C_A	01
#define KPL_OPCODE_B_C_D_C_B	10
#define KPL_OPCODE_B_C_D_C_C	11

#define KPL_OPCODE_B_D_A	00

#define KPL_OPCODE_C_A		1001
#define KPL_OPCODE_C_B		1010
#define KPL_OPCODE_C_C		1011
#define KPL_OPCODE_C_D		1100
#define KPL_OPCODE_C_E_0	0000
#define KPL_OPCODE_C_E_1	0001
#define KPL_OPCODE_C_E_2	0010
#define KPL_OPCODE_C_E_3	0011
#define KPL_OPCODE_C_E_4	0100
#define KPL_OPCODE_C_E_5	0101
#define KPL_OPCODE_C_E_6	0110
#define KPL_OPCODE_C_E_7	0111
#define KPL_OPCODE_C_E_8	1101
#define KPL_OPCODE_C_E_9	1110
#define KPL_OPCODE_C_E_10	1111
		
#define KPL_OPCODE_C_C_A	01
#define KPL_OPCODE_C_C_B	10
#define KPL_OPCODE_C_C_C	11

#define KPL_OPCODE_C_C_C_A	01
#define KPL_OPCODE_C_C_C_B	10
#define KPL_OPCODE_C_C_C_C	11

#define KPL_OPCODE_C_E_A	00
#define KPL_OPCODE_C_E_B	01
#define KPL_OPCODE_C_E_C	10
#define KPL_OPCODE_C_E_D	11
	
#define KPL_OPCODE_C_E_A_A	10
#define KPL_OPCODE_C_E_A_B	11

#define KPL_OPCODE_C_E_B_A	110
#define KPL_OPCODE_C_E_B_B	111

#define KPL_OPCODE_C_E_B_A_A	100
#define KPL_OPCODE_C_E_B_A_B	101
#define KPL_OPCODE_C_E_B_A_C	110
#define KPL_OPCODE_C_E_B_A_D	111

#define KPL_OPCODE_C_E_B_A_C_A	1

#define KPL_OPCODE_C_E_B_B_A	0
#define KPL_OPCODE_C_E_B_B_B	1

#define KPL_OPCODE_C_E_B_B_A_A	00
#define KPL_OPCODE_C_E_B_B_A_B	01
#define KPL_OPCODE_C_E_B_B_A_C	10

#define KPL_OPCODE_C_E_C_A	0
#define KPL_OPCODE_C_E_C_B	1

#define KPL_OPCODE_C_E_C_A_A	1




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
struct kpl_inst_table_entry_t *kpl_asm_table_c_c;
struct kpl_inst_table_entry_t *kpl_asm_table_c_d;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e;

struct kpl_inst_table_entry_t *kpl_asm_table_c_c_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_c_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c_c_c;

struct kpl_inst_table_entry_t *kpl_asm_table_c_c_c_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_c_c_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c_c_c_c;

struct kpl_inst_table_entry_t *kpl_asm_table_c_e_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_c;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_d;

struct kpl_inst_table_entry_t *kpl_asm_table_c_e_a_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_a_b;

struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_b;

struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_a_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_a_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_a_c;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_a_d;

struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_a_c_a;

struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_b_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_b_b;

struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_b_a_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_b_a_b;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_b_b_a_c;

struct kpl_inst_table_entry_t *kpl_asm_table_c_e_c_a;
struct kpl_inst_table_entry_t *kpl_asm_table_c_e_c_b;

struct kpl_inst_table_entry_t *kpl_asm_table_c_e_c_a_a;


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

struct str_map_t kpl_inst_u0_map =
{
        4,
        {
                { ".U32.U32", 0},
                { ".S32.U32", 1},
		{ ".U32.S32", 2},
		{ "", 3}
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
                { "", 0},
                { ".U32", 1}
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




/*
 * Public Functions
 */


/* Build table of all the instructions */
void kpl_asm_init(void)
{
        /* Allocate storage for the instruction tables */
	kpl_asm_table = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_a = 		xcalloc(8, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c = 		xcalloc(16, sizeof(struct kpl_inst_table_entry_t));
	
	kpl_asm_table_a_a = 		xcalloc(64, sizeof(struct kpl_inst_table_entry_t));
	
	kpl_asm_table_b_a = 		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_b = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_d = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	
	kpl_asm_table_b_c_a = 		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_b = 		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_c = 		xcalloc(3, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_d = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	
	kpl_asm_table_b_c_d_a = 	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_d_b = 	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_d_c = 	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	
	kpl_asm_table_b_c_d_c_a = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_d_c_b = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_b_c_d_c_c = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_b_d_a = 		xcalloc(32, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_a = 		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_b = 		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_c = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_d = 		xcalloc(32, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_c_a = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_c_b = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_c_c = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_c_c_a = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_c_c_b = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_c_c_c =	 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_e_a = 		xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_b = 		xcalloc(8, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_c = 		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_d = 		xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	
	kpl_asm_table_c_e_a_a = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_a_b = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	
	kpl_asm_table_c_e_b_a = 	xcalloc(8, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_a_b = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_e_b_a_a = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_b_a_b = 	xcalloc(8, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_b_a_c = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_b_a_d = 	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

        kpl_asm_table_c_e_b_a_c_a = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_e_b_b_a = 	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_b_b_b = 	xcalloc(16, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_e_b_b_a_a = 	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_b_b_a_b = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_b_b_a_c = 	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_e_c_a = 	xcalloc(2, sizeof(struct kpl_inst_table_entry_t));
	kpl_asm_table_c_e_c_b = 	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));

	kpl_asm_table_c_e_c_a_a = 	xcalloc(4, sizeof(struct kpl_inst_table_entry_t));



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
        kpl_asm_table[KPL_OPCODE_C].next_table_low                            	= 59;
        kpl_asm_table[KPL_OPCODE_C].next_table_high                           	= 62;

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

        kpl_asm_table_b_c_d_c_c[KPL_OPCODE_B_C_D_C_C].next_table =
                kpl_asm_table_b_c_d_c_c;
        kpl_asm_table_b_c_d_c[KPL_OPCODE_B_C_D_C_C].next_table_low          	= 54;
        kpl_asm_table_b_c_d_c[KPL_OPCODE_B_C_D_C_C].next_table_high         	= 54;

        kpl_asm_table_b[KPL_OPCODE_B_D].next_table =
                kpl_asm_table_b_d;
        kpl_asm_table_b[KPL_OPCODE_B_D].next_table_low              	  	= 54;
        kpl_asm_table_b[KPL_OPCODE_B_D].next_table_high                      	= 54;

        kpl_asm_table_c[KPL_OPCODE_C_A].next_table =
                kpl_asm_table_c_a;
        kpl_asm_table_c[KPL_OPCODE_C_A].next_table_low                      	= 58;
        kpl_asm_table_c[KPL_OPCODE_C_A].next_table_high                    	= 58;

        kpl_asm_table_c[KPL_OPCODE_C_B].next_table =
                kpl_asm_table_c_b;
        kpl_asm_table_c[KPL_OPCODE_C_B].next_table_low                         	= 58;
        kpl_asm_table_c[KPL_OPCODE_C_B].next_table_high                      	= 58;

        kpl_asm_table_c[KPL_OPCODE_C_C].next_table =
                kpl_asm_table_c_c;
        kpl_asm_table_c[KPL_OPCODE_C_C].next_table_low                       	= 57;
        kpl_asm_table_c[KPL_OPCODE_C_C].next_table_high                     	= 58;

        kpl_asm_table_c[KPL_OPCODE_C_D].next_table =
                kpl_asm_table_c_d;
        kpl_asm_table_c[KPL_OPCODE_C_D].next_table_low                    	= 54;
        kpl_asm_table_c[KPL_OPCODE_C_D].next_table_high                		= 58;

        kpl_asm_table_c[KPL_OPCODE_C_E_0].next_table =
                kpl_asm_table_c_e;
        kpl_asm_table_c[KPL_OPCODE_C_E_0].next_table_low                   	= 62;
        kpl_asm_table_c[KPL_OPCODE_C_E_0].next_table_high                       	= 63;

        kpl_asm_table_c[KPL_OPCODE_C_E_1].next_table =
                kpl_asm_table_c_e;
        kpl_asm_table_c[KPL_OPCODE_C_E_1].next_table_low                          = 62;
        kpl_asm_table_c[KPL_OPCODE_C_E_1].next_table_high                         = 63;

        kpl_asm_table_c[KPL_OPCODE_C_E_2].next_table =
                kpl_asm_table_c_e;
        kpl_asm_table_c[KPL_OPCODE_C_E_2].next_table_low                          = 62;
        kpl_asm_table_c[KPL_OPCODE_C_E_2].next_table_high                         = 63;

        kpl_asm_table_c[KPL_OPCODE_C_E_3].next_table =
                kpl_asm_table_c_e;
        kpl_asm_table_c[KPL_OPCODE_C_E_3].next_table_low                          = 62;
        kpl_asm_table_c[KPL_OPCODE_C_E_3].next_table_high                         = 63;

        kpl_asm_table_c[KPL_OPCODE_C_E_4].next_table =
                kpl_asm_table_c_e;
        kpl_asm_table_c[KPL_OPCODE_C_E_4].next_table_low                          = 62;
        kpl_asm_table_c[KPL_OPCODE_C_E_4].next_table_high                         = 63;

        kpl_asm_table_c[KPL_OPCODE_C_E_5].next_table =
                kpl_asm_table_c_e;
        kpl_asm_table_c[KPL_OPCODE_C_E_5].next_table_low                          = 62;
        kpl_asm_table_c[KPL_OPCODE_C_E_5].next_table_high                         = 63;

        kpl_asm_table_c[KPL_OPCODE_C_E_6].next_table =
                kpl_asm_table_c_e;
        kpl_asm_table_c[KPL_OPCODE_C_E_6].next_table_low                          = 62;
        kpl_asm_table_c[KPL_OPCODE_C_E_6].next_table_high                         = 63;

        kpl_asm_table_c[KPL_OPCODE_C_E_7].next_table =
                kpl_asm_table_c_e;
        kpl_asm_table_c[KPL_OPCODE_C_E_7].next_table_low                          = 62;
        kpl_asm_table_c[KPL_OPCODE_C_E_7].next_table_high                         = 63;

        kpl_asm_table_c[KPL_OPCODE_C_E_8].next_table =
                kpl_asm_table_c_e;
        kpl_asm_table_c[KPL_OPCODE_C_E_8].next_table_low                          = 62;
        kpl_asm_table_c[KPL_OPCODE_C_E_8].next_table_high                         = 63;

        kpl_asm_table_c[KPL_OPCODE_C_E_9].next_table =
                kpl_asm_table_c_e;
        kpl_asm_table_c[KPL_OPCODE_C_E_9].next_table_low                          = 62;
        kpl_asm_table_c[KPL_OPCODE_C_E_9].next_table_high                         = 63;

        kpl_asm_table_c[KPL_OPCODE_C_E_10].next_table =
                kpl_asm_table_c_e;
        kpl_asm_table_c[KPL_OPCODE_C_E_10].next_table_low                          = 62;
        kpl_asm_table_c[KPL_OPCODE_C_E_10].next_table_high                         = 63;

        kpl_asm_table_c_c[KPL_OPCODE_C_C_A].next_table =
                kpl_asm_table_c_c_a;
        kpl_asm_table_c_c[KPL_OPCODE_C_C_A].next_table_low                   	= 55;
        kpl_asm_table_c_c[KPL_OPCODE_C_C_A].next_table_high                   	= 56;

        kpl_asm_table_c_c[KPL_OPCODE_C_C_B].next_table =
                kpl_asm_table_c_c_b;
        kpl_asm_table_c_c[KPL_OPCODE_C_C_B].next_table_low                    	= 55;
        kpl_asm_table_c_c[KPL_OPCODE_C_C_B].next_table_high                   	= 56;

        kpl_asm_table_c_c[KPL_OPCODE_C_C_C].next_table =
                kpl_asm_table_c_c_c;
        kpl_asm_table_c_c[KPL_OPCODE_C_C_C].next_table_low                    	= 55;
        kpl_asm_table_c_c[KPL_OPCODE_C_C_C].next_table_high                  	= 56;

        kpl_asm_table_c_c_c[KPL_OPCODE_C_C_C_A].next_table =
                kpl_asm_table_c_c_c_a;
        kpl_asm_table_c_c_c[KPL_OPCODE_C_C_C_A].next_table_low             	= 54;
        kpl_asm_table_c_c_c[KPL_OPCODE_C_C_C_A].next_table_high               	= 54;

        kpl_asm_table_c_c_c[KPL_OPCODE_C_C_C_B].next_table =
                kpl_asm_table_c_c_c_b;
        kpl_asm_table_c_c_c[KPL_OPCODE_C_C_C_B].next_table_low               	= 54;
        kpl_asm_table_c_c_c[KPL_OPCODE_C_C_C_B].next_table_high               	= 54;

        kpl_asm_table_c_c_c[KPL_OPCODE_C_C_C_C].next_table =
                kpl_asm_table_c_c_c_c;
        kpl_asm_table_c_c_c[KPL_OPCODE_C_C_C_C].next_table_low              	= 54;
        kpl_asm_table_c_c_c[KPL_OPCODE_C_C_C_C].next_table_high               	= 54;

        kpl_asm_table_c_e[KPL_OPCODE_C_E_A].next_table =
                kpl_asm_table_c_e_a;
        kpl_asm_table_c_e[KPL_OPCODE_C_E_A].next_table_low                   	= 60;
        kpl_asm_table_c_e[KPL_OPCODE_C_E_A].next_table_high                	= 61;

        kpl_asm_table_c_e[KPL_OPCODE_C_E_B].next_table =
                kpl_asm_table_c_e_b;
        kpl_asm_table_c_e[KPL_OPCODE_C_E_B].next_table_low                	= 59;
        kpl_asm_table_c_e[KPL_OPCODE_C_E_B].next_table_high                  	= 61;

        kpl_asm_table_c_e[KPL_OPCODE_C_E_C].next_table =
                kpl_asm_table_c_e_c;
        kpl_asm_table_c_e[KPL_OPCODE_C_E_C].next_table_low                  	= 61;
        kpl_asm_table_c_e[KPL_OPCODE_C_E_C].next_table_high               	= 61;

        kpl_asm_table_c_e[KPL_OPCODE_C_E_D].next_table =
                kpl_asm_table_c_e_d;
        kpl_asm_table_c_e[KPL_OPCODE_C_E_D].next_table_low                  	= 59;
        kpl_asm_table_c_e[KPL_OPCODE_C_E_D].next_table_high                   	= 59;

        kpl_asm_table_c_e_a[KPL_OPCODE_C_E_A_A].next_table =
                kpl_asm_table_c_e_a_a;
        kpl_asm_table_c_e_a[KPL_OPCODE_C_E_A_A].next_table_low               	= 59;
        kpl_asm_table_c_e_a[KPL_OPCODE_C_E_A_A].next_table_high              	= 59;

        kpl_asm_table_c_e_a[KPL_OPCODE_C_E_A_B].next_table =
                kpl_asm_table_c_e_a_b;
        kpl_asm_table_c_e_a[KPL_OPCODE_C_E_A_B].next_table_low               	= 59;
        kpl_asm_table_c_e_a[KPL_OPCODE_C_E_A_B].next_table_high              	= 59;

        kpl_asm_table_c_e_b[KPL_OPCODE_C_E_B_A].next_table =
                kpl_asm_table_c_e_b_a;
        kpl_asm_table_c_e_b[KPL_OPCODE_C_E_B_A].next_table_low               	= 56;
        kpl_asm_table_c_e_b[KPL_OPCODE_C_E_B_A].next_table_high              	= 58;

        kpl_asm_table_c_e_b[KPL_OPCODE_C_E_B_B].next_table =
                kpl_asm_table_c_e_b_b;
        kpl_asm_table_c_e_b[KPL_OPCODE_C_E_B_B].next_table_low              	= 58;
        kpl_asm_table_c_e_b[KPL_OPCODE_C_E_B_B].next_table_high             	= 58;

        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_A].next_table =
                kpl_asm_table_c_e_a_a;
        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_A].next_table_low          	= 55;
        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_A].next_table_high         	= 55;

        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_B].next_table =
                kpl_asm_table_c_e_b_a_b;
        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_B].next_table_low            	= 53;
        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_B].next_table_high        	= 55;

        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_C].next_table =
                kpl_asm_table_c_e_b_a_c;
        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_C].next_table_low         	= 55;
        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_C].next_table_high          	= 55;

        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_D].next_table =
                kpl_asm_table_c_e_b_a_d;
        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_D].next_table_low           	= 54;
        kpl_asm_table_c_e_b_a[KPL_OPCODE_C_E_B_A_D].next_table_high            	= 55;

        kpl_asm_table_c_e_b_a_c[KPL_OPCODE_C_E_B_A_C_A].next_table =
                kpl_asm_table_c_e_b_a_c_a;
        kpl_asm_table_c_e_b_a_c[KPL_OPCODE_C_E_B_A_C_A].next_table_low         	= 54;
        kpl_asm_table_c_e_b_a_c[KPL_OPCODE_C_E_B_A_C_A].next_table_high		= 54;

        kpl_asm_table_c_e_b_b[KPL_OPCODE_C_E_B_B_A].next_table =
                kpl_asm_table_c_e_b_b_a;
        kpl_asm_table_c_e_b_b[KPL_OPCODE_C_E_B_B_A].next_table_low           	= 56;
        kpl_asm_table_c_e_b_b[KPL_OPCODE_C_E_B_B_A].next_table_high           	= 57;

        kpl_asm_table_c_e_b_b[KPL_OPCODE_C_E_B_B_B].next_table =
                kpl_asm_table_c_e_b_b_b;
        kpl_asm_table_c_e_b_b[KPL_OPCODE_C_E_B_B_B].next_table_low           	= 54;
        kpl_asm_table_c_e_b_b[KPL_OPCODE_C_E_B_B_B].next_table_high           	= 57;

        kpl_asm_table_c_e_b_b_a[KPL_OPCODE_C_E_B_B_A_A].next_table =
                kpl_asm_table_c_e_b_b_a_a;
        kpl_asm_table_c_e_b_b_a[KPL_OPCODE_C_E_B_B_A_A].next_table_low    	= 54;
        kpl_asm_table_c_e_b_b_a[KPL_OPCODE_C_E_B_B_A_A].next_table_high     	= 55;

        kpl_asm_table_c_e_b_b_a[KPL_OPCODE_C_E_B_B_A_B].next_table =
                kpl_asm_table_c_e_b_b_a_b;
        kpl_asm_table_c_e_b_b_a[KPL_OPCODE_C_E_B_B_A_B].next_table_low       	= 55;
        kpl_asm_table_c_e_b_b_a[KPL_OPCODE_C_E_B_B_A_B].next_table_high     	= 55;

        kpl_asm_table_c_e_b_b_a[KPL_OPCODE_C_E_B_B_A_C].next_table =
                kpl_asm_table_c_e_b_b_a_c;
        kpl_asm_table_c_e_b_b_a[KPL_OPCODE_C_E_B_B_A_C].next_table_low        	= 54;
        kpl_asm_table_c_e_b_b_a[KPL_OPCODE_C_E_B_B_A_C].next_table_high        	= 55;

        kpl_asm_table_c_e_c[KPL_OPCODE_C_E_C_A].next_table =
                kpl_asm_table_c_e_c_a;
        kpl_asm_table_c_e_c[KPL_OPCODE_C_E_C_A].next_table_low              	= 58;
        kpl_asm_table_c_e_c[KPL_OPCODE_C_E_C_A].next_table_high              	= 58;

        kpl_asm_table_c_e_c[KPL_OPCODE_C_E_C_B].next_table =
                kpl_asm_table_c_e_c_b;
        kpl_asm_table_c_e_c[KPL_OPCODE_C_E_C_B].next_table_low                	= 59;
        kpl_asm_table_c_e_c[KPL_OPCODE_C_E_C_B].next_table_high               	= 60;

        kpl_asm_table_c_e_c_a[KPL_OPCODE_C_E_C_A_A].next_table =
                kpl_asm_table_c_e_c_a_a;
        kpl_asm_table_c_e_c_a[KPL_OPCODE_C_E_C_A_A].next_table_low  		= 54;
        kpl_asm_table_c_e_c_a[KPL_OPCODE_C_E_C_A_A].next_table_high           	= 57;

#define DEFINST(_name, _fmt_str,...) \
kpl_asm_table_init(KPL_INST_##_name, #_name, _fmt_str, PP_NARG(__VA_ARGS__), __VA_ARGS__);
	
#include "asm.dat" 
#undef DEFINST

}


void kpl_asm_done(void)
{

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
	free(kpl_asm_table_c_c);
	free(kpl_asm_table_c_d);
	free(kpl_asm_table_c_e);

	free(kpl_asm_table_c_c_a);
	free(kpl_asm_table_c_c_b);
	free(kpl_asm_table_c_c_c);

	free(kpl_asm_table_c_c_c_a);
	free(kpl_asm_table_c_c_c_b);
	free(kpl_asm_table_c_c_c_c);

	free(kpl_asm_table_c_e_a);
	free(kpl_asm_table_c_e_b);
	free(kpl_asm_table_c_e_c);
	free(kpl_asm_table_c_e_d);

	free(kpl_asm_table_c_e_a_a);
	free(kpl_asm_table_c_e_a_b);
	
	free(kpl_asm_table_c_e_b_a);
	free(kpl_asm_table_c_e_b_b);
	
	free(kpl_asm_table_c_e_b_a_a);
	free(kpl_asm_table_c_e_b_a_b);
	free(kpl_asm_table_c_e_b_a_c);
	free(kpl_asm_table_c_e_b_a_d);
	
	free(kpl_asm_table_c_e_b_a_c_a);
	
	free(kpl_asm_table_c_e_b_b_a);
	free(kpl_asm_table_c_e_b_b_b);

	free(kpl_asm_table_c_e_b_b_a_a);
	free(kpl_asm_table_c_e_b_b_a_b);
	free(kpl_asm_table_c_e_b_b_a_c);

	free(kpl_asm_table_c_e_c_a);
	free(kpl_asm_table_c_e_c_b);

	free(kpl_asm_table_c_e_c_a_a);
}


void kpl_disasm(char *file_name)
{
}


void kpl_disasm_buffer(void *buf, int size)
{
#if 0
        struct kpl_inst_table_entry_t *current_table;
        /* We start with the first table kpl_asm_table, with the
         * opcode field as argument
         */
        current_table = kpl_asm_table;
        int num_args;
	int current_table_low = 0;
        int current_table_high = 1;
        unsigned int kpl_table_arg;
        int loop_iteration = 0;

        inst->addr = ip;
        inst->dword.bytes = (unsigned int) (buf);

        kpl_table_arg =
                BITS32(inst->dword.bytes, current_table_high,
                       current_table_low);


        /* Find next tables if the instruction belongs to another table */
        while(1)
        {
                if (current_table[kpl_table_arg].next_table
                    && loop_iteration < num_args)
                {
                        current_table_high = current_table[kpl_table_arg].next_table_low;
                        current_table = current_table[kpl_table_arg].next_table;
                        kpl_table_arg = BITS32(inst->dword.bytes, current_table_high, current_table_low);
                        loop_iteration++;
                }
                else if (loop_iteration > num_args)
                {
                        fatal("Cannot find the correct table containing the instruction\n");
                }
                else
                        break;
        }
        inst->info = &current_table[kpl_table_arg];
#endif
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
		index = BITS64(inst->dword.bytes, high, low);
		if (!table[index].next_table)
		{
			inst->info = table[index].info;
			return;
		}

		/* Go to next table */
		table = table[index].next_table;
		low = table[index].next_table_low;
		high = table[index].next_table_high;
	}
}

