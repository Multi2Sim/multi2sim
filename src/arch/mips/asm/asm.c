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
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"


/* Opcodes and secondary opcodes */
#define MIPS_OPCODE_SPECIAL              0x00
#define MIPS_OPCODE_SPECIAL2             0x1c
#define MIPS_OPCODE_SPECIAL3             0x1f
#define MIPS_OPCODE_REGIMM               0x01
#define MIPS_OPCODE_COP0                 0x10
#define MIPS_OPCODE_COP1                 0x11
#define MIPS_OPCODE_COP2                 0x12
#define MIPS_OPCODE_COP1X                0x13

#define MIPS_OPCODE_SPECIAL_MOVCI        0x01
#define MIPS_OPCODE_SPECIAL_SRL          0x02
#define MIPS_OPCODE_SPECIAL_SRLV         0x06

#define MIPS_OPCODE_COP0_C0              0x01
#define MIPS_OPCODE_COP0_NOTC0           0x00
#define MIPS_OPCODE_COP0_NOTC0_MFMC0     0x0b

#define MIPS_OPCODE_COP1_BC1             0x08
#define MIPS_OPCODE_COP1_S               0x10
#define MIPS_OPCODE_COP1_S_MOVCF         0x10
#define MIPS_OPCODE_COP1_D               0x11
#define MIPS_OPCODE_COP1_D_MOVCF         0x11
#define MIPS_OPCODE_COP1_W               0x14
#define MIPS_OPCODE_COP1_L               0x15
#define MIPS_OPCODE_COP1_PS              0x16

#define MIPS_OPCODE_COP2_BC2             0x08

#define MIPS_OPCODE_SPECIAL3_BSHFL       0x20

/* Pointers to the tables of instructions */
struct mips_inst_info_t *mips_asm_table;
struct mips_inst_info_t *mips_asm_table_special;
struct mips_inst_info_t *mips_asm_table_special_movci;
struct mips_inst_info_t *mips_asm_table_special_srl;
struct mips_inst_info_t *mips_asm_table_special_srlv;

struct mips_inst_info_t *mips_asm_table_regimm;

struct mips_inst_info_t *mips_asm_table_cop0;
struct mips_inst_info_t *mips_asm_table_cop0_c0;
struct mips_inst_info_t *mips_asm_table_cop0_notc0;
struct mips_inst_info_t *mips_asm_table_cop0_notc0_mfmc0;

struct mips_inst_info_t *mips_asm_table_cop1;
struct mips_inst_info_t *mips_asm_table_cop1_bc1;
struct mips_inst_info_t *mips_asm_table_cop1_s;
struct mips_inst_info_t *mips_asm_table_cop1_s_movcf;
struct mips_inst_info_t *mips_asm_table_cop1_d;
struct mips_inst_info_t *mips_asm_table_cop1_d_movcf;
struct mips_inst_info_t *mips_asm_table_cop1_w;
struct mips_inst_info_t *mips_asm_table_cop1_l;
struct mips_inst_info_t *mips_asm_table_cop1_ps;

struct mips_inst_info_t *mips_asm_table_cop2;
struct mips_inst_info_t *mips_asm_table_cop2_bc2;

struct mips_inst_info_t *mips_asm_table_special2;

struct mips_inst_info_t *mips_asm_table_special3;
struct mips_inst_info_t *mips_asm_table_special3_bshfl;

/* Build table of all the instructions */
void mips_asm_init()
{
	/* Allocate storage for the instruction tables */
	mips_asm_table                   = xcalloc(64, sizeof(struct mips_inst_info_t));
	mips_asm_table_special           = xcalloc(64, sizeof(struct mips_inst_info_t));
	mips_asm_table_special_movci     = xcalloc(2, sizeof(struct mips_inst_info_t));
	mips_asm_table_special_srl       = xcalloc(2, sizeof(struct mips_inst_info_t));
	mips_asm_table_special_srlv      = xcalloc(2, sizeof(struct mips_inst_info_t));

	mips_asm_table_regimm            = xcalloc(32, sizeof(struct mips_inst_info_t));

	mips_asm_table_cop0              = xcalloc(32, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop0_c0           = xcalloc(64, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop0_notc0        = xcalloc(32, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop0_notc0_mfmc0  = xcalloc(2, sizeof(struct mips_inst_info_t));

	mips_asm_table_cop1              = xcalloc(32, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop1_bc1          = xcalloc(4, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop1_s            = xcalloc(64, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop1_s_movcf      = xcalloc(2, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop1_d            = xcalloc(64, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop1_d_movcf      = xcalloc(2, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop1_w            = xcalloc(64, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop1_l            = xcalloc(64, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop1_ps           = xcalloc(64, sizeof(struct mips_inst_info_t));

	mips_asm_table_cop2              = xcalloc(32, sizeof(struct mips_inst_info_t));
	mips_asm_table_cop2_bc2          = xcalloc(4, sizeof(struct mips_inst_info_t));

	mips_asm_table_special2          = xcalloc(64, sizeof(struct mips_inst_info_t));

	mips_asm_table_special3          = xcalloc(64, sizeof(struct mips_inst_info_t));
	mips_asm_table_special3_bshfl    = xcalloc(32, sizeof(struct mips_inst_info_t));

	/* Initiate values for the 'next_table', 'next_table_low' and 'next_table_high'
	 * fields of the tables */
	mips_asm_table[MIPS_OPCODE_SPECIAL].next_table =
		mips_asm_table_special;
	mips_asm_table[MIPS_OPCODE_SPECIAL].next_table_low                  = 0;
	mips_asm_table[MIPS_OPCODE_SPECIAL].next_table_high                 = 5;

	mips_asm_table_special[MIPS_OPCODE_SPECIAL_SRL].next_table =
		mips_asm_table_special_srl;
	mips_asm_table_special[MIPS_OPCODE_SPECIAL_SRL].next_table_low        = 21; 
	mips_asm_table_special[MIPS_OPCODE_SPECIAL_SRL].next_table_high       = 21; 

	mips_asm_table_special[MIPS_OPCODE_SPECIAL_SRLV].next_table = 
		mips_asm_table_special_srlv;
	mips_asm_table_special[MIPS_OPCODE_SPECIAL_SRLV].next_table_low       = 6;
	mips_asm_table_special[MIPS_OPCODE_SPECIAL_SRLV].next_table_high      = 6;

	mips_asm_table_special[MIPS_OPCODE_SPECIAL_MOVCI].next_table = 
		mips_asm_table_special_movci;
	mips_asm_table_special[MIPS_OPCODE_SPECIAL_MOVCI].next_table_low      = 16;
	mips_asm_table_special[MIPS_OPCODE_SPECIAL_MOVCI].next_table_high     = 16;

	mips_asm_table[MIPS_OPCODE_REGIMM].next_table =
		mips_asm_table_regimm;
	mips_asm_table[MIPS_OPCODE_REGIMM].next_table_low                       = 16;
	mips_asm_table[MIPS_OPCODE_REGIMM].next_table_high                      = 20;

	mips_asm_table[MIPS_OPCODE_COP0].next_table = 
		mips_asm_table_cop0;
	mips_asm_table[MIPS_OPCODE_COP0].next_table_low                         = 25;
	mips_asm_table[MIPS_OPCODE_COP0].next_table_high                        = 25;

	mips_asm_table_cop0[MIPS_OPCODE_COP0_C0].next_table = 
		mips_asm_table_cop0_c0;
	mips_asm_table_cop0[MIPS_OPCODE_COP0_C0].next_table_low                 = 0;
	mips_asm_table_cop0[MIPS_OPCODE_COP0_C0].next_table_high                = 5;

	mips_asm_table_cop0[MIPS_OPCODE_COP0_NOTC0].next_table = 
		mips_asm_table_cop0_notc0;
	mips_asm_table_cop0[MIPS_OPCODE_COP0_NOTC0].next_table_low              = 21;
	mips_asm_table_cop0[MIPS_OPCODE_COP0_NOTC0].next_table_high             = 25;

	mips_asm_table_cop0_notc0[MIPS_OPCODE_COP0_NOTC0_MFMC0].next_table = 
		mips_asm_table_cop0_notc0_mfmc0;
	mips_asm_table_cop0_notc0[MIPS_OPCODE_COP0_NOTC0_MFMC0].next_table_low   = 5;
	mips_asm_table_cop0_notc0[MIPS_OPCODE_COP0_NOTC0_MFMC0].next_table_high  = 5;

	mips_asm_table[MIPS_OPCODE_COP1].next_table =
		mips_asm_table_cop1;
	mips_asm_table[MIPS_OPCODE_COP1].next_table_low                          = 21;
	mips_asm_table[MIPS_OPCODE_COP1].next_table_high                         = 25;

	mips_asm_table_cop1[MIPS_OPCODE_COP1_BC1].next_table =
		mips_asm_table_cop1_bc1;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_BC1].next_table_low                 = 16;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_BC1].next_table_high                = 17;

	mips_asm_table_cop1[MIPS_OPCODE_COP1_S].next_table =
		mips_asm_table_cop1_s;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_S].next_table_low                   = 0;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_S].next_table_high                  = 5;

	mips_asm_table_cop1_s[MIPS_OPCODE_COP1_S_MOVCF].next_table =
		mips_asm_table_cop1_s_movcf;
	mips_asm_table_cop1_s[MIPS_OPCODE_COP1_S_MOVCF].next_table_low           = 16;
	mips_asm_table_cop1_s[MIPS_OPCODE_COP1_S_MOVCF].next_table_high          = 16;

	mips_asm_table_cop1[MIPS_OPCODE_COP1_D].next_table = 
		mips_asm_table_cop1_d;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_D].next_table_low                   = 0;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_D].next_table_high                  = 5;

	mips_asm_table_cop1_d[MIPS_OPCODE_COP1_D_MOVCF].next_table =
		mips_asm_table_cop1_d_movcf;
	mips_asm_table_cop1_d[MIPS_OPCODE_COP1_D_MOVCF].next_table_low           = 16;
	mips_asm_table_cop1_d[MIPS_OPCODE_COP1_D_MOVCF].next_table_high          = 16;

	mips_asm_table_cop1[MIPS_OPCODE_COP1_W].next_table =
		mips_asm_table_cop1_w;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_W].next_table_low                   = 0;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_W].next_table_high                  = 5;

	mips_asm_table_cop1[MIPS_OPCODE_COP1_L].next_table =
		mips_asm_table_cop1_l;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_L].next_table_low                   = 0;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_L].next_table_high                  = 5;

	mips_asm_table_cop1[MIPS_OPCODE_COP1_PS].next_table =
		mips_asm_table_cop1_ps;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_PS].next_table_low                  = 0;
	mips_asm_table_cop1[MIPS_OPCODE_COP1_PS].next_table_high                 = 5;

	mips_asm_table[MIPS_OPCODE_COP2].next_table = mips_asm_table_cop2;
	mips_asm_table[MIPS_OPCODE_COP2].next_table_low                          = 21;
	mips_asm_table[MIPS_OPCODE_COP2].next_table_high                         = 25;

	mips_asm_table[MIPS_OPCODE_SPECIAL2].next_table =
		mips_asm_table_special2;
	mips_asm_table[MIPS_OPCODE_SPECIAL2].next_table_low                      = 0;
	mips_asm_table[MIPS_OPCODE_SPECIAL2].next_table_high                     = 5;

	mips_asm_table[MIPS_OPCODE_SPECIAL3].next_table =
		mips_asm_table_special3;
	mips_asm_table[MIPS_OPCODE_SPECIAL3].next_table_low                      = 0;
	mips_asm_table[MIPS_OPCODE_SPECIAL3].next_table_high                     = 5;

	mips_asm_table_special3[MIPS_OPCODE_SPECIAL3_BSHFL].next_table =
		mips_asm_table_special3_bshfl;
	mips_asm_table_special3[MIPS_OPCODE_SPECIAL3_BSHFL].next_table_low        = 6;
	mips_asm_table_special3[MIPS_OPCODE_SPECIAL3_BSHFL].next_table_high       = 10;

	/* Build the tables from asm.dat */
	struct mips_inst_info_t *current_table;
	unsigned int table_arg[4];
	int i = 0;	

#define DEFINST(_name,_fmt_str,_op0,_op1,_op2,_op3)			\
	current_table = mips_asm_table;					\
	table_arg[0] = _op0;						\
	table_arg[1] = _op1;						\
	table_arg[2] = _op2;						\
	table_arg[3] = _op3;						\
	for(i=0;i<4;i++)						\
		{							\
			if (current_table[table_arg[i]].next_table)	\
				{					\
					current_table = current_table[table_arg[i]].next_table; \
				}					\
			else						\
				break;					\
		}							\
	current_table[table_arg[i]].inst = MIPS_INST_##_name;		\
	current_table[table_arg[i]].name = #_name;			\
	current_table[table_arg[i]].fmt_str = _fmt_str;			\
	current_table[table_arg[i]].size = 4;				\
	current_table[table_arg[i]].opcode = MIPS_INST_##_name;		       
#include "asm.dat"
#undef DEFINST
}

/* Decode the instruction in inst->dword.byte */
void mips_inst_decode(struct mips_inst_t *inst)
{
	struct mips_inst_info_t *current_table;

	/* We start with the first table mips_asm_table, with the
	 * opcode field as argument */
	current_table = mips_asm_table;
	int current_table_low = 26;
	int current_table_high = 31;
	unsigned int mips_table_arg;
	int loop_iteration = 0;

	mips_table_arg =
		BITS32(inst->dword.bytes, current_table_high,
		current_table_low);

	/* Find next tables if the instruction belongs to another table */
	while (1)
	{
		if (current_table[mips_table_arg].next_table
			&& loop_iteration < 4)
		{
			current_table_high =
				current_table[mips_table_arg].next_table_high;
			current_table_low =
				current_table[mips_table_arg].next_table_low;
			current_table =
				current_table[mips_table_arg].next_table;
			mips_table_arg =
				BITS32(inst->dword.bytes, current_table_high,
				current_table_low);
			loop_iteration++;
		}
		else if (loop_iteration > 4)
		{
			fatal("Can not find the correct table containing the instruction\n");
		}
		else
			break;

	}

	inst->info = &current_table[mips_table_arg];

}

/*************************************************************************/
/* Print the address of the instruction and the instruction in hex */
void mips_inst_hex_dump(FILE *f, void *inst_ptr, unsigned int inst_addr)
{
	fprintf(f, "\n%8x:\t%08x \t", inst_addr, *(unsigned int *) inst_ptr);
}

/*************************************************************************/
static int mips_token_comp(char *fmt_str, char *token_str, int *token_len)
{
	*token_len = strlen(token_str);
	return !strncmp(fmt_str, token_str, *token_len) &&
		!isalnum(fmt_str[*token_len]);
}

/*******************************************************************************/
void mips_inst_dump(FILE *f, char *str, int inst_str_size, void *inst_ptr,
	unsigned int inst_index, unsigned int inst_addr,
	unsigned int *print_symbol_address)
{
	int i = 0;
	struct mips_inst_t inst;
	int token_len;
	char *inst_str;
	char **inst_str_ptr;
	char *fmt_str;
	unsigned int rd, rt, rs, sa;

	inst.addr = inst_index;

	inst.dword.bytes = *(unsigned int *) (inst_ptr);

	mips_inst_decode(&inst);

	inst_str = str;
	inst_str_ptr = &str;
	fmt_str = inst.info->fmt_str;
	if (fmt_str)
	{
		while (*fmt_str)
		{
			rd = inst.dword.standard.rd;
			rt = inst.dword.standard.rt;
			rs = inst.dword.standard.rs;
			sa = inst.dword.standard.sa;

			/* 
			 *  DEAL WITH PSEUDO INSTRUCTIONS
			 */

			/* SLL ZERO, ZERO, ZERO => NOP */
			if (mips_token_comp(fmt_str, "sll", &token_len))
			{
				if ((rd | rt | sa) == 0)
				{
					str_printf(inst_str_ptr,
						&inst_str_size, "nop");
					break;
				}
			}
			/* ADDU RD, RS, ZERO => MOVE RD, RS */
			else if (mips_token_comp(fmt_str, "addu", &token_len))
			{
				if (rt == 0)
				{
					str_printf(inst_str_ptr,
						&inst_str_size, "move\t");
					mips_inst_dump_RD(inst_str_ptr,
						&inst_str_size, &inst);
					str_printf(inst_str_ptr,
						&inst_str_size, ",");
					mips_inst_dump_RS(inst_str_ptr,
						&inst_str_size, &inst);
					break;
				}
			}
			/* BGEZAL ZERO, OFFSET => BAL OFFSET */
			else if (mips_token_comp(fmt_str, "bgezal",
					&token_len))
			{
				if (rs == 0)
				{
					str_printf(inst_str_ptr,
						&inst_str_size, "bal\t");
					mips_inst_dump_OFFSETBR(inst_str_ptr,
						&inst_str_size, &inst,
						inst_addr,
						print_symbol_address);
					break;
				}
			}
			/* BEQ ZERO, ZERO, OFFSET => B OFFSET */
			/* BEQ RS, ZERO, OFFSET => BEQZ RS, OFFSET */
			else if (mips_token_comp(fmt_str, "beq", &token_len))
			{
				if ((rs | rt) == 0)
				{
					str_printf(inst_str_ptr,
						&inst_str_size, "b\t");
					mips_inst_dump_OFFSETBR(inst_str_ptr,
						&inst_str_size, &inst,
						inst_addr,
						print_symbol_address);
					break;
				}
				else if (rt == 0)
				{
					str_printf(inst_str_ptr,
						&inst_str_size, "beqz\t");
					mips_inst_dump_RS(inst_str_ptr,
						&inst_str_size, &inst);
					str_printf(inst_str_ptr,
						&inst_str_size, ",");
					mips_inst_dump_OFFSETBR(inst_str_ptr,
						&inst_str_size, &inst,
						inst_addr,
						print_symbol_address);
					break;
				}
			}
			/* ADDIU RT, ZERO, IMM => LI RT, IMM */
			else if (mips_token_comp(fmt_str, "addiu",
					&token_len))
			{
				if (rs == 0)
				{
					str_printf(inst_str_ptr,
						&inst_str_size, "li\t");
					mips_inst_dump_RT(inst_str_ptr,
						&inst_str_size, &inst);
					str_printf(inst_str_ptr,
						&inst_str_size, ",");
					mips_inst_dump_IMM(inst_str_ptr,
						&inst_str_size, &inst);
					break;
				}
			}
			/* ORI RT, ZERO, IMM => LI RT, IMM */
			else if (mips_token_comp(fmt_str, "ori", &token_len))
			{
				if (rs == 0)
				{
					str_printf(inst_str_ptr,
						&inst_str_size, "li\t");
					mips_inst_dump_RT(inst_str_ptr,
						&inst_str_size, &inst);
					str_printf(inst_str_ptr,
						&inst_str_size, ",");
					mips_inst_dump_IMMHEX(inst_str_ptr,
						&inst_str_size, &inst);
					break;
				}
			}
			/* BNE RS, ZERO, OFFSET => BNEZ RS, OFFSET */
			else if (mips_token_comp(fmt_str, "bne", &token_len))
			{
				if (rt == 0)
				{
					str_printf(inst_str_ptr,
						&inst_str_size, "bnez\t");
					mips_inst_dump_RS(inst_str_ptr,
						&inst_str_size, &inst);
					str_printf(inst_str_ptr,
						&inst_str_size, ",");
					mips_inst_dump_OFFSETBR(inst_str_ptr,
						&inst_str_size, &inst,
						inst_addr,
						print_symbol_address);
					break;
				}
			}
			/* SUBU RD, ZERO, RT => NEGU RD, RT */
			else if (mips_token_comp(fmt_str, "subu", &token_len))
			{
				if (rs == 0)
				{
					str_printf(inst_str_ptr,
						&inst_str_size, "negu\t");
					mips_inst_dump_RD(inst_str_ptr,
						&inst_str_size, &inst);
					str_printf(inst_str_ptr,
						&inst_str_size, ",");
					mips_inst_dump_RT(inst_str_ptr,
						&inst_str_size, &inst);
					break;
				}
			}
			/* DONE WITH PSEUDO INSTRUCTIONS */

			if (*fmt_str != '%')
			{
				if (!(*fmt_str == ' '
						&& *inst_str_ptr == inst_str))
				{
					if (*fmt_str == '_')
						str_printf(inst_str_ptr,
							&inst_str_size, ".");
					else
						str_printf(inst_str_ptr,
							&inst_str_size, "%c",
							*fmt_str);
				}
				++fmt_str;
				continue;
			}
			if (i == 0)
				str_printf(inst_str_ptr, &inst_str_size,
					"\t");
			i = 1;
			++fmt_str;
			if (mips_token_comp(fmt_str, "sa", &token_len))
				mips_inst_dump_SA(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "rd", &token_len))
				mips_inst_dump_RD(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "rt", &token_len))
				mips_inst_dump_RT(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "rs", &token_len))
				mips_inst_dump_RS(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "target",
					&token_len))
			{
				mips_inst_dump_TARGET(inst_str_ptr,
					&inst_str_size, &inst,
					print_symbol_address);
			}
			else if (mips_token_comp(fmt_str, "offset",
					&token_len))
				mips_inst_dump_OFFSET(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "offsetbr",
					&token_len))
			{
				mips_inst_dump_OFFSETBR(inst_str_ptr,
					&inst_str_size, &inst,
					inst_addr, print_symbol_address);
			}
			else if (mips_token_comp(fmt_str, "Imm", &token_len))
				mips_inst_dump_IMM(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "Immhex",
					&token_len))
				mips_inst_dump_IMMHEX(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "base", &token_len))
				mips_inst_dump_BASE(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "sel", &token_len))
				mips_inst_dump_SEL(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "cc", &token_len))
				mips_inst_dump_CC(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "pos", &token_len))
				mips_inst_dump_POS(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "size", &token_len))
				mips_inst_dump_SIZE(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "fs", &token_len))
				mips_inst_dump_FS(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "ft", &token_len))
				mips_inst_dump_FT(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "fd", &token_len))
				mips_inst_dump_FD(inst_str_ptr,
					&inst_str_size, &inst);
			else if (mips_token_comp(fmt_str, "code", &token_len))
				mips_inst_dump_CODE(inst_str_ptr,
					&inst_str_size, &inst);
			else
				fatal("%s: token not recognized\n", fmt_str);

			fmt_str += token_len;

		}
		fprintf(f, "%s", inst_str);
	}
	else
	{
		fatal("Instruction not implememted.\n");
	}
}

/*******************************************************************************/

/* Register names */
char *mips_reg_name[mips_reg_count] = {
	"zero",
	"at",
	"v0",
	"v1",
	"a0",
	"a1",
	"a2",
	"a3",
	"t0",
	"t1",
	"t2",
	"t3",
	"t4",
	"t5",
	"t6",
	"t7",
	"s0",
	"s1",
	"s2",
	"s3",
	"s4",
	"s5",
	"s6",
	"s7",
	"t8",
	"t9",
	"k0",
	"k1",
	"gp",
	"sp",
	"s8",
	"ra"
};

/*********************************************************************************
 *                           Dump functions
 ********************************************************************************/
void mips_inst_dump_SA(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int sa;

	sa = inst->dword.standard.sa;
	if (sa != 0)
		str_printf(inst_str_ptr, inst_str_size, "0x%x", sa);
	else
		str_printf(inst_str_ptr, inst_str_size, "%d", sa);
}

void mips_inst_dump_RD(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int rd;
	int token_len;

	rd = inst->dword.standard.rd;

	if (mips_token_comp(inst->info->name, "RDHWR", &token_len))
		str_printf(inst_str_ptr, inst_str_size, "$%d", rd);
	else if (mips_token_comp(inst->info->name, "JALR", &token_len))
	{
		if (rd == (int) 31)
		{
		}
		else
		{
			str_printf(inst_str_ptr, inst_str_size, ",");
			str_printf(inst_str_ptr, inst_str_size, "%s",
				mips_reg_name[rd]);
		}
	}
	else
		str_printf(inst_str_ptr, inst_str_size, "%s",
			mips_reg_name[rd]);
}

void mips_inst_dump_RT(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int rt;

	rt = inst->dword.standard.rt;
	str_printf(inst_str_ptr, inst_str_size, "%s", mips_reg_name[rt]);
}

void mips_inst_dump_RS(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int rs;

	rs = inst->dword.standard.rs;

	str_printf(inst_str_ptr, inst_str_size, "%s", mips_reg_name[rs]);
}

void mips_inst_dump_TARGET(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst, unsigned int *print_symbol_address)
{
	unsigned int target;

	target = inst->dword.target.target;
	target = target << 2;
	*print_symbol_address = target;
	str_printf(inst_str_ptr, inst_str_size, "%x", target);
}

void mips_inst_dump_OFFSET(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int offset;

	offset = inst->dword.offset_imm.offset;
	if (offset & 0x8000)
	{
		offset = -((offset ^ 0xffff) + 1);
	}

	str_printf(inst_str_ptr, inst_str_size, "%d", (int) offset);
}

void mips_inst_dump_OFFSETBR(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst, unsigned int inst_addr,
	unsigned int *print_symbol_address)
{
	unsigned int offsetbr;

	offsetbr = inst->dword.cc.offsetbr;
	if (offsetbr & 0x8000)
	{
		offsetbr = -((offsetbr ^ 0xffff) + 1);
	}
	offsetbr = (offsetbr << 2) + inst_addr + 4;
	*print_symbol_address = offsetbr;
	str_printf(inst_str_ptr, inst_str_size, "%x", offsetbr);
}

void mips_inst_dump_IMM(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int imm;

	imm = inst->dword.offset_imm.offset;
	if (imm & 0x8000)
	{
		imm = -((imm ^ 0xffff) + 1);
	}

	str_printf(inst_str_ptr, inst_str_size, "%d", (int) imm);
}

void mips_inst_dump_IMMHEX(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int immhex;

	immhex = inst->dword.offset_imm.offset;

	str_printf(inst_str_ptr, inst_str_size, "0x%x", immhex);
}


void mips_inst_dump_BASE(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int base;

	base = inst->dword.offset_imm.base;

	str_printf(inst_str_ptr, inst_str_size, "%s", mips_reg_name[base]);
}


void mips_inst_dump_SEL(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int sel;

	sel = inst->dword.sel.sel;

	str_printf(inst_str_ptr, inst_str_size, "%d", sel);
}

void mips_inst_dump_CC(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int cc;

	cc = inst->dword.cc.cc;

	str_printf(inst_str_ptr, inst_str_size, "$fcc%d", cc);
}

void mips_inst_dump_POS(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int pos;

	pos = inst->dword.standard.sa;

	str_printf(inst_str_ptr, inst_str_size, "0x%x", pos);
}

void mips_inst_dump_FS(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int fs;
	int token_len;

	fs = inst->dword.standard.rd;
	if ((mips_token_comp(inst->info->name, "CFC1", &token_len)) ||
		(mips_token_comp(inst->info->name, "CTC1", &token_len)))
		str_printf(inst_str_ptr, inst_str_size, "$%d", fs);
	else
		str_printf(inst_str_ptr, inst_str_size, "$f%d", fs);
}

void mips_inst_dump_SIZE(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int size;
	unsigned int pos;

	int token_len;

	pos = inst->dword.standard.sa;
	size = inst->dword.standard.rd;

	if (mips_token_comp(inst->info->name, "INS", &token_len))
		size = size + 1 - pos;
	else if (mips_token_comp(inst->info->name, "EXT", &token_len))
		size++;
	str_printf(inst_str_ptr, inst_str_size, "0x%x", size);
}

void mips_inst_dump_FT(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int ft;

	ft = inst->dword.standard.rt;

	str_printf(inst_str_ptr, inst_str_size, "$f%d", ft);
}

void mips_inst_dump_FD(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int fd;

	fd = inst->dword.standard.sa;

	str_printf(inst_str_ptr, inst_str_size, "$f%d", fd);
}

void mips_inst_dump_CODE(char **inst_str_ptr, int *inst_str_size,
	struct mips_inst_t *inst)
{
	unsigned int code;

	code = inst->dword.code.code;

	str_printf(inst_str_ptr, inst_str_size, "0x%x", code);
}

/********************************************************************/
void mips_emu_disasm(char *path)
{
	struct elf_file_t *elf_file;
	struct elf_section_t *section;
	struct elf_symbol_t *symbol, *print_symbol;
	
	char inst_str[MAX_STRING_SIZE];
	int curr_sym;
	int i;
	void *inst_ptr;
	unsigned int print_symbol_address = 0;

	/* Initializations - build tables of istructions */
	mips_asm_init();
	
	/* Create an elf file from the executable */
	elf_file = elf_file_create_from_path(path);

	/* Read Sections */
	for (i=0; i < list_count(elf_file->section_list); i++)
	{
		/* Get section and skip it if it does not contain code */
		section = list_get(elf_file->section_list, i);
		if (!(section->header->sh_flags & SHF_EXECINSTR))
			continue;

		/* Title */
		printf("\n\nDisassembly of section %s:", section->name);
		
		curr_sym = 0;
		symbol = list_get(elf_file->symbol_table, curr_sym);
		
		/* Decode and dump instructions */
		for (inst_ptr = section->buffer.ptr; inst_ptr < section->buffer.ptr +
			     section->buffer.size; inst_ptr += 4)
		{
			/* Symbol */
			while (symbol
			       && symbol->value <
			       (section->header->sh_addr + section->buffer.pos))
			{
				curr_sym++;
				symbol = list_get(elf_file->symbol_table, curr_sym);
			}
			if (symbol
			    && symbol->value ==
			    (section->header->sh_addr + section->buffer.pos))
				printf("\n\n%08x <%s>:",
				       section->header->sh_addr + section->buffer.pos,
				       symbol->name);
			
			mips_inst_hex_dump(stdout, inst_ptr,
					   (section->header->sh_addr + section->buffer.pos));
			
			mips_inst_dump(stdout, inst_str, MAX_STRING_SIZE, inst_ptr,
				       section->buffer.pos, (section->header->sh_addr + section->buffer.pos),
				       &print_symbol_address);
			if (print_symbol_address != 0)
			{
				print_symbol = elf_symbol_get_by_address(elf_file,
									 print_symbol_address, 0);
				if (print_symbol->value == print_symbol_address)
					printf(" <%s>", print_symbol->name);
				else
					printf(" <%s+0x%x>", print_symbol->name,
					       print_symbol_address -
					       print_symbol->value);
				print_symbol_address = 0;
			}
						
			section->buffer.pos += 4;
		}
	}
	printf("\n");
	elf_file_free(elf_file);
	mips_asm_done();
}

void mips_disasm(unsigned int buf, unsigned int ip, volatile struct mips_inst_t *inst)
{
	struct mips_inst_info_t *current_table;
	/* We start with the first table mips_asm_table, with the
	 * opcode field as argument
	 */
	current_table = mips_asm_table;
	int current_table_low = 26;
	int current_table_high = 31;
	unsigned int mips_table_arg;
	int loop_iteration = 0;

	inst->addr = ip;
	inst->dword.bytes = (unsigned int) (buf);

	mips_table_arg = 
		BITS32(inst->dword.bytes, current_table_high,
		       current_table_low);

	/* Find next tables if the instruction belongs to another table */
	while(1)
	{
		if (current_table[mips_table_arg].next_table
		    && loop_iteration < 4)
		{
			current_table_high = current_table[mips_table_arg].next_table_high;
			current_table_low = current_table[mips_table_arg].next_table_low;
			current_table = current_table[mips_table_arg].next_table;
			mips_table_arg = BITS32(inst->dword.bytes, current_table_high,
									current_table_low);
			loop_iteration++;
		}
		else if (loop_iteration > 4)
		{
			fatal("Cannot find the correct table containing the instruction\n");
		}
		else
			break;
	}
	inst->info = &current_table[mips_table_arg];
}
	void mips_asm_done()
{

	free(mips_asm_table);
	free(mips_asm_table_special);
	free(mips_asm_table_special_movci);
	free(mips_asm_table_special_srl);
	free(mips_asm_table_special_srlv);

	free(mips_asm_table_regimm);

	free(mips_asm_table_cop0);
	free(mips_asm_table_cop0_c0);
	free(mips_asm_table_cop0_notc0);
	free(mips_asm_table_cop0_notc0_mfmc0);

	free(mips_asm_table_cop1);
	free(mips_asm_table_cop1_bc1);
	free(mips_asm_table_cop1_s);
	free(mips_asm_table_cop1_s_movcf);
	free(mips_asm_table_cop1_d);
	free(mips_asm_table_cop1_d_movcf);
	free(mips_asm_table_cop1_w);
	free(mips_asm_table_cop1_l);
	free(mips_asm_table_cop1_ps);

	free(mips_asm_table_cop2);
	free(mips_asm_table_cop2_bc2);

	free(mips_asm_table_special2);

	free(mips_asm_table_special3);
	free(mips_asm_table_special3_bshfl);
}

void mips_inst_debug_dump(struct mips_inst_t *inst, FILE *f)
{
	char inst_str[MAX_STRING_SIZE];
	void *inst_ptr;
	unsigned int print_symbol_address = 0;
	inst_ptr = &inst->dword.bytes;
	mips_inst_dump(f, inst_str, MAX_STRING_SIZE, inst_ptr, inst->addr, inst->addr, &print_symbol_address);
}
