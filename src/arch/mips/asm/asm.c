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

#include <lib/class/array.h>
#include <lib/class/elf-reader.h>
#include <lib/class/string.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
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


/*
 * Class 'MIPSAsm'
 */

void MIPSAsmCreate(MIPSAsm *self)
{
	/* Allocate storage for the instruction tables */
	self->dec_table                   = xcalloc(64, sizeof(struct mips_inst_info_t));
	self->dec_table_special           = xcalloc(64, sizeof(struct mips_inst_info_t));
	self->dec_table_special_movci     = xcalloc(2, sizeof(struct mips_inst_info_t));
	self->dec_table_special_srl       = xcalloc(2, sizeof(struct mips_inst_info_t));
	self->dec_table_special_srlv      = xcalloc(2, sizeof(struct mips_inst_info_t));

	self->dec_table_regimm            = xcalloc(32, sizeof(struct mips_inst_info_t));

	self->dec_table_cop0              = xcalloc(32, sizeof(struct mips_inst_info_t));
	self->dec_table_cop0_c0           = xcalloc(64, sizeof(struct mips_inst_info_t));
	self->dec_table_cop0_notc0        = xcalloc(32, sizeof(struct mips_inst_info_t));
	self->dec_table_cop0_notc0_mfmc0  = xcalloc(2, sizeof(struct mips_inst_info_t));

	self->dec_table_cop1              = xcalloc(32, sizeof(struct mips_inst_info_t));
	self->dec_table_cop1_bc1          = xcalloc(4, sizeof(struct mips_inst_info_t));
	self->dec_table_cop1_s            = xcalloc(64, sizeof(struct mips_inst_info_t));
	self->dec_table_cop1_s_movcf      = xcalloc(2, sizeof(struct mips_inst_info_t));
	self->dec_table_cop1_d            = xcalloc(64, sizeof(struct mips_inst_info_t));
	self->dec_table_cop1_d_movcf      = xcalloc(2, sizeof(struct mips_inst_info_t));
	self->dec_table_cop1_w            = xcalloc(64, sizeof(struct mips_inst_info_t));
	self->dec_table_cop1_l            = xcalloc(64, sizeof(struct mips_inst_info_t));
	self->dec_table_cop1_ps           = xcalloc(64, sizeof(struct mips_inst_info_t));

	self->dec_table_cop2              = xcalloc(32, sizeof(struct mips_inst_info_t));
	self->dec_table_cop2_bc2          = xcalloc(4, sizeof(struct mips_inst_info_t));

	self->dec_table_special2          = xcalloc(64, sizeof(struct mips_inst_info_t));

	self->dec_table_special3          = xcalloc(64, sizeof(struct mips_inst_info_t));
	self->dec_table_special3_bshfl    = xcalloc(32, sizeof(struct mips_inst_info_t));

	/* Initiate values for the 'next_table', 'next_table_low' and 'next_table_high'
	 * fields of the tables */
	self->dec_table[MIPS_OPCODE_SPECIAL].next_table =
		self->dec_table_special;
	self->dec_table[MIPS_OPCODE_SPECIAL].next_table_low                  = 0;
	self->dec_table[MIPS_OPCODE_SPECIAL].next_table_high                 = 5;

	self->dec_table_special[MIPS_OPCODE_SPECIAL_SRL].next_table =
		self->dec_table_special_srl;
	self->dec_table_special[MIPS_OPCODE_SPECIAL_SRL].next_table_low        = 21;
	self->dec_table_special[MIPS_OPCODE_SPECIAL_SRL].next_table_high       = 21;

	self->dec_table_special[MIPS_OPCODE_SPECIAL_SRLV].next_table =
		self->dec_table_special_srlv;
	self->dec_table_special[MIPS_OPCODE_SPECIAL_SRLV].next_table_low       = 6;
	self->dec_table_special[MIPS_OPCODE_SPECIAL_SRLV].next_table_high      = 6;

	self->dec_table_special[MIPS_OPCODE_SPECIAL_MOVCI].next_table =
		self->dec_table_special_movci;
	self->dec_table_special[MIPS_OPCODE_SPECIAL_MOVCI].next_table_low      = 16;
	self->dec_table_special[MIPS_OPCODE_SPECIAL_MOVCI].next_table_high     = 16;

	self->dec_table[MIPS_OPCODE_REGIMM].next_table =
		self->dec_table_regimm;
	self->dec_table[MIPS_OPCODE_REGIMM].next_table_low                       = 16;
	self->dec_table[MIPS_OPCODE_REGIMM].next_table_high                      = 20;

	self->dec_table[MIPS_OPCODE_COP0].next_table =
		self->dec_table_cop0;
	self->dec_table[MIPS_OPCODE_COP0].next_table_low                         = 25;
	self->dec_table[MIPS_OPCODE_COP0].next_table_high                        = 25;

	self->dec_table_cop0[MIPS_OPCODE_COP0_C0].next_table =
		self->dec_table_cop0_c0;
	self->dec_table_cop0[MIPS_OPCODE_COP0_C0].next_table_low                 = 0;
	self->dec_table_cop0[MIPS_OPCODE_COP0_C0].next_table_high                = 5;

	self->dec_table_cop0[MIPS_OPCODE_COP0_NOTC0].next_table =
		self->dec_table_cop0_notc0;
	self->dec_table_cop0[MIPS_OPCODE_COP0_NOTC0].next_table_low              = 21;
	self->dec_table_cop0[MIPS_OPCODE_COP0_NOTC0].next_table_high             = 25;

	self->dec_table_cop0_notc0[MIPS_OPCODE_COP0_NOTC0_MFMC0].next_table =
		self->dec_table_cop0_notc0_mfmc0;
	self->dec_table_cop0_notc0[MIPS_OPCODE_COP0_NOTC0_MFMC0].next_table_low   = 5;
	self->dec_table_cop0_notc0[MIPS_OPCODE_COP0_NOTC0_MFMC0].next_table_high  = 5;

	self->dec_table[MIPS_OPCODE_COP1].next_table =
		self->dec_table_cop1;
	self->dec_table[MIPS_OPCODE_COP1].next_table_low                          = 21;
	self->dec_table[MIPS_OPCODE_COP1].next_table_high                         = 25;

	self->dec_table_cop1[MIPS_OPCODE_COP1_BC1].next_table =
		self->dec_table_cop1_bc1;
	self->dec_table_cop1[MIPS_OPCODE_COP1_BC1].next_table_low                 = 16;
	self->dec_table_cop1[MIPS_OPCODE_COP1_BC1].next_table_high                = 17;

	self->dec_table_cop1[MIPS_OPCODE_COP1_S].next_table =
		self->dec_table_cop1_s;
	self->dec_table_cop1[MIPS_OPCODE_COP1_S].next_table_low                   = 0;
	self->dec_table_cop1[MIPS_OPCODE_COP1_S].next_table_high                  = 5;

	self->dec_table_cop1_s[MIPS_OPCODE_COP1_S_MOVCF].next_table =
		self->dec_table_cop1_s_movcf;
	self->dec_table_cop1_s[MIPS_OPCODE_COP1_S_MOVCF].next_table_low           = 16;
	self->dec_table_cop1_s[MIPS_OPCODE_COP1_S_MOVCF].next_table_high          = 16;

	self->dec_table_cop1[MIPS_OPCODE_COP1_D].next_table =
		self->dec_table_cop1_d;
	self->dec_table_cop1[MIPS_OPCODE_COP1_D].next_table_low                   = 0;
	self->dec_table_cop1[MIPS_OPCODE_COP1_D].next_table_high                  = 5;

	self->dec_table_cop1_d[MIPS_OPCODE_COP1_D_MOVCF].next_table =
		self->dec_table_cop1_d_movcf;
	self->dec_table_cop1_d[MIPS_OPCODE_COP1_D_MOVCF].next_table_low           = 16;
	self->dec_table_cop1_d[MIPS_OPCODE_COP1_D_MOVCF].next_table_high          = 16;

	self->dec_table_cop1[MIPS_OPCODE_COP1_W].next_table =
		self->dec_table_cop1_w;
	self->dec_table_cop1[MIPS_OPCODE_COP1_W].next_table_low                   = 0;
	self->dec_table_cop1[MIPS_OPCODE_COP1_W].next_table_high                  = 5;

	self->dec_table_cop1[MIPS_OPCODE_COP1_L].next_table =
		self->dec_table_cop1_l;
	self->dec_table_cop1[MIPS_OPCODE_COP1_L].next_table_low                   = 0;
	self->dec_table_cop1[MIPS_OPCODE_COP1_L].next_table_high                  = 5;

	self->dec_table_cop1[MIPS_OPCODE_COP1_PS].next_table =
		self->dec_table_cop1_ps;
	self->dec_table_cop1[MIPS_OPCODE_COP1_PS].next_table_low                  = 0;
	self->dec_table_cop1[MIPS_OPCODE_COP1_PS].next_table_high                 = 5;

	self->dec_table[MIPS_OPCODE_COP2].next_table = self->dec_table_cop2;
	self->dec_table[MIPS_OPCODE_COP2].next_table_low                          = 21;
	self->dec_table[MIPS_OPCODE_COP2].next_table_high                         = 25;

	self->dec_table[MIPS_OPCODE_SPECIAL2].next_table =
		self->dec_table_special2;
	self->dec_table[MIPS_OPCODE_SPECIAL2].next_table_low                      = 0;
	self->dec_table[MIPS_OPCODE_SPECIAL2].next_table_high                     = 5;

	self->dec_table[MIPS_OPCODE_SPECIAL3].next_table =
		self->dec_table_special3;
	self->dec_table[MIPS_OPCODE_SPECIAL3].next_table_low                      = 0;
	self->dec_table[MIPS_OPCODE_SPECIAL3].next_table_high                     = 5;

	self->dec_table_special3[MIPS_OPCODE_SPECIAL3_BSHFL].next_table =
		self->dec_table_special3_bshfl;
	self->dec_table_special3[MIPS_OPCODE_SPECIAL3_BSHFL].next_table_low        = 6;
	self->dec_table_special3[MIPS_OPCODE_SPECIAL3_BSHFL].next_table_high       = 10;

	/* Build the tables from asm.dat */
	struct mips_inst_info_t *current_table;
	unsigned int table_arg[4];
	int i = 0;

#define DEFINST(_name,_fmt_str,_op0,_op1,_op2,_op3)			\
	current_table = self->dec_table;				\
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


void MIPSAsmDestroy(MIPSAsm *self)
{
	free(self->dec_table);
	free(self->dec_table_special);
	free(self->dec_table_special_movci);
	free(self->dec_table_special_srl);
	free(self->dec_table_special_srlv);

	free(self->dec_table_regimm);

	free(self->dec_table_cop0);
	free(self->dec_table_cop0_c0);
	free(self->dec_table_cop0_notc0);
	free(self->dec_table_cop0_notc0_mfmc0);

	free(self->dec_table_cop1);
	free(self->dec_table_cop1_bc1);
	free(self->dec_table_cop1_s);
	free(self->dec_table_cop1_s_movcf);
	free(self->dec_table_cop1_d);
	free(self->dec_table_cop1_d_movcf);
	free(self->dec_table_cop1_w);
	free(self->dec_table_cop1_l);
	free(self->dec_table_cop1_ps);

	free(self->dec_table_cop2);
	free(self->dec_table_cop2_bc2);

	free(self->dec_table_special2);

	free(self->dec_table_special3);
	free(self->dec_table_special3_bshfl);
}


void MIPSAsmDisassembleBinary(MIPSAsm *self, char *path)
{
	ELFReader *reader;
	ELFSection *section;
	ELFSymbol *symbol;
	ELFSymbol *print_symbol;

	MIPSInst *inst;

	int curr_sym;
	void *ptr;

	/* Initialize */
	reader = new(ELFReader, path);
	inst = new(MIPSInst, self);

	/* Read Sections */
	ArrayForEach(reader->section_array, section, ELFSection)
	{
		/* Skip if section does not contain code */
		if (!(section->header->sh_flags & SHF_EXECINSTR))
			continue;

		/* Title */
		printf("\n\nDisassembly of section %s:",
				section->name->text);

		curr_sym = 0;
		symbol = asELFSymbol(ArrayGet(reader->symbol_array, curr_sym));

		/* Decode and dump instructions */
		for (ptr = section->buffer->ptr; ptr < section->buffer->ptr +
			     section->buffer->size; ptr += 4)
		{
			/* Symbol */
			while (symbol && symbol->sym->st_value <
			       	       section->header->sh_addr + section->buffer->pos)
			{
				curr_sym++;
				symbol = asELFSymbol(ArrayGet(reader->symbol_array,
						curr_sym));
			}
			if (symbol && symbol->sym->st_value ==
					section->header->sh_addr + section->buffer->pos)
				printf("\n\n%08x <%s>:",
				       section->header->sh_addr + section->buffer->pos,
				       symbol->name);

			/* Decode and dump */
			MIPSInstDecode(inst, section->header->sh_addr +
					section->buffer->pos, ptr);
			MIPSInstDumpHex(inst, stdout);
			MIPSInstDump(inst, stdout);

			/* Symbol name */
			if (inst->target)
			{
				print_symbol = ELFReaderGetSymbolByAddress(reader,
						inst->target, NULL);
				if (print_symbol->sym->st_value == inst->target)
					printf(" <%s>", print_symbol->name);
				else
					printf(" <%s+0x%x>", print_symbol->name,
					       inst->target - print_symbol->sym->st_value);
			}

			/* Next instruction */
			section->buffer->pos += 4;
		}
	}

	/* End */
	printf("\n");
	delete(reader);
	delete(inst);
}
