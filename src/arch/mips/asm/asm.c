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
#include "inst.h"


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
	current_table[table_arg[i]].opcode = MIPS_INST_##_name;		\
	current_table[table_arg[i]].name = #_name;			\
	current_table[table_arg[i]].fmt_str = _fmt_str;			\
	current_table[table_arg[i]].size = 4;
#include "asm.dat"
#undef DEFINST
}


void mips_emu_disasm(char *path)
{
	struct elf_file_t *elf_file;
	struct elf_section_t *section;
	struct elf_symbol_t *symbol, *print_symbol;

	int curr_sym;
	int i;
	void *inst_ptr;

	MIPSInst *inst;
	MIPSAsm *as;

	/* Initializations - build tables of istructions */
	as = new(MIPSAsm);
	mips_asm_init();

	/* Create an elf file from the executable */
	elf_file = elf_file_create_from_path(path);

	/* Create an instruction */
	inst = new(MIPSInst, as);

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

			/* Decode and dump */
			MIPSInstDecode(inst, section->header->sh_addr + section->buffer.pos,
					inst_ptr);
			MIPSInstDumpHex(inst, stdout);
			MIPSInstDump(inst, stdout);

			/* Symbol name */
			if (inst->target)
			{
				print_symbol = elf_symbol_get_by_address(elf_file,
									 inst->target, 0);
				if (print_symbol->value == inst->target)
					printf(" <%s>", print_symbol->name);
				else
					printf(" <%s+0x%x>", print_symbol->name,
					       inst->target - print_symbol->value);
			}

			section->buffer.pos += 4;
		}
	}
	printf("\n");
	elf_file_free(elf_file);
	mips_asm_done();

	delete(inst);
	delete(as);
}

void mips_disasm(unsigned int buf, unsigned int ip, volatile MIPSInst *inst)
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
	inst->bytes.word = (unsigned int) (buf);

	mips_table_arg =
		BITS32(inst->bytes.word, current_table_high,
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
			mips_table_arg = BITS32(inst->bytes.word, current_table_high,
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




/*
 * Class 'MIPSAsm'
 */

void MIPSAsmCreate(MIPSAsm *self)
{
}


void MIPSAsmDestroy(MIPSAsm *self)
{
}


