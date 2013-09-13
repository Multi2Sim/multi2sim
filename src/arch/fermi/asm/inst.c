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

#include <lib/class/string.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"
#include "inst.h"


/*
 * Class 'FrmInst'
 */

void FrmInstCreate(FrmInst *self, FrmAsm *as)
{
	/* Initialize */
	self->as = as;
}


void FrmInstDestroy(FrmInst *self)
{
}


void FrmInstDecode(FrmInst *self, unsigned int addr, void *ptr)
{
	FrmAsm *as = self->as;
	unsigned int op;

	/* 10-bit opcode by default */
	self->bytes.dword = * (unsigned long long *) ptr;
	op = ((self->bytes.bytes[7] & 0xfc) << 2) |
			((self->bytes.bytes[0]) & 0xf);
	/* 9-bit opcode */
	if (op == 0x0e0 || op == 0x0f0 || /* FCMP */
			op == 0x0a0 || op == 0x0b0 ||
			op == 0x080 || op == 0x090 || /* FSETP */
			op == 0x061 || op == 0x071 || /* DSETP */
			op == 0x022 || op == 0x032 || /* IADD32I */
			op == 0x063 || op == 0x073 || /* ISETP */
			op == 0x043 || op == 0x053 || /* ISET */
			op == 0x205 || op == 0x215 || /* LD */
			op == 0x245 || op == 0x255)   /* ST */
		op = ((self->bytes.bytes[7] & 0xf8) << 1) |
			((self->bytes.bytes[0]) & 0xf);

	self->info = as->dec_table[op];
	self->addr = addr;
}


static struct str_map_t frm_inst_ftzfmz_map =
{
	3,
	{
		{"", 0},
		{ ".FTZ", 1 },
		{ ".FMZ", 2 }
	}
};

static struct str_map_t frm_inst_rnd_map =
{
	4,
	{
		{ "", 0},
		{ ".RM", 1 },
		{ ".RP", 2 },
		{ ".RZ", 3}
	}
};

static struct str_map_t frm_inst_rnd1_map =
{
	4,
	{
		{ "", 0},
		{ ".FLOOR", 1 },
		{ ".CEIL", 2 },
		{ ".TRUNC", 3 },
	}
};

static struct str_map_t frm_inst_cmp_map =
{
	14,
	{
		{ ".LT", 1},
		{ ".EQ", 2},
		{ ".LE", 3},
		{ ".GT", 4},
		{ ".NE", 5},
		{ ".GE", 6},
		{ ".NUM", 7},
		{ ".NAN", 8},
		{ ".LTU", 9},
		{ ".EQU", 10},
		{ ".LEU", 11},
		{ ".GTU", 12},
		{ ".NEU", 13},
		{ ".GEU", 14}
	}
};

static struct str_map_t frm_inst_round_map =
{
	2,
	{
		{ "", 0},
		{ ".ROUND", 1 }
	}
};

static struct str_map_t frm_inst_sat_map =
{
	2,
	{
		{ "", 0},
		{ ".SAT", 1 }
	}
};

static struct str_map_t frm_inst_logic_map =
{
	3,
	{
		{ ".AND", 0},
		{ ".OR", 1},
		{ ".XOR", 2}
	}
};


static struct str_map_t frm_inst_op_map =
{
	8,
	{
		{ ".COS", 0},
		{ ".SIN", 1},
		{ ".EX2", 2},
		{ ".LG2", 3},
		{ ".RCP", 4},
		{ ".RSQ", 5},
		{ ".RCP64H", 6},
		{ ".RSQ64H", 7}
	}
};

static struct str_map_t frm_inst_op56_map =
{
	3,
	{
		{ ".POPC", 0},
		{ ".AND", 1},
		{ ".OR", 2},
	}
};

static struct str_map_t frm_inst_op67_map =
{
	4,
	{
		{ ".AND", 0},
		{ ".OR", 1},
		{ ".XOR", 2},
		{ ".PASS_B", 3}
	}
};

static struct str_map_t frm_inst_dtype_n_map =
{
	4,
	{
		{ "", 0},
		{ ".F16", 1},
		{ ".F32", 2},
		{ ".F64", 3}
	}
};

static struct str_map_t frm_inst_dtype_map =
{
	4,
	{
		{ "", 0},
		{ ".F16", 1},
		{ ".F32", 2},
		{ ".F64", 3}
	}
};

static struct str_map_t frm_inst_stype_n_map =
{
	4,
	{
		{ "", 0},
		{ "16", 1},
		{ "32", 2},
		{ "64", 3}
	}
};

static struct str_map_t frm_inst_stype_map =
{
	4,
	{
		{ "", 0},
		{ ".F16", 1},
		{ ".F32", 2},
		{ ".F64", 3}
	}
};

static struct str_map_t frm_inst_type_map =
{
	7,
	{
		{ ".U8", 0},
		{ ".S8", 1},
		{ ".U16", 2},
		{ ".S16", 3},
		{ "", 4},
		{ ".64", 5},
		{ ".128", 6}
	}
};

static struct str_map_t frm_inst_cop_map =
{
	4,
	{
		{ "", 0},
		{ ".CG", 1},
		{ ".CS", 2},
		{ ".CV", 3}
	}
};

static struct str_map_t frm_inst_NOP_op_map =
{
	9,
	{
		{ "", 0 },
		{ ".FMA64", 1 },
		{ ".FMA32", 2 },
		{ ".XLU", 3 },
		{ ".ALU", 4 },
		{ ".AGU", 5 },
		{ ".SU", 6 },
		{ ".FU", 7 },
		{ ".FMUL", 8 },
	}
};

static struct str_map_t frm_inst_ccop_map =  // more needs to be added (look over fermi isa NOP)
{
	4,
	{
		{ ".F", 0 },
		{ ".LT", 1 },
		{ ".EQ", 2 },
		{ ".T", 15 }
	}
};


StringMap frm_inst_sreg_map =
{
	{ "SR_Laneld", FrmInstSRegLaneld },
	{ "SR_VirtCfg", FrmInstSRegVirtCfg },
	{ "SR_VirtId", FrmInstSRegVirtId },
	{ "SR_PM0", FrmInstSRegPM0 },
	{ "SR_PM1", FrmInstSRegPM1 },
	{ "SR_PM2", FrmInstSRegPM2 },
	{ "SR_PM3", FrmInstSRegPM3 },
	{ "SR_PM4", FrmInstSRegPM4 },
	{ "SR_PM5", FrmInstSRegPM5 },
	{ "SR_PM6", FrmInstSRegPM6 },
	{ "SR_PM7", FrmInstSRegPM7 },
	{ "SR_PRIM_TYPE", FrmInstSRegPrimType },
	{ "SR_INVOCATION_ID", FrmInstSRegInvocationID },
	{ "SR_Y_DIRECTION", FrmInstSRegYDirection },
	{ "SR_MACHINE_ID_0", FrmInstSRegMachineID0 },
	{ "SR_MACHINE_ID_1", FrmInstSRegMachineID1 },
	{ "SR_MACHINE_ID_2", FrmInstSRegMachineID2 },
	{ "SR_MACHINE_ID_3", FrmInstSRegMachineID3 },
	{ "SR_AFFINITY", FrmInstSRegAffinity },
	{ "SR_Tid", FrmInstSRegTid },
	{ "SR_Tid_X", FrmInstSRegTidX },
	{ "SR_Tid_Y", FrmInstSRegTidY },
	{ "SR_Tid_Z", FrmInstSRegTidZ },
	{ "SR_CTAParam", FrmInstSRegCTAParam },
	{ "SR_CTAid_X", FrmInstSRegCTAidX },
	{ "SR_CTAid_Y", FrmInstSRegCTAidY },
	{ "SR_CTAid_Z", FrmInstSRegCTAidZ },
	{ "SR_NTid", FrmInstSRegNTid },
	{ "SR_NTid_X", FrmInstSRegNTidX },
	{ "SR_NTid_Y", FrmInstSRegNTidY },
	{ "SR_NTid_Z", FrmInstSRegNTidZ },
	{ "SR_GridParam", FrmInstSRegGridParam },
	{ "SR_NCTAid_X", FrmInstSRegNCTAidX },
	{ "SR_NCTAid_Y", FrmInstSRegNCTAidY },
	{ "SR_NCTAid_Z", FrmInstSRegNCTAidZ },
	{ "SR_SWinLo", FrmInstSRegSWinLo },
	{ "SR_SWINSZ", FrmInstSRegSWINSZ },
	{ "SR_SMemSz", FrmInstSRegSMemSz },
	{ "SR_SMemBanks", FrmInstSRegSMemBanks },
	{ "SR_LWinLo", FrmInstSRegLWinLo },
	{ "SR_LWINSZ", FrmInstSRegLWINSZ },
	{ "SR_LMemLoSz", FrmInstSRegLMemLoSz },
	{ "SR_LMemHiOff", FrmInstSRegLMemHiOff },
	{ "SR_EqMask", FrmInstSRegEqMask },
	{ "SR_LtMask", FrmInstSRegLtMask },
	{ "SR_LeMask", FrmInstSRegLeMask },
	{ "SR_GtMask", FrmInstSRegGtMask },
	{ "SR_GeMask", FrmInstSRegGeMask },
	{ "SR_ClockLo", FrmInstSRegClockLo },
	{ "SR_ClockHi", FrmInstSRegClockHi },
	{ NULL, 0 }
};


void FrmInstDumpBuf(FrmInst *self, char *str, int size)
{
	char *fmt_str;
	char *orig_str;
	int len;

	/* Check if instruction is supported */
	if (!self->info)
		fatal("%s: instruction not supported (offset=0x%x)",
			__FUNCTION__, self->addr);

	/* Store copy of format string and original destination buffer */
	fmt_str = self->info->fmt_str;
	orig_str = str;

	/* Process format string */
	while (*fmt_str && size)
	{
		/* Character is a literal symbol */
		if (*fmt_str != '%' && *fmt_str != '+')
		{
			if (*fmt_str != ' ' || str != orig_str)
				str_printf(&str, &size, "%c", *fmt_str);
			++fmt_str;
			continue;
		}

		if (*fmt_str == '+')
			++fmt_str;

		/* Character is a token */
		++fmt_str;

		/* 1st level token such as pred, dst, src1, src2, src2_mod, imm, offs*/
		if (asm_is_token(fmt_str, "pred_no@P0", &len))
		{
			unsigned long long int pred;
			pred = self->bytes.general0.pred;
			if (pred < 7 && pred != 0)
				str_printf(&str, &size, "@P%lld", pred);
			else if (pred > 7)
				str_printf(&str, &size, "@!P%lld", pred - 8);
		}

		else if (asm_is_token(fmt_str, "pred", &len))
		{
			unsigned long long int pred;
			pred = self->bytes.general0.pred;
			if (pred < 7)
				str_printf(&str, &size, "@P%lld", pred);
			else if (pred > 7)
				str_printf(&str, &size, "@!P%lld", pred - 8);
		}

		else if (asm_is_token(fmt_str,"dst", &len))
		{
			unsigned long long int dst;
			dst = self->bytes.general0.dst;
			if (dst != 63)
				str_printf(&str, &size, "R%lld", dst);
			else
				str_printf(&str, &size, "RZ");
		}

		else if (asm_is_token(fmt_str,"src1_abs_neg", &len))
		{
			unsigned long long int src1;
			unsigned long long int src1_abs;

			src1 = self->bytes.general0.src1;
			src1_abs = self->bytes.mod0_A.abs_src1;
			if (src1 != 63 && src1_abs == 1)
				str_printf(&str, &size, "|R%lld|", src1);
			else if (src1 != 63 && src1_abs == 0)
				str_printf(&str, &size, "R%lld", src1);
			else
				str_printf(&str, &size, "RZ");
		}

		else if (asm_is_token(fmt_str, "src1_neg", &len))
		{
			unsigned long long int src1;
			unsigned long long int src_mod;

			src1 = self->bytes.general0.src1;
			src_mod = self->bytes.mod0_D.fma_mod;
			if (src1 != 63)
			{
				if (src_mod == 0x2)
					str_printf(&str, &size, "-R%lld", src1);
				else
					str_printf(&str, &size, "R%lld", src1);
			}
			else
				str_printf(&str, &size, "RZ");
		}

		else if (asm_is_token(fmt_str, "src1_offs", &len))
		{
			unsigned long long int src1;
			unsigned long long int offs;

			src1 = self->bytes.general0.src1;
			offs = self->bytes.offs.offset;

			if (src1 != 63)
			{
				str_printf(&str, &size, "[R%lld", src1);
			//else
			//	str_printf(&str, &size, "[RZ");

				if (offs != 0)
					str_printf(&str, &size, "+%#llx", offs);
			}

			else
				str_printf(&str, &size, "[0x%#llx", offs);

			str_printf(&str, &size, "]");
		}

		else if (asm_is_token(fmt_str,"src1", &len))
		{
			unsigned long long int src1;

			src1 = self->bytes.general0.src1;
			if (src1 != 63)
				str_printf(&str, &size, "R%lld", src1);
			else
				str_printf(&str, &size, "RZ");
		}

		else if (asm_is_token(fmt_str,"src23", &len))
		{
			unsigned long long int bank_id;
			unsigned long long int offset_in_bank;
			unsigned long long int src3;
			unsigned long long int src2_neg;
			unsigned long long int src3_neg;

			src3 = self->bytes.general0_mod1_B.src3;
			src2_neg = self->bytes.mod0_A.neg_src1;
			src3_neg = self->bytes.mod0_A.neg_src2;

			if (self->bytes.general0.src2_mod == 0)
			{
				if (self->bytes.general0.src2 != 0x3f)
				{
					if (src2_neg)
						str_printf(&str, &size, "-R%d, ", self->bytes.general0.src2 & 0x3f);
					else
						str_printf(&str, &size, "R%d, ", self->bytes.general0.src2 & 0x3f);
				}
				else
					str_printf(&str, &size, "RZ, ");
				if (src3 != 63)
				{
					if (src3_neg)
						str_printf(&str, &size, "-R%lld", src3);
					else
						str_printf(&str, &size, "R%lld", src3);
				}
				else
					str_printf(&str, &size, "RZ");
			}
			else if (self->bytes.general0.src2_mod == 1)
			{
				bank_id = ((self->bytes.general0.src2 & 0x1) << 4) | 
					(self->bytes.general0.src2 >> 16);
				offset_in_bank= self->bytes.general0.src2 &
					0xfffc;
				str_printf(&str, &size, "c [0x%llx] [0x%llx], ", bank_id, offset_in_bank);
				if (src3 != 63)
				{
					if (src3_neg)
						str_printf(&str, &size, "-R%lld", src3);
					else
						str_printf(&str, &size, "R%lld", src3);
				}
				else
					str_printf(&str, &size, "RZ");
			}
			else if (self->bytes.general0.src2_mod == 2)
			{
				if (src3 != 63)
				{
					if (src3_neg)
						str_printf(&str, &size, "-R%lld, ", src3);
					else
						str_printf(&str, &size, "R%lld, ", src3);
				}
				else
					str_printf(&str, &size, "RZ, ");
				bank_id = ((self->bytes.general0.src2 & 0x1) << 4) | 
					(self->bytes.general0.src2 >> 16);
				offset_in_bank= self->bytes.general0.src2 &
					0xfffc;
				str_printf(&str, &size, "c [0x%llx] [0x%llx]", bank_id, offset_in_bank);
			}
			else if (self->bytes.general0.src2_mod == 3)
			{
				str_printf(&str, &size, "0x%x, ", self->bytes.general0.src2);
				if (src3 != 63)
				{
					if (src3_neg)
						str_printf(&str, &size, "-R%lld", src3);
					else
						str_printf(&str, &size, "R%lld", src3);
				}
				else
					str_printf(&str, &size, "RZ");
			}
		}

		/* This is a special case for src2 and src3. For FFMA,
		   the sequence of output from cuobjdump is somehow depends on the src2_mod
		   it prints src3 first when src2_mod > 2, however prints src2 first when src2_mod < 2 */
		else if (asm_is_token(fmt_str,"src2_FFMA", &len))
		{
			unsigned long long int bank_id;
			unsigned long long int offset_in_bank;
			unsigned long long int src3;

			bank_id = self->bytes.general0.src2 >> 16;
			offset_in_bank= self->bytes.general0.src2 & 0xffff;
			src3 = self->bytes.general0_mod1_B.src3;

			/* print out src2 */
			if (self->bytes.general0.src2_mod < 2)
			{
				if (self->bytes.general0.src2_mod == 0)
					str_printf(&str, &size, "R%d", self->bytes.general0.src2 & 0x3f);
				else if (self->bytes.general0.src2_mod == 1)
				{
					if (bank_id == 0 && offset_in_bank == 0)
						str_printf(&str, &size, "c [0x0] [0x0]");
					else if (bank_id == 0 && offset_in_bank != 0)
						str_printf(&str, &size, "c [0x0] [%#llx]", offset_in_bank);
					else if (bank_id != 0 && offset_in_bank == 0)
						str_printf(&str, &size, "c [%#llx] [0x0]", bank_id);
					else
						str_printf(&str, &size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
				}
			}

			/* print out src3 */
			else
			{
				if (src3 != 63)
					str_printf(&str, &size, ", R%lld", src3);
				else
					str_printf(&str, &size, ", RZ");
			}
		}

		/* This is a special case for src2. 
		 * For LDC, src2 is always a constant memory address, no matter
		 * what the value of src2_mod is. */
		else if (asm_is_token(fmt_str,"src2_LDC", &len))
		{
			unsigned long long int bank_id;
			unsigned long long int src1;
			unsigned long long int offset_in_bank;

			bank_id = self->bytes.general0.src2 >> 16;
			src1= self->bytes.general0.src1;
			offset_in_bank= self->bytes.general0.src2 & 0xffff;

			/* print out src2 */
			if (offset_in_bank == 0)
				str_printf(&str, &size, "c [0x%llx] [R%llu]",
						bank_id, src1);
			else
				str_printf(&str, &size, "c [0x%llx] [R%llu+0x%llx]",
						bank_id, src1, offset_in_bank);
		}

		else if (asm_is_token(fmt_str,"src2_frm_sr", &len))
		{
			char *sreg;
			sreg = StringMapValue(frm_inst_sreg_map, self->bytes.general0.src2 & 0xff);
			str_printf(&str, &size, "%s", sreg);

		}

		else if (asm_is_token(fmt_str,"src2_abs_neg", &len))
		{
			unsigned long long int src2;
			unsigned long long int src2_abs;

			src2 = self->bytes.general0.src2;
			src2_abs = self->bytes.mod0_A.abs_src2;
			if (src2 != 63 && src2_abs == 1)
				str_printf(&str, &size, "|R%lld|", src2);
			else if (src2 != 63 && src2_abs == 0)
				str_printf(&str, &size, "R%lld", src2);
			else
				str_printf(&str, &size, "RZ");
		}

		else if (asm_is_token(fmt_str,"src2_neg", &len))
		{
			unsigned long long int bank_id;
			unsigned long long int offset_in_bank;

			if (self->bytes.general0.src2_mod == 0)
			{
				if (self->bytes.mod0_A.neg_src2)
					str_printf(&str, &size, "-");
				str_printf(&str, &size, "R%d", self->bytes.general0.src2 & 0x3f);
			}
			else if (self->bytes.general0.src2_mod == 1)
			{
				bank_id = self->bytes.general0.src2 >> 16;
				offset_in_bank= self->bytes.general0.src2 & 0xffff;
				if (bank_id == 0 && offset_in_bank == 0)
					str_printf(&str, &size, "c [0x0] [0x0]");
				else if (bank_id == 0 && offset_in_bank != 0)
					str_printf(&str, &size, "c [0x0] [%#llx]", offset_in_bank);
				else if (bank_id != 0 && offset_in_bank == 0)
					str_printf(&str, &size, "c [%#llx] [0x0]", bank_id);
				else
					str_printf(&str, &size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
			}
			else if (self->bytes.general0.src2_mod == 2)
				str_printf(&str, &size, "0x%x", self->bytes.general0.src2);
			else if (self->bytes.general0.src2_mod == 3)
				str_printf(&str, &size, "0x%x", self->bytes.general0.src2);
		}

		else if (asm_is_token(fmt_str,"src2", &len))
		{
			unsigned long long int bank_id;
			unsigned long long int offset_in_bank;

			if (self->bytes.general0.src2_mod == 0)
			{
				if (self->bytes.general0.src2 != 0x3f)
					str_printf(&str, &size, "R%d", self->bytes.general0.src2 & 0x3f);
				else
					str_printf(&str, &size, "RZ");
			}
			else if (self->bytes.general0.src2_mod == 1 || 
					self->bytes.general0.src2_mod == 2)
			{
				bank_id = ((self->bytes.general0.src2 & 0x1) << 4) | 
					(self->bytes.general0.src2 >> 16);
				offset_in_bank= self->bytes.general0.src2 &
					0xfffc;
				str_printf(&str, &size, "c [0x%llx] [0x%llx]", bank_id, offset_in_bank);
			}
			else if (self->bytes.general0.src2_mod == 3)
				str_printf(&str, &size, "0x%x", self->bytes.general0.src2);
		}

		else if (asm_is_token(fmt_str, "src3_FFMA", &len))
		{
			unsigned long long int src3;
			unsigned long long int bit26;
			unsigned long long int bit28;

			src3 = self->bytes.general0_mod1_B.src3;
			bit26 = self->bytes.general0.src2 & 0x1;
			bit28 = self->bytes.general0.src2 >> 2 &0x1;

			/* print out src2 */
			if (self->bytes.general0.src2_mod < 2)
			{
				if (src3 != 63)
					str_printf(&str, &size, "R%lld", src3);
				else
					str_printf(&str, &size, "RZ");
			}

			else
			{
				/* FIXME : we need to figure out how bit26 and bit28 control src2*/
				if (bit26 == 0 && bit28 == 0)
					str_printf(&str, &size, "c [0x0] [0x0]");
				else if (bit26 == 1 && bit28 == 0)
					str_printf(&str, &size, "c [0x10] [0x0]");
				else if (bit26 == 0 && bit28 == 1)
					str_printf(&str, &size, "c [0x0] [0x4]");
				else
					str_printf(&str, &size, "c [0x10] [0x4]");
			}
		}

		else if (asm_is_token(fmt_str,"src3", &len))
		{
			unsigned long long int src3;
			src3 = self->bytes.general0_mod1_B.src3;
			if (src3 != 63)
				str_printf(&str, &size, "R%lld", src3);
			else
				str_printf(&str, &size, "RZ");
		}

		else if (asm_is_token(fmt_str,"tgt_lmt", &len))
		{
			if (self->bytes.tgt.noinc)
				str_printf(&str,&size, ".LMT");
		}

		else if (asm_is_token(fmt_str,"tgt_noinc", &len))
		{
			if (!self->bytes.tgt.noinc)
				str_printf(&str,&size, ".noinc");
		}

		else if (asm_is_token(fmt_str,"tgt_u", &len))
		{
			if (self->bytes.tgt.u)
				str_printf(&str,&size, ".U");
		}

		else if (asm_is_token(fmt_str,"tgt", &len))
		{
			unsigned long long int target;

			target = self->bytes.tgt.target;
			target = SEXT64(target, 24);
			target += self->addr + 8;
			str_printf (&str,&size, "%#llx", target);
		}

		else if (asm_is_token(fmt_str,"imm32", &len))
		{
			long long int imm32;
			/* FIXME
			 * Careful - We need to check whether each instruction encodes the immediate
			 * value as a signed or unsigned number. */
				imm32 = (signed)self->bytes.imm.imm32;
			/* Print sign */
			if (imm32 < 0)
			{
				str_printf(&str, &size, "-");
				imm32 = -imm32;
			}
			str_printf(&str, &size, "%#llx", imm32);
		}

		else if (asm_is_token(fmt_str,"gen1_cmp", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_cmp_map,
						self->bytes.general1.cmp));
		}

		else if (asm_is_token(fmt_str,"gen1_logicftz", &len))
		{
			str_printf(&str, &size, ".FTZ%s", str_map_value(&frm_inst_logic_map,
						self->bytes.general1.logic));
		}

		else if (asm_is_token(fmt_str,"gen1_logic", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_logic_map,
						self->bytes.general1.logic));
		}

		/* 2nd level token such as mod0, mod1, P, Q*/
		else if (asm_is_token(fmt_str, "mod0_A_ftz", &len))
		{
			if (self->bytes.mod0_A.satftz)
				str_printf(&str, &size, ".FTZ");
		}

		else if (asm_is_token(fmt_str, "mod0_A_op", &len))
		{
			unsigned long long int op;
			op = self->bytes.mod0_A.abs_src2 << 1 || self->bytes.mod0_A.satftz;
			str_printf(&str, &size, "%s", str_map_value (&frm_inst_op56_map,op));
		}

		else if (asm_is_token(fmt_str, "mod0_A_redarv", &len))
		{
			if (self->bytes.mod0_A.abs_src1)
				str_printf(&str, &size, ".ARV");
			else
				str_printf(&str, &size, ".RED");
		}

		else if (asm_is_token(fmt_str, "mod0_A_s", &len))
		{
			if (self->bytes.mod0_A.s)
				str_printf(&str, &size, ".S");
		}

		else if (asm_is_token(fmt_str, "mod0_A_u32", &len))
		{
			if (!self->bytes.mod0_A.satftz)
				str_printf(&str, &size, ".U32");
		}

		else if (asm_is_token(fmt_str, "mod0_A_w", &len))
		{
			if (self->bytes.mod0_A.neg_src1)
				str_printf(&str, &size, ".W");
		}

		else if (asm_is_token(fmt_str, "mod0_B_brev", &len))
		{
			if (self->bytes.mod0_B.cop)
				str_printf(&str, &size, ".brev");
		}

		else if (asm_is_token(fmt_str, "mod0_B_cop", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_cop_map, self->bytes.mod0_B.cop));
		}

		else if (asm_is_token(fmt_str, "mod0_B_type", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_type_map, self->bytes.mod0_B.type));
		}

		else if (asm_is_token(fmt_str, "mod0_B_u32", &len))
		{
			if (!self->bytes.mod0_B.type)
				str_printf(&str, &size, ".U32");
		}

		else if (asm_is_token(fmt_str, "mod0_C_ccop", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_ccop_map, self->bytes.mod0_C.shamt));
		}

		else if (asm_is_token(fmt_str, "mod0_C_shamt", &len))
		{
			unsigned long long int shamt;
			shamt = self->bytes.mod0_C.shamt;
			str_printf(&str, &size, "%#llx", shamt);
		}

		else if (asm_is_token(fmt_str, "mod0_C_s", &len))
		{
			if (self->bytes.mod0_C.s)
				str_printf(&str, &size, ".S");
		}

		else if (asm_is_token(fmt_str, "mod0_D_ftzfmz", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_ftzfmz_map, self->bytes.mod0_D.ftzfmz));
		}

		else if (asm_is_token(fmt_str, "mod0_D_op67", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_op67_map, self->bytes.mod0_D.ftzfmz));
		}

		else if (asm_is_token(fmt_str, "mod0_D_op", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_op_map, self->bytes.mod0_D.ftzfmz));
		}

		else if (asm_is_token(fmt_str, "mod0_D_round", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_round_map, self->bytes.mod0_D.ftzfmz >> 1));
		}

		else if (asm_is_token(fmt_str, "mod0_D_sat", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_sat_map, self->bytes.mod0_D.sat));
		}

		else if (asm_is_token(fmt_str, "mod0_D_x", &len))
		{
			unsigned long long int x;
			x = self->bytes.mod0_D.ftzfmz & 0x1;
			if (x)
				str_printf(&str, &size, ".X");
		}

		else if (asm_is_token(fmt_str, "gen0_mod1_B_rnd", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_rnd_map, self->bytes.general0_mod1_B.rnd));
		}

		else if (asm_is_token(fmt_str, "gen0_mod1_C_rnd", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_rnd1_map, self->bytes.general0_mod1_C.rnd));
		}

		else if (asm_is_token(fmt_str, "gen0_mod1_D_cmp", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_cmp_map, self->bytes.general0_mod1_D.cmp));
		}

		else if (asm_is_token(fmt_str, "gen0_src1_dtype_n", &len))
		{
			unsigned long long int dtype_n;
			dtype_n = self->bytes.general0.src1 & 0x3;
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_dtype_n_map, dtype_n));
		}

		else if (asm_is_token(fmt_str, "gen0_src1_dtype", &len))
		{
			unsigned long long int dtype;
			dtype = self->bytes.general0.src1 & 0x3;
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_dtype_map, dtype));
		}

		else if (asm_is_token(fmt_str, "gen0_src1_stype_n", &len))
		{
			unsigned long long int stype_n;
			stype_n = self->bytes.general0.src1 >> 3 & 0x3;
			str_printf(&str, &size, "%s", str_map_value (&frm_inst_stype_n_map, stype_n));
		}

		else if (asm_is_token(fmt_str, "gen0_src1_stype", &len))
		{
			unsigned long long int stype;
			stype = self->bytes.general0.src1 >> 3 & 0x3;
			str_printf(&str, &size, "%s", str_map_value (&frm_inst_stype_map, stype));
		}

		else if (asm_is_token(fmt_str, "dtype_sn", &len))
		{
			unsigned long long int dtype_n;
			if (self->bytes.mod0_A.abs_src1)
				str_printf(&str, &size, ".S");
			else
				str_printf(&str, &size, ".U");
			dtype_n = self->bytes.general0.src1 & 0x3;
			str_printf(&str, &size, "%s", str_map_value (&frm_inst_stype_n_map, dtype_n));
		}

		else if (asm_is_token(fmt_str, "stype_sn", &len))
		{
			unsigned long long int stype_n;
			if (self->bytes.mod0_A.neg_src1)
				str_printf(&str, &size, ".S");
		        else
			        str_printf(&str, &size, ".U");
			stype_n = self->bytes.general0.src1 >> 3 & 0x3;
			str_printf(&str, &size, "%s", str_map_value (&frm_inst_stype_n_map, stype_n));
		}

		else if (asm_is_token(fmt_str, "offs_op1_e", &len))
		{
			unsigned long long int e;
			e = self->bytes.offs.op1 & 0x1;
			if (e)
				str_printf(&str, &size, ".e");
		}

		else if (asm_is_token(fmt_str,"offs", &len))
		{
			unsigned long long int offs;
			offs = self->bytes.offs.offset;
			if (offs)
				str_printf(&str, &size, "+%#llx", offs);
		}

		else if (asm_is_token(fmt_str, "P", &len))
		{
			unsigned long long int P;
			P = self->bytes.general1.dst >> 3;
			str_printf(&str, &size, "P%lld", P);
		}

		else if (asm_is_token(fmt_str, "Q", &len))
		{
			unsigned long long int Q;
			Q = self->bytes.general1.dst & 0x7;
			if (Q != 7)
				str_printf(&str, &size, "p%lld", Q);
			else
				str_printf(&str, &size, "pt");
		}

		else if (asm_is_token(fmt_str, "R", &len))
		{
			unsigned long long int R;
			R = self->bytes.general1.R;
			if (R < 7)
				str_printf(&str, &size, "P%lld", R);
			else if (R == 8)
				str_printf(&str, &size, "!P0");
			else
				str_printf(&str, &size, "pt");
		}

		else if (asm_is_token(fmt_str, "FADD_sat", &len))
		{
			unsigned long long int sat;
			sat = self->bytes.general0_mod1_B.src3 & 0x1;
			if (sat)
				str_printf(&str, &size, ".SAT");
		}

		else if (asm_is_token(fmt_str, "MUFU_op", &len))
		{
			unsigned long long int op;
			op = self->bytes.imm.imm32;
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_op_map, op));
		}

		else if (asm_is_token(fmt_str, "NOP_op", &len))
		{
			unsigned long long int op;
			op = self->bytes.offs.mod1 >> 9 & 0x4;
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_NOP_op_map, op));
		}

		else if (asm_is_token(fmt_str, "IMAD_hi", &len))
		{
			unsigned long long int hi;
			hi= self->bytes.mod0_D.ftzfmz & 0x1;
			if (hi)
				str_printf(&str, &size, ".HI");
		}

		else if (asm_is_token(fmt_str, "IMAD_mod1", &len))
		{
			unsigned long long int mod1;
			mod1 = self->bytes.mod0_D.ftzfmz >> 1;
			if (!mod1)
				str_printf(&str, &size, ".U32");
		}

		else if (asm_is_token(fmt_str, "IMAD_mod2", &len))
		{
			unsigned long long int mod2;
			mod2 = self->bytes.mod0_D.sat;
			if (!mod2)
				str_printf(&str, &size, ".U32");
		}

		else if (asm_is_token(fmt_str, "IMAD_sat", &len))
		{
			unsigned long long int sat;
			sat = self->bytes.general0_mod1_B.rnd >> 1;
			if (sat)
				str_printf(&str, &size, ".SAT");
		}
		else
		{
			fatal("%s: unknown token: %s", __FUNCTION__,
					fmt_str);
		}

		/* Skip processed token */
		fmt_str += len;
	}
}


void FrmInstDump(FrmInst *self, FILE *f)
{
	char buf[200];

	FrmInstDumpBuf(self, buf, sizeof buf);
	fprintf(f, "%s", buf);
}


void FrmInstDumpHex(FrmInst *self, FILE *f)
{
	/* Print offset */
	fprintf(f, "\t/*%04x*/     ", self->addr);

	/* Dump instruction hexadecimal bytes */
	fprintf(f, "/*0x");
	fprintf(f, "%08x", self->bytes.word[0]);
	fprintf(f, "%08x", self->bytes.word[1]);
	fprintf(f, "*/ \t");
}
