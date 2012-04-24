/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <stdio.h>
#include <debug.h>
#include <misc.h>

#include <fermi-asm.h>



/* 
 * Initialization of disassembler
 */

/* Table containing information of all instructions */
static struct frm_inst_info_t frm_inst_info[FRM_INST_COUNT];

/* Pointers to 'frm_inst_info' table indexed by instruction opcode */
#define FRM_ISNT_INFO_LONG_SIZE 1024

static struct frm_inst_info_t *frm_inst_info_long[FRM_ISNT_INFO_LONG_SIZE]; /* 8-byte instructions */

/* Table containing names of all special registers */
static char *frm_sr[FRM_SR_COUNT];


void frm_disasm_init()
{
	struct frm_inst_info_t *info;
	int i;

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _fmt, _category, _opcode) \
	info = &frm_inst_info[FRM_INST_##_name]; \
	info->inst = FRM_INST_##_name; \
	info->category = FRM_INST_CAT_##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->fmt = FRM_FMT_##_fmt; \
	info->opcode = _opcode; \
	info->size = 64;
#include <fermi-asm.dat>
#undef DEFINST

	for (i = 1; i < FRM_INST_COUNT; ++i)
	{
		info = &frm_inst_info[i];
		frm_inst_info_long[info->opcode] = info;
	}

	frm_sr[FRM_SR_Laneld] = "SR_Laneld";
	frm_sr[FRM_SR_VirtCfg] = "SR_VirtCfg";
	frm_sr[FRM_SR_VirtId] = "SR_VirtId";
	frm_sr[FRM_SR_PM0] = "SR_PM0";
	frm_sr[FRM_SR_PM1] = "SR_PM1";
	frm_sr[FRM_SR_PM2] = "SR_PM2";
	frm_sr[FRM_SR_PM3] = "SR_PM3";
	frm_sr[FRM_SR_PM4] = "SR_PM4";
	frm_sr[FRM_SR_PM5] = "SR_PM5";
	frm_sr[FRM_SR_PM6] = "SR_PM6";
	frm_sr[FRM_SR_PM7] = "SR_PM7";
	frm_sr[FRM_SR_PRIM_TYPE] = "SR_PRIM_TYPE";
	frm_sr[FRM_SR_INVOCATION_ID] = "SR_INVOCATION_ID";
	frm_sr[FRM_SR_Y_DIRECTION] = "SR_Y_DIRECTION";
	frm_sr[FRM_SR_MACHINE_ID_0] = "SR_MACHINE_ID_0";
	frm_sr[FRM_SR_MACHINE_ID_1] = "SR_MACHINE_ID_1";
	frm_sr[FRM_SR_MACHINE_ID_2] = "SR_MACHINE_ID_2";
	frm_sr[FRM_SR_MACHINE_ID_3] = "SR_MACHINE_ID_3";
	frm_sr[FRM_SR_AFFINITY] = "SR_AFFINITY";
	frm_sr[FRM_SR_Tid] = "SR_Tid";
	frm_sr[FRM_SR_Tid_X] = "SR_Tid_X";
	frm_sr[FRM_SR_Tid_Y] = "SR_Tid_Y";
	frm_sr[FRM_SR_Tid_Z] = "SR_Tid_Z";
	frm_sr[FRM_SR_CTAParam] = "SR_CTAParam";
	frm_sr[FRM_SR_CTAid_X] = "SR_CTAid_X";
	frm_sr[FRM_SR_CTAid_Y] = "SR_CTAid_Y";
	frm_sr[FRM_SR_CTAid_Z] = "SR_CTAid_Z";
	frm_sr[FRM_SR_NTid] = "SR_NTid";
	frm_sr[FRM_SR_NTid_X] = "SR_NTid_X";
	frm_sr[FRM_SR_NTid_Y] = "SR_NTid_Y";
	frm_sr[FRM_SR_NTid_Z] = "SR_NTid_Z";
	frm_sr[FRM_SR_GridParam] = "SR_GridParam";
	frm_sr[FRM_SR_NCTAid_X] = "SR_NCTAid_X";
	frm_sr[FRM_SR_NCTAid_Y] = "SR_NCTAid_Y";
	frm_sr[FRM_SR_NCTAid_Z] = "SR_NCTAid_Z";
	frm_sr[FRM_SR_SWinLo] = "SR_SWinLo";
	frm_sr[FRM_SR_SWINSZ] = "SR_SWINSZ";
	frm_sr[FRM_SR_SMemSz] = "SR_SMemSz";
	frm_sr[FRM_SR_SMemBanks] = "SR_SMemBanks";
	frm_sr[FRM_SR_LWinLo] = "SR_LWinLo";
	frm_sr[FRM_SR_LWINSZ] = "SR_LWINSZ";
	frm_sr[FRM_SR_LMemLoSz] = "SR_LMemLoSz";
	frm_sr[FRM_SR_LMemHiOff] = "SR_LMemHiOff";
	frm_sr[FRM_SR_EqMask] = "SR_EqMask";
	frm_sr[FRM_SR_LtMask] = "SR_LtMask";
	frm_sr[FRM_SR_LeMask] = "SR_LeMask";
	frm_sr[FRM_SR_GtMask] = "SR_GtMask";
	frm_sr[FRM_SR_GeMask] = "SR_GeMask";
	frm_sr[FRM_SR_ClockLo] = "SR_ClockLo";
	frm_sr[FRM_SR_ClockHi] = "SR_ClockHi";
}




/* 
 * Dump an instrution
 */

void frm_inst_dump(FILE *f, char *str, int inst_str_size, unsigned char *buf, int inst_index)
{
	struct frm_inst_t inst;
	int byte_index;
	char *inst_str;
	char **inst_str_ptr;
	char *fmt_str;

	inst.addr = 8*inst_index;
	for (byte_index = 0; byte_index < 8; ++byte_index)
		inst.dword.bytes[byte_index] = *(buf+8*inst_index+byte_index);
	frm_inst_decode(&inst);

	inst_str = str;
	inst_str_ptr = &str;
	fmt_str = inst.info->fmt_str;
	while (*fmt_str)
	{
		if (*fmt_str != '%')
		{
			if (!(*fmt_str == ' ' && *inst_str_ptr == inst_str))
				str_printf(inst_str_ptr, &inst_str_size, "%c", *fmt_str);
			++fmt_str;
			continue;
		}

		++fmt_str;
		if (*fmt_str == 'p')
			frm_inst_dump_pred(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'e')
			frm_inst_dump_ext(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'd')
			frm_inst_dump_dst(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'P')
			frm_inst_dump_dst_P(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'Q')
			frm_inst_dump_dst_Q(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'a')
			frm_inst_dump_src_1(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'b')
			frm_inst_dump_src_2(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'c')
			frm_inst_dump_src_3(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'i')
			frm_inst_dump_imm(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else
			fatal("%c: token not recognized\n", *fmt_str);
		++fmt_str;
	}

	fprintf(f, "%s\n", inst_str);
}


void frm_inst_hex_dump(FILE *f, unsigned char *buf, int inst_index)
{
	int byte_index;

	fprintf(f, "Instruction Hex: ");
	/* Print most significant byte first */
	for (byte_index = 7; byte_index >= 0; --byte_index)
	{
		fprintf(f, "%02x", *(buf+8*inst_index+byte_index));
	}
	fprintf(f, "\n");
}




/* 
 * Decoder
 */

void frm_inst_decode(struct frm_inst_t *inst)
{
	unsigned int op;

	/* 10-bit opcode by default */
	op = ((inst->dword.bytes[7] & 0xfc) << 2) | ((inst->dword.bytes[0]) & 0xf);
	/* 9-bit opcode */
	if (op == 0x0e0 || op == 0x0f0 || /* FCMP */
		op == 0x080 || op == 0x090 || /* FSETP */
		op == 0x061 || op == 0x071 || /* DSETP */
		op == 0x063 || op == 0x073 || /* ISETP */
		op == 0x205 || op == 0x215 || /* LD */
		op == 0x245 || op == 0x255)   /* ST */
		op = ((inst->dword.bytes[7] & 0xf8) << 1) | ((inst->dword.bytes[0]) & 0xf);

	inst->info = frm_inst_info_long[op];
}


void frm_inst_dump_pred(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int pred;

	if (fmt == FRM_FMT_FP_OP2)
		pred = inst->dword.fp_op2.pred;
	else if (fmt == FRM_FMT_FP_OP3)
		pred = inst->dword.fp_op3.pred;
	else if (fmt == FRM_FMT_FP_OP3_I)
		pred = inst->dword.fp_op3_p.pred;
	else if (fmt == FRM_FMT_FP_OP3_P)
		pred = inst->dword.fp_op3_p.pred;
	else if (fmt == FRM_FMT_FP_OP4)
		pred = inst->dword.fp_op4.pred;
	else if (fmt == FRM_FMT_INT_OP3)
		pred = inst->dword.int_op3.pred;
	else if (fmt == FRM_FMT_INT_OP3_P)
		pred = inst->dword.int_op3_p.pred;
	else if (fmt == FRM_FMT_INT_OP4)
		pred = inst->dword.int_op4.pred;
	else if (fmt == FRM_FMT_INT_OP4_S)
		pred = inst->dword.int_op4_s.pred;
	else if (fmt == FRM_FMT_LDST_OP2)
		pred = inst->dword.ldst_op2.pred;
	else if (fmt == FRM_FMT_MOV_OP2)
		pred = inst->dword.mov_op2.pred;
	else if (fmt == FRM_FMT_CTRL_NONE)
		;
	else if (fmt == FRM_FMT_CTRL_OP1)
		pred = inst->dword.ctrl_op1.pred;
	else if (fmt == FRM_FMT_MISC_OP2_SREG)
		pred = inst->dword.misc_op2_sreg.pred;
	else
		fatal("%d: pred not recognized", fmt);

	if (pred < 7)
		str_printf(inst_str_ptr, inst_str_size, "@P%lld", pred);
	else if (pred > 7)
		str_printf(inst_str_ptr, inst_str_size, "@!P%lld", pred-8);
	else
		;
}


void frm_inst_dump_ext(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	if (fmt == FRM_FMT_FP_OP2)
	{
		if (inst->dword.fp_op2.op == 0)
			str_printf(inst_str_ptr, inst_str_size, ".COS");
		else if (inst->dword.fp_op2.op == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SIN");
		else if (inst->dword.fp_op2.op == 2)
			str_printf(inst_str_ptr, inst_str_size, ".EX2");
		else if (inst->dword.fp_op2.op == 3)
			str_printf(inst_str_ptr, inst_str_size, ".LG2");
		else if (inst->dword.fp_op2.op == 4)
			str_printf(inst_str_ptr, inst_str_size, ".RCP");
		else if (inst->dword.fp_op2.op == 5)
			str_printf(inst_str_ptr, inst_str_size, ".RSQ");
		else if (inst->dword.fp_op2.op == 6)
			str_printf(inst_str_ptr, inst_str_size, ".RCP64H");
		else if (inst->dword.fp_op2.op == 7)
			str_printf(inst_str_ptr, inst_str_size, ".RSQ64H");
		else
			fatal("%d: FRM_FMT_FP_OP2.op not recognized", inst->dword.fp_op2.op);

		if (inst->dword.fp_op2.sat == 0)
			;
		else if (inst->dword.fp_op2.sat == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SAT");
		else
			fatal("%d: FRM_FMT_FP_OP2.sat not recognized", inst->dword.fp_op2.sat);
	}
	else if (fmt == FRM_FMT_FP_OP3)
	{
		if (inst->dword.fp_op3.ftz == 0)
			;
		else if (inst->dword.fp_op3.ftz == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FTZ");
		else
			fatal("%d: FRM_FMT_FP_OP3.ftz not recognized", inst->dword.fp_op3.ftz);

		if (inst->dword.fp_op3.rnd == 0)
			;
		else if (inst->dword.fp_op3.rnd == 1)
			str_printf(inst_str_ptr, inst_str_size, ".RM");
		else if (inst->dword.fp_op3.rnd == 2)
			str_printf(inst_str_ptr, inst_str_size, ".RP");
		else if (inst->dword.fp_op3.rnd == 3)
			str_printf(inst_str_ptr, inst_str_size, ".RZ");
		else
			fatal("%d: FRM_FMT_FP_OP3.rnd not recognized", inst->dword.fp_op3.rnd);

		if (inst->dword.fp_op3.sat == 0)
			;
		else if (inst->dword.fp_op3.sat == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SAT");
		else
			fatal("%d: FRM_FMT_FP_OP3.sat not recognized", inst->dword.fp_op3.sat);
	}
	else if (fmt == FRM_FMT_FP_OP3_I)
	{
		if (inst->dword.fp_op3_i.ftz == 0)
			;
		else if (inst->dword.fp_op3_i.ftz == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FTZ");
		else
			fatal("%d: FRM_FMT_FP_OP3_I.ftz not recognized", inst->dword.fp_op3_i.ftz);
	}
	else if (fmt == FRM_FMT_FP_OP3_P)
	{
		if (inst->dword.fp_op3_p.comp_op == 1)
			str_printf(inst_str_ptr, inst_str_size, ".LT");
		else if (inst->dword.fp_op3_p.comp_op == 2)
			str_printf(inst_str_ptr, inst_str_size, ".EQ");
		else if (inst->dword.fp_op3_p.comp_op == 3)
			str_printf(inst_str_ptr, inst_str_size, ".LE");
		else if (inst->dword.fp_op3_p.comp_op == 4)
			str_printf(inst_str_ptr, inst_str_size, ".GT");
		else if (inst->dword.fp_op3_p.comp_op == 5)
			str_printf(inst_str_ptr, inst_str_size, ".NE");
		else if (inst->dword.fp_op3_p.comp_op == 6)
			str_printf(inst_str_ptr, inst_str_size, ".GE");
		else if (inst->dword.fp_op3_p.comp_op == 7)
			str_printf(inst_str_ptr, inst_str_size, ".NUM");
		else if (inst->dword.fp_op3_p.comp_op == 8)
			str_printf(inst_str_ptr, inst_str_size, ".NAN");
		else if (inst->dword.fp_op3_p.comp_op == 9)
			str_printf(inst_str_ptr, inst_str_size, ".LTU");
		else if (inst->dword.fp_op3_p.comp_op == 10)
			str_printf(inst_str_ptr, inst_str_size, ".EQU");
		else if (inst->dword.fp_op3_p.comp_op == 11)
			str_printf(inst_str_ptr, inst_str_size, ".LEU");
		else if (inst->dword.fp_op3_p.comp_op == 12)
			str_printf(inst_str_ptr, inst_str_size, ".GTU");
		else if (inst->dword.fp_op3_p.comp_op == 14)
			str_printf(inst_str_ptr, inst_str_size, ".GEU");
		else
			fatal("%d: FRM_FMT_FP_OP3_P.comp_op not recognized", inst->dword.fp_op3_p.comp_op);

		if (inst->dword.fp_op3_p.logic_op == 0)
			str_printf(inst_str_ptr, inst_str_size, ".AND");
		else if (inst->dword.fp_op3_p.logic_op == 1)
			str_printf(inst_str_ptr, inst_str_size, ".OR");
		else if (inst->dword.fp_op3_p.logic_op == 2)
			str_printf(inst_str_ptr, inst_str_size, ".XOR");
		else
			fatal("%d: FRM_FMT_FP_OP3_P.logic_op not recognized", inst->dword.fp_op3_p.logic_op);
	}
	else if (fmt == FRM_FMT_FP_OP4)
	{
		if (inst->dword.fp_op4.ftzfmz == 0)
			;
		else if (inst->dword.fp_op4.ftzfmz == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FTZ");
		else if (inst->dword.fp_op4.ftzfmz == 2)
			;
		else
			fatal("%d: FRM_FMT_FP_OP4.ftzfmz not recognized", inst->dword.fp_op4.ftzfmz);

		if (inst->dword.fp_op4.rnd == 0)
			;
		else if (inst->dword.fp_op4.rnd == 1)
			str_printf(inst_str_ptr, inst_str_size, ".RM");
		else if (inst->dword.fp_op4.rnd == 2)
			str_printf(inst_str_ptr, inst_str_size, ".RP");
		else if (inst->dword.fp_op4.rnd == 3)
			str_printf(inst_str_ptr, inst_str_size, ".RZ");
		else
			fatal("%d: FRM_FMT_FP_OP4.rnd not recognized", inst->dword.fp_op4.rnd);

		if (inst->dword.fp_op4.sat == 0)
			;
		else if (inst->dword.fp_op4.sat == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SAT");
		else
			fatal("%d: FRM_FMT_FP_OP4.sat not recognized", inst->dword.fp_op4.sat);
	}
	else if (fmt == FRM_FMT_INT_OP3)
		;
	else if (fmt == FRM_FMT_INT_OP3_P)
	{
		if (inst->dword.int_op3_p.comp_op == 1)
			str_printf(inst_str_ptr, inst_str_size, ".LT");
		else if (inst->dword.int_op3_p.comp_op == 2)
			str_printf(inst_str_ptr, inst_str_size, ".EQ");
		else if (inst->dword.int_op3_p.comp_op == 3)
			str_printf(inst_str_ptr, inst_str_size, ".LE");
		else if (inst->dword.int_op3_p.comp_op == 4)
			str_printf(inst_str_ptr, inst_str_size, ".GT");
		else if (inst->dword.int_op3_p.comp_op == 5)
			str_printf(inst_str_ptr, inst_str_size, ".NE");
		else if (inst->dword.int_op3_p.comp_op == 6)
			str_printf(inst_str_ptr, inst_str_size, ".GE");
		else
			fatal("%d: FRM_FMT_INT_OP3_P.comp_op not recognized", inst->dword.int_op3_p.comp_op);

		if (inst->dword.int_op3_p.logic_op == 0)
			str_printf(inst_str_ptr, inst_str_size, ".AND");
		else if (inst->dword.int_op3_p.logic_op == 1)
			str_printf(inst_str_ptr, inst_str_size, ".OR");
		else if (inst->dword.int_op3_p.logic_op == 2)
			str_printf(inst_str_ptr, inst_str_size, ".XOR");
		else
			fatal("%d: FRM_FMT_INT_OP3_P.logic_op not recognized", inst->dword.int_op3_p.logic_op);
	}
	else if (fmt == FRM_FMT_INT_OP4)
		;
	else if (fmt == FRM_FMT_INT_OP4_S)
		;
	else if (fmt == FRM_FMT_LDST_OP2)
	{
		if (inst->dword.ldst_op2.e == 0)
			;
		else if (inst->dword.ldst_op2.e == 1)
			str_printf(inst_str_ptr, inst_str_size, ".E");
		else
			fatal("%d: FRM_FMT_LDST_OP2.e not recognized", inst->dword.ldst_op2.e);

		if (inst->dword.ldst_op2.cop == 0)
			;
		else if (inst->dword.ldst_op2.cop == 1)
			str_printf(inst_str_ptr, inst_str_size, ".CG");
		else if (inst->dword.ldst_op2.cop == 2)
			str_printf(inst_str_ptr, inst_str_size, ".CS");
		else if (inst->dword.ldst_op2.cop == 3)
			str_printf(inst_str_ptr, inst_str_size, ".CV");
		else
			fatal("%d: FRM_FMT_LDST_OP2.cop not recognized", inst->dword.ldst_op2.cop);

		if (inst->dword.ldst_op2.type == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U8");
		else if (inst->dword.ldst_op2.type == 1)
			str_printf(inst_str_ptr, inst_str_size, ".S8");
		else if (inst->dword.ldst_op2.type == 2)
			str_printf(inst_str_ptr, inst_str_size, ".U16");
		else if (inst->dword.ldst_op2.type == 3)
			str_printf(inst_str_ptr, inst_str_size, ".S16");
		else if (inst->dword.ldst_op2.type == 4)
			;
		else if (inst->dword.ldst_op2.type == 5)
			str_printf(inst_str_ptr, inst_str_size, ".64");
		else if (inst->dword.ldst_op2.type == 6)
			str_printf(inst_str_ptr, inst_str_size, ".128");
		else
			fatal("%d: FRM_FMT_LDST_OP2.type not recognized", inst->dword.ldst_op2.type);
	}
	else if (fmt == FRM_FMT_MOV_OP2)
		;
	else if (fmt == FRM_FMT_CTRL_NONE)
		;
	else if (fmt == FRM_FMT_CTRL_OP1)
	{
		if (inst->dword.ctrl_op1.u == 0)
			;
		else if (inst->dword.ctrl_op1.u == 1)
			str_printf(inst_str_ptr, inst_str_size, ".U");
		else
			fatal("%d: FRM_FMT_CTRL_OP1.u not recognized", inst->dword.ctrl_op1.u);

		if (inst->dword.ctrl_op1.lmt == 0)
			;
		else if (inst->dword.ctrl_op1.lmt == 1)
			str_printf(inst_str_ptr, inst_str_size, ".LMT");
		else
			fatal("%d: FRM_FMT_CTRL_OP1.lmt not recognized", inst->dword.ctrl_op1.lmt);
	}
	else if (fmt == FRM_FMT_MISC_OP2_SREG)
		;
	else
		fatal("%d: ext not recognized", fmt);
}


void frm_inst_dump_dst(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int dst;

	if (fmt == FRM_FMT_FP_OP2)
		dst = inst->dword.fp_op2.dst;
	else if (fmt == FRM_FMT_FP_OP3)
		dst = inst->dword.fp_op3.dst;
	else if (fmt == FRM_FMT_FP_OP3_I)
		;
	else if (fmt == FRM_FMT_FP_OP3_P)
		dst = inst->dword.fp_op3_i.dst;
	else if (fmt == FRM_FMT_FP_OP4)
		dst = inst->dword.fp_op4.dst;
	else if (fmt == FRM_FMT_INT_OP3)
		dst = inst->dword.int_op3.dst;
	else if (fmt == FRM_FMT_INT_OP3_P)
		;
	else if (fmt == FRM_FMT_INT_OP4)
		dst = inst->dword.int_op4.dst;
	else if (fmt == FRM_FMT_INT_OP4_S)
		dst = inst->dword.int_op4_s.dst;
	else if (fmt == FRM_FMT_LDST_OP2)
		dst = inst->dword.ldst_op2.dst;
	else if (fmt == FRM_FMT_MOV_OP2)
		dst = inst->dword.mov_op2.dst;
	else if (fmt == FRM_FMT_CTRL_NONE)
		;
	else if (fmt == FRM_FMT_CTRL_OP1)
		;
	else if (fmt == FRM_FMT_MISC_OP2_SREG)
		dst = inst->dword.misc_op2_sreg.dst;
	else
		fatal("%d: dst not recognized", fmt);

	str_printf(inst_str_ptr, inst_str_size, "R%lld", dst);
}


void frm_inst_dump_dst_P(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int P;

	if (fmt == FRM_FMT_FP_OP3_P)
		P = inst->dword.fp_op3_p.P;
	else if (fmt == FRM_FMT_INT_OP3_P)
		P = inst->dword.int_op3_p.P;
	else
		fatal("%d: P not recognized", fmt);

	str_printf(inst_str_ptr, inst_str_size, "P%lld", P);
}


void frm_inst_dump_dst_Q(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int p;

	if (fmt == FRM_FMT_FP_OP3_P)
		p = inst->dword.fp_op3_p.p;
	else if (fmt == FRM_FMT_INT_OP3_P)
		p = inst->dword.int_op3_p.p;
	else
		fatal("%d: p not recognized", fmt);

	if (p != 7)
		str_printf(inst_str_ptr, inst_str_size, "p%lld", p);
	else
		str_printf(inst_str_ptr, inst_str_size, "pt");
}


void frm_inst_dump_src_1(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int src1;

	if (fmt == FRM_FMT_FP_OP2)
		src1 = inst->dword.fp_op2.src1;
	else if (fmt == FRM_FMT_FP_OP3)
		src1 = inst->dword.fp_op3.src1;
	else if (fmt == FRM_FMT_FP_OP3_I)
		src1 = inst->dword.fp_op3_i.src1;
	else if (fmt == FRM_FMT_FP_OP3_P)
		src1 = inst->dword.fp_op3_p.src1;
	else if (fmt == FRM_FMT_FP_OP4)
		src1 = inst->dword.fp_op4.src1;
	else if (fmt == FRM_FMT_INT_OP3)
		src1 = inst->dword.int_op3.src1;
	else if (fmt == FRM_FMT_INT_OP3_P)
		src1 = inst->dword.int_op3_p.src1;
	else if (fmt == FRM_FMT_INT_OP4)
		src1 = inst->dword.int_op4.src1;
	else if (fmt == FRM_FMT_INT_OP4_S)
		src1 = inst->dword.int_op4_s.src1;
	else if (fmt == FRM_FMT_LDST_OP2)
		;
	else if (fmt == FRM_FMT_MOV_OP2)
		;
	else if (fmt == FRM_FMT_CTRL_NONE)
		;
	else if (fmt == FRM_FMT_CTRL_OP1)
		;
	else if (fmt == FRM_FMT_MISC_OP2_SREG)
		;
	else
		fatal("%d: src1 not recognized", fmt);

	str_printf(inst_str_ptr, inst_str_size, "R%lld", src1);
}


void frm_inst_dump_src_2(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int bank_id;
	unsigned long long int offset_in_bank;
	char *sreg;

	if (fmt == FRM_FMT_FP_OP2)
		;
	else if (fmt == FRM_FMT_FP_OP3)
	{
		if (inst->dword.fp_op3.sel_src2 == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_op3.src2 & 0x3f);
		else if (inst->dword.fp_op3.sel_src2 == 1)
		{
			bank_id = inst->dword.fp_op3.src2 >> 16;
			offset_in_bank= inst->dword.fp_op3.src2 & 0xffff;
			str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_op3.sel_src2 == 2)
			;
		else if (inst->dword.fp_op3.sel_src2 == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_OP3.sel_src2 not recognized", inst->dword.fp_op3.sel_src2);
	}
	else if (fmt == FRM_FMT_FP_OP3_I)
		;
	else if (fmt == FRM_FMT_FP_OP3_P)
	{
		if (inst->dword.fp_op3_p.sel_src2 == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_op3_p.src2 & 0x3f);
		else if (inst->dword.fp_op3_p.sel_src2 == 1)
		{
			bank_id = inst->dword.fp_op3_p.src2 >> 16;
			offset_in_bank= inst->dword.fp_op3_p.src2 & 0xffff;
			str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_op3_p.sel_src2 == 2)
			;
		else if (inst->dword.fp_op3_p.sel_src2 == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_OP3_P.sel_src2 not recognized", inst->dword.fp_op3_p.sel_src2);
	}
	else if (fmt == FRM_FMT_FP_OP4)
	{
		if (inst->dword.fp_op4.sel_src2 == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_op4.src2 & 0x3f);
		else if (inst->dword.fp_op4.sel_src2 == 1)
		{
			bank_id = inst->dword.fp_op4.src2 >> 16;
			offset_in_bank= inst->dword.fp_op4.src2 & 0xffff;
			str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_op4.sel_src2 == 2)
			;
		else if (inst->dword.fp_op4.sel_src2 == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_OP4.sel_src2 not recognized", inst->dword.fp_op4.sel_src2);
	}
	else if (fmt == FRM_FMT_INT_OP3)
	{
		bank_id = inst->dword.int_op3.src2 >> 16;
		offset_in_bank= inst->dword.int_op3.src2 & 0xffff;
		str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
	}
	else if (fmt == FRM_FMT_INT_OP3_P)
	{
		bank_id = inst->dword.int_op3_p.src2 >> 16;
		offset_in_bank= inst->dword.int_op3_p.src2 & 0xffff;
		str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
	}
	else if (fmt == FRM_FMT_INT_OP4)
	{
		bank_id = inst->dword.int_op4.src2 >> 16;
		offset_in_bank= inst->dword.int_op4.src2 & 0xffff;
		str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
	}
	else if (fmt == FRM_FMT_INT_OP4_S)
	{
		bank_id = inst->dword.int_op4_s.src2 >> 16;
		offset_in_bank= inst->dword.int_op4_s.src2 & 0xffff;
		str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
	}
	else if (fmt == FRM_FMT_LDST_OP2)
	{
		if (inst->dword.ldst_op2.imm != 0)
			str_printf(inst_str_ptr, inst_str_size, "[R%d+%#x]", inst->dword.ldst_op2.src1, inst->dword.ldst_op2.imm);
		else
			str_printf(inst_str_ptr, inst_str_size, "[R%d]", inst->dword.ldst_op2.src1);
	}
	else if (fmt == FRM_FMT_MOV_OP2)
	{
		bank_id = inst->dword.mov_op2.src2 >> 16;
		offset_in_bank= inst->dword.mov_op2.src2 & 0xffff;
		str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
	}
	else if (fmt == FRM_FMT_CTRL_NONE)
		;
	else if (fmt == FRM_FMT_CTRL_OP1)
		;
	else if (fmt == FRM_FMT_MISC_OP2_SREG)
	{
		sreg = frm_sr[inst->dword.misc_op2_sreg.sreg];
		str_printf(inst_str_ptr, inst_str_size, "%s", sreg);
	}
	else
		fatal("%d: src2 not recognized", fmt);
}

void frm_inst_dump_src_3(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int opp;

	if (fmt == FRM_FMT_FP_OP4)
		str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_op4.src3);
	else if (fmt == FRM_FMT_INT_OP3_P)
	{
		opp = inst->dword.int_op3_p.opp;
		if (opp != 7)
			str_printf(inst_str_ptr, inst_str_size, "p%lld", opp);
		else
			str_printf(inst_str_ptr, inst_str_size, "pt");
	}
	else if (fmt == FRM_FMT_INT_OP4)
		str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_op4.src3);
	else if (fmt == FRM_FMT_INT_OP4_S)
		str_printf(inst_str_ptr, inst_str_size, "%#x", inst->dword.int_op4_s.shamt);
	else
		fatal("%d: src3 not recognized", fmt);
}

void frm_inst_dump_imm(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int imm;

	if (fmt == FRM_FMT_FP_OP3_I)
		imm = inst->dword.fp_op3_i.imm;
	else if (fmt == FRM_FMT_CTRL_OP1)
		imm = inst->addr + 8 + inst->dword.ctrl_op1.imm;
	else
		fatal("%d: imm not recognized", fmt);

	str_printf(inst_str_ptr, inst_str_size, "%#llx", imm);
}


