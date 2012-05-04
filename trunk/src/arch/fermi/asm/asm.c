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
			frm_inst_dump_P(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'Q')
			frm_inst_dump_Q(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'a')
			frm_inst_dump_src1(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'b')
			frm_inst_dump_src2(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'c')
			frm_inst_dump_src3(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'i')
			frm_inst_dump_imm(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'R')
			frm_inst_dump_R(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 'o')
			frm_inst_dump_offset(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
		else if (*fmt_str == 's')
			frm_inst_dump_shamt(inst_str_ptr, &inst_str_size, &inst, inst.info->fmt);
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

	if (fmt == FRM_FMT_FP_FFMA)
		pred = inst->dword.fp_ffma.pred;
	else if (fmt == FRM_FMT_FP_FADD)
		pred = inst->dword.fp_fadd.pred;
	else if (fmt == FRM_FMT_FP_FADD32I)
		pred = inst->dword.fp_fadd32i.pred;
	else if (fmt == FRM_FMT_FP_FCMP)
		pred = inst->dword.fp_fcmp.pred;
	else if (fmt == FRM_FMT_FP_FMUL)
		pred = inst->dword.fp_fmul.pred;
	else if (fmt == FRM_FMT_FP_FMUL32I)
		pred = inst->dword.fp_fmul32i.pred;
	else if (fmt == FRM_FMT_FP_FSETP)
		pred = inst->dword.fp_fsetp.pred;
	else if (fmt == FRM_FMT_FP_MUFU)
		pred = inst->dword.fp_mufu.pred;
	else if (fmt == FRM_FMT_FP_DFMA)
		pred = inst->dword.fp_dfma.pred;
	else if (fmt == FRM_FMT_FP_DADD)
		pred = inst->dword.fp_dadd.pred;
	else if (fmt == FRM_FMT_FP_DMUL)
		pred = inst->dword.fp_dmul.pred;
	else if (fmt == FRM_FMT_FP_DSETP)
		pred = inst->dword.fp_dsetp.pred;
	else if (fmt == FRM_FMT_INT_IMAD)
		pred = inst->dword.int_imad.pred;
	else if (fmt == FRM_FMT_INT_IMUL)
		pred = inst->dword.int_imul.pred;
	else if (fmt == FRM_FMT_INT_IADD)
		pred = inst->dword.int_iadd.pred;
	else if (fmt == FRM_FMT_INT_ISCADD)
		pred = inst->dword.int_iscadd.pred;
	else if (fmt == FRM_FMT_INT_BFE)
		pred = inst->dword.int_bfe.pred;
	else if (fmt == FRM_FMT_INT_BFI)
		pred = inst->dword.int_bfi.pred;
	else if (fmt == FRM_FMT_INT_SHR)
		pred = inst->dword.int_shr.pred;
	else if (fmt == FRM_FMT_INT_SHL)
		pred = inst->dword.int_shl.pred;
	else if (fmt == FRM_FMT_INT_LOP)
		pred = inst->dword.int_lop.pred;
	else if (fmt == FRM_FMT_INT_LOP32I)
		pred = inst->dword.int_lop32i.pred;
	else if (fmt == FRM_FMT_INT_ISETP)
		pred = inst->dword.int_isetp.pred;
	else if (fmt == FRM_FMT_INT_ICMP)
		pred = inst->dword.int_icmp.pred;
	else if (fmt == FRM_FMT_CONV_F2F)
		pred = inst->dword.conv_f2f.pred;
	else if (fmt == FRM_FMT_CONV_F2I)
		pred = inst->dword.conv_f2i.pred;
	else if (fmt == FRM_FMT_CONV_I2F)
		pred = inst->dword.conv_i2f.pred;
	else if (fmt == FRM_FMT_CONV_I2I)
		pred = inst->dword.conv_i2i.pred;
	else if (fmt == FRM_FMT_MOV_MOV)
		pred = inst->dword.mov_mov.pred;
	else if (fmt == FRM_FMT_MOV_MOV32I)
		pred = inst->dword.mov_mov32i.pred;
	else if (fmt == FRM_FMT_MOV_SEL)
		pred = inst->dword.mov_sel.pred;
	else if (fmt == FRM_FMT_LDST_LD)
		pred = inst->dword.ldst_ld.pred;
	else if (fmt == FRM_FMT_LDST_ST)
		pred = inst->dword.ldst_st.pred;
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
	if (fmt == FRM_FMT_FP_FFMA)
	{
		if (inst->dword.fp_ffma.ftzfmz == 0)
			;
		else if (inst->dword.fp_ffma.ftzfmz == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FTZ");
		else if (inst->dword.fp_ffma.ftzfmz == 2)
			str_printf(inst_str_ptr, inst_str_size, ".FMZ");
		else
			fatal("%d: FRM_FMT_FP_FFMA.ftzfmz not recognized", inst->dword.fp_ffma.ftzfmz);

		if (inst->dword.fp_ffma.rnd == 0)
			;
		else if (inst->dword.fp_ffma.rnd == 1)
			str_printf(inst_str_ptr, inst_str_size, ".RM");
		else if (inst->dword.fp_ffma.rnd == 2)
			str_printf(inst_str_ptr, inst_str_size, ".RP");
		else if (inst->dword.fp_ffma.rnd == 3)
			str_printf(inst_str_ptr, inst_str_size, ".RZ");
		else
			fatal("%d: FRM_FMT_FP_FFMA.rnd not recognized", inst->dword.fp_ffma.rnd);

		if (inst->dword.fp_ffma.sat == 0)
			;
		else if (inst->dword.fp_ffma.sat == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SAT");
		else
			fatal("%d: FRM_FMT_FP_FFMA.sat not recognized", inst->dword.fp_ffma.sat);
	}
	else if (fmt == FRM_FMT_FP_FADD)
	{
		if (inst->dword.fp_fadd.ftz == 0)
			;
		else if (inst->dword.fp_fadd.ftz == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FTZ");
		else
			fatal("%d: FRM_FMT_FP_FADD.ftz not recognized", inst->dword.fp_fadd.ftz);

		if (inst->dword.fp_fadd.rnd == 0)
			;
		else if (inst->dword.fp_fadd.rnd == 1)
			str_printf(inst_str_ptr, inst_str_size, ".RM");
		else if (inst->dword.fp_fadd.rnd == 2)
			str_printf(inst_str_ptr, inst_str_size, ".RP");
		else if (inst->dword.fp_fadd.rnd == 3)
			str_printf(inst_str_ptr, inst_str_size, ".RZ");
		else
			fatal("%d: FRM_FMT_FP_FADD.rnd not recognized", inst->dword.fp_fadd.rnd);

		if (inst->dword.fp_fadd.sat == 0)
			;
		else if (inst->dword.fp_fadd.sat == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SAT");
		else
			fatal("%d: FRM_FMT_FP_FADD.sat not recognized", inst->dword.fp_fadd.sat);
	}
	else if (fmt == FRM_FMT_FP_FADD32I)
	{
		if (inst->dword.fp_fadd32i.ftz == 0)
			;
		else if (inst->dword.fp_fadd32i.ftz == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FTZ");
		else
			fatal("%d: FRM_FMT_FP_FADD32I.ftz not recognized", inst->dword.fp_fadd32i.ftz);
	}
	else if (fmt == FRM_FMT_FP_FCMP)
	{
		if (inst->dword.fp_fcmp.cmp == 1)
			str_printf(inst_str_ptr, inst_str_size, ".LT");
		else if (inst->dword.fp_fcmp.cmp == 2)
			str_printf(inst_str_ptr, inst_str_size, ".EQ");
		else if (inst->dword.fp_fcmp.cmp == 3)
			str_printf(inst_str_ptr, inst_str_size, ".LE");
		else if (inst->dword.fp_fcmp.cmp == 4)
			str_printf(inst_str_ptr, inst_str_size, ".GT");
		else if (inst->dword.fp_fcmp.cmp == 5)
			str_printf(inst_str_ptr, inst_str_size, ".NE");
		else if (inst->dword.fp_fcmp.cmp == 6)
			str_printf(inst_str_ptr, inst_str_size, ".GE");
		else if (inst->dword.fp_fcmp.cmp == 7)
			str_printf(inst_str_ptr, inst_str_size, ".NUM");
		else if (inst->dword.fp_fcmp.cmp == 8)
			str_printf(inst_str_ptr, inst_str_size, ".NAN");
		else if (inst->dword.fp_fcmp.cmp == 9)
			str_printf(inst_str_ptr, inst_str_size, ".LTU");
		else if (inst->dword.fp_fcmp.cmp == 10)
			str_printf(inst_str_ptr, inst_str_size, ".EQU");
		else if (inst->dword.fp_fcmp.cmp == 11)
			str_printf(inst_str_ptr, inst_str_size, ".LEU");
		else if (inst->dword.fp_fcmp.cmp == 12)
			str_printf(inst_str_ptr, inst_str_size, ".GTU");
		else if (inst->dword.fp_fcmp.cmp == 14)
			str_printf(inst_str_ptr, inst_str_size, ".GEU");
		else
			fatal("%d: FRM_FMT_FP_FCMP.cmp not recognized", inst->dword.fp_fcmp.cmp);

		if (inst->dword.fp_fcmp.ftz == 0)
			;
		else if (inst->dword.fp_fcmp.ftz == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FTZ");
		else
			fatal("%d: FRM_FMT_FP_FCMP.ftz not recognized", inst->dword.fp_fcmp.ftz);
	}
	else if (fmt == FRM_FMT_FP_FMUL)
	{
		if (inst->dword.fp_fmul.ftzfmz == 0)
			;
		else if (inst->dword.fp_fmul.ftzfmz == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FTZ");
		else if (inst->dword.fp_fmul.ftzfmz == 2)
			str_printf(inst_str_ptr, inst_str_size, ".FMZ");
		else
			fatal("%d: FRM_FMT_FP_FMUL.ftzfmz not recognized", inst->dword.fp_fmul.ftzfmz);

		if (inst->dword.fp_fmul.rnd == 0)
			;
		else if (inst->dword.fp_fmul.rnd == 1)
			str_printf(inst_str_ptr, inst_str_size, ".RM");
		else if (inst->dword.fp_fmul.rnd == 2)
			str_printf(inst_str_ptr, inst_str_size, ".RP");
		else if (inst->dword.fp_fmul.rnd == 3)
			str_printf(inst_str_ptr, inst_str_size, ".RZ");
		else
			fatal("%d: FRM_FMT_FP_FMUL.rnd not recognized", inst->dword.fp_fmul.rnd);

		if (inst->dword.fp_fmul.sat == 0)
			;
		else if (inst->dword.fp_fmul.sat == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SAT");
		else
			fatal("%d: FRM_FMT_FP_FMUL.sat not recognized", inst->dword.fp_fmul.sat);
	}
	else if (fmt == FRM_FMT_FP_FMUL32I)
	{
		if (inst->dword.fp_fmul32i.ftzfmz == 0)
			;
		else if (inst->dword.fp_fmul32i.ftzfmz == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FTZ");
		else if (inst->dword.fp_fmul32i.ftzfmz == 2)
			str_printf(inst_str_ptr, inst_str_size, ".FMZ");
		else
			fatal("%d: FRM_FMT_FP_FMUL32I.ftzfmz not recognized", inst->dword.fp_fmul32i.ftzfmz);
	}
	else if (fmt == FRM_FMT_FP_FSETP)
	{
		if (inst->dword.fp_fsetp.cmp == 1)
			str_printf(inst_str_ptr, inst_str_size, ".LT");
		else if (inst->dword.fp_fsetp.cmp == 2)
			str_printf(inst_str_ptr, inst_str_size, ".EQ");
		else if (inst->dword.fp_fsetp.cmp == 3)
			str_printf(inst_str_ptr, inst_str_size, ".LE");
		else if (inst->dword.fp_fsetp.cmp == 4)
			str_printf(inst_str_ptr, inst_str_size, ".GT");
		else if (inst->dword.fp_fsetp.cmp == 5)
			str_printf(inst_str_ptr, inst_str_size, ".NE");
		else if (inst->dword.fp_fsetp.cmp == 6)
			str_printf(inst_str_ptr, inst_str_size, ".GE");
		else if (inst->dword.fp_fsetp.cmp == 7)
			str_printf(inst_str_ptr, inst_str_size, ".NUM");
		else if (inst->dword.fp_fsetp.cmp == 8)
			str_printf(inst_str_ptr, inst_str_size, ".NAN");
		else if (inst->dword.fp_fsetp.cmp == 9)
			str_printf(inst_str_ptr, inst_str_size, ".LTU");
		else if (inst->dword.fp_fsetp.cmp == 10)
			str_printf(inst_str_ptr, inst_str_size, ".EQU");
		else if (inst->dword.fp_fsetp.cmp == 11)
			str_printf(inst_str_ptr, inst_str_size, ".LEU");
		else if (inst->dword.fp_fsetp.cmp == 12)
			str_printf(inst_str_ptr, inst_str_size, ".GTU");
		else if (inst->dword.fp_fsetp.cmp == 14)
			str_printf(inst_str_ptr, inst_str_size, ".GEU");
		else
			fatal("%d: FRM_FMT_FP_FSETP.cmp not recognized", inst->dword.fp_fsetp.cmp);

		if (inst->dword.fp_fsetp.logic == 0)
			str_printf(inst_str_ptr, inst_str_size, ".AND");
		else if (inst->dword.fp_fsetp.logic == 1)
			str_printf(inst_str_ptr, inst_str_size, ".OR");
		else if (inst->dword.fp_fsetp.logic == 2)
			str_printf(inst_str_ptr, inst_str_size, ".XOR");
		else
			fatal("%d: FRM_FMT_FP_FSETP.logic not recognized", inst->dword.fp_fsetp.logic);
	}
	else if (fmt == FRM_FMT_FP_MUFU)
	{
		if (inst->dword.fp_mufu.op == 0)
			str_printf(inst_str_ptr, inst_str_size, ".COS");
		else if (inst->dword.fp_mufu.op == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SIN");
		else if (inst->dword.fp_mufu.op == 2)
			str_printf(inst_str_ptr, inst_str_size, ".EX2");
		else if (inst->dword.fp_mufu.op == 3)
			str_printf(inst_str_ptr, inst_str_size, ".LG2");
		else if (inst->dword.fp_mufu.op == 4)
			str_printf(inst_str_ptr, inst_str_size, ".RCP");
		else if (inst->dword.fp_mufu.op == 5)
			str_printf(inst_str_ptr, inst_str_size, ".RSQ");
		else if (inst->dword.fp_mufu.op == 6)
			str_printf(inst_str_ptr, inst_str_size, ".RCP64H");
		else if (inst->dword.fp_mufu.op == 7)
			str_printf(inst_str_ptr, inst_str_size, ".RSQ64H");
		else
			fatal("%d: FRM_FMT_FP_MUFU.op not recognized", inst->dword.fp_mufu.op);

		if (inst->dword.fp_mufu.sat == 0)
			;
		else if (inst->dword.fp_mufu.sat == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SAT");
		else
			fatal("%d: FRM_FMT_FP_MUFU.sat not recognized", inst->dword.fp_mufu.sat);
	}
	else if (fmt == FRM_FMT_FP_DFMA)
		;
	else if (fmt == FRM_FMT_FP_DADD)
	{
		if (inst->dword.fp_dadd.rnd == 0)
			;
		else if (inst->dword.fp_dadd.rnd == 1)
			str_printf(inst_str_ptr, inst_str_size, ".RM");
		else if (inst->dword.fp_dadd.rnd == 2)
			str_printf(inst_str_ptr, inst_str_size, ".RP");
		else if (inst->dword.fp_dadd.rnd == 3)
			str_printf(inst_str_ptr, inst_str_size, ".RZ");
		else
			fatal("%d: FRM_FMT_FP_DADD.rnd not recognized", inst->dword.fp_dadd.rnd);
	}
	else if (fmt == FRM_FMT_FP_DMUL)
	{
		if (inst->dword.fp_dmul.rnd == 0)
			;
		else if (inst->dword.fp_dmul.rnd == 1)
			str_printf(inst_str_ptr, inst_str_size, ".RM");
		else if (inst->dword.fp_dmul.rnd == 2)
			str_printf(inst_str_ptr, inst_str_size, ".RP");
		else if (inst->dword.fp_dmul.rnd == 3)
			str_printf(inst_str_ptr, inst_str_size, ".RZ");
		else
			fatal("%d: FRM_FMT_FP_DMUL.rnd not recognized", inst->dword.fp_dmul.rnd);
	}
	else if (fmt == FRM_FMT_FP_DSETP)
	{
		if (inst->dword.fp_dsetp.cmp == 1)
			str_printf(inst_str_ptr, inst_str_size, ".LT");
		else if (inst->dword.fp_dsetp.cmp == 2)
			str_printf(inst_str_ptr, inst_str_size, ".EQ");
		else if (inst->dword.fp_dsetp.cmp == 3)
			str_printf(inst_str_ptr, inst_str_size, ".LE");
		else if (inst->dword.fp_dsetp.cmp == 4)
			str_printf(inst_str_ptr, inst_str_size, ".GT");
		else if (inst->dword.fp_dsetp.cmp == 5)
			str_printf(inst_str_ptr, inst_str_size, ".NE");
		else if (inst->dword.fp_dsetp.cmp == 6)
			str_printf(inst_str_ptr, inst_str_size, ".GE");
		else if (inst->dword.fp_dsetp.cmp == 7)
			str_printf(inst_str_ptr, inst_str_size, ".NUM");
		else if (inst->dword.fp_dsetp.cmp == 8)
			str_printf(inst_str_ptr, inst_str_size, ".NAN");
		else if (inst->dword.fp_dsetp.cmp == 9)
			str_printf(inst_str_ptr, inst_str_size, ".LTU");
		else if (inst->dword.fp_dsetp.cmp == 10)
			str_printf(inst_str_ptr, inst_str_size, ".EQU");
		else if (inst->dword.fp_dsetp.cmp == 11)
			str_printf(inst_str_ptr, inst_str_size, ".LEU");
		else if (inst->dword.fp_dsetp.cmp == 12)
			str_printf(inst_str_ptr, inst_str_size, ".GTU");
		else if (inst->dword.fp_dsetp.cmp == 14)
			str_printf(inst_str_ptr, inst_str_size, ".GEU");
		else
			fatal("%d: FRM_FMT_FP_DSETP.cmp not recognized", inst->dword.fp_dsetp.cmp);

		if (inst->dword.fp_dsetp.logic == 0)
			str_printf(inst_str_ptr, inst_str_size, ".AND");
		else if (inst->dword.fp_dsetp.logic == 1)
			str_printf(inst_str_ptr, inst_str_size, ".OR");
		else if (inst->dword.fp_dsetp.logic == 2)
			str_printf(inst_str_ptr, inst_str_size, ".XOR");
		else
			fatal("%d: FRM_FMT_FP_DSETP.logic not recognized", inst->dword.fp_dsetp.logic);
	}
	else if (fmt == FRM_FMT_INT_IMAD)
	{
		if (inst->dword.int_imad.mod1 == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U32");
		else if (inst->dword.int_imad.mod1 == 1)
			;
		else
			fatal("%d: FRM_FMT_INT_IMAD.mod1 not recognized", inst->dword.int_imad.mod1);

		if (inst->dword.int_imad.mod2 == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U32");
		else if (inst->dword.int_imad.mod2 == 1)
			;
		else
			fatal("%d: FRM_FMT_INT_IMAD.mod2 not recognized", inst->dword.int_imad.mod2);

		if (inst->dword.int_imad.hi == 0)
			;
		else if (inst->dword.int_imad.hi == 1)
			str_printf(inst_str_ptr, inst_str_size, ".HI");
		else
			fatal("%d: FRM_FMT_INT_IMAD.hi not recognized", inst->dword.int_imad.hi);

		if (inst->dword.int_imad.sat == 0)
			;
		else if (inst->dword.int_imad.sat == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SAT");
		else
			fatal("%d: FRM_FMT_INT_IMAD.sat not recognized", inst->dword.int_imad.sat);
	}
	else if (fmt == FRM_FMT_INT_IMUL)
	{
		if (inst->dword.int_imul.mod1 == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U32");
		else if (inst->dword.int_imul.mod1 == 1)
			;
		else
			fatal("%d: FRM_FMT_INT_IMUL.mod1 not recognized", inst->dword.int_imul.mod1);

		if (inst->dword.int_imul.mod2 == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U32");
		else if (inst->dword.int_imul.mod2 == 1)
			;
		else
			fatal("%d: FRM_FMT_INT_IMUL.mod2 not recognized", inst->dword.int_imul.mod2);

		if (inst->dword.int_imul.hi == 0)
			;
		else if (inst->dword.int_imul.hi == 1)
			str_printf(inst_str_ptr, inst_str_size, ".HI");
		else
			fatal("%d: FRM_FMT_INT_IMUL.hi not recognized", inst->dword.int_imul.hi);
	}
	else if (fmt == FRM_FMT_INT_IADD)
	{
		if (inst->dword.int_iadd.sat == 0)
			;
		else if (inst->dword.int_iadd.sat == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SAT");
		else
			fatal("%d: FRM_FMT_INT_IADD.sat not recognized", inst->dword.int_iadd.sat);

		if (inst->dword.int_iadd.x == 0)
			;
		else if (inst->dword.int_iadd.x == 1)
			str_printf(inst_str_ptr, inst_str_size, ".X");
		else
			fatal("%d: FRM_FMT_INT_IADD.x not recognized", inst->dword.int_iadd.x);
	}
	else if (fmt == FRM_FMT_INT_ISCADD)
		;
	else if (fmt == FRM_FMT_INT_BFE)
	{
		if (inst->dword.int_bfe.u32 == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U32");
		else if (inst->dword.int_bfe.u32 == 1)
			;
		else
			fatal("%d: FRM_FMT_INT_BFE.u32 not recognized", inst->dword.int_bfe.u32);

		if (inst->dword.int_bfe.brev == 0)
			;
		else if (inst->dword.int_bfe.brev == 1)
			str_printf(inst_str_ptr, inst_str_size, ".BREV");
		else
			fatal("%d: FRM_FMT_INT_BFE.brev not recognized", inst->dword.int_bfe.brev);
	}
	else if (fmt == FRM_FMT_INT_BFI)
		;
	else if (fmt == FRM_FMT_INT_SHR)
	{
		if (inst->dword.int_shr.u32 == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U32");
		else if (inst->dword.int_shr.u32 == 1)
			;
		else
			fatal("%d: FRM_FMT_INT_SHR.u32 not recognized", inst->dword.int_shr.u32);

		if (inst->dword.int_shr.w == 0)
			;
		else if (inst->dword.int_shr.w == 1)
			str_printf(inst_str_ptr, inst_str_size, ".W");
		else
			fatal("%d: FRM_FMT_INT_SHR.w not recognized", inst->dword.int_shr.w);
	}
	else if (fmt == FRM_FMT_INT_SHL)
	{
		if (inst->dword.int_shl.u32 == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U32");
		else if (inst->dword.int_shl.u32 == 1)
			;
		else
			fatal("%d: FRM_FMT_INT_SHL.u32 not recognized", inst->dword.int_shl.u32);

		if (inst->dword.int_shl.w == 0)
			;
		else if (inst->dword.int_shl.w == 1)
			str_printf(inst_str_ptr, inst_str_size, ".W");
		else
			fatal("%d: FRM_FMT_INT_SHL.w not recognized", inst->dword.int_shl.w);
	}
	else if (fmt == FRM_FMT_INT_LOP)
	{
		if (inst->dword.int_lop.op == 0)
			str_printf(inst_str_ptr, inst_str_size, ".AND");
		else if (inst->dword.int_lop.op == 1)
			str_printf(inst_str_ptr, inst_str_size, ".OR");
		else if (inst->dword.int_lop.op == 2)
			str_printf(inst_str_ptr, inst_str_size, ".XOR");
		else if (inst->dword.int_lop.op == 3)
			str_printf(inst_str_ptr, inst_str_size, ".PASS_B");
		else
			fatal("%d: FRM_FMT_INT_LOP.op not recognized", inst->dword.int_lop.op);
	}
	else if (fmt == FRM_FMT_INT_LOP32I)
	{
		if (inst->dword.int_lop32i.op == 0)
			str_printf(inst_str_ptr, inst_str_size, ".AND");
		else if (inst->dword.int_lop32i.op == 1)
			str_printf(inst_str_ptr, inst_str_size, ".OR");
		else if (inst->dword.int_lop32i.op == 2)
			str_printf(inst_str_ptr, inst_str_size, ".XOR");
		else if (inst->dword.int_lop32i.op == 3)
			str_printf(inst_str_ptr, inst_str_size, ".PASS_B");
		else
			fatal("%d: FRM_FMT_INT_LOP.op not recognized", inst->dword.int_lop32i.op);
	}
	else if (fmt == FRM_FMT_INT_ISETP)
	{
		if (inst->dword.int_isetp.cmp == 1)
			str_printf(inst_str_ptr, inst_str_size, ".LT");
		else if (inst->dword.int_isetp.cmp == 2)
			str_printf(inst_str_ptr, inst_str_size, ".EQ");
		else if (inst->dword.int_isetp.cmp == 3)
			str_printf(inst_str_ptr, inst_str_size, ".LE");
		else if (inst->dword.int_isetp.cmp == 4)
			str_printf(inst_str_ptr, inst_str_size, ".GT");
		else if (inst->dword.int_isetp.cmp == 5)
			str_printf(inst_str_ptr, inst_str_size, ".NE");
		else if (inst->dword.int_isetp.cmp == 6)
			str_printf(inst_str_ptr, inst_str_size, ".GE");
		else
			fatal("%d: FRM_FMT_INT_ISETP.cmp not recognized", inst->dword.int_isetp.cmp);

		if (inst->dword.int_isetp.u32 == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U32");
		else if (inst->dword.int_isetp.u32 == 1)
			;
		else
			fatal("%d: FRM_FMT_INT_ISETP.u32 not recognized", inst->dword.int_isetp.u32);

		if (inst->dword.int_isetp.logic == 0)
			str_printf(inst_str_ptr, inst_str_size, ".AND");
		else if (inst->dword.int_isetp.logic == 1)
			str_printf(inst_str_ptr, inst_str_size, ".OR");
		else if (inst->dword.int_isetp.logic == 2)
			str_printf(inst_str_ptr, inst_str_size, ".XOR");
		else
			fatal("%d: FRM_FMT_INT_ISETP.logic not recognized", inst->dword.int_isetp.logic);
	}
	else if (fmt == FRM_FMT_INT_ICMP)
	{
		if (inst->dword.int_icmp.cmp == 1)
			str_printf(inst_str_ptr, inst_str_size, ".LT");
		else if (inst->dword.int_icmp.cmp == 2)
			str_printf(inst_str_ptr, inst_str_size, ".EQ");
		else if (inst->dword.int_icmp.cmp == 3)
			str_printf(inst_str_ptr, inst_str_size, ".LE");
		else if (inst->dword.int_icmp.cmp == 4)
			str_printf(inst_str_ptr, inst_str_size, ".GT");
		else if (inst->dword.int_icmp.cmp == 5)
			str_printf(inst_str_ptr, inst_str_size, ".NE");
		else if (inst->dword.int_icmp.cmp == 6)
			str_printf(inst_str_ptr, inst_str_size, ".GE");
		else
			fatal("%d: FRM_FMT_INT_ICMP.cmp not recognized", inst->dword.int_icmp.cmp);

		if (inst->dword.int_icmp.u32 == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U32");
		else if (inst->dword.int_icmp.u32 == 1)
			;
		else
			fatal("%d: FRM_FMT_INT_ICMP.u32 not recognized", inst->dword.int_icmp.u32);
	}
	else if (fmt == FRM_FMT_CONV_F2F)
	{
		if (inst->dword.conv_f2f.ftz == 0)
			;
		else if (inst->dword.conv_f2f.ftz == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FTZ");
		else
			fatal("%d: FRM_FMT_CONV_F2F.ftz not recognized", inst->dword.conv_f2f.ftz);

		if (inst->dword.conv_f2f.dtype == 0)
			;
		else if (inst->dword.conv_f2f.dtype == 1)
			str_printf(inst_str_ptr, inst_str_size, ".F16");
		else if (inst->dword.conv_f2f.dtype == 2)
			str_printf(inst_str_ptr, inst_str_size, ".F32");
		else if (inst->dword.conv_f2f.dtype == 3)
			str_printf(inst_str_ptr, inst_str_size, ".F64");
		else
			fatal("%d: FRM_FMT_CONV_F2F.dtype not recognized", inst->dword.conv_f2f.dtype);

		if (inst->dword.conv_f2f.stype == 0)
			;
		else if (inst->dword.conv_f2f.stype == 1)
			str_printf(inst_str_ptr, inst_str_size, ".F16");
		else if (inst->dword.conv_f2f.stype == 2)
			str_printf(inst_str_ptr, inst_str_size, ".F32");
		else if (inst->dword.conv_f2f.stype == 3)
			str_printf(inst_str_ptr, inst_str_size, ".F64");
		else
			fatal("%d: FRM_FMT_CONV_F2F.stype not recognized", inst->dword.conv_f2f.stype);

		if (inst->dword.conv_f2f.rnd == 0)
			;
		else if (inst->dword.conv_f2f.rnd == 1)
			str_printf(inst_str_ptr, inst_str_size, ".RM");
		else if (inst->dword.conv_f2f.rnd == 2)
			str_printf(inst_str_ptr, inst_str_size, ".RP");
		else if (inst->dword.conv_f2f.rnd == 3)
			str_printf(inst_str_ptr, inst_str_size, ".RZ");
		else
			fatal("%d: FRM_FMT_CONV_F2F.rnd not recognized", inst->dword.conv_f2f.rnd);
	}
	else if (fmt == FRM_FMT_CONV_F2I)
	{
		if (inst->dword.conv_f2i.ftz == 0)
			;
		else if (inst->dword.conv_f2i.ftz == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FTZ");
		else
			fatal("%d: FRM_FMT_CONV_F2I.ftz not recognized", inst->dword.conv_f2i.ftz);

		if (inst->dword.conv_f2i.dtype_s == 0 && inst->dword.conv_f2i.dtype_n != 0)
			str_printf(inst_str_ptr, inst_str_size, ".U");
		else if (inst->dword.conv_f2i.dtype_s == 1 && inst->dword.conv_f2i.dtype_n != 0)
			str_printf(inst_str_ptr, inst_str_size, ".S");
		else
			fatal("%d: FRM_FMT_CONV_F2I.dtype_s not recognized", inst->dword.conv_f2i.dtype_s);

		if (inst->dword.conv_f2i.dtype_n == 0)
			;
		else if (inst->dword.conv_f2i.dtype_n == 1)
			str_printf(inst_str_ptr, inst_str_size, "16");
		else if (inst->dword.conv_f2i.dtype_n == 2)
			str_printf(inst_str_ptr, inst_str_size, "32");
		else if (inst->dword.conv_f2i.dtype_n == 3)
			str_printf(inst_str_ptr, inst_str_size, "64");
		else
			fatal("%d: FRM_FMT_CONV_F2I.dtype_n not recognized", inst->dword.conv_f2i.dtype_n);

		if (inst->dword.conv_f2i.stype == 0)
			;
		else if (inst->dword.conv_f2i.stype == 1)
			str_printf(inst_str_ptr, inst_str_size, ".F16");
		else if (inst->dword.conv_f2i.stype == 2)
			;
		else if (inst->dword.conv_f2i.stype == 3)
			str_printf(inst_str_ptr, inst_str_size, ".F64");
		else
			fatal("%d: FRM_FMT_CONV_F2I.stype not recognized", inst->dword.conv_f2i.stype);

		if (inst->dword.conv_f2i.rnd == 0)
			;
		else if (inst->dword.conv_f2i.rnd == 1)
			str_printf(inst_str_ptr, inst_str_size, ".FLOOR");
		else if (inst->dword.conv_f2i.rnd == 2)
			str_printf(inst_str_ptr, inst_str_size, ".CEIL");
		else if (inst->dword.conv_f2i.rnd == 3)
			str_printf(inst_str_ptr, inst_str_size, ".TRUNC");
		else
			fatal("%d: FRM_FMT_CONV_F2I.rnd not recognized", inst->dword.conv_f2i.rnd);
	}
	else if (fmt == FRM_FMT_CONV_I2F)
	{
		if (inst->dword.conv_i2f.dtype == 0)
			;
		else if (inst->dword.conv_i2f.dtype == 1)
			str_printf(inst_str_ptr, inst_str_size, ".F16");
		else if (inst->dword.conv_i2f.dtype == 2)
			;
		else if (inst->dword.conv_i2f.dtype == 3)
			str_printf(inst_str_ptr, inst_str_size, ".F64");
		else
			fatal("%d: FRM_FMT_CONV_I2F.dtype not recognized", inst->dword.conv_i2f.dtype);

		if (inst->dword.conv_i2f.stype_s == 0 && inst->dword.conv_i2f.stype_n != 0)
			str_printf(inst_str_ptr, inst_str_size, ".U");
		else if (inst->dword.conv_i2f.stype_s == 1 && inst->dword.conv_i2f.stype_n != 0)
			str_printf(inst_str_ptr, inst_str_size, ".S");
		else
			fatal("%d: FRM_FMT_CONV_I2F.stype_s not recognized", inst->dword.conv_i2f.stype_s);

		if (inst->dword.conv_i2f.stype_n == 0)
			;
		else if (inst->dword.conv_i2f.stype_n == 1)
			str_printf(inst_str_ptr, inst_str_size, "16");
		else if (inst->dword.conv_i2f.stype_n == 2)
			str_printf(inst_str_ptr, inst_str_size, "32");
		else if (inst->dword.conv_i2f.stype_n == 3)
			str_printf(inst_str_ptr, inst_str_size, "64");
		else
			fatal("%d: FRM_FMT_CONV_I2F.stype_n not recognized", inst->dword.conv_i2f.stype_n);

		if (inst->dword.conv_i2f.rnd == 0)
			;
		else if (inst->dword.conv_i2f.rnd == 1)
			str_printf(inst_str_ptr, inst_str_size, ".RM");
		else if (inst->dword.conv_i2f.rnd == 2)
			str_printf(inst_str_ptr, inst_str_size, ".RP");
		else if (inst->dword.conv_i2f.rnd == 3)
			str_printf(inst_str_ptr, inst_str_size, ".RZ");
		else
			fatal("%d: FRM_FMT_CONV_I2F.rnd not recognized", inst->dword.conv_i2f.rnd);
	}
	else if (fmt == FRM_FMT_CONV_I2I)
	{
		if (inst->dword.conv_i2i.dtype_s == 0 && inst->dword.conv_i2i.dtype_n != 0)
			str_printf(inst_str_ptr, inst_str_size, ".U");
		else if (inst->dword.conv_i2i.dtype_s == 1 && inst->dword.conv_i2i.dtype_n != 0)
			str_printf(inst_str_ptr, inst_str_size, ".S");
		else
			fatal("%d: FRM_FMT_CONV_I2I.dtype_s not recognized", inst->dword.conv_i2i.dtype_s);

		if (inst->dword.conv_i2i.dtype_n == 0)
			;
		else if (inst->dword.conv_i2i.dtype_n == 1)
			str_printf(inst_str_ptr, inst_str_size, "16");
		else if (inst->dword.conv_i2i.dtype_n == 2)
			str_printf(inst_str_ptr, inst_str_size, "32");
		else if (inst->dword.conv_i2i.dtype_n == 3)
			str_printf(inst_str_ptr, inst_str_size, "64");
		else
			fatal("%d: FRM_FMT_CONV_I2I.dtype_n not recognized", inst->dword.conv_i2i.dtype_n);

		if (inst->dword.conv_i2i.stype_s == 0 && inst->dword.conv_i2i.stype_n != 0)
			str_printf(inst_str_ptr, inst_str_size, ".U");
		else if (inst->dword.conv_i2i.stype_s == 1 && inst->dword.conv_i2i.stype_n != 0)
			str_printf(inst_str_ptr, inst_str_size, ".S");
		else
			fatal("%d: FRM_FMT_CONV_I2I.stype_s not recognized", inst->dword.conv_i2i.stype_s);

		if (inst->dword.conv_i2i.stype_n == 0)
			;
		else if (inst->dword.conv_i2i.stype_n == 1)
			str_printf(inst_str_ptr, inst_str_size, ".16");
		else if (inst->dword.conv_i2i.stype_n == 2)
			;
		else if (inst->dword.conv_i2i.stype_n == 3)
			str_printf(inst_str_ptr, inst_str_size, ".64");
		else
			fatal("%d: FRM_FMT_CONV_I2I.stype_n not recognized", inst->dword.conv_i2i.stype_n);

		if (inst->dword.conv_i2i.sat == 0)
			;
		else if (inst->dword.conv_i2i.sat == 1)
			str_printf(inst_str_ptr, inst_str_size, ".SAT");
		else
			fatal("%d: FRM_FMT_CONV_I2I.sat not recognized", inst->dword.conv_i2i.sat);
	}
	else if (fmt == FRM_FMT_MOV_MOV)
	{
		if (inst->dword.mov_mov.s == 0)
			;
		else if (inst->dword.mov_mov.s == 1)
			str_printf(inst_str_ptr, inst_str_size, ".S");
		else
			fatal("%d: FRM_INST_MOV_MOV.s not recognized", inst->dword.mov_mov.s);
	}
	else if (fmt == FRM_FMT_MOV_MOV32I)
	{
		if (inst->dword.mov_mov32i.s == 0)
			;
		else if (inst->dword.mov_mov32i.s == 1)
			str_printf(inst_str_ptr, inst_str_size, ".S");
		else
			fatal("%d: FRM_INST_MOV_MOV32I.s not recognized", inst->dword.mov_mov32i.s);
	}
	else if (fmt == FRM_FMT_MOV_SEL)
		;
	else if (fmt == FRM_FMT_LDST_LD)
	{
		if (inst->dword.ldst_ld.e == 0)
			;
		else if (inst->dword.ldst_ld.e == 1)
			str_printf(inst_str_ptr, inst_str_size, ".E");
		else
			fatal("%d: FRM_FMT_LDST_LD.e not recognized", inst->dword.ldst_ld.e);

		if (inst->dword.ldst_ld.cop == 0)
			;
		else if (inst->dword.ldst_ld.cop == 1)
			str_printf(inst_str_ptr, inst_str_size, ".CG");
		else if (inst->dword.ldst_ld.cop == 2)
			str_printf(inst_str_ptr, inst_str_size, ".CS");
		else if (inst->dword.ldst_ld.cop == 3)
			str_printf(inst_str_ptr, inst_str_size, ".CV");
		else
			fatal("%d: FRM_FMT_LDST_LD.cop not recognized", inst->dword.ldst_ld.cop);

		if (inst->dword.ldst_ld.type == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U8");
		else if (inst->dword.ldst_ld.type == 1)
			str_printf(inst_str_ptr, inst_str_size, ".S8");
		else if (inst->dword.ldst_ld.type == 2)
			str_printf(inst_str_ptr, inst_str_size, ".U16");
		else if (inst->dword.ldst_ld.type == 3)
			str_printf(inst_str_ptr, inst_str_size, ".S16");
		else if (inst->dword.ldst_ld.type == 4)
			;
		else if (inst->dword.ldst_ld.type == 5)
			str_printf(inst_str_ptr, inst_str_size, ".64");
		else if (inst->dword.ldst_ld.type == 6)
			str_printf(inst_str_ptr, inst_str_size, ".128");
		else
			fatal("%d: FRM_FMT_LDST_LD.type not recognized", inst->dword.ldst_ld.type);
	}
	else if (fmt == FRM_FMT_LDST_ST)
	{
		if (inst->dword.ldst_st.e == 0)
			;
		else if (inst->dword.ldst_st.e == 1)
			str_printf(inst_str_ptr, inst_str_size, ".E");
		else
			fatal("%d: FRM_FMT_LDST_ST.e not recognized", inst->dword.ldst_ld.e);

		if (inst->dword.ldst_st.cop == 0)
			;
		else if (inst->dword.ldst_st.cop == 1)
			str_printf(inst_str_ptr, inst_str_size, ".CG");
		else if (inst->dword.ldst_st.cop == 2)
			str_printf(inst_str_ptr, inst_str_size, ".CS");
		else if (inst->dword.ldst_st.cop == 3)
			str_printf(inst_str_ptr, inst_str_size, ".WT");
		else
			fatal("%d: FRM_FMT_LDST_ST.cop not recognized", inst->dword.ldst_st.cop);

		if (inst->dword.ldst_st.type == 0)
			str_printf(inst_str_ptr, inst_str_size, ".U8");
		else if (inst->dword.ldst_st.type == 1)
			str_printf(inst_str_ptr, inst_str_size, ".S8");
		else if (inst->dword.ldst_st.type == 2)
			str_printf(inst_str_ptr, inst_str_size, ".U16");
		else if (inst->dword.ldst_st.type == 3)
			str_printf(inst_str_ptr, inst_str_size, ".S16");
		else if (inst->dword.ldst_st.type == 4)
			;
		else if (inst->dword.ldst_st.type == 5)
			str_printf(inst_str_ptr, inst_str_size, ".64");
		else if (inst->dword.ldst_st.type == 6)
			str_printf(inst_str_ptr, inst_str_size, ".128");
		else
			fatal("%d: FRM_FMT_LDST_ST.type not recognized", inst->dword.ldst_st.type);
	}
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

	if (fmt == FRM_FMT_FP_FFMA)
		dst = inst->dword.fp_ffma.dst;
	else if (fmt == FRM_FMT_FP_FADD)
		dst = inst->dword.fp_fadd.dst;
	else if (fmt == FRM_FMT_FP_FADD32I)
		dst = inst->dword.fp_fadd32i.dst;
	else if (fmt == FRM_FMT_FP_FCMP)
		dst = inst->dword.fp_fcmp.dst;
	else if (fmt == FRM_FMT_FP_FMUL)
		dst = inst->dword.fp_fmul.dst;
	else if (fmt == FRM_FMT_FP_FMUL32I)
		dst = inst->dword.fp_fmul32i.dst;
	else if (fmt == FRM_FMT_FP_MUFU)
		dst = inst->dword.fp_mufu.dst;
	else if (fmt == FRM_FMT_FP_DFMA)
		dst = inst->dword.fp_dfma.dst;
	else if (fmt == FRM_FMT_FP_DADD)
		dst = inst->dword.fp_dadd.dst;
	else if (fmt == FRM_FMT_FP_DMUL)
		dst = inst->dword.fp_dmul.dst;
	else if (fmt == FRM_FMT_INT_IMAD)
		dst = inst->dword.int_imad.dst;
	else if (fmt == FRM_FMT_INT_IMUL)
		dst = inst->dword.int_imul.dst;
	else if (fmt == FRM_FMT_INT_IADD)
		dst = inst->dword.int_iadd.dst;
	else if (fmt == FRM_FMT_INT_ISCADD)
		dst = inst->dword.int_iscadd.dst;
	else if (fmt == FRM_FMT_INT_BFE)
		dst = inst->dword.int_bfe.dst;
	else if (fmt == FRM_FMT_INT_BFI)
		dst = inst->dword.int_bfi.dst;
	else if (fmt == FRM_FMT_INT_SHR)
		dst = inst->dword.int_shr.dst;
	else if (fmt == FRM_FMT_INT_SHL)
		dst = inst->dword.int_shl.dst;
	else if (fmt == FRM_FMT_INT_LOP)
		dst = inst->dword.int_lop.dst;
	else if (fmt == FRM_FMT_INT_LOP32I)
		dst = inst->dword.int_lop32i.dst;
	else if (fmt == FRM_FMT_INT_ICMP)
		dst = inst->dword.int_icmp.dst;
	else if (fmt == FRM_FMT_CONV_F2F)
		dst = inst->dword.conv_f2f.dst;
	else if (fmt == FRM_FMT_CONV_F2I)
		dst = inst->dword.conv_f2i.dst;
	else if (fmt == FRM_FMT_CONV_I2F)
		dst = inst->dword.conv_i2f.dst;
	else if (fmt == FRM_FMT_CONV_I2I)
		dst = inst->dword.conv_i2i.dst;
	else if (fmt == FRM_FMT_MOV_MOV)
		dst = inst->dword.mov_mov.dst;
	else if (fmt == FRM_FMT_MOV_MOV32I)
		dst = inst->dword.mov_mov32i.dst;
	else if (fmt == FRM_FMT_MOV_SEL)
		dst = inst->dword.mov_sel.dst;
	else if (fmt == FRM_FMT_LDST_LD)
		dst = inst->dword.ldst_ld.dst;
	else if (fmt == FRM_FMT_LDST_ST)
		dst = inst->dword.ldst_st.dst;
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


void frm_inst_dump_P(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int P;

	if (fmt == FRM_FMT_FP_FSETP)
		P = inst->dword.fp_fsetp.P;
	else if (fmt == FRM_FMT_FP_DSETP)
		P = inst->dword.fp_dsetp.P;
	else if (fmt == FRM_FMT_INT_ISETP)
		P = inst->dword.int_isetp.P;
	else
		fatal("%d: %%P not recognized", fmt);

	str_printf(inst_str_ptr, inst_str_size, "P%lld", P);
}


void frm_inst_dump_Q(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int Q;

	if (fmt == FRM_FMT_FP_FSETP)
		Q = inst->dword.fp_fsetp.Q;
	else if (fmt == FRM_FMT_FP_DSETP)
		Q = inst->dword.fp_dsetp.Q;
	else if (fmt == FRM_FMT_INT_ISETP)
		Q = inst->dword.int_isetp.Q;
	else
		fatal("%d: %%Q not recognized", fmt);

	if (Q != 7)
		str_printf(inst_str_ptr, inst_str_size, "p%lld", Q);
	else
		str_printf(inst_str_ptr, inst_str_size, "pt");
}

void frm_inst_dump_src1(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int src1;

	if (fmt == FRM_FMT_FP_FFMA)
		src1 = inst->dword.fp_ffma.src1;
	else if (fmt == FRM_FMT_FP_FADD)
		src1 = inst->dword.fp_fadd.src1;
	else if (fmt == FRM_FMT_FP_FADD32I)
		src1 = inst->dword.fp_fadd32i.src1;
	else if (fmt == FRM_FMT_FP_FCMP)
		src1 = inst->dword.fp_fcmp.src1;
	else if (fmt == FRM_FMT_FP_FMUL)
		src1 = inst->dword.fp_fmul.src1;
	else if (fmt == FRM_FMT_FP_FMUL32I)
		src1 = inst->dword.fp_fmul32i.src1;
	else if (fmt == FRM_FMT_FP_FSETP)
		src1 = inst->dword.fp_fsetp.src1;
	else if (fmt == FRM_FMT_FP_MUFU)
		src1 = inst->dword.fp_mufu.src1;
	else if (fmt == FRM_FMT_FP_DFMA)
		src1 = inst->dword.fp_dfma.src1;
	else if (fmt == FRM_FMT_FP_DADD)
		src1 = inst->dword.fp_dadd.src1;
	else if (fmt == FRM_FMT_FP_DMUL)
		src1 = inst->dword.fp_dmul.src1;
	else if (fmt == FRM_FMT_FP_DSETP)
		src1 = inst->dword.fp_dsetp.src1;
	else if (fmt == FRM_FMT_INT_IMAD)
		src1 = inst->dword.int_imad.src1;
	else if (fmt == FRM_FMT_INT_IMUL)
		src1 = inst->dword.int_imul.src1;
	else if (fmt == FRM_FMT_INT_IADD)
		src1 = inst->dword.int_iadd.src1;
	else if (fmt == FRM_FMT_INT_ISCADD)
		src1 = inst->dword.int_iscadd.src1;
	else if (fmt == FRM_FMT_INT_BFE)
		src1 = inst->dword.int_bfe.src1;
	else if (fmt == FRM_FMT_INT_BFI)
		src1 = inst->dword.int_bfi.src1;
	else if (fmt == FRM_FMT_INT_SHR)
		src1 = inst->dword.int_shr.src1;
	else if (fmt == FRM_FMT_INT_SHL)
		src1 = inst->dword.int_shl.src1;
	else if (fmt == FRM_FMT_INT_LOP)
		src1 = inst->dword.int_lop.src1;
	else if (fmt == FRM_FMT_INT_LOP32I)
		src1 = inst->dword.int_lop32i.src1;
	else if (fmt == FRM_FMT_INT_ISETP)
		src1 = inst->dword.int_isetp.src1;
	else if (fmt == FRM_FMT_INT_ICMP)
		src1 = inst->dword.int_icmp.src1;
	else if (fmt == FRM_FMT_MOV_SEL)
		src1 = inst->dword.mov_sel.src1;
	else if (fmt == FRM_FMT_LDST_LD)
		src1 = inst->dword.ldst_ld.src1;
	else if (fmt == FRM_FMT_LDST_ST)
		src1 = inst->dword.ldst_st.src1;
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


void frm_inst_dump_src2(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int bank_id;
	unsigned long long int offset_in_bank;
	char *sreg;

	if (fmt == FRM_FMT_FP_FFMA)
	{
		if (inst->dword.fp_ffma.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_ffma.src2 & 0x3f);
		else if (inst->dword.fp_ffma.src2_mod == 1)
		{
			bank_id = inst->dword.fp_ffma.src2 >> 16;
			offset_in_bank= inst->dword.fp_ffma.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_ffma.src2_mod == 2)
			;
		else if (inst->dword.fp_ffma.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_FFMA.src2_mod not recognized", inst->dword.fp_ffma.src2_mod);
	}
	else if (fmt == FRM_FMT_FP_FADD)
	{
		if (inst->dword.fp_fadd.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_fadd.src2 & 0x3f);
		else if (inst->dword.fp_fadd.src2_mod == 1)
		{
			bank_id = inst->dword.fp_fadd.src2 >> 16;
			offset_in_bank= inst->dword.fp_fadd.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_fadd.src2_mod == 2)
			;
		else if (inst->dword.fp_fadd.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_FADD.src2_mod not recognized", inst->dword.fp_fadd.src2_mod);
	}
	else if (fmt == FRM_FMT_FP_FCMP)
	{
		if (inst->dword.fp_fcmp.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_fcmp.src2 & 0x3f);
		else if (inst->dword.fp_fcmp.src2_mod == 1)
		{
			bank_id = inst->dword.fp_fcmp.src2 >> 16;
			offset_in_bank= inst->dword.fp_fcmp.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_fcmp.src2_mod == 2)
			;
		else if (inst->dword.fp_fcmp.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_FCMP.src2_mod not recognized", inst->dword.fp_fcmp.src2_mod);
	}
	else if (fmt == FRM_FMT_FP_FMUL)
	{
		if (inst->dword.fp_fmul.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_fmul.src2 & 0x3f);
		else if (inst->dword.fp_fmul.src2_mod == 1)
		{
			bank_id = inst->dword.fp_fmul.src2 >> 16;
			offset_in_bank= inst->dword.fp_fmul.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_fmul.src2_mod == 2)
			;
		else if (inst->dword.fp_fmul.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_FMUL.src2_mod not recognized", inst->dword.fp_fmul.src2_mod);
	}
	else if (fmt == FRM_FMT_FP_FSETP)
	{
		if (inst->dword.fp_fsetp.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_fsetp.src2 & 0x3f);
		else if (inst->dword.fp_fsetp.src2_mod == 1)
		{
			bank_id = inst->dword.fp_fsetp.src2 >> 16;
			offset_in_bank= inst->dword.fp_fsetp.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_fsetp.src2_mod == 2)
			;
		else if (inst->dword.fp_fsetp.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_FSETP.src2_mod not recognized", inst->dword.fp_fsetp.src2_mod);
	}
	else if (fmt == FRM_FMT_FP_DFMA)
	{
		if (inst->dword.fp_dfma.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_dfma.src2 & 0x3f);
		else if (inst->dword.fp_dfma.src2_mod == 1)
		{
			bank_id = inst->dword.fp_dfma.src2 >> 16;
			offset_in_bank= inst->dword.fp_dfma.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_dfma.src2_mod == 2)
			;
		else if (inst->dword.fp_dfma.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_DFMA.src2_mod not recognized", inst->dword.fp_dfma.src2_mod);
	}
	else if (fmt == FRM_FMT_FP_DADD)
	{
		if (inst->dword.fp_dadd.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_dadd.src2 & 0x3f);
		else if (inst->dword.fp_dadd.src2_mod == 1)
		{
			bank_id = inst->dword.fp_dadd.src2 >> 16;
			offset_in_bank= inst->dword.fp_dadd.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_dadd.src2_mod == 2)
			;
		else if (inst->dword.fp_dadd.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_DADD.src2_mod not recognized", inst->dword.fp_dadd.src2_mod);
	}
	else if (fmt == FRM_FMT_FP_DMUL)
	{
		if (inst->dword.fp_dmul.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_dmul.src2 & 0x3f);
		else if (inst->dword.fp_dmul.src2_mod == 1)
		{
			bank_id = inst->dword.fp_dmul.src2 >> 16;
			offset_in_bank= inst->dword.fp_dmul.src2 & 0xffff;
			str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_dmul.src2_mod == 2)
			;
		else if (inst->dword.fp_dmul.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_DMUL.src2_mod not recognized", inst->dword.fp_dmul.src2_mod);
	}
	else if (fmt == FRM_FMT_FP_DSETP)
	{
		if (inst->dword.fp_dsetp.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_dsetp.src2 & 0x3f);
		else if (inst->dword.fp_dsetp.src2_mod == 1)
		{
			bank_id = inst->dword.fp_dsetp.src2 >> 16;
			offset_in_bank= inst->dword.fp_dsetp.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.fp_dsetp.src2_mod == 2)
			;
		else if (inst->dword.fp_dsetp.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_FP_DSETP.src2_mod not recognized", inst->dword.fp_dsetp.src2_mod);
	}
	else if (fmt == FRM_FMT_INT_IMAD)
	{
		if (inst->dword.int_imad.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_imad.src2 & 0x3f);
		else if (inst->dword.int_imad.src2_mod == 1)
		{
			bank_id = inst->dword.int_imad.src2 >> 16;
			offset_in_bank= inst->dword.int_imad.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.int_imad.src2_mod == 2)
			;
		else if (inst->dword.int_imad.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_INT_IMAD.src2_mod not recognized", inst->dword.int_imad.src2_mod);
	}
	else if (fmt == FRM_FMT_INT_IMUL)
	{
		if (inst->dword.int_imul.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_imul.src2 & 0x3f);
		else if (inst->dword.int_imul.src2_mod == 1)
		{
			bank_id = inst->dword.int_imul.src2 >> 16;
			offset_in_bank= inst->dword.int_imul.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.int_imul.src2_mod == 2)
			;
		else if (inst->dword.int_imul.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_INT_IMUL.src2_mod not recognized", inst->dword.int_imul.src2_mod);
	}
	else if (fmt == FRM_FMT_INT_IADD)
	{
		if (inst->dword.int_iadd.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_iadd.src2 & 0x3f);
		else if (inst->dword.int_iadd.src2_mod == 1)
		{
			bank_id = inst->dword.int_iadd.src2 >> 16;
			offset_in_bank= inst->dword.int_iadd.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.int_iadd.src2_mod == 2)
			;
		else if (inst->dword.int_iadd.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_INT_IADD.src2_mod not recognized", inst->dword.int_iadd.src2_mod);
	}
	else if (fmt == FRM_FMT_INT_ISCADD)
	{
		if (inst->dword.int_iscadd.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_iscadd.src2 & 0x3f);
		else if (inst->dword.int_iscadd.src2_mod == 1)
		{
			bank_id = inst->dword.int_iscadd.src2 >> 16;
			offset_in_bank= inst->dword.int_iscadd.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.int_iscadd.src2_mod == 2)
			;
		else if (inst->dword.int_iscadd.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_INT_ISCADD.src2_mod not recognized", inst->dword.int_iscadd.src2_mod);
	}
	else if (fmt == FRM_FMT_INT_BFE)
	{
		if (inst->dword.int_bfe.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_bfe.src2 & 0x3f);
		else if (inst->dword.int_bfe.src2_mod == 1)
		{
			bank_id = inst->dword.int_bfe.src2 >> 16;
			offset_in_bank= inst->dword.int_bfe.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.int_bfe.src2_mod == 2)
			;
		else if (inst->dword.int_bfe.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_INT_BFE.src2_mod not recognized", inst->dword.int_bfe.src2_mod);
	}
	else if (fmt == FRM_FMT_INT_BFI)
	{
		if (inst->dword.int_bfi.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_bfi.src2 & 0x3f);
		else if (inst->dword.int_bfi.src2_mod == 1)
		{
			bank_id = inst->dword.int_bfi.src2 >> 16;
			offset_in_bank= inst->dword.int_bfi.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.int_bfi.src2_mod == 2)
			;
		else if (inst->dword.int_bfi.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_INT_BFI.src2_mod not recognized", inst->dword.int_bfi.src2_mod);
	}
	else if (fmt == FRM_FMT_INT_SHR)
	{
		if (inst->dword.int_shr.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_shr.src2 & 0x3f);
		else if (inst->dword.int_shr.src2_mod == 1)
		{
			bank_id = inst->dword.int_shr.src2 >> 16;
			offset_in_bank= inst->dword.int_shr.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.int_shr.src2_mod == 2)
			;
		else if (inst->dword.int_shr.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_INT_SHR.src2_mod not recognized", inst->dword.int_shr.src2_mod);
	}
	else if (fmt == FRM_FMT_INT_SHL)
	{
		if (inst->dword.int_shl.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_shl.src2 & 0x3f);
		else if (inst->dword.int_shl.src2_mod == 1)
		{
			bank_id = inst->dword.int_shl.src2 >> 16;
			offset_in_bank= inst->dword.int_shl.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.int_shl.src2_mod == 2)
			;
		else if (inst->dword.int_shl.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_INT_SHL.src2_mod not recognized", inst->dword.int_shl.src2_mod);
	}
	else if (fmt == FRM_FMT_INT_LOP)
	{
		if (inst->dword.int_lop.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_lop.src2 & 0x3f);
		else if (inst->dword.int_lop.src2_mod == 1)
		{
			bank_id = inst->dword.int_lop.src2 >> 16;
			offset_in_bank= inst->dword.int_lop.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.int_lop.src2_mod == 2)
			;
		else if (inst->dword.int_lop.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_INT_LOP.src2_mod not recognized", inst->dword.int_lop.src2_mod);
	}
	else if (fmt == FRM_FMT_INT_ISETP)
	{
		if (inst->dword.int_isetp.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_isetp.src2 & 0x3f);
		else if (inst->dword.int_isetp.src2_mod == 1)
		{
			bank_id = inst->dword.int_isetp.src2 >> 16;
			offset_in_bank= inst->dword.int_isetp.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.int_isetp.src2_mod == 2)
			;
		else if (inst->dword.int_isetp.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_INT_ISETP.src2_mod not recognized", inst->dword.int_isetp.src2_mod);
	}
	else if (fmt == FRM_FMT_INT_ICMP)
	{
		if (inst->dword.int_icmp.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_icmp.src2 & 0x3f);
		else if (inst->dword.int_icmp.src2_mod == 1)
		{
			bank_id = inst->dword.int_icmp.src2 >> 16;
			offset_in_bank= inst->dword.int_icmp.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.int_icmp.src2_mod == 2)
			;
		else if (inst->dword.int_icmp.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_INT_ICMP.src2_mod not recognized", inst->dword.int_icmp.src2_mod);
	}
	else if (fmt == FRM_FMT_CONV_F2F)
	{
		if (inst->dword.conv_f2f.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.conv_f2f.src2 & 0x3f);
		else if (inst->dword.conv_f2f.src2_mod == 1)
		{
			bank_id = inst->dword.conv_f2f.src2 >> 16;
			offset_in_bank= inst->dword.conv_f2f.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.conv_f2f.src2_mod == 2)
			;
		else if (inst->dword.conv_f2f.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_CONV_F2F.src2_mod not recognized", inst->dword.conv_f2f.src2_mod);
	}
	else if (fmt == FRM_FMT_CONV_F2I)
	{
		if (inst->dword.conv_f2i.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.conv_f2i.src2 & 0x3f);
		else if (inst->dword.conv_f2i.src2_mod == 1)
		{
			bank_id = inst->dword.conv_f2i.src2 >> 16;
			offset_in_bank= inst->dword.conv_f2i.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.conv_f2i.src2_mod == 2)
			;
		else if (inst->dword.conv_f2i.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_CONV_F2I.src2_mod not recognized", inst->dword.conv_f2i.src2_mod);
	}
	else if (fmt == FRM_FMT_CONV_I2F)
	{
		if (inst->dword.conv_i2f.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.conv_i2f.src2 & 0x3f);
		else if (inst->dword.conv_i2f.src2_mod == 1)
		{
			bank_id = inst->dword.conv_i2f.src2 >> 16;
			offset_in_bank= inst->dword.conv_i2f.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.conv_i2f.src2_mod == 2)
			;
		else if (inst->dword.conv_i2f.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_CONV_I2F.src2_mod not recognized", inst->dword.conv_i2f.src2_mod);
	}
	else if (fmt == FRM_FMT_CONV_I2I)
	{
		if (inst->dword.conv_i2i.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.conv_i2i.src2 & 0x3f);
		else if (inst->dword.conv_i2i.src2_mod == 1)
		{
			bank_id = inst->dword.conv_i2i.src2 >> 16;
			offset_in_bank= inst->dword.conv_i2i.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.conv_i2i.src2_mod == 2)
			;
		else if (inst->dword.conv_i2i.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_CONV_I2I.src2_mod not recognized", inst->dword.conv_i2i.src2_mod);
	}
	else if (fmt == FRM_FMT_MOV_MOV)
	{
		if (inst->dword.mov_mov.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.mov_mov.src2 & 0x3f);
		else if (inst->dword.mov_mov.src2_mod == 1)
		{
			bank_id = inst->dword.mov_mov.src2 >> 16;
			offset_in_bank= inst->dword.mov_mov.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.mov_mov.src2_mod == 2)
			;
		else if (inst->dword.mov_mov.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_MOV_MOV.src2_mod not recognized", inst->dword.mov_mov.src2_mod);
	}
	else if (fmt == FRM_FMT_MOV_SEL)
	{
		if (inst->dword.mov_sel.src2_mod == 0)
			str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.mov_sel.src2 & 0x3f);
		else if (inst->dword.mov_sel.src2_mod == 1)
		{
			bank_id = inst->dword.mov_sel.src2 >> 16;
			offset_in_bank= inst->dword.mov_sel.src2 & 0xffff;
			if (bank_id == 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [0x0]");
			else if (bank_id == 0 && offset_in_bank != 0)
				str_printf(inst_str_ptr, inst_str_size, "c [0x0] [%#llx]", offset_in_bank);
			else if (bank_id != 0 && offset_in_bank == 0)
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [0x0]", bank_id);
			else
				str_printf(inst_str_ptr, inst_str_size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
		}
		else if (inst->dword.mov_sel.src2_mod == 2)
			;
		else if (inst->dword.mov_sel.src2_mod == 3)
			;
		else
			fatal("%d: FRM_FMT_MOV_SEL.src2_mod not recognized", inst->dword.mov_sel.src2_mod);
	}
	else if (fmt == FRM_FMT_LDST_LD)
		;
	else if (fmt == FRM_FMT_LDST_ST)
		;
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

void frm_inst_dump_src3(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	if (fmt == FRM_FMT_FP_FFMA)
		str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_ffma.src3);
	else if (fmt == FRM_FMT_FP_DFMA)
		str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.fp_dfma.src3);
	else if (fmt == FRM_FMT_INT_IMAD)
		str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_imad.src3);
	else if (fmt == FRM_FMT_INT_BFI)
		str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_bfi.src3);
	else if (fmt == FRM_FMT_INT_ICMP)
		str_printf(inst_str_ptr, inst_str_size, "R%d", inst->dword.int_icmp.src3);
	else
		fatal("%d: src3 not recognized", fmt);
}

void frm_inst_dump_imm(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int imm;

	if (fmt == FRM_FMT_FP_FADD32I)
		imm = inst->dword.fp_fadd32i.imm32;
	else if (fmt == FRM_FMT_FP_FMUL32I)
		imm = inst->dword.fp_fmul32i.imm32;
	else if (fmt == FRM_FMT_INT_LOP32I)
		imm = inst->dword.int_lop32i.imm32;
	else if (fmt == FRM_FMT_MOV_MOV32I)
		imm = inst->dword.mov_mov32i.imm32;
	else if (fmt == FRM_FMT_CTRL_OP1)
		imm = inst->addr + 8 + inst->dword.ctrl_op1.imm;
	else
		fatal("%d: %%i not recognized", fmt);

	str_printf(inst_str_ptr, inst_str_size, "%#llx", imm);
}

void frm_inst_dump_R(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int R;

	if (fmt == FRM_FMT_FP_FSETP)
		R = inst->dword.fp_fsetp.R;
	else if (fmt == FRM_FMT_FP_DSETP)
		R = inst->dword.fp_dsetp.R;
	else if (fmt == FRM_FMT_INT_ISETP)
		R = inst->dword.int_isetp.R;
	else if (fmt == FRM_FMT_MOV_SEL)
		R = inst->dword.mov_sel.R;
	else
		fatal("%d: %%R not recognized", fmt);

	if (R != 7)
		str_printf(inst_str_ptr, inst_str_size, "p%lld", R);
	else
		str_printf(inst_str_ptr, inst_str_size, "pt");
}

void frm_inst_dump_offset(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int offset;

	if (fmt == FRM_FMT_LDST_LD)
		offset = inst->dword.ldst_ld.offset;
	else if (fmt == FRM_FMT_LDST_ST)
		offset = inst->dword.ldst_st.offset;
	else
		fatal("%d: %%o not recognized", fmt);

	if (offset != 0)
		str_printf(inst_str_ptr, inst_str_size, "+%#llx", offset);
	else
		;
}

void frm_inst_dump_shamt(char **inst_str_ptr, int *inst_str_size, struct frm_inst_t *inst, enum frm_fmt_enum fmt)
{
	unsigned long long int shamt;

	if (fmt == FRM_FMT_INT_ISCADD)
		shamt = inst->dword.int_iscadd.shamt;
	else
		fatal("%d: %%s not recognized", fmt);

	str_printf(inst_str_ptr, inst_str_size, "%#llx", shamt);
}


